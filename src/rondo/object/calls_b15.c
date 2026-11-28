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
#include "calls_b15.h"

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

static void clear_slot_in_y(uint16_t ret)
{
    jsr_push(ret);
    ClearObjectInY(cpu.y);
    jsr_pop();
}

/* LDA #kind and JSR StartObjectAtPlace: the object at X's place, in slot Y. */
static void start_at_place(uint8_t kind, uint16_t ret)
{
    lda(kind);
    jsr_push(ret);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
    jsr_pop();
}

static void move_x(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
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

static void near_x(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    abi_ended(ended_of_distance(DistanceXToPlayer(slot)));
    jsr_pop();
}
static void record_event(uint8_t slot, uint16_t ret)
{
    uint8_t stored, event, byte;
    bool marked;

    jsr_push(ret);
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    abi_ended(l8_of_event(marked, stored, event, byte));
    jsr_pop();
}

/*
 * @routine 0EBE:371 AnimateThenDrift0EBE
 * @run 71:CB71
 * WHAT IT DOES: Faces the player and animates; with carry set from that, moves across at the speed OBJ_WORK_E picks.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void AnimateThenDrift0EBE(uint8_t slot)
{
    cpu.y = 0;
    call_routine(0xCB75, 0xCB86);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    ldy(RD8(OBJ->work_e[slot]));
    lda(rd8((uint16_t)(0xCB84 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    move_x(slot, 0xCB83);
}

void abi_AnimateThenDrift0EBE(void)
{
    AnimateThenDrift0EBE(cpu.x);
}

/*
 * @routine 161C:007 AnimateThenDrift161C
 * @run 70:B807
 * WHAT IT DOES: Faces the player and animates; with carry set from that, moves across at the speed OBJ_WORK_E picks.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void AnimateThenDrift161C(uint8_t slot)
{
    cpu.y = 0;
    call_routine(0xB80B, 0xB81C);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    ldy(RD8(OBJ->work_e[slot]));
    lda(rd8((uint16_t)(0xB81A + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    move_x(slot, 0xB819);
}

void abi_AnimateThenDrift161C(void)
{
    AnimateThenDrift161C(cpu.x);
}

/*
 * @routine 0EBE:4F2 AnimateThenStateThree0EBE
 * @run 71:CCF2
 * WHAT IT DOES: Runs the second frame list; at its end state 3 from the start.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void AnimateThenStateThree0EBE(uint8_t slot)
{
    ldy(0x06);
    call_routine(0xCCF7, 0xCB86);
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    restart_animation(slot);
    WR8(OBJ->substate[slot], 0);
}

void abi_AnimateThenStateThree0EBE(void)
{
    AnimateThenStateThree0EBE(cpu.x);
}

/*
 * @routine 161C:188 AnimateThenStateThree161C
 * @run 70:B988
 * WHAT IT DOES: Runs the second frame list; at its end state 3 from the start.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void AnimateThenStateThree161C(uint8_t slot)
{
    ldy(0x06);
    call_routine(0xB98D, 0xB81C);
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    restart_animation(slot);
    WR8(OBJ->substate[slot], 0);
}

void abi_AnimateThenStateThree161C(void)
{
    AnimateThenStateThree161C(cpu.x);
}

/*
 * @routine 0EBD:268 CountThenRecordClear0EBD
 * @run 71:C268
 * WHAT IT DOES: Runs $B22B; when OBJ_WORK_D runs out, records the event and clears.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CountThenRecordClear0EBD(uint8_t slot)
{
    call_routine(0xC26B, 0xB22B);
    lda(RD8(OBJ->work_d[slot]));
    if (cpu.a == 0) {
        return;
    }
    abi_nz(DEC8(OBJ->work_d[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    record_event(slot, 0xC278);
    ClearObject(slot);
}

void abi_CountThenRecordClear0EBD(void)
{
    CountThenRecordClear0EBD(cpu.x);
}

/*
 * @routine 10CD:439 AnimateByWorkF10CD
 * @run 71:C439
 * WHAT IT DOES: Runs the frame list OBJ_WORK_F picks, in sprite set 6.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void AnimateByWorkF10CD(uint8_t slot)
{
    lda(RD8(OBJ->work_f[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xC4C1 + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xC4C2 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y = cpu.y;
        FrameStep step;

        jsr_push(0xC44B);
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    lda(0x06);
    WR8(OBJ->sprite_set[slot], cpu.a);
}

void abi_AnimateByWorkF10CD(void)
{
    AnimateByWorkF10CD(cpu.x);
}

/*
 * @routine 101D:4B0 ListThenHittable101D
 * @run 71:C4B0
 * WHAT IT DOES: Runs the list at $C4C9; at its end the object is hit-tested and the state counts on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenHittable101D(uint8_t slot)
{
    step_list(slot, 0xC4C9, true, 0xC4BB);
    if (!list_ended()) {
        return;
    }
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    INC8(OBJ->state[slot]);
    WR8(OBJ->substate[slot], 0);
}

void abi_ListThenHittable101D(void)
{
    ListThenHittable101D(cpu.x);
}

/*
 * @routine 12DE:15A DriftThenPickBySide12DE
 * @run 71:C95A
 * WHAT IT DOES: Moves across for OBJ_WORK_E counts, then picks OBJ_WORK_H by the player's side and the state counts on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DriftThenPickBySide12DE(uint8_t slot)
{
    move_x(slot, 0xC95D);
    abi_nz(DEC8(OBJ->work_e[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    jsr_push(0xC965);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    abi_flag(P_C, (cpu.a & 0x01) != 0);
    lda((uint8_t)(cpu.a >> 1));
    lda((uint8_t)(cpu.a & 0x01));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xC8F0 + cpu.y)));
    WR8(OBJ->work_h[slot], cpu.a);
    INC8(OBJ->state[slot]);
}

void abi_DriftThenPickBySide12DE(void)
{
    DriftThenPickBySide12DE(cpu.x);
}

/*
 * @routine 101C:5DF FollowSlotZeroFor101C
 * @run 70:BDDF
 * WHAT IT DOES: Stays by slot 0 for OBJ_WORK_D counts; cleared then, or in game sub-mode 4.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FollowSlotZeroFor101C(uint8_t slot)
{
    lda(rd8(zGameSubMode));
    cmp(cpu.a, 0x04);
    if (cpu.a == 0x04) {
        ClearObject(slot);
        return;
    }
    ldy(0x00);
    jsr_push(0xBDED);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    move(slot, 0xBDF0);
    abi_nz(DEC8(OBJ->work_d[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    ClearObject(slot);
}

void abi_FollowSlotZeroFor101C(void)
{
    FollowSlotZeroFor101C(cpu.x);
}

/*
 * @routine 117B:330 PointCallThenCount117B
 * @run 70:B330
 * WHAT IT DOES: Calls $AA35 with the point $1C,$1A, takes bits 1-2 off the player's push, and after OBJ_WORK_D counts the state goes on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void PointCallThenCount117B(uint8_t slot)
{
    lda(0x1C);
    WR8(ZP->temp08, cpu.a);
    lda(0x1A);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xB33B, 0xAA35);
    lda(rd8(zPlayerPush));
    lda((uint8_t)(cpu.a & 0xF9));
    wr8(zPlayerPush, cpu.a);
    abi_nz(DEC8(OBJ->work_d[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    INC8(OBJ->state[slot]);
}

void abi_PointCallThenCount117B(void)
{
    PointCallThenCount117B(cpu.x);
}

/*
 * @routine 13FB:6AB ListCallThenSubstate13FB
 * @run 70:B6AB
 * WHAT IT DOES: Runs the list at $B6C5 and $B6CC; carry from it counts the substate on with the animation from the start.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListCallThenSubstate13FB(uint8_t slot)
{
    step_list(slot, 0xB6C5, true, 0xB6B6);
    call_routine(0xB6B9, 0xB6CC);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    restart_animation(slot);
    INC8(OBJ->substate[slot]);
}

void abi_ListCallThenSubstate13FB(void)
{
    ListCallThenSubstate13FB(cpu.x);
}

/*
 * @routine 088A:703 FrameLandCall088A
 * @run 70:AF03
 * WHAT IT DOES: Frame $0A; on landing runs $B784, counts the state on and runs $AF1E.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FrameLandCall088A(uint8_t slot)
{
    lda(0x0A);
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->work_f[slot], cpu.a);
    lda(0x04);
    jsr_push(0xAF12);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), 0x01);
    jsr_pop();
    if ((cpu.p & P_C) == 0) {
        return;
    }
    call_routine(0xAF17, 0xB784);
    INC8(OBJ->state[slot]);
    call_routine(0xAF1D, 0xAF1E);
}

void abi_FrameLandCall088A(void)
{
    FrameLandCall088A(cpu.x);
}

/*
 * @routine 117F:43E TurnToAttack117F
 * @run 71:D43E
 * WHAT IT DOES: Looks ahead (in OBJ_WORK_F the way to go); with carry clear and the player nearer than $50 across, state 3.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void TurnToAttack117F(uint8_t slot)
{
    call_routine(0xD441, 0xD459);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->work_f[slot], cpu.a);
    call_routine(0xD44A, 0xD486);
    if ((cpu.p & P_C) != 0) {
        return;
    }
    near_x(slot, 0xD44F);
    cmp(cpu.a, 0x50);
    if (cpu.a >= 0x50) {
        return;
    }
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_TurnToAttack117F(void)
{
    TurnToAttack117F(cpu.x);
}

/*
 * @routine 161D:654 TurnToAttack161D
 * @run 71:C654
 * WHAT IT DOES: Looks ahead (in OBJ_WORK_E the way to go); with carry clear and the player nearer than $4C across, state 3.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void TurnToAttack161D(uint8_t slot)
{
    call_routine(0xC657, 0xC66F);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->work_e[slot], cpu.a);
    call_routine(0xC660, 0xC6A6);
    if ((cpu.p & P_C) != 0) {
        return;
    }
    near_x(slot, 0xC665);
    cmp(cpu.a, 0x4C);
    if (cpu.a >= 0x4C) {
        return;
    }
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_TurnToAttack161D(void)
{
    TurnToAttack161D(cpu.x);
}

/*
 * @routine 117F:49F TurnToBackOff117F
 * @run 71:D49F
 * WHAT IT DOES: Looks ahead (in OBJ_WORK_F the way to go); with carry clear and the player at least $78 across, state 2.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void TurnToBackOff117F(uint8_t slot)
{
    call_routine(0xD4A2, 0xD459);
    lda(RD8(OBJ->facing_left[slot]));
    lda((uint8_t)(cpu.a ^ 0x01));
    WR8(OBJ->work_f[slot], cpu.a);
    call_routine(0xD4AD, 0xD486);
    if ((cpu.p & P_C) != 0) {
        return;
    }
    near_x(slot, 0xD4B2);
    cmp(cpu.a, 0x78);
    if (cpu.a < 0x78) {
        return;
    }
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_TurnToBackOff117F(void)
{
    TurnToBackOff117F(cpu.x);
}

/*
 * @routine 161D:6CA TurnToBackOff161D
 * @run 71:C6CA
 * WHAT IT DOES: Looks ahead (in OBJ_WORK_E the way to go); with carry clear and the player at least $74 across, state 2.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void TurnToBackOff161D(uint8_t slot)
{
    call_routine(0xC6CD, 0xC66F);
    lda(RD8(OBJ->facing_left[slot]));
    lda((uint8_t)(cpu.a ^ 0x01));
    WR8(OBJ->work_e[slot], cpu.a);
    call_routine(0xC6D8, 0xC6A6);
    if ((cpu.p & P_C) != 0) {
        return;
    }
    near_x(slot, 0xC6DD);
    cmp(cpu.a, 0x74);
    if (cpu.a < 0x74) {
        return;
    }
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_TurnToBackOff161D(void)
{
    TurnToBackOff161D(cpu.x);
}

/*
 * @routine 101F:14A KeepArgsThenClear101F
 * @run 71:D14A
 * WHAT IT DOES: Fills 64 bytes at $7E00 with the start argument (a non-zero one also flips slot 0), records the event and clears.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void KeepArgsThenClear101F(uint8_t slot)
{
    lda(RD8(OBJ->start_arg[slot]));
    if (cpu.a != 0) {
        lda(0xC0);
        WR8(OBJ->sprite_flip[0], cpu.a);
    }
    cpu.y = 0;
    lda(RD8(OBJ->start_arg[slot]));
    do {
        wr8((uint16_t)(0x7E00 + cpu.y), cpu.a);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x40);
    } while (cpu.y != 0x40);
    record_event(slot, 0xD163);
    ClearObject(slot);
}

void abi_KeepArgsThenClear101F(void)
{
    KeepArgsThenClear101F(cpu.x);
}

/*
 * @routine 101F:652 ChannelScriptThenClear101F
 * @run 71:D652
 * WHAT IT DOES: Once zField92 is set, or in game sub-mode 0: list channel $88, script $82, and the object goes.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ChannelScriptThenClear101F(uint8_t slot)
{
    lda(rd8(zField92));
    if (cpu.a == 0) {
        lda(rd8(zGameSubMode));
        if (cpu.a != 0) {
            return;
        }
    }
    lda(0x88);
    push8(cpu.x);
    list_channel2(cpu.a, 0x06, 0xD663);
    script_at(0x82, 0x05, 0xD66A);
    pull_x();
    ClearObject(slot);
}

void abi_ChannelScriptThenClear101F(void)
{
    ChannelScriptThenClear101F(cpu.x);
}

/*
 * @routine 10CB:304 ListThenSound46_10CB
 * @run 70:B304
 * WHAT IT DOES: Runs $B383 and the list at $B320; at its end sound $46, OBJ_WORK_F cleared, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenSound46_10CB(uint8_t slot)
{
    call_routine(0xB307, 0xB383);
    step_list(slot, 0xB320, true, 0xB312);
    if (!list_ended()) {
        return;
    }
    sound(0x46, 0xB319);
    WR8(OBJ->work_f[slot], 0);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenSound46_10CB(void)
{
    ListThenSound46_10CB(cpu.x);
}

/*
 * @routine 088B:791 TurnOrClearSlots12to15_088B
 * @run 70:B791
 * WHAT IT DOES: Of slots $12-$15, those with OBJ_WORK_D 2 become kind $9C, the rest are cleared.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: Exit: as the last call, compare or load left them.
 */
void TurnOrClearSlots12to15_088B(void)
{
    push8(cpu.x);
    cpu.x = 0x12;
    abi_nz(cpu.x);
    do {
        lda(RD8(OBJ->work_d[cpu.x]));
        cmp(cpu.a, 0x02);
        if (cpu.a == 0x02) {
            lda(0x9C);
            WR8(OBJ->kind[cpu.x], cpu.a);
            WR8(OBJ->state[cpu.x], 0);
        } else {
            jsr_push(0xB7A8);
            ClearObject(cpu.x);
            jsr_pop();
        }
        cpu.x = (uint8_t)(cpu.x + 1);
        cmp(cpu.x, 0x16);
    } while (cpu.x < 0x16);
    pull_x();
}

void abi_TurnOrClearSlots12to15_088B(void)
{
    TurnOrClearSlots12to15_088B();
}

/*
 * @routine 0EBF:444 WaitFrameListThenState0EBF
 * @run 71:D444
 * WHAT IT DOES: Animates for $1C counts, then state 2.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WaitFrameListThenState0EBF(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (cpu.a == 0) {
        lda(0x1C);
        WR8(OBJ->work_d[slot], cpu.a);
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    ldy(0x02);
    call_routine(0xD457, 0xD19C);
    abi_nz(DEC8(OBJ->work_d[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_WaitFrameListThenState0EBF(void)
{
    WaitFrameListThenState0EBF(cpu.x);
}

/*
 * @routine 0EBD:547 ListThenResetOther0EBD
 * @run 71:C547
 * WHAT IT DOES: At the end of the list at $C566 clears the start argument of the slot in OBJ_WORK_F; OBJ_WORK_G $80, state 1.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenResetOther0EBD(uint8_t slot)
{
    step_list(slot, 0xC566, true, 0xC552);
    if (!list_ended()) {
        return;
    }
    ldy(RD8(OBJ->work_f[slot]));
    lda(0x00);
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    lda(0x80);
    WR8(OBJ->work_g[slot], cpu.a);
    lda(0x01);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_ListThenResetOther0EBD(void)
{
    ListThenResetOther0EBD(cpu.x);
}

/*
 * @routine 12DC:3F4 FallToFloorA4_12DC
 * @run 70:BBF4
 * WHAT IT DOES: Falls with gravity; at Y $A4 it stops there, carry set.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallToFloorA4_12DC(uint8_t slot)
{
    lda(0x04);
    WR8(ZP->product.mid, cpu.a);
    lda(0x22);
    jsr_push(0xBBFD);
    probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
    jsr_pop();
    move(slot, 0xBC00);
    gravity(slot, 0xBC03);
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xA4);
    if (cpu.a < 0xA4) {
        abi_flag(P_C, false);
        return;
    }
    lda(0xA4);
    WR8(OBJ->y_lo[slot], cpu.a);
    abi_flag(P_C, true);
}

void abi_FallToFloorA4_12DC(void)
{
    FallToFloorA4_12DC(cpu.x);
}

/*
 * @routine 12DE:2F4 FallToFloor93_12DE
 * @run 71:CAF4
 * WHAT IT DOES: Falls with gravity; at Y $93 it stops there, carry set.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallToFloor93_12DE(uint8_t slot)
{
    lda(0x0A);
    WR8(ZP->product.mid, cpu.a);
    lda(0x22);
    jsr_push(0xCAFD);
    probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
    jsr_pop();
    move(slot, 0xCB00);
    gravity(slot, 0xCB03);
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0x93);
    if (cpu.a < 0x93) {
        abi_flag(P_C, false);
        return;
    }
    lda(0x93);
    WR8(OBJ->y_lo[slot], cpu.a);
    abi_flag(P_C, true);
}

void abi_FallToFloor93_12DE(void)
{
    FallToFloor93_12DE(cpu.x);
}

/*
 * @routine 0E0B:795 StartFifteenKind2B_0E0B
 * @run 70:B795
 * WHAT IT DOES: Starts numbered kind $2B objects facing the same way in slots $20-$2E.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartFifteenKind2B_0E0B(uint8_t slot)
{
    ldy(0x20);
    WR8(ZP->temp00, 0);
    do {
        clear_slot_in_y(0xB79C);
        start_at_place(0x2B, 0xB7A1);
        lda(RD8(OBJ->facing_left[slot]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        lda(RD8(ZP->temp00));
        WR8(OBJ->start_arg[cpu.y], cpu.a);
        INC8(ZP->temp00);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x2F);
    } while (cpu.y < 0x2F);
}

void abi_StartFifteenKind2B_0E0B(void)
{
    StartFifteenKind2B_0E0B(cpu.x);
}

/*
 * @routine 12DD:037 StateWhileOwner12DD
 * @run 71:C037
 * WHAT IT DOES: While slot $10 holds kind $2C the state's handler runs; else the object goes.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StateWhileOwner12DD(uint8_t slot)
{
    JumpTableLayout layout = { 0xC053 };

    lda(RD8(OBJ->kind[0x10]));
    cmp(cpu.a, 0x2C);
    if (cpu.a != 0x2C) {
        ClearObject(slot);
        return;
    }
    {
        Jump jump = JumpByStateTableBody(slot, &layout);

        jump_shim(jump.end, jump.target);
    }
}

void abi_StateWhileOwner12DD(void)
{
    StateWhileOwner12DD(cpu.x);
}

/*
 * @routine 16CC:23C StateWhileOwner16CC
 * @run 70:BA3C
 * WHAT IT DOES: While slot $16 holds kind $23 the state's handler runs; else the object goes.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StateWhileOwner16CC(uint8_t slot)
{
    JumpTableLayout layout = { 0xBA58 };

    lda(RD8(OBJ->kind[0x16]));
    cmp(cpu.a, 0x23);
    if (cpu.a != 0x23) {
        ClearObject(slot);
        return;
    }
    {
        Jump jump = JumpByStateTableBody(slot, &layout);

        jump_shim(jump.end, jump.target);
    }
}

void abi_StateWhileOwner16CC(void)
{
    StateWhileOwner16CC(cpu.x);
}

/*
 * @routine 16CB:14B CountHitsThenState16CB
 * @run 70:B14B
 * WHAT IT DOES: Counts OBJ_WORK_J, runs $B175, $B195 and $B1F4, then the state's handler.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CountHitsThenState16CB(uint8_t slot)
{
    JumpTableLayout layout = { 0xB169 };

    INC8(OBJ->work_j[slot]);
    call_routine(0xB151, 0xB175);
    call_routine(0xB154, 0xB195);
    call_routine(0xB157, 0xB1F4);
    {
        Jump jump = JumpByStateTableBody(slot, &layout);

        jump_shim(jump.end, jump.target);
    }
}

void abi_CountHitsThenState16CB(void)
{
    CountHitsThenState16CB(cpu.x);
}

/*
 * @routine 0E0B:6CD KeepLastPointLandSubstate0E0B
 * @run 70:B6CD
 * WHAT IT DOES: Sets PlayerKeepsLastPoint, stops on landing, then the substate's handler.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void KeepLastPointLandSubstate0E0B(uint8_t slot)
{
    JumpTableLayout layout = { 0xB6E9 };

    lda(0x01);
    wr8(PlayerKeepsLastPoint, cpu.a);
    lda(0x04);
    jsr_push(0xB6D7);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), 0x01);
    jsr_pop();
    {
        Jump jump = JumpByPhaseTableBody(slot, &layout);

        jump_shim(jump.end, jump.target);
    }
}

void abi_KeepLastPointLandSubstate0E0B(void)
{
    KeepLastPointLandSubstate0E0B(cpu.x);
}
