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
#include "calls_b6.h"
#include "calls_b29.h"

static void lda(uint8_t value) { cpu.a = value; abi_nz(value); }

static void cmp(uint8_t reg, uint8_t value)
{
    abi_flag(P_C, reg >= value);
    abi_nz((uint8_t)(reg - value));
}

/* JSR LoadObjectRecord's vector. */
static void load_record(uint8_t slot, uint16_t ret)
{
    uint8_t nz, last;
    bool carry;

    jsr_push(ret);
    last = LoadObjectRecord(slot, &nz, &carry);
    load_record_exit_shim(last, nz, carry);
    jsr_pop();
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

static void move_x(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
}

/* BNE after StepObjectFrames: true when the list ended. */
static bool list_ended(void)
{
    return (cpu.p & P_Z) != 0;
}
static void add_a(uint8_t value, bool carry_in)
{
    Sum sum = add_bytes(cpu.a, value, carry_in);

    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
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

static void near_x(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    abi_ended(ended_of_distance(DistanceXToPlayer(slot)));
    jsr_pop();
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
 * @routine 18DC:27D ShieldOfLeader18DC
 * @run 70:BA7D
 * WHAT IT DOES: Rides the kind $A4 object in slot $10 with the box its start argument picks; gone without it.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ShieldOfLeader18DC(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xBA85);
        lda(RD8(OBJ->start_arg[slot]));
        word_index((uint8_t)(cpu.a << 1));
        lda(rd8((uint16_t)(0xBAC0 + cpu.y)));
        WR8(OBJ->hitbox_half_w[slot], cpu.a);
        lda(rd8((uint16_t)(0xBAC1 + cpu.y)));
        WR8(OBJ->hitbox_half_h[slot], cpu.a);
        lda(rd8((uint16_t)(0xBAC2 + cpu.y)));
        WR8(OBJ->hitbox_dx[slot], cpu.a);
        lda(rd8((uint16_t)(0xBAC3 + cpu.y)));
        WR8(OBJ->hitbox_dy[slot], cpu.a);
        return;
    }
    ldy(0x10);
    lda(RD8(OBJ->kind[cpu.y]));
    cmp(cpu.a, 0xA4);
    if (cpu.a != 0xA4) {
        ClearObject(slot);
        return;
    }
    jsr_push(0xBAB3);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    lda(RD8(OBJ->facing_left[0x10]));
    WR8(OBJ->facing_left[slot], cpu.a);
    lda(RD8(OBJ->hitbox_on[0x10]));
    WR8(OBJ->hitbox_on[slot], cpu.a);
}

void abi_ShieldOfLeader18DC(void)
{
    ShieldOfLeader18DC(cpu.x);
}

/*
 * @routine 072C:495 DissolveFall072C
 * @run 70:BC95
 * WHAT IT DOES: Every fourth count a dissolve step and falling; below the second screen becomes kind $22.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DissolveFall072C(uint8_t slot)
{
    step_list(slot, 0xBCD9, true, 0xBCA0);
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->work_o[slot]));
    add_a(0x13, false);
    lda((uint8_t)(cpu.a & 0x3F));
    WR8(OBJ->work_o[slot], cpu.a);
    call_routine(0xBCB5, 0xBCE6);
    lda(RD8(OBJ->vy_lo[slot]));
    abi_flag(P_C, false);
    FallThenBecome22_072C(slot);
}

void abi_DissolveFall072C(void)
{
    DissolveFall072C(cpu.x);
}

/*
 * @routine 0EBF:760 LandAnimateByRise0EBF
 * @run 71:D760
 * WHAT IT DOES: Fades by OBJ_WORK_H, falls and lands (state on), showing frame $66 falling or the list rising.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LandAnimateByRise0EBF(uint8_t slot)
{
    lda(RD8(OBJ->work_h[slot]));
    if (cpu.a != 0) {
        for (int i = 0; i < 4; i++) {
            abi_flag(P_C, (cpu.a & 0x01) != 0);
            lda((uint8_t)(cpu.a >> 1));
        }
        cpu.y = cpu.a;
        abi_nz(cpu.y);
        add_a(0x8C, false);
        WR8(OBJ->sprite_attr[slot], cpu.a);
        abi_nz(DEC8(OBJ->work_h[slot]));
    }
    lda(RD8(OBJ->y_lo[slot]));
    lda((uint8_t)(cpu.a & 0xF0));
    cmp(cpu.a, 0xD0);
    if (cpu.a == 0xD0) {
        call_routine(0xD77F, 0xD7B5);
    }
    lda(0x0A);
    WR8(ZP->product.mid, cpu.a);
    jsr_push(0xD786);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), 0x01);
    jsr_pop();
    if ((cpu.p & P_C) != 0) {
        WR8(OBJ->substate[slot], 0);
        INC8(OBJ->state[slot]);
    }
    lda(RD8(OBJ->vy_hi[slot]));
    if ((cpu.p & P_N) == 0) {
        lda(0x66);
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    lda(0xA4);
    WR8(ZP->temp10, cpu.a);
    lda(0xD7);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y = cpu.y;
        FrameStep step = StepObjectFrames(slot, &y);

        step_frames_exit_shim(step, y);
    }
}

void abi_LandAnimateByRise0EBF(void)
{
    LandAnimateByRise0EBF(cpu.x);
}

/*
 * @routine 0F6C:4FA ChaseOrLeap0F6C
 * @run 70:BCFA
 * WHAT IT DOES: Runs toward the player; turned away or at an edge, turns or goes to state 5.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ChaseOrLeap0F6C(uint8_t slot)
{
    call_routine(0xBCFD, 0xBEF7);
    if ((cpu.p & P_C) != 0) {
        return;
    }
    call_routine(0xBD03, 0xBD80);
    if ((cpu.p & P_C) != 0) {
        return;
    }
    call_routine(0xBD09, 0xC365);
    move_x(slot, 0xBD0C);
    near_x(slot, 0xBD0F);
    cmp(cpu.a, 0x60);
    if (cpu.a >= 0x60) {
        jsr_push(0xBD16);
        side_of_player_exit_shim(SideOfPlayer(slot));
        jsr_pop();
        lda((uint8_t)(cpu.a & 0x01));
        lda((uint8_t)(cpu.a ^ RD8(OBJ->facing_left[slot])));
        if (cpu.a != 0) {
            goto leap;
        }
    }
    call_routine(0xBD20, 0xC324);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->y_lo[0]));
    cmp(cpu.a, 0x9A);
    if (cpu.a != 0x9A) {
        call_routine(0xBD2C, 0xC377);
        negate_exit_shim(NegateVelocityX(slot));
        return;
    }
leap:
    lda(0x0C);
    WR8(OBJ->work_d[slot], cpu.a);
    call_routine(0xBD38, 0xC35F);
    lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_ChaseOrLeap0F6C(void)
{
    ChaseOrLeap0F6C(cpu.x);
}

/*
 * @routine 101D:09D GateOnScreen4_101D
 * @run 71:C09D
 * WHAT IT DOES: On screen 4 shuts the way behind (sound $57); $20 counts on, hit-tested and the state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void GateOnScreen4_101D(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (cpu.a == 0) {
        lda(rd8(ScrollXHiScreen));
        cmp(cpu.a, 0x04);
        if (cpu.a != 0x04) {
            return;
        }
        sound(0x57, 0xC0AE);
        lda(0x04);
        wr8(ScrollXMinHi, cpu.a);
        lda(RD8(OBJ->start_arg[slot]));
        add_a(0x9C, false);
        WR8(OBJ->frame[slot], cpu.a);
        lda(RD8(OBJ->sprite_attr[slot]));
        lda((uint8_t)(cpu.a | 0x10));
        WR8(OBJ->sprite_attr[slot], cpu.a);
        lda(0x20);
        WR8(OBJ->work_j[slot], cpu.a);
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    abi_nz(DEC8(OBJ->work_j[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    WR8(OBJ->sprite_attr[slot], 0);
    INC8(OBJ->state[slot]);
    WR8(OBJ->substate[slot], 0);
}

void abi_GateOnScreen4_101D(void)
{
    GateOnScreen4_101D(cpu.x);
}

/*
 * @routine 088A:3B8 IntroList088A
 * @run 70:ABB8
 * WHAT IT DOES: The entrance: roars (sound $99) after a delay, its list (shorter once seen), then the music and a leap.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void IntroList088A(uint8_t slot)
{
    lda(rd8(zWaveCounter));
    lda((uint8_t)(cpu.a & 0x01));
    lda((uint8_t)(cpu.a | RD8(OBJ->work_e[slot])));
    if (cpu.a == 0) {
        abi_nz(DEC8(OBJ->work_d[slot]));
        if ((cpu.p & P_Z) != 0) {
                lda(0x01);
                WR8(OBJ->work_e[slot], cpu.a);
            sound(0x99, 0xABD0);
        }
    }
    lda(0x31);
    WR8(ZP->temp10, cpu.a);
    lda(0xAC);
    WR8(ZP->temp11, cpu.a);
    lda(rd8(zField92));
    if (cpu.a != 0) {
        lda(0xAE);
        WR8(ZP->temp10, cpu.a);
        lda(0xAC);
        WR8(ZP->temp11, cpu.a);
    }
    {
        uint8_t y = cpu.y;
        FrameStep step;

        jsr_push(0xABE7);
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (!list_ended()) {
        return;
    }
    RMW8(ZP->field8e, old, old & 0xF7);
    RMW8(ZP->field8e, old, old & 0xEF);
    jsr_push(0xABF0);
    play_track_exit_shim(CdPlayAgain(cpu.x, cpu.y));
    jsr_pop();
    lda(0x01);
    wr8(zField92, cpu.a);
    call_routine(0xABF7, 0xACB9);
    lda(0xFC);
    WR8(OBJ->vy_hi[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_IntroList088A(void)
{
    IntroList088A(cpu.x);
}

/*
 * @routine 1180:05A ArmByLeader1180
 * @run 71:D85A
 * WHAT IT DOES: A limb of the kind $34 object before it: its place, facing, hit test and the box for its frame.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ArmByLeader1180(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record_tail_(slot);
        return;
    }
    lda(RD8_IDX((uint16_t)(COL(OBJ->kind) - 1), slot));
    cmp(cpu.a, 0x34);
    if (cpu.a != 0x34) {
        ClearObject(slot);
        return;
    }
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y - 1);
    abi_nz(cpu.y);
    jsr_push(0xD872);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    lda(RD8_IDX((uint16_t)(COL(OBJ->frame) - 1), slot));
    add_a((uint8_t)~0x02, true);
    cmp(cpu.a, 0x22);
    if (cpu.a >= 0x22) {
        lda(0x22);
    }
    word_index((uint8_t)(cpu.a << 1));
    lda(rd8((uint16_t)(0xD8DA + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(rd8((uint16_t)(0xD8DB + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    lda(rd8((uint16_t)(0xD8DC + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(rd8((uint16_t)(0xD8DD + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
    lda(RD8_IDX((uint16_t)(COL(OBJ->facing_left) - 1), slot));
    WR8(OBJ->facing_left[slot], cpu.a);
    lda(RD8_IDX((uint16_t)(COL(OBJ->hitbox_on) - 1), slot));
    WR8(OBJ->hitbox_on[slot], cpu.a);
}

void abi_ArmByLeader1180(void)
{
    ArmByLeader1180(cpu.x);
}
