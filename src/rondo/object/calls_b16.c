/*
 * More calling routines of stages $09 to $10 and the secret stages.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"
#include "richter.h"
#include "calls_b.h"
#include "calls_b3.h"
#include "calls_b16.h"

static void lda(uint8_t value) { cpu.a = value; abi_nz(value); }

static void cmp(uint8_t reg, uint8_t value)
{
    abi_flag(P_C, reg >= value);
    abi_nz((uint8_t)(reg - value));
}

/* LDA #command and JSR SoundCommand's vector. */
static void sound(uint8_t command, uint16_t ret)
{
    CommandCall call;

    lda(command);
    jsr_push(ret);
    call = SoundCommand(command, cpu.x, cpu.y);
    sound_command_exit_shim(command, call);
    jsr_pop();
}

/* The frame list's address into zTemp11 then zTemp10 (or the other way round), and JSR StepObjectFrames. */
static void step_list(uint8_t slot, uint16_t list, bool low_first, uint16_t ret)
{
    uint8_t y = cpu.y;
    FrameStep step;

    if (low_first) {
        lda((uint8_t)list);
        WR8(ZP->temp10, cpu.a);
        lda((uint8_t)(list >> 8));
        WR8(ZP->temp11, cpu.a);
    } else {
        lda((uint8_t)(list >> 8));
        WR8(ZP->temp11, cpu.a);
        lda((uint8_t)list);
        WR8(ZP->temp10, cpu.a);
    }
    jsr_push(ret);
    step = StepObjectFrames(slot, &y);
    step_frames_exit_shim(step, y);
    jsr_pop();
}

static void restart_animation(uint8_t slot)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
}

static void gravity(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    gravity_exit_shim(ApplyGravity(slot));
    jsr_pop();
}

/* BNE after StepObjectFrames: true when the list ended. */
static bool list_ended(void)
{
    return (cpu.p & P_Z) != 0;
}
static void move(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
}
/* LDA #list, LDX #slot, JSR StartListChannelKind2. */
static void list_channel2(uint8_t number, uint8_t at_slot, uint16_t ret)
{
    lda(number);
    cpu.x = at_slot;
    abi_nz(at_slot);
    jsr_push(ret);
    list_channel_exit_shim(StartListChannelKind2(cpu.a, cpu.x));
    jsr_pop();
}

static void add_a(uint8_t value, bool carry_in)
{
    Sum sum = add_bytes(cpu.a, value, carry_in);

    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
}

static void random_a(uint16_t ret)
{
    jsr_push(ret);
    random_exit_shim(NextRandom());
    jsr_pop();
}

static void face_by_side(uint8_t slot, uint16_t column, uint16_t ret)
{
    jsr_push(ret);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x01));
    WR8_IDX(column, slot, cpu.a);
}

static void script_at(uint8_t number, uint8_t at_slot, uint16_t ret)
{
    lda(number);
    cpu.x = at_slot;
    abi_nz(at_slot);
    jsr_push(ret);
    script_slot_exit_shim(StartScriptSlotKind1(cpu.a, cpu.x));
    jsr_pop();
}

static void pull_x(void)
{
    cpu.x = pull8();
    abi_nz(cpu.x);
}

static void ldy(uint8_t value)
{
    cpu.y = value;
    abi_nz(value);
}

/* LDA, ASL, TAY: a word index from an object column. */
static void word_index(uint8_t value)
{
    abi_flag(P_C, (value & 0x80) != 0);
    lda((uint8_t)(value << 1));
    cpu.y = cpu.a;
}

static void velocity_from_a(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
}

static void near_x(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    abi_ended(ended_of_distance(DistanceXToPlayer(slot)));
    jsr_pop();
}
/* ROL of A through the carry. */
static void rol_a(void)
{
    bool out = (cpu.a & 0x80) != 0;

    lda((uint8_t)((cpu.a << 1) | ((cpu.p & P_C) != 0 ? 1 : 0)));
    abi_flag(P_C, out);
}

/* A tail JMP to LoadObjectRecord's vector. */
static void load_record_tail_(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;

    last = LoadObjectRecord(slot, &nz, &carry);
    load_record_exit_shim(last, nz, carry);
}

/*
 * @routine 0E0B:781 CountThenStartRow0E0B
 * @run 70:B781
 * WHAT IT DOES: When OBJ_WORK_D runs out: runs $B795, frame 0, a new count of $40, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CountThenStartRow0E0B(uint8_t slot)
{
    abi_nz(DEC8(OBJ->work_d[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    call_routine(0xB789, 0xB795);
    WR8(OBJ->frame[slot], 0);
    lda(0x40);
    WR8(OBJ->work_d[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_CountThenStartRow0E0B(void)
{
    CountThenStartRow0E0B(cpu.x);
}

/*
 * @routine 12DE:2DF LandThenHittable12DE
 * @run 71:CADF
 * WHAT IT DOES: Every fourth count of OBJ_WORK_D runs $CAF4; carry from it makes the object hittable, state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LandThenHittable12DE(uint8_t slot)
{
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a != 0) {
        return;
    }
    call_routine(0xCAE9, 0xCAF4);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_LandThenHittable12DE(void)
{
    LandThenHittable12DE(cpu.x);
}

/*
 * @routine 12DC:3DC LandThenHittable12DC
 * @run 70:BBDC
 * WHAT IT DOES: Runs $BCA8; every fourth count of OBJ_WORK_H runs $BBF4, and carry from it makes the object hittable.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LandThenHittable12DC(uint8_t slot)
{
    call_routine(0xBBDF, 0xBCA8);
    lda(RD8(OBJ->work_h[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a != 0) {
        return;
    }
    call_routine(0xBBE9, 0xBBF4);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_LandThenHittable12DC(void)
{
    LandThenHittable12DC(cpu.x);
}

/*
 * @routine 0EC0:41D PointCallThenCount0EC0
 * @run 71:DC1D
 * WHAT IT DOES: Calls $AA35 with the point $30,$16 and $DC93 with A $BC; after OBJ_WORK_E counts the state goes on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void PointCallThenCount0EC0(uint8_t slot)
{
    lda(0x30);
    WR8(ZP->temp08, cpu.a);
    lda(0x16);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xDC28, 0xAA35);
    lda(0xBC);
    call_routine(0xDC2D, 0xDC93);
    abi_nz(DEC8(OBJ->work_e[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_PointCallThenCount0EC0(void)
{
    PointCallThenCount0EC0(cpu.x);
}

/*
 * @routine 12DC:790 FallListThenWait12DC
 * @run 70:BF90
 * WHAT IT DOES: Runs $BBF4 and the list at $BFA9; at its end a wait of $40, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallListThenWait12DC(uint8_t slot)
{
    call_routine(0xBF93, 0xBBF4);
    step_list(slot, 0xBFA9, true, 0xBF9E);
    if (!list_ended()) {
        return;
    }
    lda(0x40);
    WR8(OBJ->work_l[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_FallListThenWait12DC(void)
{
    FallListThenWait12DC(cpu.x);
}

/*
 * @routine 12DE:600 FallListThenWait12DE
 * @run 71:CE00
 * WHAT IT DOES: Runs $CAF4 and the list at $CE19; at its end a wait of $80, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallListThenWait12DE(uint8_t slot)
{
    call_routine(0xCE03, 0xCAF4);
    step_list(slot, 0xCE19, true, 0xCE0E);
    if (!list_ended()) {
        return;
    }
    lda(0x80);
    WR8(OBJ->work_g[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_FallListThenWait12DE(void)
{
    FallListThenWait12DE(cpu.x);
}

/*
 * @routine 117E:25C PatrolList0_117E
 * @run 71:CA5C
 * WHAT IT DOES: Runs $CA7C with A 0 and $CAEF facing as OBJ_WORK_H says; every $B0 counts that way turns round.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void PatrolList0_117E(uint8_t slot)
{
    cpu.a = 0;
    call_routine(0xCA60, 0xCA7C);
    call_routine(0xCA63, 0xCAEF);
    lda(RD8(OBJ->work_h[slot]));
    WR8(OBJ->facing_left[slot], cpu.a);
    abi_nz(DEC8(OBJ->work_e[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(0xB0);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(RD8(OBJ->work_h[slot]));
    lda((uint8_t)(cpu.a ^ 0x01));
    WR8(OBJ->work_h[slot], cpu.a);
}

void abi_PatrolList0_117E(void)
{
    PatrolList0_117E(cpu.x);
}

/*
 * @routine 117E:31F PatrolList3_117E
 * @run 71:CB1F
 * WHAT IT DOES: Runs $CA7C with A 3 and $CAEF facing as OBJ_WORK_H says; every $B0 counts that way turns round.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void PatrolList3_117E(uint8_t slot)
{
    lda(0x03);
    call_routine(0xCB24, 0xCA7C);
    call_routine(0xCB27, 0xCAEF);
    lda(RD8(OBJ->work_h[slot]));
    WR8(OBJ->facing_left[slot], cpu.a);
    abi_nz(DEC8(OBJ->work_e[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(0xB0);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(RD8(OBJ->work_h[slot]));
    lda((uint8_t)(cpu.a ^ 0x01));
    WR8(OBJ->work_h[slot], cpu.a);
}

void abi_PatrolList3_117E(void)
{
    PatrolList3_117E(cpu.x);
}

/*
 * @routine 117E:425 TurnToAttack117E
 * @run 71:CC25
 * WHAT IT DOES: Runs $CA7C with A 2, keeps the facing in OBJ_WORK_H, runs $CC46; with the player at least $48 across, state 3.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void TurnToAttack117E(uint8_t slot)
{
    lda(0x02);
    call_routine(0xCC2A, 0xCA7C);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->work_h[slot], cpu.a);
    call_routine(0xCC33, 0xCC46);
    near_x(slot, 0xCC36);
    cmp(cpu.a, 0x48);
    if (cpu.a < 0x48) {
        return;
    }
    restart_animation(slot);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_TurnToAttack117E(void)
{
    TurnToAttack117E(cpu.x);
}

/*
 * @routine 18DB:296 FaceThenSubstate18DB
 * @run 70:B296
 * WHAT IT DOES: Faces the player and runs $B2C4; unless that sets carry, the substate's handler.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FaceThenSubstate18DB(uint8_t slot)
{
    JumpTableLayout layout = { 0xB2B6 };

    face_by_side(slot, COL(OBJ->facing_left), 0xB299);
    call_routine(0xB2A1, 0xB2C4);
    if ((cpu.p & P_C) != 0) {
        return;
    }
    {
        Jump jump = JumpByPhaseTableBody(slot, &layout);

        jump_shim(jump.end, jump.target);
    }
}

void abi_FaceThenSubstate18DB(void)
{
    FaceThenSubstate18DB(cpu.x);
}

/*
 * @routine 0EBC:61F FallWithoutPrevious0EBC
 * @run 70:BE1F
 * WHAT IT DOES: With the previous slot empty falls at 4; else runs $BDFF and the state's handler.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallWithoutPrevious0EBC(uint8_t slot)
{
    JumpTableLayout layout = { 0xBE41 };

    lda(RD8_IDX((uint16_t)(COL(OBJ->kind) - 1), slot));
    if (cpu.a == 0) {
        lda(0x04);
        WR8(OBJ->vy_hi[slot], cpu.a);
        axis_move_exit_shim(ApplyVelocityY(slot));
        return;
    }
    call_routine(0xBE2F, 0xBDFF);
    {
        Jump jump = JumpByStateTableBody(slot, &layout);

        jump_shim(jump.end, jump.target);
    }
}

void abi_FallWithoutPrevious0EBC(void)
{
    FallWithoutPrevious0EBC(cpu.x);
}

/*
 * @routine 101C:3DA ListSpawnTwice101C
 * @run 70:BBDA
 * WHAT IT DOES: Runs the list at $BBFC; on each new frame calls $BC4D twice, with 3,1 and with $18,0 in zTemp00-01.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListSpawnTwice101C(uint8_t slot)
{
    step_list(slot, 0xBBFC, true, 0xBBE5);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(0x03);
    WR8(ZP->temp00, cpu.a);
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    call_routine(0xBBF2, 0xBC4D);
    lda(0x18);
    WR8(ZP->temp00, cpu.a);
    WR8(ZP->temp01, 0);
    call_routine(0xBBFB, 0xBC4D);
}

void abi_ListSpawnTwice101C(void)
{
    ListSpawnTwice101C(cpu.x);
}

/*
 * @routine 101D:7DA LeadTrail101D
 * @run 71:C7DA
 * WHAT IT DOES: Counts OBJ_WORK_H, runs three of the overlay's steps, moves the $80-byte trail at $C7FC along one and puts OBJ_WORK_E plus OBJ_WORK_D at its head.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LeadTrail101D(uint8_t slot)
{
    INC8(OBJ->work_h[slot]);
    call_routine(0xC7E0, 0xC87C);
    call_routine(0xC7E3, 0xC887);
    call_routine(0xC7E6, 0xC894);
    ldy(0x7F);
    do {
        lda(rd8((uint16_t)(0xC7FB + cpu.y)));
        wr8((uint16_t)(0xC7FC + cpu.y), cpu.a);
        cpu.y = (uint8_t)(cpu.y - 1);
        abi_nz(cpu.y);
    } while (cpu.y != 0);
    lda(RD8(OBJ->work_e[slot]));
    add_a(RD8(OBJ->work_d[slot]), false);
    wr8(0xC7FC, cpu.a);
}

void abi_LeadTrail101D(void)
{
    LeadTrail101D(cpu.x);
}

/*
 * @routine 12DC:21F LeadTrail12DC
 * @run 70:BA1F
 * WHAT IT DOES: Counts OBJ_WORK_H, runs three of the overlay's steps, moves the $F0-byte trail at $BA41 along one and puts OBJ_WORK_E plus OBJ_WORK_D at its head.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LeadTrail12DC(uint8_t slot)
{
    INC8(OBJ->work_h[slot]);
    call_routine(0xBA25, 0xBB31);
    call_routine(0xBA28, 0xBB5C);
    call_routine(0xBA2B, 0xBB88);
    ldy(0xEF);
    do {
        lda(rd8((uint16_t)(0xBA40 + cpu.y)));
        wr8((uint16_t)(0xBA41 + cpu.y), cpu.a);
        cpu.y = (uint8_t)(cpu.y - 1);
        abi_nz(cpu.y);
    } while (cpu.y != 0);
    lda(RD8(OBJ->work_e[slot]));
    add_a(RD8(OBJ->work_d[slot]), false);
    wr8(0xBA41, cpu.a);
}

void abi_LeadTrail12DC(void)
{
    LeadTrail12DC(cpu.x);
}

/*
 * @routine 18DB:153 ChannelByFrame18DB
 * @run 70:B153
 * WHAT IT DOES: Counts OBJ_WORK_D, runs $B183 and $B1AE; while OBJ_WORK_J is 0, a frame from $52 on starts its list channel in slot 4.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ChannelByFrame18DB(uint8_t slot)
{
    INC8(OBJ->work_d[slot]);
    call_routine(0xB159, 0xB183);
    call_routine(0xB15C, 0xB1AE);
    lda(RD8(OBJ->work_j[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->frame[slot]));
    add_a((uint8_t)~0x52, true);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xB175 + cpu.y)));
    push8(cpu.x);
    list_channel2(cpu.a, 0x04, 0xB173);
    pull_x();
}

void abi_ChannelByFrame18DB(void)
{
    ChannelByFrame18DB(cpu.x);
}

/*
 * @routine 088A:6DF FallOrRun088A
 * @run 70:AEDF
 * WHAT IT DOES: Frame $0A; first falls until it is on the first screen (Y high byte 0), then runs $AF03 each time.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallOrRun088A(uint8_t slot)
{
    lda(0x0A);
    WR8(OBJ->frame[slot], cpu.a);
    lda(RD8(OBJ->substate[slot]));
    if (cpu.a == 0) {
        gravity(slot, 0xAEEC);
        move(slot, 0xAEEF);
        lda(RD8(OBJ->y_hi[slot]));
        if (cpu.a == 0) {
            abi_nz(INC8(OBJ->substate[slot]));
        }
        return;
    }
    call_routine(0xAEFB, 0xAF03);
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x01);
}

void abi_FallOrRun088A(void)
{
    FallOrRun088A(cpu.x);
}

/*
 * @routine 0F6C:770 FaceHopStateTen0F6C
 * @run 70:BF70
 * WHAT IT DOES: Faces the player, starts a hop, runs $BF97, OBJ_WORK_D $0E, runs $C35F and goes to state 10.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FaceHopStateTen0F6C(uint8_t slot)
{
    face_by_side(slot, COL(OBJ->facing_left), 0xBF73);
    WR8(OBJ->substate[slot], 0);
    lda(0x80);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(0xFD);
    WR8(OBJ->vy_hi[slot], cpu.a);
    call_routine(0xBF88, 0xBF97);
    lda(0x0E);
    WR8(OBJ->work_d[slot], cpu.a);
    call_routine(0xBF90, 0xC35F);
    lda(0x0A);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_FaceHopStateTen0F6C(void)
{
    FaceHopStateTen0F6C(cpu.x);
}

/*
 * @routine 101E:3E5 OrbitByTrail101E
 * @run 71:CBE5
 * WHAT IT DOES: Runs $CC2B, then out at the angle the trail holds at this piece's lag; slot $10's OBJ_WORK_M counts the state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void OrbitByTrail101E(uint8_t slot)
{
    call_routine(0xCBE8, 0xCC2B);
    ldy(RD8(OBJ->work_d[slot]));
    lda(rd8((uint16_t)(0xCC1B + cpu.y)));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    WR8(ZP->temp00, 0);
    lda(RD8(OBJ->work_i[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(rd8((uint16_t)(0xC7FC + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
    velocity_from_a(slot, 0xCBFF);
    move(slot, 0xCC02);
    lda(RD8(OBJ->work_m[0x10]));
    if (cpu.a == 0) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_OrbitByTrail101E(void)
{
    OrbitByTrail101E(cpu.x);
}

/*
 * @routine 0EBF:234 TurnBackAtRandom0EBF
 * @run 71:D234
 * WHAT IT DOES: Keeps the facing, picks OBJ_WORK_I from the distance to the player, runs $D25D, turns the velocity back, runs $D2C9; substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void TurnBackAtRandom0EBF(uint8_t slot)
{
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->work_k[slot], cpu.a);
    jsr_push(0xD23D);
    distance_sum_exit_shim(PlayerDistanceSum(slot, (cpu.p & P_C) != 0));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x0F));
    WR8(OBJ->work_i[slot], cpu.a);
    call_routine(0xD245, 0xD25D);
    jsr_push(0xD248);
    negate_exit_shim(NegateVelocityX(slot));
    jsr_pop();
    jsr_push(0xD24B);
    negate_exit_shim(NegateVelocityY(slot));
    jsr_pop();
    call_routine(0xD24E, 0xD2C9);
    lda(0x0C);
    WR8(OBJ->work_d[slot], cpu.a);
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_TurnBackAtRandom0EBF(void)
{
    TurnBackAtRandom0EBF(cpu.x);
}

/*
 * @routine 0EBF:2DE RunThenNewCourse0EBF
 * @run 71:D2DE
 * WHAT IT DOES: Animates and moves for OBJ_WORK_D counts; then sound $C7, $D25D, a new count of $18-$97 in OBJ_WORK_J, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RunThenNewCourse0EBF(uint8_t slot)
{
    ldy(0x04);
    call_routine(0xD2E3, 0xD19C);
    move(slot, 0xD2E6);
    abi_nz(DEC8(OBJ->work_d[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    sound(0xC7, 0xD2F0);
    call_routine(0xD2F3, 0xD25D);
    random_a(0xD2F6);
    lda((uint8_t)(cpu.a & 0x7F));
    add_a(0x18, false);
    WR8(OBJ->work_j[slot], cpu.a);
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_RunThenNewCourse0EBF(void)
{
    RunThenNewCourse0EBF(cpu.x);
}

/*
 * @routine 0EBD:27C DoorRouteByHit0EBD
 * @run 71:C27C
 * WHAT IT DOES: A flagged door: while it is not held and the player is ready, a weak hit takes the route in the start argument.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DoorRouteByHit0EBD(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record_tail_(slot);
        return;
    }
    lda(rd8(StageDoorHeld));
    lda((uint8_t)(cpu.a ^ 0x01));
    lda((uint8_t)(cpu.a & rd8(PlayerDoorReady)));
    if (cpu.a == 0) {
        return;
    }
    lda(RD8(OBJ->hit_taken[slot]));
    if (cpu.a == 0) {
        return;
    }
    cmp(cpu.a, 0x03);
    if (cpu.a >= 0x03) {
        return;
    }
    lda(0x80);
    lda((uint8_t)(cpu.a | RD8(OBJ->start_arg[slot])));
    WR8(ZP->field8c, cpu.a);
    lda(0x02);
    WR8(ZP->field82, cpu.a);
    lda(0x03);
    WR8(ZP->sub_step, cpu.a);
}

void abi_DoorRouteByHit0EBD(void)
{
    DoorRouteByHit0EBD(cpu.x);
}

/*
 * @routine 088A:610 StateSixteenWhenHit088A
 * @run 70:AE10
 * WHAT IT DOES: Hit outside states 16 and 18: sound $54, runs $B791, stops and goes to state 16.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StateSixteenWhenHit088A(uint8_t slot)
{
    lda(RD8(OBJ->hit_status[slot]));
    lda((uint8_t)(cpu.a & 0x80));
    if (cpu.a == 0) {
        return;
    }
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x10);
    if (cpu.a == 0x10) {
        return;
    }
    cmp(cpu.a, 0x12);
    if (cpu.a == 0x12) {
        return;
    }
    sound(0x54, 0xAE27);
    call_routine(0xAE2A, 0xB791);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    lda(0x10);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_StateSixteenWhenHit088A(void)
{
    StateSixteenWhenHit088A(cpu.x);
}

/*
 * @routine 0EBE:4B6 AnimateThenLeapAcross0EBE
 * @run 71:CCB6
 * WHAT IT DOES: At the end of the list leaps toward the farther half of the screen, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void AnimateThenLeapAcross0EBE(uint8_t slot)
{
    ldy(0x04);
    call_routine(0xCCBB, 0xCB86);
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(RD8(OBJ->x_lo[slot]));
    rol_a();
    rol_a();
    lda((uint8_t)(cpu.a & 0x01));
    lda((uint8_t)(cpu.a ^ 0x01));
    wr8(zHitSlot, cpu.a);
    ldy(rd8(zHitSlot));
    lda(rd8((uint16_t)(0xCCE5 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0xFD);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_AnimateThenLeapAcross0EBE(void)
{
    AnimateThenLeapAcross0EBE(cpu.x);
}

/*
 * @routine 161C:14C AnimateThenLeapAcross161C
 * @run 70:B94C
 * WHAT IT DOES: At the end of the list leaps toward the farther half of the screen, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void AnimateThenLeapAcross161C(uint8_t slot)
{
    ldy(0x04);
    call_routine(0xB951, 0xB81C);
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(RD8(OBJ->x_lo[slot]));
    rol_a();
    rol_a();
    lda((uint8_t)(cpu.a & 0x01));
    lda((uint8_t)(cpu.a ^ 0x01));
    wr8(zHitSlot, cpu.a);
    ldy(rd8(zHitSlot));
    lda(rd8((uint16_t)(0xB97B + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0xFD);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_AnimateThenLeapAcross161C(void)
{
    AnimateThenLeapAcross161C(cpu.x);
}

/*
 * @routine 101D:480 CloseDoorOnScreen7_101D
 * @run 71:C480
 * WHAT IT DOES: On screen 7: holds the scroll there, starts the list channels and script $8A, holds the door, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CloseDoorOnScreen7_101D(uint8_t slot)
{
    lda(rd8(ScrollXHiScreen));
    cmp(cpu.a, 0x07);
    if (cpu.a != 0x07) {
        return;
    }
    lda(0x07);
    wr8(ScrollXMinHi, cpu.a);
    push8(cpu.x);
    list_channel2(0x86, 0x05, 0xC494);
    lda(rd8(zCharacter));
    if (cpu.a != 0) {
        list_channel2(0x87, 0x04, 0xC49F);
    }
    script_at(0x8A, 0x05, 0xC4A6);
    lda(0x01);
    wr8(StageDoorHeld, cpu.a);
    pull_x();
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_CloseDoorOnScreen7_101D(void)
{
    CloseDoorOnScreen7_101D(cpu.x);
}

/*
 * @routine 117B:300 TouchThenTurn117B
 * @run 70:B300
 * WHAT IT DOES: Calls $AA35 with the point $1C,$1A, taking the push off again; on contact with slot 0 rising or falling, faces the player and goes on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void TouchThenTurn117B(uint8_t slot)
{
    lda(0x83);
    WR8(OBJ->sprite_attr[slot], cpu.a);
    lda(0x1C);
    WR8(ZP->temp08, cpu.a);
    lda(0x1A);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xB310, 0xAA35);
    push8((uint8_t)((cpu.p | P_B) & (uint8_t)~P_T));
    lda(rd8(zPlayerPush));
    lda((uint8_t)(cpu.a & 0xF9));
    wr8(zPlayerPush, cpu.a);
    cpu.p = pull8();
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->vy_hi[0]));
    if (cpu.a == 0) {
        return;
    }
    face_by_side(slot, COL(OBJ->work_e), 0xB322);
    lda(0x05);
    WR8(OBJ->work_d[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_TouchThenTurn117B(void)
{
    TouchThenTurn117B(cpu.x);
}

/*
 * @routine 12DE:078 AnimateByWorkDThenState12DE
 * @run 71:C878
 * WHAT IT DOES: Runs the frame list bits 4-6 of OBJ_WORK_D halved pick, then the state's handler.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void AnimateByWorkDThenState12DE(uint8_t slot)
{
    JumpTableLayout layout = { 0xC8A3 };

    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x70));
    for (int i = 0; i < 5; i++) {
        abi_flag(P_C, (cpu.a & 0x01) != 0);
        lda((uint8_t)(cpu.a >> 1));
    }
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xC9B9 + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xC9BA + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y = cpu.y;
        FrameStep step;

        jsr_push(0xC891);
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    {
        Jump jump = JumpByStateTableBody(slot, &layout);

        jump_shim(jump.end, jump.target);
    }
}

void abi_AnimateByWorkDThenState12DE(void)
{
    AnimateByWorkDThenState12DE(cpu.x);
}
