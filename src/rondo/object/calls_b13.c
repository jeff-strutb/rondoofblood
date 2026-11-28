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
#include "calls_b13.h"

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

static void move_y(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    axis_move_exit_shim(ApplyVelocityY(slot));
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
static void add_a(uint8_t value, bool carry_in)
{
    Sum sum = add_bytes(cpu.a, value, carry_in);

    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
}

static void pull_x(void)
{
    cpu.x = pull8();
    abi_nz(cpu.x);
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

/*
 * @routine 0EBA:7A9 FallToLineInA0EBA
 * @run 70:AFA9
 * WHAT IT DOES: Falls toward the line given in A, gravity taken off as it goes; at the line it stops there (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallToLineInA0EBA(uint8_t slot)
{
    WR8(ZP->temp00, cpu.a);
    move_y(slot, 0xAFAE);
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, RD8(ZP->temp00));
    if ((cpu.p & P_Z) == 0 && (cpu.p & P_C) != 0) {
        lda(RD8(OBJ->vy_lo[slot]));
    {
        Sum sum = sub_bytes(cpu.a, 0x40, true);

        lda(sum.value);
        abi_flag(P_C, sum.carry);
        abi_flag(P_V, sum.overflow);
    }
        WR8(OBJ->vy_lo[slot], cpu.a);
        lda(RD8(OBJ->vy_hi[slot]));
    {
        Sum sum = sub_bytes(cpu.a, 0x00, (cpu.p & P_C) != 0);

        lda(sum.value);
        abi_flag(P_C, sum.carry);
        abi_flag(P_V, sum.overflow);
    }
        WR8(OBJ->vy_hi[slot], cpu.a);
        return;
    }
    lda(RD8(ZP->temp00));
    WR8(OBJ->y_lo[slot], cpu.a);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
}

void abi_FallToLineInA0EBA(void)
{
    FallToLineInA0EBA(cpu.x);
}

/*
 * @routine 16CB:195 ShudderWhenHit16CB
 * @run 70:B195
 * WHAT IT DOES: A hit of 5 or more shakes the object for 16 counts of OBJ_WORK_M (sound $8D); in state 2 it also goes to state 4 (secret stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ShudderWhenHit16CB(uint8_t slot)
{
    lda(RD8(OBJ->work_m[slot]));
    if (cpu.a != 0) {
        call_routine(0xB19D, 0xB1C2);
        abi_nz(DEC8(OBJ->work_m[slot]));
        return;
    }
    lda(RD8(OBJ->hit_taken[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a < 0x05) {
        return;
    }
    lda(0x10);
    WR8(OBJ->work_m[slot], cpu.a);
    sound(0x8D, 0xB1B2);
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x02);
    if (cpu.a != 0x02) {
        return;
    }
    lda(0x04);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_ShudderWhenHit16CB(void)
{
    ShudderWhenHit16CB(cpu.x);
}

/*
 * @routine 0EBF:699 SwayDownThenState0EBF
 * @run 71:D699
 * WHAT IT DOES: Drifts down swaying by the cosine of OBJ_WORK_J; above Y $E0 on the first screen the state counts on (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SwayDownThenState0EBF(uint8_t slot)
{
    lda(0xA0);
    WR8(ZP->temp00, cpu.a);
    lda(0x00);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_j[slot]));
    jsr_push(0xD6A7);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    lda(rd8(zProduct2));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8(zProduct1));
    WR8(OBJ->vx_lo[slot], cpu.a);
    move_x(slot, 0xD6B4);
    INC8(OBJ->work_j[slot]);
    move(slot, 0xD6BA);
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xE0);
    if (cpu.a >= 0xE0) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_SwayDownThenState0EBF(void)
{
    SwayDownThenState0EBF(cpu.x);
}

/*
 * @routine 117F:20B OrbitPrevious117F
 * @run 71:D20B
 * WHAT IT DOES: Placed on the previous slot (or the one before, 4 down), then out at angle OBJ_WORK_E (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void OrbitPrevious117F(uint8_t slot)
{
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y - 1);
    abi_nz(cpu.y);
    lda(RD8(OBJ->work_m[slot]));
    if (cpu.a != 0) {
        jsr_push(0xD216);
        copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
        jsr_pop();
    } else {
        cpu.y = (uint8_t)(cpu.y - 1);
        abi_nz(cpu.y);
        jsr_push(0xD21C);
        copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
        jsr_pop();
        lda(RD8(OBJ->y_lo[slot]));
        add_a(0x04, false);
        WR8(OBJ->y_lo[slot], cpu.a);
        if ((cpu.p & P_C) != 0) {
            abi_nz(INC8(OBJ->y_hi[slot]));
        }
    }
    lda(RD8(OBJ->work_k[slot]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_j[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    velocity_from_a(slot, 0xD23A);
    move(slot, 0xD23D);
}

void abi_OrbitPrevious117F(void)
{
    OrbitPrevious117F(cpu.x);
}

/*
 * @routine 12DC:759 TurnThenSubstate12DC
 * @run 70:BF59
 * WHAT IT DOES: Turns OBJ_WORK_D toward the facing's angle in eighths, clears OBJ_WORK_E-F and goes to the substate's handler (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void TurnThenSubstate12DC(uint8_t slot)
{
    TurnAngleResult turned;
    JumpTableLayout layout = { 0xBF8A };

    cpu.y = RD8(OBJ->facing_left[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xBCD9 + cpu.y)));
    WR8(ZP->temp02, cpu.a);
    lda(0x08);
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xBF6D);
    turned = TurnAngleToward();
    turn_angle_exit_shim(turned);
    jsr_pop();
    lda((uint8_t)(cpu.a & 0xF8));
    WR8(OBJ->work_d[slot], cpu.a);
    WR8(OBJ->work_e[slot], 0);
    WR8(OBJ->work_f[slot], 0);
    {
        Jump jump = JumpByPhaseTableBody(slot, &layout);

        jump_shim(jump.end, jump.target);
    }
}

void abi_TurnThenSubstate12DC(void)
{
    TurnThenSubstate12DC(cpu.x);
}

/*
 * @routine 0EBC:382 ClearOffScreenElseState0EBC
 * @run 70:BB82
 * WHAT IT DOES: An object below the first screen, or off its right edge, is cleared; else its state's handler runs
 * (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ClearOffScreenElseState0EBC(uint8_t slot)
{
    JumpTableLayout layout = { 0xBBB3 };

    lda(RD8(OBJ->y_hi[slot]));
    cmp(cpu.a, 0x01);
    if (cpu.a != 0x01) {
        lda(RD8(OBJ->y_lo[slot]));
        cmp(cpu.a, 0xE0);
        if (cpu.a < 0xE0) {
            lda(RD8(OBJ->x_hi[slot]));
            if (cpu.a != 0 && (cpu.a & 0x80) == 0) {
                lda(RD8(OBJ->x_lo[slot]));
                cmp(cpu.a, 0x18);
                if (cpu.a >= 0x18) {
                    ClearObject(slot);
                    return;
                }
            }
            {
                Jump jump = JumpByStateTableBody(slot, &layout);

                jump_shim(jump.end, jump.target);
            }
            return;
        }
    }
    ClearObject(slot);
}

void abi_ClearOffScreenElseState0EBC(void)
{
    ClearOffScreenElseState0EBC(cpu.x);
}

/*
 * @routine 12DC:3A8 LoadWithEightStreamers12DC
 * @run 70:BBA8
 * WHAT IT DOES: Loads, fills the 240 bytes at $BA41 with $C0, and starts eight numbered kind $2D objects in the next slots (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadWithEightStreamers12DC(uint8_t slot)
{
    load_record(slot, 0xBBAB);
    lda(0x80);
    WR8(OBJ->work_k[slot], cpu.a);
    lda(0xC0);
    WR8(OBJ->work_d[slot], cpu.a);
    cpu.y = 0;
    do {
        wr8((uint16_t)(0xBA41 + cpu.y), cpu.a);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0xF0);
    } while (cpu.y < 0xF0);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    lda(0x08);
    WR8(ZP->temp00, cpu.a);
    WR8(ZP->temp01, 0);
    do {
        clear_slot_in_y(0xBBCA);
        start_at_place(0x2D, 0xBBCF);
        lda(RD8(ZP->temp01));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        cpu.y = (uint8_t)(cpu.y + 1);
        abi_nz(cpu.y);
        INC8(ZP->temp01);
        abi_nz(DEC8(ZP->temp00));
    } while ((cpu.p & P_Z) == 0);
}

void abi_LoadWithEightStreamers12DC(void)
{
    LoadWithEightStreamers12DC(cpu.x);
}

/*
 * @routine 13FB:420 ListThenWakeWithMusic13FB
 * @run 70:B420
 * WHAT IT DOES: Runs $B48C and $B4A7 and the list at $B455; then the sound stopped, CD track 3 repeating, the object hit-tested, state on (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenWakeWithMusic13FB(uint8_t slot)
{
    call_routine(0xB423, 0xB48C);
    call_routine(0xB426, 0xB4A7);
    step_list(slot, 0xB455, true, 0xB431);
    if (!list_ended()) {
        return;
    }
    RMW8(ZP->field8e, old, old | 0x10);
    sound(0xFE, 0xB43A);
    RMW8(ZP->field8e, old, old & 0xF7);
    RMW8(ZP->field8e, old, old & 0xEF);
    lda(0x03);
    jsr_push(0xB443);
    play_track_exit_shim(CdPlayRepeat(cpu.a, cpu.x, cpu.y));
    jsr_pop();
    lda(RD8(ZP->field8e));
    lda((uint8_t)(cpu.a | 0x04));
    WR8(ZP->field8e, cpu.a);
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    INC8(OBJ->state[slot]);
    WR8(OBJ->substate[slot], 0);
}

void abi_ListThenWakeWithMusic13FB(void)
{
    ListThenWakeWithMusic13FB(cpu.x);
}

/*
 * @routine 12DC:7EF DissolveStepWorkO12DC
 * @run 70:BFEF
 * WHAT IT DOES: Every other count of OBJ_WORK_H one step of a dissolve over 13 patterns, its phase OBJ_WORK_O (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DissolveStepWorkO12DC(uint8_t slot)
{
    lda(RD8(OBJ->work_h[slot]));
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->work_o[slot]));
    add_a(0x13, false);
    lda((uint8_t)(cpu.a & 0x3F));
    WR8(OBJ->work_o[slot], cpu.a);
    push8(cpu.x);
    call_routine(0xC005, 0xAB3A);
    cpu.x = 0;
    do {
        lda(cpu.x);
        word_index(cpu.a);
        lda(rd8((uint16_t)(0xC01D + cpu.y)));
        WR8(ZP->bios_c.lo, cpu.a);
        lda(rd8((uint16_t)(0xC01E + cpu.y)));
        WR8(ZP->bios_c.hi, cpu.a);
        call_routine(0xC016, 0xAB65);
        cpu.x = (uint8_t)(cpu.x + 1);
        cmp(cpu.x, 0x0D);
    } while (cpu.x < 0x0D);
    pull_x();
}

void abi_DissolveStepWorkO12DC(void)
{
    DissolveStepWorkO12DC(cpu.x);
}
