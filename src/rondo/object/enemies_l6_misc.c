/*
 * TEMPORARY: the stage misc object routines of lot L6 (enemy kinds not yet identified), by routine. Bodies return what they ended on
 * (ObjArm, enemy_objects.h); each routine's exit shim builds A, Y and the flags from it and its abi_ wrapper applies them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

/* The arms the converted routines end on (ObjArm, enemy_objects.h). */
enum { HD_STATE6, HD_FEW_HITS, HD_HALVED };
enum { TP_WAITING, TP_TURNED, TP_AWAY, TP_STEPPED };
enum { CH_LOADED, CH_OFF, CH_BOXED };
enum { WP_LOADED, WP_WAITING, WP_PRESSED, WP_ENDED, WP_STEPPED };
enum { S7_CLEAR, S7_ABOVE, S7_SET };
enum { SO_OTHER_STEP, SO_NOT_IN_TABLE, SO_NO_SLOT, SO_SPAWNED };
enum { SW_LOADED, SW_SWINGING, SW_CLEARED, SW_PLACED };

static uint8_t spawn_then_step_y_shim(ObjArm inner, uint8_t y_entry);

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
enum { M6_NP_KIND, M6_NP_NEXT };
enum { M6_HD_STATE6, M6_HD_MILD, M6_HD_HALVED };
enum { M6_B5_RUNNING, M6_B5_ENDED };
enum { M6_FR_FALLING, M6_FR_STOPPED };
enum { M6_CD_COUNTING, M6_CD_STATE6 };
enum { M6_B9_RUNNING, M6_B9_AIMED };
enum { M6_B2_RUNNING, M6_B2_STARTED };
enum { M6_TP_COUNTING, M6_TP_TURNED, M6_TP_OPPOSITE, M6_TP_STEPPED };
enum { M6_SH_OFF, M6_SH_SET };
enum { M6_CP_RECORD, M6_CP_OFF, M6_CP_SET };
enum { M6_B1_RUNNING, M6_B1_SOUNDED };
enum { M6_B5C_RUNNING, M6_B5C_ENDED };
enum { M6_TM_COUNTING, M6_TM_NEXT };
enum { M6_WP_RECORD, M6_WP_Y, M6_WP_XHI, M6_WP_XLO, M6_WP_NOPRESS, M6_WP_START, M6_WP_HELD, M6_WP_ENDED, M6_WP_SOUND };
enum { M6_S7_NONE, M6_S7_LATE, M6_S7_ENTERED };
enum { M6_ST_NOT6, M6_ST_NO_MATCH, M6_ST_NO_SLOT, M6_ST_SPAWNED };
enum { M6_SR_RUNNING, M6_SR_ENDED };
enum { M6_LN_FLYING, M6_LN_LANDED };
enum { M6_SD_START, M6_SD_SWING, M6_SD_CLEARED, M6_SD_DROPPED };

/*
 * @routine 072C:5A7 LoadRecordA06
 * @run 70:BDA7
 * WHAT IT DOES: Loads the object's record.
 * HOW: LoadObjectRecord.
 * WAITS: no.
 * Entry: X object. Exit: as LoadObjectRecord.
 */
ObjArm LoadRecordA06(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xBDAA);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    return r;
}

void abi_LoadRecordA06(void)
{
    abi_ended(record_exit_shim(LoadRecordA06(cpu.x)));
}

/*
 * @routine 092E:2D1 ClearHitsTaken
 * @run 6B:8AD1
 * WHAT IT DOES: Sets the object's hits taken (OBJ_33B0) to 0.
 * HOW: A store.
 * WAITS: no.
 * Entry: X object. Exit: all kept.
 */
ObjArm ClearHitsTaken(uint8_t slot)
{
    ObjArm r = { 0 };

    WR8(OBJ->hit_taken[slot], 0);
    return r;
}

static Ended clear_hits_taken_exit_shim(ObjArm r)
{
    (void)r;
    return ended_kept();
}

void abi_ClearHitsTaken(void)
{
    abi_ended(clear_hits_taken_exit_shim(ClearHitsTaken(cpu.x)));
}

/*
 * @routine 12DF:038 RandomTimerC0
 * @run 71:D038
 * WHAT IT DOES: Sets the object's timer OBJ_34B8 to a random number with bits 6 and 7 set.
 * HOW: NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: A the timer, N set, Z clear; C and V from the random number.
 */
ObjArm RandomTimerC0(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xD03B);
    r.random = NextRandom();
    jsr_pop();
    r.first = (uint8_t)(r.random.value | 0xC0);
    WR8(OBJ->f34b8[slot], r.first);
    return r;
}

static Ended random_timer_c0_exit_shim(ObjArm r)
{
    return then(ended_of_random(r.random), ended_load(r.first));
}

void abi_RandomTimerC0(void)
{
    abi_ended(random_timer_c0_exit_shim(RandomTimerC0(cpu.x)));
}

/*
 * @routine 0E0B:5DD NextPhaseUnlessKind25
 * @run 70:B5DD
 * WHAT IT DOES: Goes on to the next OBJ_2800 unless the object in slot $50 is of kind $25.
 * HOW: A compare and an INC.
 * WAITS: no.
 * Entry: X object. Exit: the compare, or the new OBJ_2800's N and Z.
 */
ObjArm NextPhaseUnlessKind25(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->kind[0x50]);
    r.arm = M6_NP_KIND;
    if (r.first == 0x25) {
        return r;
    }
    r.second = INC8(OBJ->f2800[slot]);
    r.arm = M6_NP_NEXT;
    return r;
}

static Ended next_phase_unless_kind25_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 0x25);

    if (r.arm == M6_NP_KIND) {
        return e;
    }
    return then(e, ended_nz(r.second));
}

void abi_NextPhaseUnlessKind25(void)
{
    abi_ended(next_phase_unless_kind25_exit_shim(NextPhaseUnlessKind25(cpu.x)));
}

/*
 * @routine 0E0B:099 HalveDirectionWhenHurt
 * @run 70:B099
 * WHAT IT DOES: Unless the object is in state 6, halves its OBJ_3618 once its hits taken reach 5.
 * HOW: Two compares and a memory shift right.
 * WAITS: no.
 * Entry: X object. Exit: the last compare, or the shift's N, Z and C.
 */
ObjArm HalveDirectionWhenHurt(uint8_t slot)
{
    ObjArm r = { 0 };
    bool carry = false;

    r.first = RD8(OBJ->state[slot]);
    r.arm = M6_HD_STATE6;
    if (r.first == 6) {
        return r;
    }
    r.second = RD8(OBJ->hit_taken[slot]);
    r.arm = M6_HD_MILD;
    if (r.second < 5) {
        return r;
    }
    r.third = LSR8_VALUE(OBJ->f3618[slot], carry);
    r.flag = carry;
    r.arm = M6_HD_HALVED;
    return r;
}

/* The second compare replaces the first; the shift's flags follow it. */
static Ended halve_direction_when_hurt_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == M6_HD_STATE6) {
        return ended_compare(r.first, 6);
    }
    e = ended_compare(r.second, 5);
    if (r.arm == M6_HD_MILD) {
        return e;
    }
    return then(e, ended_with_carry(ended_nz(r.third), r.flag));
}

void abi_HalveDirectionWhenHurt(void)
{
    abi_ended(halve_direction_when_hurt_exit_shim(HalveDirectionWhenHurt(cpu.x)));
}

/*
 * @routine 0E0B:10B LoadThenSpawn24
 * @run 70:B10B
 * WHAT IT DOES: Loads the object's record, sets OBJ_3618 to $80, clears the object in slot $11 and starts a kind $24 object there at this object's place.
 * HOW: LoadObjectRecord, ClearObjectInY, a tail jump into StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte copied, Y $11.
 */
ObjArm LoadThenSpawn24(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB10E);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    r.flag = carry;
    WR8(OBJ->f3618[slot], 0x80);
    jsr_push(0xB118);
    ClearObjectInY(0x11);
    jsr_pop();
    r.third = StartObjectAtPlace(0x24, slot, 0x11);
    return r;
}

static Ended load_then_spawn24_exit_shim(ObjArm r)
{
    return then(record_exit_shim(r), ended_with_y(ended_load(r.third), 0x11));
}

void abi_LoadThenSpawn24(void)
{
    abi_ended(load_then_spawn24_exit_shim(LoadThenSpawn24(cpu.x)));
}

/*
 * @routine 0E0B:5E8 StepListB5FEToState4
 * @run 70:B5E8
 * WHAT IT DOES: Steps the animation list at $B5FE; when it ends, the object goes to state 4 and OBJ_2800 0.
 * HOW: StepObjectFrames, then stores and an increment.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or A 4 when the list ended.
 */
ObjArm StepListB5FEToState4(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    WR8(ZP->temp10, 0xFE);
    WR8(ZP->temp11, 0xB5);
    jsr_push(0xB5F3);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = M6_B5_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(OBJ->state[slot], 4);
    WR8(OBJ->f2800[slot], 0);
    r.arm = M6_B5_ENDED;
    return r;
}

static Ended step_list_b5fe_to_state4_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == M6_B5_RUNNING) {
        return e;
    }
    return then(e, ended_load(4));
}

void abi_StepListB5FEToState4(void)
{
    abi_ended(step_list_b5fe_to_state4_exit_shim(StepListB5FEToState4(cpu.x, cpu.y)));
}

/*
 * @routine 0E0B:184 FallThenStopForRaster
 * @run 70:B184
 * WHAT IT DOES: Moves the object along Y by its speed and counts OBJ_34B8 down; at zero it clears the raster effect and its argument, clears the object's Y speed and goes
 * on to the next OBJ_2800.
 * HOW: ApplyVelocityY, DEC, stores, INC.
 * WAITS: no.
 * Entry: X object. Exit: the count, or the new OBJ_2800's N and Z.
 */
ObjArm FallThenStopForRaster(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB187);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    r.first = DEC8(OBJ->f34b8[slot]);
    r.arm = M6_FR_FALLING;
    if (r.first != 0) {
        return r;
    }
    WR8(ZP->raster_arg, 0);
    WR8(ZP->raster_effect, 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    r.second = INC8(OBJ->f2800[slot]);
    r.arm = M6_FR_STOPPED;
    return r;
}

static Ended fall_then_stop_for_raster_exit_shim(ObjArm r)
{
    Ended e = then(ended_of_axis_move(r.move), ended_nz(r.first));

    if (r.arm == M6_FR_FALLING) {
        return e;
    }
    return then(e, ended_nz(r.second));
}

void abi_FallThenStopForRaster(void)
{
    abi_ended(fall_then_stop_for_raster_exit_shim(FallThenStopForRaster(cpu.x)));
}

/*
 * @routine 0E0C:128 LoadThenRiseFast
 * @run 70:B928
 * WHAT IT DOES: Loads the object's record, moves it up once by a Y speed high of $D8, then sets OBJ_3510 to $30 and the Y speed to $FA00.
 * HOW: LoadObjectRecord, ApplyVelocityY, stores.
 * WAITS: no.
 * Entry: X object. Exit: A 0, Z set.
 */
ObjArm LoadThenRiseFast(uint8_t slot)
{
    ObjArm r = { 0 };

    jsr_push(0xB92B);
    r.first = LoadObjectRecord(slot, &r.second, &r.flag);
    jsr_pop();
    WR8(OBJ->vy_hi[slot], 0xD8);
    jsr_push(0xB933);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    WR8(OBJ->f3510[slot], 0x30);
    WR8(OBJ->vy_hi[slot], 0xFA);
    WR8(OBJ->vy_lo[slot], 0);
    return r;
}

static Ended load_then_rise_fast_exit_shim(ObjArm r)
{
    Ended e = then(then(record_exit_shim(r), ended_load(0xD8)), ended_of_axis_move(r.move));

    return then(e, ended_load(0));
}

void abi_LoadThenRiseFast(void)
{
    abi_ended(load_then_rise_fast_exit_shim(LoadThenRiseFast(cpu.x)));
}

/*
 * @routine 0E0B:34D CountDownThenState6
 * @run 70:B34D
 * WHAT IT DOES: Counts OBJ_3618 down; at zero the object goes to state 6, clears OBJ_2800 and takes a new OBJ_3618 from a table at $B36B by the low two bits of OBJ_35C0.
 * HOW: DEC, stores, a table read (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: the count; at zero, the table byte as loaded (Y its index).
 */
ObjArm CountDownThenState6(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(OBJ->f3618[slot]);
    r.arm = M6_CD_COUNTING;
    if (r.first != 0) {
        r.second = DEC8(OBJ->f3618[slot]);
        return r;
    }
    WR8(OBJ->state[slot], 6);
    WR8(OBJ->f2800[slot], 0);
    r.third = (uint8_t)(RD8(OBJ->angle_lo[slot]) & 0x03);
    r.fourth = rd8((uint16_t)(0xB36B + r.third));
    WR8(OBJ->f3618[slot], r.fourth);
    r.arm = M6_CD_STATE6;
    return r;
}

static Ended count_down_then_state6_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == M6_CD_COUNTING) {
        return then(e, ended_nz(r.second));
    }
    return then(e, ended_with_y(ended_load(r.fourth), r.third));
}

void abi_CountDownThenState6(void)
{
    abi_ended(count_down_then_state6_exit_shim(CountDownThenState6(cpu.x)));
}

/*
 * @routine 0E0C:187 StepListB9B7ThenAimAtPlayer
 * @run 70:B987
 * WHAT IT DOES: Steps the animation list at $B9B7; when it ends the object takes the angle to the player as OBJ_34B8, sets its speed along it with length $0180, clears its animation step and timer, sets OBJ_3510 to $C0 and goes on to the next state.
 * HOW: StepObjectFrames, AngleToPlayer, SetVelocityFromAngle, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the new state's N and Z (A $C0) when the list ended.
 */
ObjArm StepListB9B7ThenAimAtPlayer(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    WR8(ZP->temp10, 0xB7);
    WR8(ZP->temp11, 0xB9);
    jsr_push(0xB992);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = M6_B9_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    jsr_push(0xB997);
    r.angle = AngleToPlayer(slot);
    jsr_pop();
    WR8(OBJ->f34b8[slot], r.angle.a);
    WR8(ZP->temp00, 0x80);
    WR8(ZP->temp01, 0x01);
    r.first = RD8(OBJ->f34b8[slot]);
    jsr_push(0xB9A8);
    r.scale = SetVelocityFromAngle(r.first, slot);
    jsr_pop();
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f3510[slot], 0xC0);
    r.second = INC8(OBJ->state[slot]);
    r.arm = M6_B9_AIMED;
    return r;
}

static Ended step_list_b9b7_then_aim_at_player_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == M6_B9_RUNNING) {
        return e;
    }
    e = then(e, ended_of_angle(r.angle));
    e = then(e, ended_load(r.first));
    e = then(e, ended_of_scale(r.scale));
    return then(then(e, ended_load(0xC0)), ended_nz(r.second));
}

void abi_StepListB9B7ThenAimAtPlayer(void)
{
    abi_ended(step_list_b9b7_then_aim_at_player_exit_shim(StepListB9B7ThenAimAtPlayer(cpu.x, cpu.y)));
}

/*
 * @routine 0E0B:237 StepListB25CThenStartList
 * @run 70:B237
 * WHAT IT DOES: Steps the animation list at $B25C; when it ends it starts the script list $82 on channel 5, sets bit 2 of zField8E, sets OBJ_3250 to 2 and goes on to the next state.
 * HOW: StepObjectFrames, StartListChannelKind2, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the new state's N and Z when the list ended.
 */
ObjArm StepListB25CThenStartList(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry, flags;

    WR8(ZP->temp10, 0x5C);
    WR8(ZP->temp11, 0xB2);
    jsr_push(0xB242);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = M6_B2_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    push8(slot);
    jsr_push(0xB24C);
    StartListChannelKind2(0x82, 5);
    jsr_pop();
    slot = pull8();
    flags = RD8(ZP->field8e);
    r.first = (uint8_t)(flags | 0x04);
    WR8(ZP->field8e, r.first);
    WR8(OBJ->hitbox_on[slot], 2);
    r.second = INC8(OBJ->state[slot]);
    r.arm = M6_B2_STARTED;
    return r;
}

static Ended step_list_b25c_then_start_list_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == M6_B2_RUNNING) {
        return e;
    }
    e = then(e, ended_load(r.first));
    return then(then(e, ended_load(2)), ended_nz(r.second));
}

void abi_StepListB25CThenStartList(void)
{
    abi_ended(step_list_b25c_then_start_list_exit_shim(StepListB25CThenStartList(cpu.x, cpu.y)));
}

/*
 * @routine 0E0C:201 TurnTowardPlayerOrStepList
 * @run 70:BA01
 * WHAT IT DOES: While OBJ_3568 counts down the object waits, turning round when it reaches zero. At zero it compares its facing with the player's side: when they differ it sets OBJ_3568 to 5 and frame $2C, otherwise it steps the animation list at $B9FA.
 * HOW: DEC, EOR, SideOfPlayer, StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the last instruction's registers.
 */
ObjArm TurnTowardPlayerOrStepList(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    r.first = RD8(OBJ->f3568[slot]);
    if (r.first != 0) {
        r.second = DEC8(OBJ->f3568[slot]);
        r.arm = M6_TP_COUNTING;
        if (r.second != 0) {
            return r;
        }
        r.third = (uint8_t)(RD8(OBJ->facing_left[slot]) ^ 0x01);
        WR8(OBJ->facing_left[slot], r.third);
        r.arm = M6_TP_TURNED;
        return r;
    }
    jsr_push(0xBA17);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.fourth = (uint8_t)(r.side.sides & 0x01);
    r.fifth = RD8(OBJ->facing_left[slot]);
    r.arm = M6_TP_OPPOSITE;
    if (r.fourth != r.fifth) {
        WR8(OBJ->f3568[slot], 5);
        WR8(OBJ->frame[slot], 0x2C);
        return r;
    }
    WR8(ZP->temp10, 0xFA);
    WR8(ZP->temp11, 0xB9);
    jsr_push(0xBA34);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = M6_TP_STEPPED;
    return r;
}

static Ended turn_toward_player_or_step_list_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.first != 0) {
        e = then(e, ended_nz(r.second));
        if (r.arm == M6_TP_COUNTING) {
            return e;
        }
        return then(e, ended_load(r.third));
    }
    e = then(then(e, ended_of_side(r.side)), ended_load(r.fourth));
    e = then(e, ended_compare(r.fourth, r.fifth));
    if (r.arm == M6_TP_OPPOSITE) {
        return then(e, ended_load(0x2C));
    }
    return then(e, frame_step_exit_shim(r));
}

void abi_TurnTowardPlayerOrStepList(void)
{
    abi_ended(turn_toward_player_or_step_list_exit_shim(TurnTowardPlayerOrStepList(cpu.x, cpu.y)));
}

/*
 * @routine 0E0B:0C2 SetHitBoxFromTable
 * @run 70:B0C2
 * WHAT IT DOES: When OBJ_3250 is not zero, takes the object's hit box (four bytes: OBJ_3930, OBJ_39E0, OBJ_3988, OBJ_3A38) from the table at $BEEC, four bytes a row, the row being the frame less one.
 * HOW: A pointer built in zTemp00-01 by two adds of twice the row, four reads through it.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte read, Y 3; or the test of OBJ_3250.
 */
ObjArm SetHitBoxFromTable(uint8_t slot)
{
    ObjArm r = { 0 };
    Sum row, sum;
    uint8_t twice, y, low;

    r.first = RD8(OBJ->hitbox_on[slot]);
    r.arm = M6_SH_OFF;
    if (r.first == 0) {
        return r;
    }
    WR8(ZP->temp00, 0xEC);
    WR8(ZP->temp01, 0xBE);
    twice = (uint8_t)((uint8_t)(RD8(OBJ->frame[slot]) - 1) << 1);
    WR8(ZP->temp02, twice);
    for (y = 0; y < 2; y++) {
        low = RD8(ZP->temp00);
        sum = add_bytes(low, RD8(ZP->temp02), false);
        WR8(ZP->temp00, sum.value);
        low = RD8(ZP->temp01);
        sum = add_bytes(low, 0, sum.carry);
        WR8(ZP->temp01, sum.value);
    }
    r.sum = sum;
    r.second = rd8((uint16_t)(RD16_LH2(ZP->temp00, ZP->temp01) + 0));
    WR8(OBJ->hitbox_dx[slot], r.second);
    r.second = rd8((uint16_t)(RD16_LH2(ZP->temp00, ZP->temp01) + 1));
    WR8(OBJ->hitbox_dy[slot], r.second);
    r.second = rd8((uint16_t)(RD16_LH2(ZP->temp00, ZP->temp01) + 2));
    WR8(OBJ->hitbox_half_w[slot], r.second);
    r.second = rd8((uint16_t)(RD16_LH2(ZP->temp00, ZP->temp01) + 3));
    WR8(OBJ->hitbox_half_h[slot], r.second);
    r.arm = M6_SH_SET;
    return r;
}

static Ended set_hit_box_from_table_exit_shim(ObjArm r)
{
    if (r.arm == M6_SH_OFF) {
        return ended_load(r.first);
    }
    return then(ended_of_sum(r.sum), ended_with_y(ended_load(r.second), 3));
}

void abi_SetHitBoxFromTable(void)
{
    abi_ended(set_hit_box_from_table_exit_shim(SetHitBoxFromTable(cpu.x)));
}

/*
 * @routine 0E0C:412 CopyPlaceAndHitBoxOfSlot10
 * @run 70:BC12
 * WHAT IT DOES: In state 0 loads the object's record. Later it takes the place of the object in slot $10 and its facing; if that object's OBJ_3250 is not zero it takes the hit box (four bytes a row from the table at $C024, the row being that object's frame less one; the half width plus 8), else it clears its half width and half height.
 * HOW: LoadObjectRecord, CopyObjectPlaceBack, a pointer built in zTemp00-01, four reads through it.
 * WAITS: no.
 * Entry: X object. Exit: see the code: the last read or the clears.
 */
ObjArm CopyPlaceAndHitBoxOfSlot10(uint8_t slot)
{
    ObjArm r = { 0 };
    Sum row, sum;
    uint8_t twice, y, value, low;

    r.sixth = RD8(OBJ->state[slot]);
    if (r.sixth == 0) {
        r.arm = M6_CP_RECORD;
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        return r;
    }
    jsr_push(0xBC1F);
    r.third = CopyObjectPlaceBack(0x10, slot);
    jsr_pop();
    r.fourth = RD8(OBJ->facing_left[0x10]);
    WR8(OBJ->facing_left[slot], r.fourth);
    r.fifth = RD8(OBJ->hitbox_on[0x10]);
    r.arm = M6_CP_OFF;
    if (r.fifth == 0) {
        WR8(OBJ->hitbox_half_w[slot], 0);
        WR8(OBJ->hitbox_half_h[slot], 0);
        return r;
    }
    WR8(ZP->temp00, 0x24);
    WR8(ZP->temp01, 0xC0);
    twice = (uint8_t)((uint8_t)(RD8(OBJ->frame[0x10]) - 1) << 1);
    WR8(ZP->temp02, twice);
    for (y = 0; y < 2; y++) {
        low = RD8(ZP->temp00);
        sum = add_bytes(low, RD8(ZP->temp02), false);
        WR8(ZP->temp00, sum.value);
        low = RD8(ZP->temp01);
        sum = add_bytes(low, 0, sum.carry);
        WR8(ZP->temp01, sum.value);
    }
    value = rd8((uint16_t)(RD16_LH2(ZP->temp00, ZP->temp01) + 0));
    WR8(OBJ->hitbox_dx[slot], value);
    value = rd8((uint16_t)(RD16_LH2(ZP->temp00, ZP->temp01) + 1));
    WR8(OBJ->hitbox_dy[slot], value);
    value = rd8((uint16_t)(RD16_LH2(ZP->temp00, ZP->temp01) + 2));
    r.sum = add_bytes(value, 0x08, false);
    WR8(OBJ->hitbox_half_w[slot], r.sum.value);
    r.seventh = rd8((uint16_t)(RD16_LH2(ZP->temp00, ZP->temp01) + 3));
    WR8(OBJ->hitbox_half_h[slot], r.seventh);
    r.arm = M6_CP_SET;
    return r;
}

static Ended copy_place_and_hit_box_of_slot10_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == M6_CP_RECORD) {
        return then(ended_load(r.sixth), record_exit_shim(r));
    }
    e = then(then(ended_load(r.sixth), ended_with_y(ended_load(r.third), 0x10)), ended_nz(r.third));
    e = then(e, ended_load(r.fourth));
    e = then(e, ended_load(r.fifth));
    if (r.arm == M6_CP_OFF) {
        return e;
    }
    return then(then(e, ended_of_sum(r.sum)), ended_with_y(ended_load(r.seventh), 3));
}

void abi_CopyPlaceAndHitBoxOfSlot10(void)
{
    abi_ended(copy_place_and_hit_box_of_slot10_exit_shim(CopyPlaceAndHitBoxOfSlot10(cpu.x)));
}

/*
 * @routine 0E0B:19A StepListB1BAThenSound
 * @run 70:B19A
 * WHAT IT DOES: Steps the animation list at $B1BA; when it ends it sends sound $46, sets sprite set 4 and frame 1 and goes on to the next OBJ_2800.
 * HOW: StepObjectFrames, SoundCommand, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
 */
ObjArm StepListB1BAThenSound(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    WR8(ZP->temp10, 0xBA);
    WR8(ZP->temp11, 0xB1);
    jsr_push(0xB1A5);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = M6_B1_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    r.sounded = true;
    r.command = 0x46;
    jsr_push(0xB1AC);
    r.call = SoundCommand(0x46, slot, y);
    jsr_pop();
    WR8(OBJ->sprite_set[slot], 4);
    WR8(OBJ->frame[slot], 1);
    r.first = INC8(OBJ->f2800[slot]);
    r.arm = M6_B1_SOUNDED;
    return r;
}

static SoundEnd step_list_b1ba_then_sound_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = frame_step_exit_shim(r);

    if (r.arm == M6_B1_RUNNING) {
        s.end = e;
        return s;
    }
    s.before = then(e, ended_load(0x46));
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    s.end = then(ended_load(1), ended_nz(r.first));
    return s;
}

void abi_StepListB1BAThenSound(void)
{
    abi_sound_end(step_list_b1ba_then_sound_exit_shim(StepListB1BAThenSound(cpu.x, cpu.y)));
}

/*
 * @routine 0E0B:51C StepListB53CToState5
 * @run 70:B51C
 * WHAT IT DOES: Steps the animation list at $B53C; when it ends it clears the raster effect and its argument and the animation step and timer, and the object goes to state 5 with OBJ_2800 0.
 * HOW: StepObjectFrames, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or A 5 when the list ended.
 */
ObjArm StepListB53CToState5(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    WR8(ZP->temp10, 0x3C);
    WR8(ZP->temp11, 0xB5);
    jsr_push(0xB527);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = M6_B5C_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(ZP->raster_arg, 0);
    WR8(ZP->raster_effect, 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 5);
    WR8(OBJ->f2800[slot], 0);
    r.arm = M6_B5C_ENDED;
    return r;
}

static Ended step_list_b53c_to_state5_exit_shim(ObjArm r)
{
    Ended e = frame_step_exit_shim(r);

    if (r.arm == M6_B5C_RUNNING) {
        return e;
    }
    return then(e, ended_load(5));
}

void abi_StepListB53CToState5(void)
{
    abi_ended(step_list_b53c_to_state5_exit_shim(StepListB53CToState5(cpu.x, cpu.y)));
}

/* TurnAngleToward's exit (turn_angle_exit_shim): equal gives the angle with Z set and C set, else the last add. */
static Ended ended_of_turn(TurnAngleResult r)
{
    if (r.equal) {
        return ended_with_carry(ended_with_a(ended_nz(0), r.angle), true);
    }
    return ended_of_sum(r.last);
}

/*
 * @routine 0E0C:1C2 TurnTowardPlayerAndMove
 * @run 70:B9C2
 * WHAT IT DOES: Runs TurnTowardPlayerOrStepList, sets the object's speed from its angle OBJ_34B8 with length $01C0 and moves it, then turns the angle toward the angle to the player (step 2) with the low bit cleared; counts OBJ_3510 down and at zero sets it to $F0 and goes on to the next state.
 * HOW: TurnTowardPlayerOrStepList, SetVelocityFromAngle, ApplyVelocity, AngleToPlayer, TurnAngleToward, DEC, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: the count, or the new state's N and Z (A $F0).
 */
ObjArm TurnTowardPlayerAndMove(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    TurnAngleResult turn;
    uint8_t angle_byte;

    jsr_push(0xB9C5);
    r.sub = obj_sub(TurnTowardPlayerOrStepList(slot, y_entry));
    jsr_pop();
    WR8(ZP->temp00, 0xC0);
    WR8(ZP->temp01, 0x01);
    r.first = RD8(OBJ->f34b8[slot]);
    jsr_push(0xB9D3);
    r.scale = SetVelocityFromAngle(r.first, slot);
    jsr_pop();
    jsr_push(0xB9D6);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    jsr_push(0xB9D9);
    r.angle = AngleToPlayer(slot);
    jsr_pop();
    WR8(ZP->temp02, r.angle.a);
    WR8(ZP->temp00, 0x02);
    r.second = RD8(OBJ->f34b8[slot]);
    WR8(ZP->temp01, r.second);
    jsr_push(0xB9E7);
    r.turn = TurnAngleToward();
    jsr_pop();
    r.flag = turn.equal;
    r.third = turn.angle;
    r.sum = turn.last;
    angle_byte = (uint8_t)(turn.angle & 0xFE);
    if (!turn.equal) {
        angle_byte = (uint8_t)(turn.last.value & 0xFE);
    }
    r.fourth = angle_byte;
    WR8(OBJ->f34b8[slot], angle_byte);
    r.fifth = DEC8(OBJ->f3510[slot]);
    r.arm = M6_TM_COUNTING;
    if (r.fifth != 0) {
        return r;
    }
    WR8(OBJ->f3510[slot], 0xF0);
    r.sixth = INC8(OBJ->state[slot]);
    r.arm = M6_TM_NEXT;
    return r;
}

static Ended turn_toward_player_and_move_exit_shim(ObjArm r)
{
    TurnAngleResult turn = { 0 };
    Ended e = turn_toward_player_or_step_list_exit_shim(obj_arm(r.sub));

    turn.equal = r.flag;
    turn.angle = r.third;
    turn.last = r.sum;
    e = then(then(e, ended_load(0x01)), ended_load(r.first));
    e = then(e, ended_of_scale(r.scale));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_of_angle(r.angle));
    e = then(then(e, ended_load(0x02)), ended_load(r.second));
    e = then(e, ended_of_turn(turn));
    e = then(e, ended_load(r.fourth));
    e = then(e, ended_nz(r.fifth));
    if (r.arm == M6_TM_COUNTING) {
        return e;
    }
    return then(then(e, ended_load(0xF0)), ended_nz(r.sixth));
}

void abi_TurnTowardPlayerAndMove(void)
{
    abi_ended(turn_toward_player_and_move_exit_shim(TurnTowardPlayerAndMove(cpu.x, cpu.y)));
}

/*
 * @routine 051F:454 WaitForPressThenAnimate
 * @run 71:D454
 * WHAT IT DOES: In state 0 loads the object's record and sets sprite set 6. Later, with OBJ_2800 0, it waits until the first object is at Y low $AA and X low $CC to $D3 and the pad's new presses have bit 4; then it moves to the next OBJ_2800, clears its animation step and timer and takes the first object's X low and Y low plus $D8 as its place. With OBJ_2800 not 0 it sets the first object's OBJ_3510 to $C0 and OBJ_3568 to $10 and steps the animation list at $D443; sound $3B when a frame changes; when the list ends it clears OBJ_2800, its frame and the first object's OBJ_3510.
 * HOW: LoadObjectRecord, StepObjectFrames, SoundCommand, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
 */
ObjArm WaitForPressThenAnimate(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    r.tenth = RD8(OBJ->state[slot]);
    if (r.tenth == 0) {
        r.arm = M6_WP_RECORD;
        jsr_push(0xD45C);
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        jsr_pop();
        r.flag = carry;
        WR8(OBJ->sprite_set[slot], 6);
        return r;
    }
    r.third = RD8(OBJ->f2800[slot]);
    if (r.third == 0) {
        r.fourth = RD8(OBJ->y_lo[0]);
        r.arm = M6_WP_Y;
        if (r.fourth != 0xAA) {
            return r;
        }
        r.fifth = RD8(OBJ->x_lo[0]);
        r.arm = M6_WP_XHI;
        if (r.fifth >= 0xD4) {
            return r;
        }
        r.sixth = RD8(OBJ->x_lo[0]);
        r.arm = M6_WP_XLO;
        if (r.sixth < 0xCC) {
            return r;
        }
        r.seventh = (uint8_t)(RD8(ZP->pad_pressed) & 0x10);
        r.arm = M6_WP_NOPRESS;
        if (r.seventh == 0) {
            return r;
        }
        INC8(OBJ->f2800[slot]);
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        r.eighth = RD8(OBJ->x_lo[0]);
        WR8(OBJ->x_lo[slot], r.eighth);
        r.sum = add_bytes(RD8(OBJ->y_lo[0]), 0xD8, false);
        WR8(OBJ->y_lo[slot], r.sum.value);
        r.arm = M6_WP_START;
        return r;
    }
    WR8(OBJ->f3510[0], 0xC0);
    WR8(OBJ->f3568[0], 0x10);
    WR8(ZP->temp10, 0x43);
    WR8(ZP->temp11, 0xD4);
    jsr_push(0xD4B0);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = M6_WP_HELD;
    if (r.step == FRAME_LIST_ENDED) {
        WR8(OBJ->f2800[slot], 0);
        WR8(OBJ->frame[slot], 0);
        WR8(OBJ->f3510[0], 0);
        r.arm = M6_WP_ENDED;
        return r;
    }
    if (r.step != FRAME_CHANGED) {
        return r;
    }
    r.sounded = true;
    r.command = 0x3B;
    jsr_push(0xD4C3);
    r.call = SoundCommand(0x3B, slot, y);
    jsr_pop();
    r.arm = M6_WP_SOUND;
    return r;
}

static SoundEnd wait_for_press_then_animate_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = ended_load(r.tenth);

    if (r.arm == M6_WP_RECORD) {
        s.end = then(then(e, record_exit_shim(r)), ended_load(6));
        return s;
    }
    e = then(e, ended_load(r.third));
    if (r.third == 0) {
        e = then(e, ended_compare(r.fourth, 0xAA));
        if (r.arm == M6_WP_Y) {
            s.end = e;
            return s;
        }
        e = then(e, ended_compare(r.fifth, 0xD4));
        if (r.arm == M6_WP_XHI) {
            s.end = e;
            return s;
        }
        e = then(e, ended_compare(r.sixth, 0xCC));
        if (r.arm == M6_WP_XLO) {
            s.end = e;
            return s;
        }
        e = then(e, ended_load(r.seventh));
        if (r.arm == M6_WP_NOPRESS) {
            s.end = e;
            return s;
        }
        s.end = then(then(e, ended_load(r.eighth)), ended_of_sum(r.sum));
        return s;
    }
    e = then(then(then(e, ended_load(0x10)), ended_load(0xD4)), frame_step_exit_shim(r));
    if (r.arm == M6_WP_ENDED) {
        s.end = then(e, ended_load(0));
        return s;
    }
    s.end = e;
    if (r.arm == M6_WP_HELD) {
        return s;
    }
    s.before = then(e, ended_load(0x3B));
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    s.end = ended_kept();
    return s;
}

void abi_WaitForPressThenAnimate(void)
{
    abi_sound_end(wait_for_press_then_animate_exit_shim(WaitForPressThenAnimate(cpu.x, cpu.y)));
}

/*
 * @routine 0E0B:0AB StateSevenOnStatusBit7
 * @run 70:B0AB
 * WHAT IT DOES: An object in a state under 7 whose OBJ_31A0 has bit 7 set goes to state 7 and clears OBJ_2800. (The part after the test ran in no script; it is written from the bytes.)
 * HOW: A test, a compare, two stores.
 * WAITS: no.
 * Entry: X object. Exit: the test, the compare, or A 7.
 */
ObjArm StateSevenOnStatusBit7(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = (uint8_t)(RD8(OBJ->status_flags[slot]) & 0x80);
    r.arm = M6_S7_NONE;
    if (r.first == 0) {
        return r;
    }
    r.second = RD8(OBJ->state[slot]);
    r.arm = M6_S7_LATE;
    if (r.second >= 7) {
        return r;
    }
    WR8(OBJ->state[slot], 7);
    WR8(OBJ->f2800[slot], 0);
    r.arm = M6_S7_ENTERED;
    return r;
}

static Ended state_seven_on_status_bit7_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.first);

    if (r.arm == M6_S7_NONE) {
        return e;
    }
    e = then(then(e, ended_load(r.second)), ended_compare(r.second, 7));
    if (r.arm == M6_S7_LATE) {
        return e;
    }
    return then(e, ended_load(7));
}

void abi_StateSevenOnStatusBit7(void)
{
    abi_ended(state_seven_on_status_bit7_exit_shim(StateSevenOnStatusBit7(cpu.x)));
}

/*
 * @routine 0E0B:579 SpawnOnTimerInTable
 * @run 70:B579
 * WHAT IT DOES: When OBJ_3408 is 6 and OBJ_3460 equals one of the three bytes at $B5A5 it finds a free slot from $30, sends sound $48 and starts a kind $27 object there at its place with its facing.
 * HOW: A search loop, FindFreeSlotFrom30, SoundCommand, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept for the sound's push only when the search ran. Exit: see the wrapper.
 */
ObjArm SpawnOnTimerInTable(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t y, found_slot;
    bool found;

    r.first = RD8(OBJ->anim_step[slot]);
    r.arm = M6_ST_NOT6;
    if (r.first != 6) {
        return r;
    }
    for (y = 0;; y++) {
        r.bytes[2 * y] = RD8(OBJ->anim_timer[slot]);
        r.bytes[2 * y + 1] = rd8((uint16_t)(0xB5A5 + y));
        r.second = (uint8_t)(y + 1);
        if (r.bytes[2 * y] == r.bytes[2 * y + 1]) {
            break;
        }
        if ((y + 1) >= 3) {
            r.arm = M6_ST_NO_MATCH;
            return r;
        }
    }
    r.third = y;
    jsr_push(0xB592);
    found_slot = FindFreeSlotFrom30(&r.fifth, &found);
    jsr_pop();
    r.fourth = found_slot;
    r.flag = found;
    r.arm = M6_ST_NO_SLOT;
    if (!found) {
        return r;
    }
    r.sounded = true;
    r.command = 0x48;
    jsr_push(0xB599);
    r.call = SoundCommand(0x48, slot, found_slot);
    jsr_pop();
    jsr_push(0xB59E);
    r.sixth = StartObjectAtPlace(0x27, slot, found_slot);
    jsr_pop();
    r.seventh = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[found_slot], r.seventh);
    r.arm = M6_ST_SPAWNED;
    return r;
}

static SoundEnd spawn_on_timer_in_table_exit_shim(ObjArm r)
{
    SoundEnd s = { { 0 }, { 0 }, false, 0, { 0 } };
    Ended e = ended_compare(r.first, 6);
    uint8_t i;

    s.end = e;
    if (r.arm == M6_ST_NOT6) {
        return s;
    }
    for (i = 0; i < r.second; i++) {
        e = then(then(e, ended_load(r.bytes[2 * i])), ended_compare(r.bytes[2 * i], r.bytes[2 * i + 1]));
        if (r.bytes[2 * i] == r.bytes[2 * i + 1]) {
            break;
        }
        e = then(e, ended_with_y(ended_with_carry(ended_nz((uint8_t)(i + 1 - 3)), (i + 1) >= 3), (uint8_t)(i + 1)));
    }
    if (r.arm == M6_ST_NO_MATCH) {
        s.end = e;
        return s;
    }
    e = then(e, ended_with_y(ended_nz(r.third), r.third));
    e = then(e, ended_of_find_slot(r.fourth, r.fifth, r.flag));
    if (r.arm == M6_ST_NO_SLOT) {
        s.end = e;
        return s;
    }
    s.before = then(e, ended_load(0x48));
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    s.end = then(then(ended_load(0x27), ended_with_y(ended_load(r.sixth), r.fourth)), ended_load(r.seventh));
    return s;
}

void abi_SpawnOnTimerInTable(void)
{
    abi_sound_end(spawn_on_timer_in_table_exit_shim(SpawnOnTimerInTable(cpu.x)));
}

/*
 * @routine 0E0B:543 SetRasterSpawnThenStepList
 * @run 70:B543
 * WHAT IT DOES: Sets the raster effect to $0B with argument $BF, runs SpawnOnTimerInTable, then steps the animation list at $B568; when it ends it clears the raster effect and its argument, goes to state 4 and clears OBJ_2800.
 * HOW: SpawnOnTimerInTable, StepObjectFrames, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
 */
/* The Y SoundCommand's caller leaves the step entered with: the spawn's, or the entry's when it set none. */
static uint8_t set_raster_spawn_then_step_list_y_shim(ObjArm r, uint8_t y_entry)
{
    SoundEnd inner = spawn_on_timer_in_table_exit_shim(obj_arm(r.sub));

    return ended_y_or(inner.end, ended_y_or(inner.before, y_entry));
}

ObjArm SetRasterSpawnThenStepList(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y;

    WR8(ZP->raster_effect, 0x0B);
    WR8(ZP->raster_arg, 0xBF);
    jsr_push(0xB54E);
    r.sub = obj_sub(SpawnOnTimerInTable(slot));
    jsr_pop();
    y = set_raster_spawn_then_step_list_y_shim(r, y_entry);
    WR8(ZP->temp10, 0x68);
    WR8(ZP->temp11, 0xB5);
    jsr_push(0xB559);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = M6_SR_RUNNING;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(ZP->raster_arg, 0);
    WR8(ZP->raster_effect, 0);
    WR8(OBJ->state[slot], 4);
    WR8(OBJ->f2800[slot], 0);
    r.arm = M6_SR_ENDED;
    return r;
}

static SoundEnd set_raster_spawn_then_step_list_exit_shim(ObjArm r)
{
    SoundEnd s = spawn_on_timer_in_table_exit_shim(obj_arm(r.sub));
    Ended e = then(then(then(s.end, ended_load(0x68)), ended_load(0xB5)), frame_step_exit_shim(r));

    if (r.arm == M6_SR_RUNNING) {
        s.end = e;
        return s;
    }
    s.end = then(e, ended_load(4));
    return s;
}

void abi_SetRasterSpawnThenStepList(void)
{
    abi_sound_end(set_raster_spawn_then_step_list_exit_shim(SetRasterSpawnThenStepList(cpu.x, cpu.y)));
}

/*
 * @routine 0E0B:27B LandThenNextState
 * @run 70:B27B
 * WHAT IT DOES: Asks StopOnLanding with a probe height of 4; on landing the object goes to the next state.
 * HOW: StopOnLanding, an INC.
 * WAITS: no.
 * Entry: X object, Y the probe's column, C passed on. Exit: the mover's registers, then the new state's N and Z when landed.
 */
MoverArm LandThenNextState(uint8_t slot, uint8_t column, bool carry_in)
{
    MoverArm w = { true, { 0 }, { 0 } };

    jsr_push(0xB280);
    w.mover = StopOnLanding(4, slot, column, carry_in);
    jsr_pop();
    w.arm.arm = M6_LN_FLYING;
    if (!mover_landed(w.mover)) {
        return w;
    }
    w.arm.first = INC8(OBJ->state[slot]);
    w.arm.arm = M6_LN_LANDED;
    return w;
}

static LandWait land_then_next_state_exit_shim(MoverArm w)
{
    LandWait l = { w.asked, w.mover, { 0 } };

    if (w.arm.arm == M6_LN_LANDED) {
        l.end = ended_nz(w.arm.first);
    }
    return l;
}

void abi_LandThenNextState(void)
{
    land_wait_exit_shim(land_then_next_state_exit_shim(LandThenNextState(cpu.x, cpu.y, (cpu.p & P_C) != 0)));
}

/*
 * @routine 0E0C:045 LoadThenSwingAndDrop
 * @run 70:B845
 * @stop B8C0
 * WHAT IT DOES: In state 0 loads the object's record, puts it at Y low $F0, gives it the X speed high the table at $B86D names by its facing, moves it along X, takes the other X speed high from the table at $B86F and sets OBJ_34B8 to $80. Later it sets its Y speed from the cosine of OBJ_34B8 with length $0C00, moves along Y, adds 8 to OBJ_34B8 and, when that carries, restarts it at $80, puts the object at Y low $F0, moves along X, counts OBJ_3510 up and clears the object at 5 or sets its Y position from the table at $B8C0.
 * HOW: LoadObjectRecord, ApplyVelocityX, ScaleByCosine, ApplyVelocityY, table reads (bank 70), ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: the last instruction's registers.
 */
ObjArm LoadThenSwingAndDrop(uint8_t slot)
{
    ObjArm r = { 0 };

    r.tenth = RD8(OBJ->state[slot]);
    if (r.tenth == 0) {
        r.arm = M6_SD_START;
        jsr_push(0xB84D);
        r.first = LoadObjectRecord(slot, &r.second, &r.flag);
        jsr_pop();
        WR8(OBJ->y_lo[slot], 0xF0);
        r.third = RD8(OBJ->facing_left[slot]);
        r.fourth = rd8((uint16_t)(0xB86D + r.third));
        WR8(OBJ->vx_hi[slot], r.fourth);
        jsr_push(0xB85E);
        r.move = ApplyVelocityX(slot);
        jsr_pop();
        r.fifth = RD8(OBJ->facing_left[slot]);
        r.sixth = rd8((uint16_t)(0xB86F + r.fifth));
        WR8(OBJ->vx_hi[slot], r.sixth);
        WR8(OBJ->f34b8[slot], 0x80);
        return r;
    }
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 0x0C);
    r.first = RD8(OBJ->f34b8[slot]);
    jsr_push(0xB87F);
    r.scale = ScaleByCosine(r.first);
    jsr_pop();
    r.second = rd8(zProduct2);
    WR8(OBJ->vy_hi[slot], r.second);
    r.third = rd8(zProduct1);
    WR8(OBJ->vy_lo[slot], r.third);
    jsr_push(0xB88C);
    r.move = ApplyVelocityY(slot);
    jsr_pop();
    r.fourth = RD8(OBJ->f34b8[slot]);
    r.sum = add_bytes(r.fourth, 0x08, false);
    WR8(OBJ->f34b8[slot], r.sum.value);
    r.arm = M6_SD_SWING;
    if (!r.sum.carry) {
        return r;
    }
    WR8(OBJ->f34b8[slot], 0x80);
    WR8(OBJ->y_lo[slot], 0xF0);
    jsr_push(0xB8A4);
    r.move2 = ApplyVelocityX(slot);
    jsr_pop();
    INC8(OBJ->f3510[slot]);
    r.fifth = RD8(OBJ->f3510[slot]);
    r.arm = M6_SD_CLEARED;
    if (r.fifth >= 5) {
        ClearObject(slot);
        return r;
    }
    r.sixth = (uint8_t)(r.fifth << 1);
    r.seventh = rd8((uint16_t)(0xB8C0 + r.sixth));
    WR8(OBJ->y_lo[slot], r.seventh);
    r.eighth = rd8((uint16_t)(0xB8C1 + r.sixth));
    WR8(OBJ->y_hi[slot], r.eighth);
    r.arm = M6_SD_DROPPED;
    return r;
}

static Ended load_then_swing_and_drop_exit_shim(ObjArm r)
{
    Ended e = ended_load(r.tenth);

    if (r.arm == M6_SD_START) {
        e = then(e, record_exit_shim(r));
        e = then(then(then(e, ended_load(0xF0)), ended_with_y(ended_nz(r.third), r.third)), ended_load(r.fourth));
        e = then(e, ended_of_axis_move(r.move));
        return then(then(then(e, ended_with_y(ended_nz(r.fifth), r.fifth)), ended_load(r.sixth)), ended_load(0x80));
    }
    e = then(e, ended_load(0));
    e = then(then(e, ended_load(0x0C)), ended_load(r.first));
    e = then(e, ended_of_scale(r.scale));
    e = then(e, ended_load(r.second));
    e = then(e, ended_load(r.third));
    e = then(e, ended_of_axis_move(r.move));
    e = then(then(e, ended_load(r.fourth)), ended_of_sum(r.sum));
    if (r.arm == M6_SD_SWING) {
        return e;
    }
    e = then(then(e, ended_load(0x80)), ended_load(0xF0));
    e = then(e, ended_of_axis_move(r.move2));
    e = then(then(e, ended_load(r.fifth)), ended_compare(r.fifth, 5));
    if (r.arm == M6_SD_CLEARED) {
        return e;
    }
    e = then(e, ended_with_carry(ended_with_y(ended_load(r.sixth), r.sixth), false));
    e = then(e, ended_load(r.seventh));
    return then(e, ended_load(r.eighth));
}

void abi_LoadThenSwingAndDrop(void)
{
    abi_ended(load_then_swing_and_drop_exit_shim(LoadThenSwingAndDrop(cpu.x)));
}
