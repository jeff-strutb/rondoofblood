/*
 * TEMPORARY: the stage 05 overlay's object routines whose enemy kind is not yet identified, kept
 * together here until each kind has a file of its own.
 * The disc tables the routines read are named here by their overlay address until the typed data
 * layer reads them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

/* The arms the converted routines end on (ObjArm, enemy_objects.h); each routine's exit shim builds the registers and flags from them. */
enum { CF_BECAME, CF_FELL };
enum { SC_HIGH, SC_FAR, SC_STARTED };
enum { AR_COUNTING, AR_DONE };
enum { SF_FALL, SF_FACED, SF_TURNED };

enum { S5_HD_OUT, S5_HD_IN };
enum { S5_PH_COUNTING, S5_PH_FRAME1, S5_PH_FRAME4 };
enum { S5_E7_ENTERED, S5_E7_ZERO, S5_E7_SAME };
enum { S5_SW1_LATER, S5_SW1_STARTED };
enum { S5_AS_RUNNING, S5_AS_ENDED };
enum { S5_AN_RUNNING, S5_AN_ENDED };
enum { S5_AM_PLAIN, S5_AM_SAGGED };
enum { S5_FP_NONE, S5_FP_FACED };
enum { S5_SX_RECORD, S5_SX_STILL, S5_SX_POS_HIGH, S5_SX_POS_LOW, S5_SX_POS_WRAP, S5_SX_NEG_HIGH, S5_SX_NEG_LOW, S5_SX_NEG_WRAP };
enum { S5_CM_STATE1, S5_CM_PICKED };
enum { S5_CT_PICKED, S5_CT_COUNTING };
enum { S5_CA_HIGH, S5_CA_PICKED, S5_CA_COUNTING };
enum { S5_WB_STEPPED, S5_WB_MOVED };
enum { S5_BE_RECORD, S5_BE_EXPIRED, S5_BE_SHOWN, S5_BE_BLINK };
enum { S5_LA_STRAIGHT, S5_LA_TURNED };
enum { S5_SS_EARLY, S5_SS_NO_SLOT, S5_SS_SHOT };
enum { S5_TA_RECORD, S5_TA_EXPIRED, S5_TA_FOLLOWED };
enum { S5_CD_COUNTING, S5_CD_SOUNDED };
enum { S5_CF_COUNTING, S5_CF_SOUNDED };
enum { S5_NG_RUNNING, S5_NG_NUMBERED };
enum { S5_S29_NO_SLOT, S5_S29_SPAWNED };
enum { S5_BP_NO_SLOT, S5_BP_FULL };

static uint8_t animate_move_at_random_sound_y_shim(ObjArm r);

/*
 * @routine 0938:7F2 SelectRasterStage1
 * @run 6F:BFF2
 * WHAT IT DOES: Sets the stage value the raster effect tests to 1.
 * HOW: One store into zRasterStage.
 * WAITS: no.
 * Entry: nothing. Exit: A 1, N and Z clear; X, Y, C and V kept.
 */
ObjArm SelectRasterStage1(void)
{
    ObjArm r = { 0 };

    WR8(ZP->raster_stage, 1);
    return r;
}

static Ended select_raster_stage1_exit_shim(ObjArm r)
{
    (void)r;
    return ended_load(1);
}

void abi_SelectRasterStage1(void)
{
    abi_ended(select_raster_stage1_exit_shim(SelectRasterStage1()));
}

/*
 * @routine 093C:10A SetHitboxDxFromFrame
 * @run 70:B90A
 * WHAT IT DOES: Sets the object's f3250 to 2 and its hit rectangle's X offset from a table by
 * its frame, for frames 1 to 6; any other frame clears both.
 * HOW: The table is the byte list at overlay address $B928, indexed by frame minus one. Frame 0
 * is not in range of the first test's subtract and reads the table at index $FF.
 * WAITS: no.
 * Entry: X object. Exit: A the table byte, or the frame; N, Z, C from the last compare or
 * subtract; Y the table index when read; X kept.
 */
ObjArm SetHitboxDxFromFrame(uint8_t slot)
{
    enum { TABLE = 0xB928 };
    ObjArm r = { 0 };

    WR8(OBJ->f3250[slot], 2);
    r.first = RD8(OBJ->frame[slot]);
    r.arm = S5_HD_OUT;
    if (r.first >= 7) {
        WR8(OBJ->f3250[slot], 0);
        WR8(OBJ->hitbox_dx[slot], 0);
        return r;
    }
    r.second = (uint8_t)(r.first - 1);
    r.third = rd8((uint16_t)(TABLE + r.second));
    WR8(OBJ->hitbox_dx[slot], r.third);
    r.arm = S5_HD_IN;
    return r;
}

static Ended set_hitbox_dx_from_frame_exit_shim(ObjArm r)
{
    if (r.arm == S5_HD_OUT) {
        return ended_compare(r.first, 7);
    }
    return ended_with_overflow(ended_with_y(ended_with_carry(ended_load(r.third), r.first >= 1), r.second),
                               sub_overflow(r.first, 1, r.second));
}

void abi_SetHitboxDxFromFrame(void)
{
    abi_ended(set_hitbox_dx_from_frame_exit_shim(SetHitboxDxFromFrame(cpu.x)));
}

/*
 * @routine 093C:374 PhaseTimerToState1
 * @run 70:BB74
 * WHAT IT DOES: Counts the object's f3720 down. While it runs, the object's animation is held
 * at its start and its step byte is 1; at zero the object enters state 1 with frame 1, or
 * frame 4 when its f36c8 is set.
 * HOW: One decrement; each branch then stores its constants.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z from the last value loaded; X, Y, C and V kept.
 */
ObjArm PhaseTimerToState1(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t left = RD8(OBJ->f3720[slot]);

    left = (uint8_t)(left - 1);
    WR8(OBJ->f3720[slot], left);
    if (left != 0) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->f2800[slot], 1);
        r.arm = S5_PH_COUNTING;
        return r;
    }
    WR8(OBJ->state[slot], 1);
    WR8(OBJ->frame[slot], 1);
    r.first = RD8(OBJ->f36c8[slot]);
    r.arm = S5_PH_FRAME1;
    if (r.first == 0) {
        return r;
    }
    WR8(OBJ->frame[slot], 4);
    r.arm = S5_PH_FRAME4;
    return r;
}

static Ended phase_timer_to_state1_exit_shim(ObjArm r)
{
    switch (r.arm) {
    case S5_PH_COUNTING:
        return ended_load(1);
    case S5_PH_FRAME1:
        return ended_load(0);
    default:
        return ended_load(4);
    }
}

void abi_PhaseTimerToState1(void)
{
    abi_ended(phase_timer_to_state1_exit_shim(PhaseTimerToState1(cpu.x)));
}

/*
 * @routine 093C:586 SetHitboxFromFrameRows
 * @run 70:BD86
 * WHAT IT DOES: Sets the object's whole hit rectangle (X offset, Y offset, half width, half
 * height) from a four byte row of a table, the row chosen by the frame minus $13.
 * HOW: The table is at overlay address $BDA8.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte loaded; N, Z from it; C bit 6 of the frame minus $13
 * (the last shift); Y the row's offset; X kept.
 */
ObjArm SetHitboxFromFrameRows(uint8_t slot)
{
    enum { TABLE = 0xBDA8 };
    ObjArm r = { 0 };
    uint8_t value;

    r.first = RD8(OBJ->frame[slot]);
    r.second = (uint8_t)((uint8_t)(r.first - 0x13) << 2);
    value = rd8((uint16_t)(TABLE + r.second));
    WR8(OBJ->hitbox_dx[slot], value);
    value = rd8((uint16_t)(TABLE + 1 + r.second));
    WR8(OBJ->hitbox_dy[slot], value);
    value = rd8((uint16_t)(TABLE + 2 + r.second));
    WR8(OBJ->hitbox_half_w[slot], value);
    r.third = rd8((uint16_t)(TABLE + 3 + r.second));
    WR8(OBJ->hitbox_half_h[slot], r.third);
    return r;
}

static Ended set_hitbox_from_frame_rows_exit_shim(ObjArm r)
{
    uint8_t row = (uint8_t)(r.first - 0x13);

    return ended_with_overflow(ended_with_y(ended_with_carry(ended_load(r.third), (row & 0x40) != 0), r.second),
                               sub_overflow(r.first, 0x13, row));
}

void abi_SetHitboxFromFrameRows(void)
{
    abi_ended(set_hitbox_from_frame_rows_exit_shim(SetHitboxFromFrameRows(cpu.x)));
}

/*
 * @routine 093D:2B7 EnterState7OnFlagOrYSignChange
 * @run 71:C2B7
 * WHAT IT DOES: Sends the object to state 7 when bit 7 of its status flags is set, or when its
 * Y position's high byte is not zero and the sign of the next byte (the middle byte) is the
 * opposite of the high byte's.
 * HOW: Tests the flag, then the high byte, then the middle byte against it. The arms past a
 * non-zero high byte ran in no script: the draft stops there and liftcmp judges only the flag
 * and zero tests; the rest waits for coverage (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z from the last load; X, Y, C and V kept.
 */
ObjArm EnterState7OnFlagOrYSignChange(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->status_flags[slot]);
    r.arm = S5_E7_ENTERED;
    if ((r.first & 0x80) == 0) {
        r.second = RD8(OBJ->y_hi[slot]);
        r.arm = S5_E7_ZERO;
        if (r.second == 0) {
            return r;
        }
        r.third = RD8(OBJ->y_lo[slot]);
        r.arm = S5_E7_SAME;
        if ((r.second & 0x80) != 0) {
            if ((r.third & 0x80) != 0) {
                return r;
            }
        } else if ((r.third & 0x80) == 0) {
            return r;
        }
        r.arm = S5_E7_ENTERED;
    }
    WR8(OBJ->state[slot], 7);
    return r;
}

static Ended enter_state7_on_flag_or_y_sign_change_exit_shim(ObjArm r)
{
    switch (r.arm) {
    case S5_E7_ZERO:
        return ended_load(0);
    case S5_E7_SAME:
        return ended_load(r.third);
    default:
        return ended_load(7);
    }
}

void abi_EnterState7OnFlagOrYSignChange(void)
{
    abi_ended(enter_state7_on_flag_or_y_sign_change_exit_shim(EnterState7OnFlagOrYSignChange(cpu.x)));
}

/*
 * @routine 093D:2D7 SetHitboxFromFrameRowsIndirect
 * @run 71:C2D7
 * WHAT IT DOES: Sets the object's whole hit rectangle from a four byte row of a table, the row
 * named by a second table indexed by the frame minus $27.
 * HOW: The index table is at overlay address $C6E6, the rows at $C6FF.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte loaded; N, Z from it; C bit 6 of the row number; Y the
 * row's offset; X kept.
 */
ObjArm SetHitboxFromFrameRowsIndirect(uint8_t slot)
{
    enum { ROW_NUMBERS = 0xC6E6, ROWS = 0xC6FF };
    ObjArm r = { 0 };
    uint8_t value;

    r.first = RD8(OBJ->frame[slot]);
    r.second = rd8((uint16_t)(ROW_NUMBERS + (uint8_t)(r.first - 0x27)));
    r.third = (uint8_t)(r.second << 2);
    value = rd8((uint16_t)(ROWS + r.third));
    WR8(OBJ->hitbox_dx[slot], value);
    value = rd8((uint16_t)(ROWS + 1 + r.third));
    WR8(OBJ->hitbox_dy[slot], value);
    value = rd8((uint16_t)(ROWS + 2 + r.third));
    WR8(OBJ->hitbox_half_w[slot], value);
    r.fourth = rd8((uint16_t)(ROWS + 3 + r.third));
    WR8(OBJ->hitbox_half_h[slot], r.fourth);
    return r;
}

static Ended set_hitbox_from_frame_rows_indirect_exit_shim(ObjArm r)
{
    return ended_with_overflow(ended_with_y(ended_with_carry(ended_load(r.fourth), (r.second & 0x40) != 0), r.third),
                               sub_overflow(r.first, 0x27, (uint8_t)(r.first - 0x27)));
}

void abi_SetHitboxFromFrameRowsIndirect(void)
{
    abi_ended(set_hitbox_from_frame_rows_indirect_exit_shim(SetHitboxFromFrameRowsIndirect(cpu.x)));
}

/*
 * @routine 093D:43A LaunchHopByF3510
 * @run 71:C43A
 * WHAT IT DOES: Launches the object in a hop: X speed from a two byte table by the object's f3510,
 * Y speed -3.00 (up), frame $36, f35c0 set to 8, and its step byte advanced.
 * HOW: The table is at overlay address $C45C, indexed by f3510 (0 or 1).
 * WAITS: no.
 * Entry: X object. Exit: A 8; N, Z from the step byte's new value; Y the f3510 byte; X and C
 * kept.
 */
ObjArm LaunchHopByF3510(uint8_t slot)
{
    enum { TABLE = 0xC45C };
    uint8_t index = RD8(OBJ->f3510[slot]);

    return hop_from_index(slot, index, TABLE, 0x36);
}

void abi_LaunchHopByF3510(void)
{
    abi_ended(hop_from_index_exit_shim(LaunchHopByF3510(cpu.x)));
}

/*
 * @routine 093D:5C3 CountdownStopThenState3
 * @run 71:C5C3
 * WHAT IT DOES: Counts the object's f35c0 timer down and, when it reaches zero, stops its
 * animation and sends it to state 3.
 * HOW: One decrement of the timer; the rest only at zero.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z from the timer, or A 3 at zero; X, Y and C kept.
 */
ObjArm CountdownStopThenState3(uint8_t slot)
{
    return countdown_stop_then_state(slot, COL(OBJ->f35c0), 3);
}

void abi_CountdownStopThenState3(void)
{
    abi_ended(countdown_exit_shim(CountdownStopThenState3(cpu.x)));
}

/*
 * @routine 093B:0F6 StartWithFrame1A
 * @run 70:B0F6
 * WHAT IT DOES: The first time the object runs (state 0), starts it from its record and gives it
 * frame $1A; later states leave it alone.
 * HOW: Tests the state, then LoadObjectRecord and one store.
 * WAITS: no.
 * Entry: X object. Exit: A the state, or $1A after the start; N, Z from it; C and Y from the
 * record load when it ran; X and V kept.
 */
ObjArm StartWithFrame1A(uint8_t slot)
{
    ObjArm r = { 0 };

    r.third = RD8(OBJ->state[slot]);
    r.arm = S5_SW1_LATER;
    if (r.third != 0) {
        return r;
    }
    jsr_push(0xB0FE);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    WR8(OBJ->frame[slot], 0x1A);
    r.arm = S5_SW1_STARTED;
    return r;
}

static Ended start_with_frame1a_exit_shim(ObjArm r)
{
    if (r.arm == S5_SW1_LATER) {
        return ended_load(r.third);
    }
    return ended_then(record_exit_shim(r), ended_load(0x1A));
}

void abi_StartWithFrame1A(void)
{
    abi_ended(start_with_frame1a_exit_shim(StartWithFrame1A(cpu.x)));
}

/*
 * @routine 093C:180 ThrowByNextKind
 * @run 70:B980
 * WHAT IT DOES: Moves the object one step along a velocity built from its f3040 byte (bit 0 picks
 * the X speed from a two byte table, the low nibble times $10, negated, is the Y speed), then
 * clears both speeds again.
 * HOW: The table is at overlay address $B9A3. ApplyVelocity moves it between the two sets of
 * stores.
 * WAITS: no.
 * Entry: X object. Exit: A, N, Z, C and V from ApplyVelocity's add; Y bit 0 of f3040; X kept.
 * Clobbers zTemp10.
 */
ObjArm ThrowByNextKind(uint8_t slot)
{
    enum { SPEEDS = 0xB9A3 };
    ObjArm r = { 0 };
    uint8_t kind = RD8(OBJ->next_kind[slot]);
    uint8_t shifted;

    r.first = (uint8_t)(kind & 1);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(SPEEDS + r.first)));
    kind = RD8(OBJ->next_kind[slot]);
    shifted = (uint8_t)(kind << 4);
    WR8(OBJ->vy_hi[slot], (uint8_t)((uint8_t)(shifted ^ 0xFF) + 1));
    jsr_push(0xB99C);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    return r;
}

static Ended stage05_throw_by_next_kind_exit_shim(ObjArm r)
{
    return ended_then(ended_with_y(ended_kept(), r.first), ended_of_axis_move(r.move));
}

void abi_ThrowByNextKind(void)
{
    abi_ended(stage05_throw_by_next_kind_exit_shim(ThrowByNextKind(cpu.x)));
}

/*
 * @routine 093C:23E AnimateThenState1
 * @run 70:BA3E
 * WHAT IT DOES: Steps the object's animation along one of two lists picked by bit 0 of its f3618;
 * when the list has ended, adds one to f3618 and sends the object to state 1.
 * HOW: The two list pointers are at overlay address $BA5D. StepObjectFrames's result says whether
 * the list ended.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames (Y the list's offset when the step did not change
 * it); when the list ended, A 1, N clear, Z clear. X and V kept. Clobbers zTemp10-11.
 */
ObjArm AnimateThenState1(uint8_t slot)
{
    enum { LISTS = 0xBA5D };
    ObjArm r = { 0 };
    uint8_t count = RD8(OBJ->f3618[slot]);
    uint8_t exit_y;

    r.first = (uint8_t)((count & 1) << 1);
    exit_y = r.first;
    WR8(ZP->temp10, rd8((uint16_t)(LISTS + r.first)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + r.first)));
    jsr_push(0xBA52);
    r.step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    r.y = exit_y;
    r.arm = S5_AS_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    count = RD8(OBJ->f3618[slot]);
    count = (uint8_t)(count + 1);
    WR8(OBJ->f3618[slot], count);
    WR8(OBJ->state[slot], 1);
    r.arm = S5_AS_ENDED;
    return r;
}

static Ended animate_then_state1_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_with_y(ended_kept(), r.first), frame_step_exit_shim(r));

    if (r.arm == S5_AS_RUNNING) {
        return e;
    }
    return ended_then(e, ended_load(1));
}

void abi_AnimateThenState1(void)
{
    abi_ended(animate_then_state1_exit_shim(AnimateThenState1(cpu.x)));
}

/*
 * @routine 093C:338 AnimateThenNextStep
 * @run 70:BB38
 * WHAT IT DOES: Steps the object's animation along the list that its f3670 and f36c8 pick (two
 * pointers a table row, f3670 times two plus f36c8 as the byte offset); when the list has ended
 * the step byte goes up by one.
 * HOW: The pointers are at overlay address $BB54. V is left by the offset's add.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, and when the list ended N, Z from the new step byte
 * with A 0; V from the offset's add; Y the offset when the step did not change it. Clobbers
 * zTemp10-11.
 */
ObjArm AnimateThenNextStep(uint8_t slot)
{
    enum { LISTS = 0xBB54 };
    ObjArm r = { 0 };
    uint8_t exit_y;

    r.first = RD8(OBJ->f3670[slot]);
    r.first = (uint8_t)(r.first << 1);
    r.second = RD8(OBJ->f36c8[slot]);
    r.third = (uint8_t)(r.first + r.second);
    exit_y = r.third;
    WR8(ZP->temp10, rd8((uint16_t)(LISTS + r.third)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + r.third)));
    jsr_push(0xBB4E);
    r.step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    r.y = exit_y;
    r.arm = S5_AN_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    r.fourth = RD8(OBJ->f2800[slot]);
    r.fourth = (uint8_t)(r.fourth + 1);
    WR8(OBJ->f2800[slot], r.fourth);
    r.arm = S5_AN_ENDED;
    return r;
}

static Ended animate_then_next_step_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_with_overflow(ended_with_y(ended_kept(), r.third), add_overflow(r.first, r.second, r.third)),
                         frame_step_exit_shim(r));

    if (r.arm == S5_AN_RUNNING) {
        return e;
    }
    return ended_then(e, ended_nz(r.fourth));
}

void abi_AnimateThenNextStep(void)
{
    abi_ended(animate_then_next_step_exit_shim(AnimateThenNextStep(cpu.x)));
}

/*
 * @routine 093C:48F FollowParentYAndLeave
 * @run 70:BC8F
 * WHAT IT DOES: Copies the Y position's middle byte of the object whose slot is in this object's
 * f3568 (its parent) into this object's, and removes this object when the parent is in state 4.
 * HOW: ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: A the parent's state, N, Z, C from its compare with 4; Y the parent's
 * slot; X and V kept.
 */
ObjArm FollowParentYAndLeave(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t middle;

    r.second = RD8(OBJ->f3568[slot]);
    middle = RD8(OBJ->y_lo[r.second]);
    WR8(OBJ->y_lo[slot], middle);
    r.first = RD8(OBJ->state[r.second]);
    if (r.first == 4) {
        ClearObject(slot);
    }
    return r;
}

static Ended follow_parent_y_and_leave_exit_shim(ObjArm r)
{
    return ended_with_y(ended_compare(r.first, 4), r.second);
}

void abi_FollowParentYAndLeave(void)
{
    abi_ended(follow_parent_y_and_leave_exit_shim(FollowParentYAndLeave(cpu.x)));
}

/*
 * @routine 093D:579 AnimateThenNextStepB
 * @run 71:C579
 * WHAT IT DOES: Steps the object's animation along the list at overlay address $C590; when the list
 * has ended, restarts it and adds one to the step byte.
 * HOW: The list pointer goes into zTemp10-11; StepObjectFrames says whether the list ended.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, and at the end N, Z from the new step byte; A 0;
 * X and V kept. Clobbers zTemp10-11.
 */
ObjArm AnimateThenNextStepB(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t exit_y = 0;

    WR8(ZP->temp11, 0xC5);
    WR8(ZP->temp10, 0x90);
    jsr_push(0xC584);
    r.step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    r.y = exit_y;
    r.arm = S5_AN_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.first = RD8(OBJ->f2800[slot]);
    r.first = (uint8_t)(r.first + 1);
    WR8(OBJ->f2800[slot], r.first);
    r.arm = S5_AN_ENDED;
    return r;
}

static Ended animate_then_next_step_b_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == S5_AN_RUNNING) {
        return e;
    }
    return ended_then(e, ended_nz(r.first));
}

void abi_AnimateThenNextStepB(void)
{
    abi_ended(animate_then_next_step_b_exit_shim(AnimateThenNextStepB(cpu.x)));
}

/*
 * @routine 093D:78C CountdownThenFallOrBecomeType9B
 * @run 71:C78C
 * WHAT IT DOES: Counts the object's f3510 down; while it runs, pulls the object down with gravity
 * and moves it by its velocity; at zero it turns into an object of type $9B in state 0.
 * HOW: ApplyGravityUnlimited, then ApplyVelocity as a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: as ApplyVelocity while counting; at zero A $9B, N set, Z clear (C, V
 * kept); X and Y kept. Clobbers zTemp10.
 */
ObjArm CountdownThenFallOrBecomeType9B(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t left = RD8(OBJ->f3510[slot]);

    left = (uint8_t)(left - 1);
    WR8(OBJ->f3510[slot], left);
    if (left == 0) {
        r.arm = CF_BECAME;
        r.first = become_type(slot, 0x9B).first;
        return r;
    }
    jsr_push(0xC79D);
    ApplyGravityUnlimited(slot);
    jsr_pop();
    r.arm = CF_FELL;
    r.move = ApplyVelocity(slot);
    return r;
}

/* At zero become_type's LDA; otherwise ApplyVelocity's exit. */
static Ended countdown_then_fall_exit_shim(ObjArm r)
{
    return r.arm == CF_BECAME ? ended_load(r.first) : ended_of_axis_move(r.move);
}

void abi_CountdownThenFallOrBecomeType9B(void)
{
    abi_ended(countdown_then_fall_exit_shim(CountdownThenFallOrBecomeType9B(cpu.x)));
}

/*
 * @routine 093C:1A5 PickTimersAtRandom
 * @run 70:B9A5
 * WHAT IT DOES: Gives the object a pair of timers (f3510 and life_timer) from a table of four
 * pairs, the pair picked by a random number.
 * HOW: NextRandom, two bits of it doubled into an index into the table at overlay address $B9B9.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the second timer, N and Z from it; C 0 (the doubling); V as
 * NextRandom's last add left it; Y the table offset; X kept.
 */
ObjArm PickTimersAtRandom(uint8_t slot)
{
    enum { TABLE = 0xB9B9 };

    return pick_pair_at_random(slot, TABLE, 0xB9A8, false);
}

void abi_PickTimersAtRandom(void)
{
    abi_random_end(pick_pair_at_random_exit_shim(PickTimersAtRandom(cpu.x)));
}

/*
 * @routine 093C:1C1 PickAngleAtRandom
 * @run 70:B9C1
 * WHAT IT DOES: Gives the object an angle (f35c0 and f3568) from a table of four pairs, the pair
 * picked by a random number.
 * HOW: As PickTimersAtRandom, with the table at overlay address $B9D5.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as PickTimersAtRandom.
 */
ObjArm PickAngleAtRandom(uint8_t slot)
{
    enum { TABLE = 0xB9D5 };

    return pick_pair_at_random(slot, TABLE, 0xB9C4, true);
}

void abi_PickAngleAtRandom(void)
{
    abi_random_end(pick_pair_at_random_exit_shim(PickAngleAtRandom(cpu.x)));
}

/*
 * @routine 093C:4A3 AnimateMoveAndSag
 * @run 70:BCA3
 * WHAT IT DOES: Steps the object's animation along the list its life_timer picks (a table of
 * pointers at overlay address $BCD1), moves it by its velocity, and while life_timer is 1 adds
 * $18/256 to its Y speed.
 * HOW: StepObjectFrames, ApplyVelocity, then a 16-bit add through the Y speed bytes.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C from the compare with 1, or from the last add
 * (the Y speed's high byte); V from ApplyVelocity or that add; Y the list's offset unless the
 * animation step changed it; X kept. Clobbers zTemp10-11.
 */
ObjArm AnimateMoveAndSag(uint8_t slot)
{
    enum { LISTS = 0xBCD1 };
    ObjArm r = { 0 };
    uint8_t exit_y, value;
    Sum sum;

    r.first = RD8(OBJ->life_timer[slot]);
    r.second = (uint8_t)(r.first << 1);
    exit_y = r.second;
    WR8(ZP->temp10, rd8((uint16_t)(LISTS + r.second)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + r.second)));
    jsr_push(0xBCB5);
    r.step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    r.y = exit_y;
    jsr_push(0xBCB8);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.third = RD8(OBJ->life_timer[slot]);
    r.arm = S5_AM_PLAIN;
    if (r.third != 1) {
        return r;
    }
    value = RD8(OBJ->vy_lo[slot]);
    sum = add_bytes(value, 0x18, false);
    WR8(OBJ->vy_lo[slot], sum.value);
    value = RD8(OBJ->vy_hi[slot]);
    r.sum = add_bytes(value, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], r.sum.value);
    r.arm = S5_AM_SAGGED;
    return r;
}

static Ended animate_move_and_sag_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_with_y(ended_kept(), r.second), frame_step_exit_shim(r));

    e = ended_then(e, ended_of_axis_move(r.move));
    e = ended_then(e, ended_compare(r.third, 1));
    if (r.arm == S5_AM_PLAIN) {
        return e;
    }
    return ended_then(e, ended_with_overflow(ended_with_carry(ended_load(r.sum.value), r.sum.carry), r.sum.overflow));
}

void abi_AnimateMoveAndSag(void)
{
    abi_ended(animate_move_and_sag_exit_shim(AnimateMoveAndSag(cpu.x)));
}

/*
 * @routine 093C:6E9 StepFramesFacingPlayerC
 * @run 70:BEE9
 * WHAT IT DOES: Turns the object to face the player and steps its animation along the list whose
 * pointer is the pair of bytes at overlay address $BEFE plus Y.
 * HOW: SideOfPlayer, the pointer into zTemp10-11, then StepObjectFrames as a tail jump.
 * WAITS: no.
 * Entry: X object, Y the list's offset, D clear. Exit: as StepObjectFrames. Clobbers zTemp00,
 * zTemp10-11.
 */
ObjArm StepFramesFacingPlayerC(uint8_t slot, uint8_t index)
{
    enum { LISTS = 0xBEFE };
    ObjArm r = { 0 };
    uint8_t sides, exit_y = index;

    jsr_push(0xBEEC);
    sides = SideOfPlayer(slot).sides;
    jsr_pop();
    WR8(OBJ->facing_left[slot], (uint8_t)(sides & 1));
    WR8(ZP->temp10, rd8((uint16_t)(LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + index)));
    r.step = StepObjectFrames(slot, &exit_y);
    r.y = exit_y;
    return r;
}

void abi_StepFramesFacingPlayerC(void)
{
    abi_ended(frame_step_exit_shim(StepFramesFacingPlayerC(cpu.x, cpu.y)));
}

/*
 * @routine 093D:282 FaceThePlayerInSomeStates
 * @run 71:C282
 * WHAT IT DOES: Turns the object to face the player when a table at overlay address $C293 says its
 * state is one that does.
 * HOW: Reads the table by the state byte; non-zero runs SideOfPlayer into the facing byte.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the table byte or the side bit; N, Z from it; C from
 * SideOfPlayer's last compare when it ran; Y the state; X kept. Clobbers zTemp00.
 */
ObjArm FaceThePlayerInSomeStates(uint8_t slot)
{
    enum { TABLE = 0xC293 };
    ObjArm r = { 0 };

    r.first = RD8(OBJ->state[slot]);
    r.second = rd8((uint16_t)(TABLE + r.first));
    r.arm = S5_FP_NONE;
    if (r.second == 0) {
        return r;
    }
    jsr_push(0xC28D);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.third = (uint8_t)(r.side.sides & 1);
    WR8(OBJ->facing_left[slot], r.third);
    r.arm = S5_FP_FACED;
    return r;
}

static SideEnd face_the_player_in_some_states_exit_shim(ObjArm r)
{
    SideEnd s = { ended_kept(), false, { 0, false, false } };

    if (r.arm == S5_FP_NONE) {
        s.end = ended_with_y(ended_load(r.second), r.first);
        return s;
    }
    s.side = r.side;
    s.side_used = true;
    s.end = ended_with_y(ended_load(r.third), r.first);
    return s;
}

void abi_FaceThePlayerInSomeStates(void)
{
    abi_side_end(face_the_player_in_some_states_exit_shim(FaceThePlayerInSomeStates(cpu.x)));
}

/*
 * @routine 093D:4AB NextPhaseAtRandom
 * @run 71:C4AB
 * WHAT IT DOES: Steps the object to its next phase: counts f3568 up, sets life_timer from a four
 * byte table by it, turns f3510 to the side the player is on, and picks a step byte of 1 or 2 at
 * random.
 * HOW: A table at overlay address $C4CE, SideOfPlayer, NextRandom.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the new step byte, N and Z from it, C and V clear; Y the table
 * offset; X kept. Clobbers zTemp00.
 */
ObjArm NextPhaseAtRandom(uint8_t slot)
{
    enum { TABLE = 0xC4CE };
    ObjArm r = { 0 };
    uint8_t phase, side, bits;
    RandomResult random;

    INC8(OBJ->f3568[slot]);
    phase = RD8(OBJ->f3568[slot]);
    r.first = (uint8_t)(phase & 3);
    WR8(OBJ->life_timer[slot], rd8((uint16_t)(TABLE + r.first)));
    jsr_push(0xC4BD);
    side = SideOfPlayer(slot).sides;
    jsr_pop();
    WR8(OBJ->f3510[slot], (uint8_t)(side & 1));
    jsr_push(0xC4C5);
    random = NextRandom();
    jsr_pop();
    bits = (uint8_t)(random.value & 1);
    r.second = (uint8_t)(bits + 1);
    WR8(OBJ->f2800[slot], r.second);
    return r;
}

static Ended next_phase_at_random_exit_shim(ObjArm r)
{
    return ended_with_y(ended_with_overflow(ended_with_carry(ended_load(r.second), false), false), r.first);
}

void abi_NextPhaseAtRandom(void)
{
    abi_ended(next_phase_at_random_exit_shim(NextPhaseAtRandom(cpu.x)));
}

/*
 * @routine 093B:4C3 ShowOnScreenAndWrapX
 * @run 70:B4C3
 * WHAT IT DOES: The first time the object runs (state 0) starts it from its record. After that
 * it sets frame 2, hides it (frame 0) when its screen X is past the first seven screens and
 * the low sum is negative, and, while the scroll moves, wraps the object's X by $80 across the
 * page boundary at either end of the level (X high $FF with low under $E0 to high 1, or high 1
 * with low from $20 up to high $FF), by the scroll's direction.
 * HOW: Adds the scroll's X low and screen to the object's X; tests ScrollSpeedX's sign.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as LoadObjectRecord in state 0; otherwise A, N, Z, C and V
 * from the last compare, load or add; X, Y kept. Clobbers zTemp00 (in states but 0).
 */
ObjArm ShowOnScreenAndWrapX(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t low, high, scroll_low, scroll_high;
    Sum sum;

    if (state == 0) {
        r.arm = S5_SX_RECORD;
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        return r;
    }
    WR8(OBJ->frame[slot], 2);
    low = RD8(OBJ->x_lo[slot]);
    scroll_low = RD8(SCROLL->x_lo[0]);
    sum = add_bytes(low, scroll_low, false);
    WR8(ZP->temp00, sum.value);
    high = RD8(OBJ->x_hi[slot]);
    scroll_high = RD8(SCROLLE->x_hi_screen[0]);
    r.sum2 = add_bytes(high, scroll_high, sum.carry);
    if (r.sum2.value >= 7) {
        r.third = RD8(ZP->temp00);
        if ((r.third & 0x80) != 0) {
            WR8(OBJ->frame[slot], 0);
        }
    }
    r.fourth = RD8(SCROLLE->speed_x[0]);
    r.arm = S5_SX_STILL;
    if (r.fourth == 0) {
        return r;
    }
    if ((r.fourth & 0x80) == 0) {
        r.fifth = RD8(OBJ->x_hi[slot]);
        r.arm = S5_SX_POS_HIGH;
        if (r.fifth != 0xFF) {
            return r;
        }
        r.sixth = RD8(OBJ->x_lo[slot]);
        r.arm = S5_SX_POS_LOW;
        if (r.sixth >= 0xE0) {
            return r;
        }
        r.sum3 = add_bytes(r.sixth, 0x80, false);
        WR8(OBJ->x_lo[slot], r.sum3.value);
        WR8(OBJ->x_hi[slot], 1);
        r.arm = S5_SX_POS_WRAP;
        return r;
    }
    r.fifth = RD8(OBJ->x_hi[slot]);
    r.arm = S5_SX_NEG_HIGH;
    if (r.fifth != 1) {
        return r;
    }
    r.sixth = RD8(OBJ->x_lo[slot]);
    r.arm = S5_SX_NEG_LOW;
    if (r.sixth < 0x20) {
        return r;
    }
    r.sum3 = add_bytes(r.sixth, 0x80, false);
    WR8(OBJ->x_lo[slot], r.sum3.value);
    WR8(OBJ->x_hi[slot], 0xFF);
    r.arm = S5_SX_NEG_WRAP;
    return r;
}

static Ended show_on_screen_and_wrap_x_exit_shim(ObjArm r)
{
    Ended e = ended_with_overflow(ended_compare(r.sum2.value, 7), r.sum2.overflow);

    if (r.arm == S5_SX_RECORD) {
        return record_exit_shim(r);
    }
    if (r.sum2.value >= 7) {
        e = ended_then(e, ended_load(r.third));
    }
    e = ended_then(e, ended_load(r.fourth));
    if (r.arm == S5_SX_STILL) {
        return e;
    }
    if ((r.fourth & 0x80) == 0) {
        e = ended_then(e, ended_compare(r.fifth, 0xFF));
        if (r.arm == S5_SX_POS_HIGH) {
            return e;
        }
        e = ended_then(e, ended_compare(r.sixth, 0xE0));
        if (r.arm == S5_SX_POS_LOW) {
            return e;
        }
        return ended_then(e, ended_with_overflow(ended_with_carry(ended_load(1), r.sum3.carry), r.sum3.overflow));
    }
    e = ended_then(e, ended_compare(r.fifth, 1));
    if (r.arm == S5_SX_NEG_HIGH) {
        return e;
    }
    e = ended_then(e, ended_compare(r.sixth, 0x20));
    if (r.arm == S5_SX_NEG_LOW) {
        return e;
    }
    return ended_then(e, ended_with_overflow(ended_with_carry(ended_load(0xFF), r.sum3.carry), r.sum3.overflow));
}

void abi_ShowOnScreenAndWrapX(void)
{
    abi_ended(show_on_screen_and_wrap_x_exit_shim(ShowOnScreenAndWrapX(cpu.x)));
}

/*
 * @routine 093C:6AB StartChargeWhenPlayerNear
 * @run 70:BEAB
 * WHAT IT DOES: Animates the object facing the player; while the object is on the first screen
 * and under $78 pixels from the player along X, it stops its animation, keeps only bit 7 of its
 * flags, sets its Y speed's high byte to 3, picks a timer ($44 plus 7 random bits) in f36c8 and
 * enters state 4 with step 2.
 * HOW: StepFramesFacingPlayerC (Y 0), the high bytes ORed, DistanceXToPlayer, NextRandom.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A 2 and N, Z clear, C 0, V from the add of the timer when it
 * started; otherwise as the last compare, OR or animation step left them; Y 0 or as the step
 * left it; X kept.
 */
ObjArm StartChargeWhenPlayerNear(uint8_t slot)
{
    ObjArm r;
    uint8_t high, y_high, flags;

    jsr_push(0xBEAF);
    r = StepFramesFacingPlayerC(slot, 0);
    jsr_pop();
    high = RD8(OBJ->x_hi[slot]);
    y_high = RD8(OBJ->y_hi[slot]);
    r.first = (uint8_t)(high | y_high);
    r.arm = SC_HIGH;
    if (r.first != 0) {
        return r;
    }
    jsr_push(0xBEBA);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    r.arm = SC_FAR;
    if (r.distance.distance >= 0x78) {
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    flags = RD8(OBJ->flags[slot]);
    WR8(OBJ->flags[slot], (uint8_t)(flags & 0x80));
    WR8(OBJ->vy_hi[slot], 3);
    jsr_push(0xBED4);
    r.random = NextRandom();
    jsr_pop();
    r.second = (uint8_t)(r.random.value & 0x7F);
    WR8(OBJ->f36c8[slot], (uint8_t)(r.second + 0x44));
    WR8(OBJ->state[slot], 4);
    WR8(OBJ->f2800[slot], 2);
    r.arm = SC_STARTED;
    return r;
}

/* LDY #0 and the step, the ORA of the high bytes; the distance and its CMP with $78; when it started, NextRandom's exit, the timer's add of $44 after CLC, LDA #2. */
static Ended start_charge_when_player_near_exit_shim(ObjArm r)
{
    Ended e = ended_then(ldy_frame_step_shim(0, r), ended_load(r.first));
    Sum sum;

    if (r.arm == SC_HIGH) {
        return e;
    }
    e = ended_then(ended_then(e, ended_of_distance(r.distance)), ended_compare(r.distance.distance, 0x78));
    if (r.arm == SC_FAR) {
        return e;
    }
    sum = add_bytes(r.second, 0x44, false);
    e = ended_then(e, ended_of_random(r.random));
    e = ended_then(e, ended_with_overflow(ended_with_carry(ended_load(sum.value), false), sum.overflow));
    return ended_then(e, ended_load(2));
}

void abi_StartChargeWhenPlayerNear(void)
{
    abi_ended(start_charge_when_player_near_exit_shim(StartChargeWhenPlayerNear(cpu.x)));
}

/*
 * @routine 093D:191 WaitThenState2B
 * @run 71:C191
 * WHAT IT DOES: A two-step wait like WaitThenState2: the first pass sets f34b8 to $1C and the step
 * to 1; later passes animate the object facing the player (list index 2) and count the timer down;
 * at zero the object enters state 2.
 * HOW: StepFramesFacingPlayerC with Y 2, then a decrement.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as WaitThenState2.
 */
ObjArm WaitThenState2B(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t step = RD8(OBJ->f2800[slot]);
    uint8_t left;
    ObjArm frames;

    if (step == 0) {
        WR8(OBJ->life_timer[slot], 0x1C);
        step = RD8(OBJ->f2800[slot]);
        step = (uint8_t)(step + 1);
        WR8(OBJ->f2800[slot], step);
        r.arm = WT_START;
        r.first = step;
        return r;
    }
    jsr_push(0xC1A4);
    frames = StepFramesFacingPlayerC(slot, 2);
    jsr_pop();
    r.step = frames.step;
    r.y = frames.y;
    left = RD8(OBJ->life_timer[slot]);
    left = (uint8_t)(left - 1);
    WR8(OBJ->life_timer[slot], left);
    r.arm = WT_COUNTING;
    r.first = left;
    if (left != 0) {
        return r;
    }
    WR8(OBJ->state[slot], 2);
    r.arm = WT_DONE;
    return r;
}

void abi_WaitThenState2B(void)
{
    abi_ended(wait_then_state2_exit_shim(WaitThenState2B(cpu.x)));
}

/*
 * @routine 093C:28D ChooseNextMoveWhenFacingAway
 * @run 70:BA8D
 * WHAT IT DOES: When the side the player is on matches the object's facing (compared with bit 0
 * of f3618), picks a phase and timer at random from two tables and advances the step; otherwise it
 * enters state 1.
 * HOW: SideOfPlayer; NextRandom twice; tables at overlay addresses $BAD5, $BAE5 and $BAF1.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the last byte loaded, N and Z from the step byte's new
 * value (when picking); C and V from the second add; Y the last table index; X kept. Clobbers
 * zTemp00, zTemp08.
 */
ObjArm ChooseNextMoveWhenFacingAway(uint8_t slot)
{
    enum { PHASES = 0xBAD5, TIMERS = 0xBAE5, PIECES = 0xBAF1 };
    ObjArm r = { 0 };
    uint8_t facing = RD8(OBJ->facing_left[slot]);
    uint8_t count = RD8(OBJ->f3618[slot]);
    uint8_t y, phase, shifted, value;
    RandomResult random;

    r.first = (uint8_t)((facing ^ count) & 1);
    WR8(ZP->temp08, r.first);
    jsr_push(0xBA9A);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.second = (uint8_t)(r.side.sides & 1);
    r.third = RD8(ZP->temp08);
    r.arm = S5_CM_STATE1;
    if (r.second != r.first) {
        WR8(OBJ->state[slot], 1);
        return r;
    }
    jsr_push(0xBAA9);
    random = NextRandom();
    jsr_pop();
    y = (uint8_t)(random.value & 0x0F);
    phase = rd8((uint16_t)(PHASES + y));
    WR8(OBJ->f3670[slot], phase);
    shifted = (uint8_t)(phase << 2);
    WR8(ZP->temp00, shifted);
    jsr_push(0xBAB9);
    random = NextRandom();
    jsr_pop();
    value = RD8(ZP->temp00);
    r.sum = add_bytes((uint8_t)(random.value & 3), value, false);
    y = r.sum.value;
    value = rd8((uint16_t)(TIMERS + y));
    WR8(OBJ->f3720[slot], value);
    count = RD8(OBJ->f3618[slot]);
    r.y = (uint8_t)(count & 1);
    r.fourth = rd8((uint16_t)(PIECES + r.y));
    WR8(OBJ->f36c8[slot], r.fourth);
    r.fifth = INC8(OBJ->f2800[slot]);
    r.arm = S5_CM_PICKED;
    return r;
}

static Ended choose_next_move_when_facing_away_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == S5_CM_STATE1) {
        e = ended_then(ended_of_side(r.side), ended_load(r.second));
        e = ended_then(e, ended_compare(r.second, r.third));
        return ended_then(e, ended_load(1));
    }
    return ended_with_y(ended_with_overflow(ended_with_carry(ended_with_a(ended_nz(r.fifth), r.fourth), r.sum.carry),
                                            r.sum.overflow), r.y);
}

void abi_ChooseNextMoveWhenFacingAway(void)
{
    abi_ended(choose_next_move_when_facing_away_exit_shim(ChooseNextMoveWhenFacingAway(cpu.x)));
}

/*
 * @routine 093C:1E4 CountTimerPairDownOrRepick
 * @run 70:B9E4
 * WHAT IT DOES: Counts the sixteen bit timer (f3510 low, life_timer high) down by one; when it is
 * already zero, picks new timers at random (PickTimersAtRandom), stops the animation and enters
 * state 2.
 * HOW: The two bytes are ORed to test for zero; the decrement is a subtract with the borrow carried.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the new high byte and N, Z from it, C and V from the borrow
 * subtract; or A 2 and N, Z clear with C, V and Y as the random pick left them.
 */
ObjArm CountTimerPairDownOrRepick(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t high = RD8(OBJ->life_timer[slot]);
    uint8_t low = (uint8_t)(high | RD8(OBJ->f3510[slot]));
    Sum sum;

    if (low == 0) {
        r.arm = S5_CT_PICKED;
        jsr_push(0xB9EF);
        r.sub = obj_sub(PickTimersAtRandom(slot));
        jsr_pop();
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 2);
        return r;
    }
    r.arm = S5_CT_COUNTING;
    low = RD8(OBJ->f3510[slot]);
    sum = add_bytes(low, 0xFE, true);
    WR8(OBJ->f3510[slot], sum.value);
    high = RD8(OBJ->life_timer[slot]);
    r.sum = add_bytes(high, 0xFF, sum.carry);
    WR8(OBJ->life_timer[slot], r.sum.value);
    return r;
}

Ended count_timer_pair_down_or_repick_exit_shim(ObjArm r)
{
    RandomEnd picked;

    if (r.arm == S5_CT_COUNTING) {
        return ended_of_sum(r.sum);
    }
    picked = pick_pair_at_random_exit_shim(obj_arm(r.sub));
    return ended_then(ended_then(ended_of_random(picked.random), picked.end), ended_load(2));
}

void abi_CountTimerPairDownOrRepick(void)
{
    abi_ended(count_timer_pair_down_or_repick_exit_shim(CountTimerPairDownOrRepick(cpu.x)));
}

/*
 * @routine 093C:20D CountAnglePairDownOrRepick
 * @run 70:BA0D
 * WHAT IT DOES: On the first screen across only: counts the sixteen bit value f35c0 (low) and
 * f3568 (high) down by one; when it is already zero, picks a new angle at random
 * (PickAngleAtRandom), stops the animation, clears the step and enters state 3.
 * HOW: As CountTimerPairDownOrRepick.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as CountTimerPairDownOrRepick, or A the X high byte and N, Z from
 * it when off the first screen.
 */
ObjArm CountAnglePairDownOrRepick(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t value, low, high;
    Sum sum;

    r.first = RD8(OBJ->x_hi[slot]);
    r.arm = S5_CA_HIGH;
    if (r.first != 0) {
        return r;
    }
    value = RD8(OBJ->f3568[slot]);
    value = (uint8_t)(value | RD8(OBJ->f35c0[slot]));
    if (value == 0) {
        r.arm = S5_CA_PICKED;
        jsr_push(0xBA1D);
        r.sub = obj_sub(PickAngleAtRandom(slot));
        jsr_pop();
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->f2800[slot], 0);
        WR8(OBJ->state[slot], 3);
        return r;
    }
    r.arm = S5_CA_COUNTING;
    low = RD8(OBJ->f35c0[slot]);
    sum = add_bytes(low, 0xFE, true);
    WR8(OBJ->f35c0[slot], sum.value);
    high = RD8(OBJ->f3568[slot]);
    r.sum = add_bytes(high, 0xFF, sum.carry);
    WR8(OBJ->f3568[slot], r.sum.value);
    return r;
}

Ended count_angle_pair_down_or_repick_exit_shim(ObjArm r)
{
    RandomEnd picked;

    if (r.arm == S5_CA_HIGH) {
        return ended_load(r.first);
    }
    if (r.arm == S5_CA_COUNTING) {
        return ended_of_sum(r.sum);
    }
    picked = pick_pair_at_random_exit_shim(obj_arm(r.sub));
    return ended_then(ended_then(ended_of_random(picked.random), picked.end), ended_load(3));
}

void abi_CountAnglePairDownOrRepick(void)
{
    abi_ended(count_angle_pair_down_or_repick_exit_shim(CountAnglePairDownOrRepick(cpu.x)));
}

/*
 * @routine 093C:54C AnimateHitboxThenExpire
 * @run 70:BD4C
 * WHAT IT DOES: Animates the object along the list at overlay address $BD63, sets its hit
 * rectangle from the frame (SetHitboxFromFrameRows), and counts f3510 down; at zero the object
 * removes itself.
 * HOW: StepObjectFrames, SetHitboxFromFrameRows, a decrement, ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X object, D clear. Exit: N, Z from the timer; A, C, V, Y as SetHitboxFromFrameRows left
 * them; X kept. Clobbers zTemp10-11.
 */
ObjArm AnimateHitboxThenExpire(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t exit_y = 0;

    WR8(ZP->temp11, 0xBD);
    WR8(ZP->temp10, 0x63);
    jsr_push(0xBD57);
    r.step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    r.y = exit_y;
    jsr_push(0xBD5A);
    r.sub = obj_sub(SetHitboxFromFrameRows(slot));
    jsr_pop();
    r.first = RD8(OBJ->f3510[slot]);
    r.first = (uint8_t)(r.first - 1);
    WR8(OBJ->f3510[slot], r.first);
    if (r.first == 0) {
        ClearObject(slot);
    }
    return r;
}

static Ended animate_hitbox_then_expire_exit_shim(ObjArm r)
{
    Ended e = ended_then(frame_step_exit_shim(r), set_hitbox_from_frame_rows_exit_shim(obj_arm(r.sub)));

    return ended_then(e, ended_nz(r.first));
}

void abi_AnimateHitboxThenExpire(void)
{
    abi_ended(animate_hitbox_then_expire_exit_shim(AnimateHitboxThenExpire(cpu.x)));
}

/*
 * @routine 093D:33A WalkWhileFrameChangesB
 * @run 71:C33A
 * WHAT IT DOES: Animates the object along the list whose pointer is the pair of bytes at overlay
 * address $C356 plus Y and, on the frames where the picture changed, gives it an X speed from a
 * table at $C35A by its f3510 and moves it along X.
 * HOW: StepObjectFrames, then ApplyVelocityX.
 * WAITS: no.
 * Entry: X object, Y the list's offset, D clear. Exit: after a change as ApplyVelocityX, Y f3510;
 * otherwise as StepObjectFrames; X kept. Clobbers zTemp10-11.
 */
ObjArm WalkWhileFrameChangesB(uint8_t slot, uint8_t index)
{
    enum { LISTS = 0xC356, SPEEDS = 0xC35A };
    ObjArm r = { 0 };
    uint8_t exit_y = index;

    WR8(ZP->temp10, rd8((uint16_t)(LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + index)));
    jsr_push(0xC347);
    r.step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    r.y = exit_y;
    r.arm = S5_WB_STEPPED;
    if (r.step != FRAME_CHANGED) {
        return r;
    }
    r.first = RD8(OBJ->f3510[slot]);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(SPEEDS + r.first)));
    jsr_push(0xC355);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    r.arm = S5_WB_MOVED;
    return r;
}

static Ended walk_while_frame_changes_b_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == S5_WB_STEPPED) {
        return e;
    }
    return ended_then(ended_then(e, ended_with_y(ended_kept(), r.first)), ended_of_axis_move(r.move));
}

void abi_WalkWhileFrameChangesB(void)
{
    abi_ended(walk_while_frame_changes_b_exit_shim(WalkWhileFrameChangesB(cpu.x, cpu.y)));
}

/*
 * @routine 093D:7A0 StartThenBlinkAndExpire
 * @run 71:C7A0
 * WHAT IT DOES: The first time (state 0) starts the object from its record and sets f3510 to
 * $10. After that it counts f3510 down, removing the object at zero; otherwise it shows frame 0,
 * or its life_timer as the frame on every fourth count.
 * HOW: LoadObjectRecord; a decrement; ClearObject as a tail jump.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as the last store, load, AND or decrement left them; Y 11 after
 * the record start.
 */
ObjArm StartThenBlinkAndExpire(uint8_t slot)
{
    ObjArm r = { 0 };

    r.third = RD8(OBJ->state[slot]);
    if (r.third == 0) {
        r.arm = S5_BE_RECORD;
        jsr_push(0xC7A8);
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        jsr_pop();
        WR8(OBJ->f3510[slot], 0x10);
        return r;
    }
    r.fourth = RD8(OBJ->f3510[slot]);
    r.fourth = (uint8_t)(r.fourth - 1);
    WR8(OBJ->f3510[slot], r.fourth);
    r.arm = S5_BE_EXPIRED;
    if (r.fourth == 0) {
        ClearObject(slot);
        return r;
    }
    WR8(OBJ->frame[slot], 0);
    r.fifth = RD8(OBJ->f3510[slot]);
    r.fifth = (uint8_t)(r.fifth & 3);
    r.arm = S5_BE_SHOWN;
    if (r.fifth == 0) {
        r.sixth = RD8(OBJ->life_timer[slot]);
        WR8(OBJ->frame[slot], r.sixth);
        r.arm = S5_BE_BLINK;
    }
    return r;
}

static Ended start_then_blink_and_expire_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == S5_BE_RECORD) {
        return ended_then(record_exit_shim(r), ended_load(0x10));
    }
    e = ended_then(ended_load(r.third), ended_nz(r.fourth));
    if (r.arm == S5_BE_EXPIRED) {
        return e;
    }
    e = ended_then(e, ended_load(r.fifth));
    if (r.arm == S5_BE_SHOWN) {
        return e;
    }
    return ended_then(e, ended_load(r.sixth));
}

void abi_StartThenBlinkAndExpire(void)
{
    abi_ended(start_then_blink_and_expire_exit_shim(StartThenBlinkAndExpire(cpu.x)));
}

/*
 * @routine 093D:53D LeapAwayFromPlayerTable
 * @run 71:C53D
 * WHAT IT DOES: Advances the step and launches the object: its X speed from a table at overlay
 * address $C569 by the player's distance in 32 pixel steps, Y speed -4.5 ($FB80); when its
 * f3510 is not zero the X speed is then reversed.
 * HOW: DistanceXToPlayer shifted to five bits and doubled, NegateVelocityX.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z from f3510 (when zero) or the negated speed's high byte
 * with C and V from its add; Y the table offset.
 */
ObjArm LeapAwayFromPlayerTable(uint8_t slot)
{
    enum { TABLE = 0xC569 };
    ObjArm r = { 0 };

    INC8(OBJ->f2800[slot]);
    jsr_push(0xC543);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    r.first = (uint8_t)((r.distance.distance >> 5) << 1);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(TABLE + 1 + r.first)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(TABLE + r.first)));
    WR8(OBJ->vy_hi[slot], 0xFB);
    WR8(OBJ->vy_lo[slot], 0x80);
    r.second = RD8(OBJ->f3510[slot]);
    r.arm = S5_LA_STRAIGHT;
    if (r.second == 0) {
        return r;
    }
    jsr_push(0xC568);
    r.sum = NegateVelocityX(slot);
    jsr_pop();
    r.arm = S5_LA_TURNED;
    return r;
}

static Ended leap_away_from_player_table_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_of_distance(r.distance), ended_with_y(ended_with_carry(ended_load(r.second), false), r.first));

    if (r.arm == S5_LA_STRAIGHT) {
        return e;
    }
    return ended_then(e, ended_of_sum(r.sum));
}

void abi_LeapAwayFromPlayerTable(void)
{
    abi_ended(leap_away_from_player_table_exit_shim(LeapAwayFromPlayerTable(cpu.x)));
}

/*
 * @routine 093C:731 PickStateByPlayerDistanceB
 * @run 70:BF31
 * WHAT IT DOES: PickStateByPlayerDistance in this stage's overlay, with its state table at
 * overlay address $BF57.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as PickStateByPlayerDistance.
 */
ObjArm PickStateByPlayerDistanceB(uint8_t slot, bool carry_in)
{
    return pick_state_by_distance(slot, carry_in, 0xBF57, 0xBF4D);
}

void abi_PickStateByPlayerDistanceB(void)
{
    abi_ended(pick_state_by_distance_exit_shim(PickStateByPlayerDistanceB(cpu.x, (cpu.p & P_C) != 0)));
}

/*
 * @routine 093C:7AA PickSpeedAndTurnFromTableC
 * @run 70:BFAA
 * WHAT IT DOES: PickSpeedAndTurnFromTable in this stage's overlay, with its table at overlay
 * address $BFD6.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as PickSpeedAndTurnFromTable.
 */
ObjArm PickSpeedAndTurnFromTableC(uint8_t slot)
{
    return pick_speed_and_turn(slot, 0xBFD6, 0xBFCE);
}

void abi_PickSpeedAndTurnFromTableC(void)
{
    abi_ended(pick_speed_and_turn_exit_shim(PickSpeedAndTurnFromTableC(cpu.x)));
}

/*
 * @routine 093D:13E TurnOnRandomBeatWalkB
 * @run 71:C13E
 * @stop C16F
 * WHAT IT DOES: A walk that turns on a random beat: at its first step it turns to the player's side,
 * takes an X speed from a table at overlay address $C16F (two bytes by side), picks life_timer
 * at random from eight bytes at $C173, stops the animation and moves on; after that it runs
 * TurnOnRandomBeatWalkFollowB.
 * HOW: The shared body; the two tables lie between the two parts and stay data.
 * WAITS: no.
 * Entry: X object, D clear. Exit: first step: A the timer, N, Z from the new step, C and V from
 * NextRandom, Y its index; later as TurnOnRandomBeatWalkFollowB.
 */
ObjArm TurnOnRandomBeatWalkB(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t phase = RD8(OBJ->f2800[slot]);

    if (phase != 0) {
        r.arm = TW_FOLLOW;
        r.sub = obj_sub(TurnOnRandomBeatWalkFollowB(slot));
        return r;
    }
    r.arm = TW_FIRST;
    r.sub = obj_sub(walk_on_random_beat_first(slot, 0xC13E, 0xC16F, 0xC173));
    return r;
}

void abi_TurnOnRandomBeatWalkB(void)
{
    abi_ended(turn_on_random_beat_walk_exit_shim(TurnOnRandomBeatWalkB(cpu.x)));
}

/*
 * @routine 093D:17B TurnOnRandomBeatWalkFollowB
 * @run 71:C17B
 * WHAT IT DOES: The later steps of TurnOnRandomBeatWalkB: animates the object facing the player (list 2),
 * counts life_timer down and moves it along X; at zero it enters state 5 with step 0.
 * HOW: StepFramesFacingPlayerC, a decrement, ApplyVelocityX as a tail jump.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as the animation step, the decrement and ApplyVelocityX leave them
 * (A 5 and N, Z clear at zero).
 */
ObjArm TurnOnRandomBeatWalkFollowB(uint8_t slot)
{
    return walk_on_random_beat_follow(slot, 0xC180, StepFramesFacingPlayerC);
}

void abi_TurnOnRandomBeatWalkFollowB(void)
{
    abi_ended(walk_on_random_beat_follow_exit_shim(TurnOnRandomBeatWalkFollowB(cpu.x)));
}

/*
 * @routine 093D:51F SpawnShotFromFrame
 * @run 71:C51F
 * WHAT IT DOES: When the object's frame is $2B or more, starts an object of type $35 at its place
 * in a free slot from $30, facing as this one does, with this frame in its life_timer.
 * HOW: FindFreeSlotFrom30, StartObjectAtPlace, three copies.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the frame (the last byte copied), N, Z from it; C as the slot
 * search left it; Y the slot; X kept.
 */
ObjArm SpawnShotFromFrame(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t found_slot;
    bool found;

    r.first = RD8(OBJ->frame[slot]);
    r.arm = S5_SS_EARLY;
    if (r.first < 0x2B) {
        return r;
    }
    jsr_push(0xC529);
    found_slot = FindFreeSlotFrom30(&r.third, &found);
    jsr_pop();
    r.second = found_slot;
    r.flag = found;
    r.arm = S5_SS_NO_SLOT;
    if (!found) {
        return r;
    }
    jsr_push(0xC530);
    r.fourth = StartObjectAtPlace(0x35, slot, found_slot);
    jsr_pop();
    r.fifth = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[found_slot], r.fifth);
    r.fifth = RD8(OBJ->frame[slot]);
    WR8(OBJ->life_timer[found_slot], r.fifth);
    r.arm = S5_SS_SHOT;
    return r;
}

static Ended spawn_shot_from_frame_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 0x2B);

    if (r.arm == S5_SS_EARLY) {
        return e;
    }
    e = ended_then(e, ended_of_find_slot(r.second, r.third, r.flag));
    if (r.arm == S5_SS_NO_SLOT) {
        return e;
    }
    e = ended_then(e, ended_load(r.fourth));
    return ended_then(e, ended_load(r.fifth));
}

void abi_SpawnShotFromFrame(void)
{
    abi_ended(spawn_shot_from_frame_exit_shim(SpawnShotFromFrame(cpu.x)));
}

/*
 * @routine 093D:2FD StartAndMakeNeighbour
 * @run 71:C2FD
 * WHAT IT DOES: Starts the object from its record, empties the slot after it and starts an object
 * of type $2D there at its place, sets its own f3670 to 1 and its life_timer to $50.
 * HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A $50 and N, Z clear; C from the record; Y the neighbour's slot;
 * X kept.
 */
ObjArm StartAndMakeNeighbour(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xC300);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    r.third = (uint8_t)(slot + 1);
    jsr_push(0xC306);
    ClearObjectInY(r.third);
    jsr_pop();
    jsr_push(0xC30B);
    r.fourth = StartObjectAtPlace(0x2D, slot, r.third);
    jsr_pop();
    WR8(OBJ->f3670[slot], 1);
    WR8(OBJ->life_timer[slot], 0x50);
    return r;
}

static Ended start_and_make_neighbour_exit_shim(ObjArm r)
{
    Ended e = ended_then(record_exit_shim(r), ended_with_y(ended_nz(r.third), r.third));

    e = ended_then(e, ended_load(r.fourth));
    return ended_then(e, ended_load(0x50));
}

void abi_StartAndMakeNeighbour(void)
{
    abi_ended(start_and_make_neighbour_exit_shim(StartAndMakeNeighbour(cpu.x)));
}

/*
 * @routine 093D:76B StartWithFrameAndFace
 * @run 71:C76B
 * @stop C787
 * WHAT IT DOES: The first time (state 0) starts the object from its record, gives it a frame from
 * a table at overlay address $C787 by its life_timer, and turns its X speed round when the player is
 * on its left; later states go on as CountdownThenFallOrBecomeType9B.
 * HOW: LoadObjectRecord, SideOfPlayer, NegateVelocityX.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as the last call or test left them; Y the life_timer used as the
 * table index.
 */
ObjArm StartWithFrameAndFace(uint8_t slot)
{
    enum { FRAMES = 0xC787 };
    ObjArm r = { 0 };
    uint8_t state = RD8(OBJ->state[slot]);
    uint8_t y, value;
    bool carry;

    if (state != 0) {
        r.arm = SF_FALL;
        r.sub = obj_sub(CountdownThenFallOrBecomeType9B(slot));
        return r;
    }
    jsr_push(0xC773);
    r.first = LoadObjectRecord(slot, &r.second, &carry);
    jsr_pop();
    r.flag = carry;
    y = RD8(OBJ->life_timer[slot]);
    value = rd8((uint16_t)(FRAMES + y));
    WR8(OBJ->frame[slot], value);
    r.third = value;
    r.y = y;
    jsr_push(0xC77F);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.fourth = (uint8_t)(r.side.sides & 1);
    r.arm = SF_FACED;
    if (r.fourth == 0) {
        return r;
    }
    jsr_push(0xC786);
    r.sum = NegateVelocityX(slot);
    jsr_pop();
    r.arm = SF_TURNED;
    return r;
}

/* Past state 0, CountdownThenFallOrBecomeType9B's exit. Otherwise the record, the frame's LDA with Y its index, SideOfPlayer, the AND, and NegateVelocityX when turned. */
static Ended start_with_frame_and_face_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == SF_FALL) {
        return countdown_then_fall_exit_shim(obj_arm(r.sub));
    }
    e = ended_then(ended_of_record(r.first, r.second, r.flag), ended_with_y(ended_load(r.third), r.y));
    e = ended_then(ended_then(e, ended_of_side(r.side)), ended_load(r.fourth));
    return r.arm == SF_FACED ? e : ended_then(e, ended_of_sum(r.sum));
}

void abi_StartWithFrameAndFace(void)
{
    abi_ended(start_with_frame_and_face_exit_shim(StartWithFrameAndFace(cpu.x)));
}

/*
 * @routine 093C:5C4 StartThenAnimateAndExpire
 * @run 70:BDC4
 * WHAT IT DOES: The first time (state 0) starts the object from its record, sets f2a10 to its
 * life_timer plus 3 and f3510 to $28. After that it animates along the list whose pointer its
 * life_timer picks (a table at overlay address $BDF8) and counts f3510 down; at zero the object
 * removes itself, otherwise it goes on as FollowParentYAndLeave.
 * HOW: LoadObjectRecord; StepObjectFrames; the last two as tail jumps.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as the last step that ran left them.
 */
ObjArm StartThenAnimateAndExpire(uint8_t slot)
{
    enum { LISTS = 0xBDF8 };
    ObjArm r = { 0 };
    uint8_t timer, exit_y;

    r.third = RD8(OBJ->state[slot]);
    if (r.third == 0) {
        r.arm = S5_TA_RECORD;
        jsr_push(0xBDCC);
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        jsr_pop();
        timer = RD8(OBJ->life_timer[slot]);
        r.sum = add_bytes(timer, 3, false);
        WR8(OBJ->f2a10[slot], r.sum.value);
        WR8(OBJ->f3510[slot], 0x28);
        return r;
    }
    timer = RD8(OBJ->life_timer[slot]);
    r.fourth = (uint8_t)(timer << 1);
    exit_y = r.fourth;
    WR8(ZP->temp10, rd8((uint16_t)(LISTS + r.fourth)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + r.fourth)));
    jsr_push(0xBDED);
    r.step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    r.y = exit_y;
    r.fifth = RD8(OBJ->f3510[slot]);
    r.fifth = (uint8_t)(r.fifth - 1);
    WR8(OBJ->f3510[slot], r.fifth);
    r.arm = S5_TA_EXPIRED;
    if (r.fifth == 0) {
        ClearObject(slot);
        return r;
    }
    r.sub = obj_sub(FollowParentYAndLeave(slot));
    r.arm = S5_TA_FOLLOWED;
    return r;
}

static Ended start_then_animate_and_expire_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == S5_TA_RECORD) {
        return ended_then(ended_then(record_exit_shim(r), ended_of_sum(r.sum)), ended_load(0x28));
    }
    e = ended_then(ended_with_y(ended_load(r.third), r.fourth), frame_step_exit_shim(r));
    e = ended_then(e, ended_nz(r.fifth));
    if (r.arm == S5_TA_EXPIRED) {
        return e;
    }
    return ended_then(e, follow_parent_y_and_leave_exit_shim(obj_arm(r.sub)));
}

void abi_StartThenAnimateAndExpire(void)
{
    abi_ended(start_then_animate_and_expire_exit_shim(StartThenAnimateAndExpire(cpu.x)));
}

/*
 * @routine 093C:534 CountdownSoundThenNextState
 * @run 70:BD34
 * WHAT IT DOES: Counts the object's f3510 down; at zero plays sound $26, sets f3510 to $40 and
 * f3250 to 2, and moves the object on to its next state.
 * HOW: A decrement; the rest only at zero.
 * WAITS: no.
 * Entry: X object. Exit: N, Z from the timer, or A 2 and N, Z from the new state; C, V and Y as
 * the sound call left them at zero; X kept.
 */
ObjArm CountdownSoundThenNextState(uint8_t slot, uint8_t entry_y)
{
    ObjArm r = { 0 };
    uint8_t state;

    r.first = RD8(OBJ->f3510[slot]);
    r.first = (uint8_t)(r.first - 1);
    WR8(OBJ->f3510[slot], r.first);
    r.arm = S5_CD_COUNTING;
    if (r.first != 0) {
        return r;
    }
    r.sounded = true;
    r.command = 0x26;
    jsr_push(0xBD3E);
    r.call = SoundCommand(0x26, slot, entry_y);
    jsr_pop();
    WR8(OBJ->f3510[slot], 0x40);
    WR8(OBJ->f3250[slot], 2);
    state = RD8(OBJ->state[slot]);
    r.second = (uint8_t)(state + 1);
    WR8(OBJ->state[slot], r.second);
    r.arm = S5_CD_SOUNDED;
    return r;
}

static SoundEnd countdown_sound_then_next_state_exit_shim(ObjArm r)
{
    SoundEnd s = { ended_kept(), ended_kept(), false, 0, { 0 } };

    if (r.arm == S5_CD_COUNTING) {
        s.end = ended_nz(r.first);
        return s;
    }
    s.sounded = true;
    s.command = r.command;
    s.before = ended_load(0x26);
    s.call = r.call;
    s.end = ended_with_a(ended_nz(r.second), 2);
    return s;
}

void abi_CountdownSoundThenNextState(void)
{
    abi_sound_end(countdown_sound_then_next_state_exit_shim(CountdownSoundThenNextState(cpu.x, cpu.y)));
}

/*
 * @routine 093D:0F5 SoundEveryFourthFrame
 * @run 71:C0F5
 * WHAT IT DOES: Plays sound $22 on the frames when the low two bits of the object's f3618 are zero.
 * HOW: One test and the call.
 * WAITS: no.
 * Entry: X object. Exit: A the masked bits and N, Z from them when no sound; else as the sound
 * call left them.
 */
ObjArm SoundEveryFourthFrame(uint8_t slot, uint8_t entry_y)
{
    return sound_every_fourth_frame(slot, entry_y, 0xC101);
}

void abi_SoundEveryFourthFrame(void)
{
    abi_sound_end(sound_every_fourth_frame_exit_shim(SoundEveryFourthFrame(cpu.x, cpu.y)));
}

/*
 * @routine 093C:474 CountdownSoundThenFollowParent
 * @run 70:BC74
 * WHAT IT DOES: Counts f3510 down; at zero it plays sound $25, sets f3250 to 2, stops its
 * animation and advances its step. Then, every frame, it goes on as FollowParentYAndLeave.
 * HOW: A decrement, SoundCommand, FollowParentYAndLeave as a tail jump.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as FollowParentYAndLeave.
 */
ObjArm CountdownSoundThenFollowParent(uint8_t slot, uint8_t entry_y)
{
    ObjArm r = { 0 };

    r.first = (uint8_t)(RD8(OBJ->f3510[slot]) - 1);
    WR8(OBJ->f3510[slot], r.first);
    r.arm = S5_CF_COUNTING;
    if (r.first == 0) {
        r.sounded = true;
        r.command = 0x25;
        jsr_push(0xBC7E);
        r.call = SoundCommand(0x25, slot, entry_y);
        jsr_pop();
        WR8(OBJ->f3250[slot], 2);
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        r.second = INC8(OBJ->state[slot]);
        r.arm = S5_CF_SOUNDED;
    }
    r.sub = obj_sub(FollowParentYAndLeave(slot));
    return r;
}

static SoundEnd countdown_sound_then_follow_parent_exit_shim(ObjArm r)
{
    SoundEnd s = { ended_kept(), ended_kept(), false, 0, { 0 } };
    Ended e = ended_nz(r.first);

    if (r.arm == S5_CF_SOUNDED) {
        s.sounded = true;
        s.command = r.command;
        s.before = ended_then(e, ended_load(0x25));
        s.call = r.call;
        e = ended_nz(r.second);
    }
    s.end = ended_then(e, follow_parent_y_and_leave_exit_shim(obj_arm(r.sub)));
    return s;
}

void abi_CountdownSoundThenFollowParent(void)
{
    abi_sound_end(countdown_sound_then_follow_parent_exit_shim(CountdownSoundThenFollowParent(cpu.x, cpu.y)));
}

/*
 * @routine 093C:39A AnimateThenNumberGroupAndLeave
 * @run 70:BB9A
 * WHAT IT DOES: Steps the frame list picked by bit 0 of f3618 (two pointers at overlay address
 * $BBE5). When the list has ended it numbers the group it belongs to: from the slot in f37d0 up to
 * f3828, each object of kind $26 below state 4 gets its place in the group in next_kind and the
 * group's first slot in f37d0. Then it removes itself.
 * HOW: StepObjectFrames, a loop over the slots, ClearObject.
 * WAITS: no.
 * Entry: X object, D clear. Exit: when the list went on, as StepObjectFrames; when it ended, A, N, Z,
 * C from the loop's last compare and Y the end slot (X kept); clobbers zTemp00, zTemp01, zTemp03.
 */
ObjArm AnimateThenNumberGroupAndLeave(uint8_t slot)
{
    enum { LISTS = 0xBBE5 };
    ObjArm r = { 0 };
    uint8_t exit_y, y, held = 0, count;

    r.first = (uint8_t)((RD8(OBJ->f3618[slot]) & 1) << 1);
    exit_y = r.first;
    WR8(ZP->temp10, rd8((uint16_t)(LISTS + r.first)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + r.first)));
    jsr_push(0xBBAE);
    r.step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    r.y = exit_y;
    r.arm = S5_NG_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    y = RD8(OBJ->f37d0[slot]);
    WR8(ZP->temp00, 0);
    held = RD8(OBJ->f3828[slot]);
    WR8(ZP->temp01, held);
    r.third = INC8(ZP->temp01);
    do {
        held = RD8(OBJ->kind[y]);
        if (held == 0x26) {
            held = RD8(OBJ->state[y]);
            if (held < 4) {
                count = RD8(ZP->temp00);
                if (count == 0) {
                    WR8(ZP->temp03, y);
                }
                count = RD8(ZP->temp00);
                WR8(OBJ->next_kind[y], count);
                held = RD8(ZP->temp03);
                WR8(OBJ->f37d0[y], held);
                count = RD8(ZP->temp00);
                WR8(ZP->temp00, (uint8_t)(count + 1));
            }
        }
        y++;
        r.third = RD8(ZP->temp01);
    } while (y < r.third);
    r.second = y;
    r.fourth = held;
    r.arm = S5_NG_NUMBERED;
    jsr_push(0xBBE4);
    ClearObject(slot);
    jsr_pop();
    return r;
}

static Ended animate_then_number_group_and_leave_exit_shim(ObjArm r)
{
    if (r.arm == S5_NG_NUMBERED) {
        return ended_with_a(ended_with_y(ended_compare(r.second, r.third), r.second), r.fourth);
    }
    return ended_then(ended_with_y(ended_kept(), r.first), frame_step_exit_shim(r));
}

void abi_AnimateThenNumberGroupAndLeave(void)
{
    abi_ended(animate_then_number_group_and_leave_exit_shim(AnimateThenNumberGroupAndLeave(cpu.x)));
}

/*
 * @routine 093D:6A4 FollowPreviousPieceB
 * @run 71:C6A4
 * WHAT IT DOES: FollowPreviousPiece with the frame base $27 and its row table at overlay address $C71B.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as FollowPreviousPiece.
 */
ObjArm FollowPreviousPieceB(uint8_t slot)
{
    return follow_previous_piece(slot, 0x27, 0xC71B, 0xC6B2);
}

void abi_FollowPreviousPieceB(void)
{
    abi_ended(follow_previous_piece_exit_shim(FollowPreviousPieceB(cpu.x)));
}

/*
 * @routine 093D:02B AnimateMoveThenSetUpAtRandom
 * @run 71:C02B
 * WHAT IT DOES: Animates the object facing the player (list 4), moves it, and counts life_timer
 * down; at zero it plays sound $93, picks a speed and turn (PickSpeedAndTurnFromTableC), sets
 * f36c8 to $18 plus seven random bits, stops its animation and advances its step.
 * HOW: StepFramesFacingPlayerC, ApplyVelocity, a decrement, then the calls named.
 * WAITS: no.
 * Entry: X object, D clear. Exit: while counting, N, Z from the timer, C and V from ApplyVelocity,
 * Y as the animation step left it; at zero A the timer value, N, Z from the new step, C and V from
 * the add of $18.
 */
ObjArm AnimateMoveThenSetUpAtRandom(uint8_t slot)
{
    ObjArm r;
    RandomResult random;
    uint8_t left;

    jsr_push(0xC030);
    r = StepFramesFacingPlayerC(slot, 4);
    jsr_pop();
    jsr_push(0xC033);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    left = (uint8_t)(RD8(OBJ->life_timer[slot]) - 1);
    WR8(OBJ->life_timer[slot], left);
    r.first = left;
    r.arm = AR_COUNTING;
    if (left != 0) {
        return r;
    }
    r.sounded = true;
    r.command = 0x93;
    jsr_push(0xC03D);
    r.call = SoundCommand(0x93, slot, animate_move_at_random_sound_y_shim(r));
    jsr_pop();
    jsr_push(0xC040);
    r.sub = obj_sub(PickSpeedAndTurnFromTableC(slot));
    jsr_pop();
    jsr_push(0xC043);
    random = NextRandom();
    jsr_pop();
    r.random = random;
    r.second = (uint8_t)(random.value & 0x7F);
    WR8(OBJ->f36c8[slot], (uint8_t)(r.second + 0x18));
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.third = INC8(OBJ->f2800[slot]);
    r.arm = AR_DONE;
    return r;
}

/* LDY #4 and the step, the move, the DEC's N and Z; at zero LDA #$93 and the sound, then PickSpeedAndTurnFromTableC's
 * exit, NextRandom's, and A the sum ($18 added with CLC), N and Z from the INC, C and V the add's. */
static SoundEnd animate_move_at_random_exit_shim(ObjArm r)
{
    SoundEnd s = { ended_kept(), ended_kept(), false, 0, { 0 } };
    Ended e = ended_then(ended_then(ldy_frame_step_shim(4, r), ended_of_axis_move(r.move)), ended_nz(r.first));
    Sum sum;

    s.end = e;
    if (r.arm == AR_COUNTING) {
        return s;
    }
    s.sounded = true;
    s.command = r.command;
    s.before = ended_then(e, ended_load(0x93));
    s.call = r.call;
    sum = add_bytes(r.second, 0x18, false);
    e = ended_then(pick_speed_and_turn_exit_shim(obj_arm(r.sub)), ended_of_random(r.random));
    s.end = ended_then(e, ended_with_a(ended_with_overflow(ended_with_carry(ended_nz(r.third), sum.carry), sum.overflow), sum.value));
    return s;
}

/* The Y SoundCommand is entered with: the LDY #4, or the Y the step set. */
static uint8_t animate_move_at_random_sound_y_shim(ObjArm r)
{
    return ended_y_or(ldy_frame_step_shim(4, r), 0);
}

void abi_AnimateMoveThenSetUpAtRandom(void)
{
    abi_sound_end(animate_move_at_random_exit_shim(AnimateMoveThenSetUpAtRandom(cpu.x)));
}

/*
 * @routine 093C:417 StartSidewaysAndSpawn29
 * @run 70:BC17
 * WHAT IT DOES: Loads the record, sets f3510 to $28, reverses its X speed when it faces left, moves along X
 * and takes its speeds from a table of eight-byte rows at overlay address $BC64 (X high, X low, Y high, Y low
 * pairs by facing) picked by life_timer; then starts an object of type $29 at its place from a free slot from
 * $50, which takes its f3568.
 * HOW: LoadObjectRecord, NegateVelocityX, ApplyVelocityX, FindFreeSlotFrom50, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the copied f3568 and N, Z from it, C 1, Y the new slot, V from the table
 * index add; with no free slot, as FindFreeSlotFrom50 left them.
 */
ObjArm StartSidewaysAndSpawn29(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t nz, facing, y, found_slot;
    bool carry, found;
    Sum index;

    jsr_push(0xBC1A);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    WR8(OBJ->f3510[slot], 0x28);
    facing = RD8(OBJ->facing_left[slot]);
    if (facing != 0) {
        jsr_push(0xBC27);
        NegateVelocityX(slot);
        jsr_pop();
    }
    jsr_push(0xBC2A);
    ApplyVelocityX(slot);
    jsr_pop();
    WR8(ZP->temp00, (uint8_t)(RD8(OBJ->life_timer[slot]) << 3));
    facing = RD8(OBJ->facing_left[slot]);
    index = add_bytes((uint8_t)(facing << 2), RD8(ZP->temp00), false);
    r.other = index.overflow;
    y = index.value;
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xBC65 + y)));
    WR8(OBJ->vx_lo[slot], rd8((uint16_t)(0xBC64 + y)));
    WR8(OBJ->vy_hi[slot], rd8((uint16_t)(0xBC67 + y)));
    WR8(OBJ->vy_lo[slot], rd8((uint16_t)(0xBC66 + y)));
    jsr_push(0xBC56);
    found_slot = FindFreeSlotFrom50(&r.second, &found);
    jsr_pop();
    r.first = found_slot;
    r.flag = found;
    r.arm = S5_S29_NO_SLOT;
    if (!found) {
        return r;
    }
    jsr_push(0xBC5D);
    StartObjectAtPlace(0x29, slot, found_slot);
    jsr_pop();
    r.third = RD8(OBJ->f3568[slot]);
    WR8(OBJ->f3568[found_slot], r.third);
    r.arm = S5_S29_SPAWNED;
    return r;
}

static Ended start_sideways_and_spawn29_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_with_overflow(ended_kept(), r.other), ended_of_find_slot(r.first, r.second, r.flag));

    if (r.arm == S5_S29_NO_SLOT) {
        return e;
    }
    return ended_then(e, ended_load(r.third));
}

void abi_StartSidewaysAndSpawn29(void)
{
    abi_ended(start_sideways_and_spawn29_exit_shim(StartSidewaysAndSpawn29(cpu.x)));
}

/*
 * @routine 093C:4FD StartSidewaysAndSpawn29B
 * @run 70:BCFD
 * WHAT IT DOES: Loads the record, sets f3510 to $40, reverses its X speed when it faces left and moves along X;
 * starts an object of type $29 at its place from a free slot from $50 (when there is one), with life_timer 1 and
 * this object's f3568; then takes an X speed (high byte) from a table at overlay address $BD32 by its facing and
 * moves along X again.
 * HOW: LoadObjectRecord, NegateVelocityX, ApplyVelocityX, FindFreeSlotFrom50, StartObjectAtPlace, ApplyVelocityX.
 * The reversal ran in no script: the draft stops there, so liftcmp does not judge it.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C, V from the last ApplyVelocityX; Y the facing.
 */
ObjArm StartSidewaysAndSpawn29B(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t nz, facing, found_slot, found_last, value;
    bool carry, found;

    jsr_push(0xBD00);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    WR8(OBJ->f3510[slot], 0x40);
    facing = RD8(OBJ->facing_left[slot]);
    if (facing != 0) {
        jsr_push(0xBD0D);
        NegateVelocityX(slot);
        jsr_pop();
    }
    jsr_push(0xBD10);
    ApplyVelocityX(slot);
    jsr_pop();
    jsr_push(0xBD13);
    found_slot = FindFreeSlotFrom50(&found_last, &found);
    jsr_pop();
    if (found) {
        jsr_push(0xBD1A);
        StartObjectAtPlace(0x29, slot, found_slot);
        jsr_pop();
        WR8(OBJ->life_timer[found_slot], 1);
        value = RD8(OBJ->f3568[slot]);
        WR8(OBJ->f3568[found_slot], value);
    }
    r.first = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(0xBD32 + r.first)));
    jsr_push(0xBD31);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    return r;
}

static Ended start_sideways_and_spawn29b_exit_shim(ObjArm r)
{
    return ended_then(ended_with_y(ended_kept(), r.first), ended_of_axis_move(r.move));
}

void abi_StartSidewaysAndSpawn29B(void)
{
    abi_ended(start_sideways_and_spawn29b_exit_shim(StartSidewaysAndSpawn29B(cpu.x)));
}

/*
 * @routine 093D:619 BurstIntoEightPieces
 * @run 71:C619
 * WHAT IT DOES: Empties the slot after its own, then starts up to eight objects of type $34 at its place, each from
 * a free slot from $30, with its facing and, from a table of six-byte rows at overlay address $C674 by the piece
 * number, its life_timer, f3510, X speed and Y speed; it stops at the first piece that finds no slot. Then it becomes
 * kind $9C in state 0.
 * HOW: ClearObjectInY, a loop of FindFreeSlotFrom30 and StartObjectAtPlace; the piece number is kept in zHitSlot and
 * the object in zRichterObject while the table is read.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A $9C, N set, Z clear; C and Y from the last compare or slot search.
 */
ObjArm BurstIntoEightPieces(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t piece = 0, found_slot, twice, row;
    bool found;
    Sum index;

    r.first = (uint8_t)(slot + 1);
    jsr_push(0xC61F);
    ClearObjectInY(r.first);
    jsr_pop();
    for (;;) {
        WR8(ZP->hit_slot, piece);
        jsr_push(0xC625);
        found_slot = FindFreeSlotFrom30(&r.fifth, &found);
        jsr_pop();
        r.fourth = found_slot;
        r.arm = S5_BP_NO_SLOT;
        if (!found) {
            break;
        }
        jsr_push(0xC62C);
        StartObjectAtPlace(0x34, slot, found_slot);
        jsr_pop();
        WR8(OBJ->facing_left[found_slot], RD8(OBJ->facing_left[slot]));
        WR8(ZP->richter_object, slot);
        twice = (uint8_t)(RD8(ZP->hit_slot) << 1);
        WR8(ZP->temp00, twice);
        index = add_bytes((uint8_t)(twice << 1), RD8(ZP->temp00), false);
        row = index.value;
        WR8(OBJ->life_timer[found_slot], rd8((uint16_t)(0xC674 + row)));
        WR8(OBJ->f3510[found_slot], rd8((uint16_t)(0xC675 + row)));
        WR8(OBJ->vx_hi[found_slot], rd8((uint16_t)(0xC676 + row)));
        WR8(OBJ->vx_lo[found_slot], rd8((uint16_t)(0xC677 + row)));
        WR8(OBJ->vy_hi[found_slot], rd8((uint16_t)(0xC678 + row)));
        WR8(OBJ->vy_lo[found_slot], rd8((uint16_t)(0xC679 + row)));
        slot = RD8(ZP->richter_object);
        piece = RD8(ZP->hit_slot);
        piece++;
        r.second++;
        r.third = piece;
        r.other = index.overflow;
        r.arm = S5_BP_FULL;
        if (piece >= 8) {
            break;
        }
    }
    WR8(OBJ->kind[slot], 0x9C);
    WR8(OBJ->state[slot], 0);
    return r;
}

/* Each started piece's pass ends on its own count and add; a search that found no slot follows the last of those, or the first Y. */
static Ended burst_into_eight_pieces_exit_shim(ObjArm r)
{
    Ended e = ended_with_y(ended_kept(), r.first);

    if (r.second != 0) {
        e = ended_with_overflow(ended_with_y(ended_with_carry(ended_compare(r.third, 8), r.third >= 8), r.third), r.other);
    }
    if (r.arm == S5_BP_NO_SLOT) {
        e = ended_then(e, ended_of_find_slot(r.fourth, r.fifth, false));
    }
    return ended_then(e, ended_load(0x9C));
}

void abi_BurstIntoEightPieces(void)
{
    abi_ended(burst_into_eight_pieces_exit_shim(BurstIntoEightPieces(cpu.x)));
}

/*
 * @routine 093C:12E StartGroupThenThrow
 * @run 70:B92E
 * WHAT IT DOES: Loads the record. When bit 7 of next_kind is clear, next_kind is the number of followers: it
 * starts that many objects of type $26 in the slots after its own, each numbered from $81 in next_kind and
 * given this object's slot in f37d0 and the group's last slot in f3828 (this object's f37d0 is its own slot,
 * f3828 its slot plus the count). Then it clears bit 7 of next_kind, faces opposite to bit 0 of it, and runs
 * ThrowByNextKind, PickTimersAtRandom and PickAngleAtRandom.
 * HOW: LoadObjectRecord, a loop of ClearObjectInY and StartObjectAtPlace, then the three calls.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as PickAngleAtRandom.
 */
ObjArm StartGroupThenThrow(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t nz, count, y, mark, value, left;
    bool carry;
    Sum sum;

    jsr_push(0xB931);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    count = RD8(OBJ->next_kind[slot]);
    if ((count & 0x80) == 0) {
        WR8(OBJ->f37d0[slot], slot);
        count = RD8(OBJ->next_kind[slot]);
        sum = add_bytes(slot, count, false);
        WR8(OBJ->f3828[slot], sum.value);
        y = (uint8_t)(slot + 1);
        WR8(ZP->temp00, 0x81);
        do {
            jsr_push(0xB94B);
            ClearObjectInY(y);
            jsr_pop();
            jsr_push(0xB950);
            StartObjectAtPlace(0x26, slot, y);
            jsr_pop();
            mark = RD8(ZP->temp00);
            WR8(OBJ->next_kind[y], mark);
            WR8(OBJ->f37d0[y], slot);
            value = RD8(OBJ->f3828[slot]);
            WR8(OBJ->f3828[y], value);
            INC8(ZP->temp00);
            y++;
            left = DEC8(OBJ->next_kind[slot]);
        } while (left != 0);
    }
    count = RD8(OBJ->next_kind[slot]);
    count = (uint8_t)(count & 0x7F);
    WR8(OBJ->next_kind[slot], count);
    WR8(OBJ->facing_left[slot], (uint8_t)((count & 1) ^ 1));
    jsr_push(0xB979);
    ThrowByNextKind(slot);
    jsr_pop();
    jsr_push(0xB97C);
    PickTimersAtRandom(slot);
    jsr_pop();
    jsr_push(0xB97F);
    r.sub = obj_sub(PickAngleAtRandom(slot));
    jsr_pop();
    return r;
}

static RandomEnd start_group_then_throw_exit_shim(ObjArm r)
{
    return pick_pair_at_random_exit_shim(obj_arm(r.sub));
}

void abi_StartGroupThenThrow(void)
{
    abi_random_end(start_group_then_throw_exit_shim(StartGroupThenThrow(cpu.x)));
}

/*
 * @routine 093C:05D ContactWithPlayerBox14By20
 * @run 70:B85D
 * WHAT IT DOES: Tests whether the object, as a block of width $14 and height $20, touches the player: sets those
 * two sizes and runs ContactWithPlayer.
 * HOW: Two stores and ContactWithPlayer as a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: as ContactWithPlayer.
 */
ContactArm ContactWithPlayerBox14By20(uint8_t slot)
{
    WR8(ZP->temp08, 0x14);
    WR8(ZP->temp09, 0x20);
    return ContactWithPlayer(slot);
}

static Ended contact_with_player_box14_by20_exit_shim(ContactArm a)
{
    return ended_then(ended_load(0x20), contact_exit_shim(a));
}

void abi_ContactWithPlayerBox14By20(void)
{
    abi_ended(contact_with_player_box14_by20_exit_shim(ContactWithPlayerBox14By20(cpu.x)));
}
