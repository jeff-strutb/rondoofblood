/*
 * More routines of Stage 07's overlays: the tables of state and substate steps, a hearts pickup, guards held around
 * their owner, walkers that turn at edges, a chain of nine links and its trail of angles, a stood-on platform, and the
 * tile animation that follows the scroll.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "ops6280.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"
#include "richter.h"
#include "stage_ops_c.h"
#include "stage07_more_c.h"

/*
 * @routine 0BFB:0C6 AddFiftyHeartsOnHit0BFB
 * @run 70:B0C6
 * WHAT IT DOES: In state 0 becomes kind $84 when zField92 bit 0 is set, else loads its record and sets its Y low to
 * $32; later, on a hit of 1 to 4 sounds $0D, adds 50 to the hearts (zField94, BCD, at most 99), runs the score
 * blend at $9181 and removes itself (Stage 07's overlay).
 * HOW: A test of the state; a decimal add with a cap; BlendStageScoreAddRows, ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void AddFiftyHeartsOnHit0BFB(void)
{
    op_lda(RD8(OBJ->state[cpu.x]));
    if (flag_z()) {
        op_lda(RD8(ZP->field92));
        op_and(0x01);
        if (!flag_z()) {
            op_lda(0x84);
            WR8(OBJ->kind[cpu.x], cpu.a);
            op_lda(RD8(ZP->field92));
            op_ora(0x01);
            WR8(ZP->field92, cpu.a);
            return;
        }
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xB0E3);
            last = LoadObjectRecord(cpu.x, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(0x32);
        WR8(OBJ->y_lo[cpu.x], cpu.a);
        return;
    }
    op_lda(RD8(OBJ->hit_taken[cpu.x]));
    if (!flag_z()) {
        op_cmp(0x05);
        if (!flag_c()) {
            op_lda(0x0D);
            {
                uint8_t command;

                jsr_push(0xB0F7);
                command = cpu.a;
                sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
                jsr_pop();
            }
            op_sed();
            op_lda(RD8(ZP->field94));
            op_clc();
            op_adc(0x50);
            WR8(ZP->field94, cpu.a);
            if (flag_c()) {
                op_lda(0x99);
                WR8(ZP->field94, cpu.a);
            }
            op_cld();
            call_routine(0xB109, 0x9181);
            ClearObject(cpu.x);
            return;
        }
    }
}

void abi_AddFiftyHeartsOnHit0BFB(void)
{
    AddFiftyHeartsOnHit0BFB();
}

/*
 * @routine 0BFB:1D9 ShoveByState0BFB
 * @run 70:B1D9
 * WHAT IT DOES: Runs the step of the object's state: 0 loads its record, 1 ShoveToEdgeWhenHit0BFB, 2 does nothing
 * (Stage 07's overlay).
 * HOW: Twice the state indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ShoveByState0BFBSteps = 0xB1EB,   /* the routine for each state, low and high byte */
};

void ShoveByState0BFB(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ShoveByState0BFBSteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ShoveByState0BFBSteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ShoveByState0BFB(void)
{
    ShoveByState0BFB(cpu.x);
}

/*
 * @routine 0BFB:2A5 BreakPairByState0BFB
 * @run 70:B2A5
 * WHAT IT DOES: Runs the step of the object's state: 0 LoadRecordBoxDown08_0BFB, 1 BreakWithNextSlot0BFB, 2 does
 * nothing (Stage 07's overlay).
 * HOW: Twice the state indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    BreakPairByState0BFBSteps = 0xB2B7,   /* the routine for each state, low and high byte */
};

void BreakPairByState0BFB(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(BreakPairByState0BFBSteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(BreakPairByState0BFBSteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_BreakPairByState0BFB(void)
{
    BreakPairByState0BFB(cpu.x);
}

/*
 * @routine 0BFB:2EE GuardThenFallByState0BFB
 * @run 70:B2EE
 * WHAT IT DOES: Runs the step of the object's state: 0 LoadRecordBoxDown4B_0BFB, 1 SoundAndGuardOnHit0BFB, 2
 * FallThenClearLow0BFB (Stage 07's overlay).
 * HOW: Twice the state indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    GuardThenFallByState0BFBSteps = 0xB300,   /* the routine for each state, low and high byte */
};

void GuardThenFallByState0BFB(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(GuardThenFallByState0BFBSteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(GuardThenFallByState0BFBSteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_GuardThenFallByState0BFB(void)
{
    GuardThenFallByState0BFB(cpu.x);
}

/*
 * @routine 0BFB:4B0 HoldGuardsAroundIt0BFB
 * @run 70:B4B0
 * WHAT IT DOES: Opens once its guards are down (OpenWhenGuardsDown0BFB), takes state 5 when hit in state 4, and
 * runs its state's step ($B4FF); then in states 4 and 5 puts each of the three slots before it that holds kind $2D
 * in a state below 2 at its own place, moved by that slot's fixed offset (Stage 07's overlay).
 * HOW: A loop over X-3 to X-1 of CopyObjectPlace and ApplyVelocity on the slot, the offsets stored as its X and Y
 * speed high bytes; zRichterObject keeps X.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    HoldGuardsAroundIt0BFBOffsets = 0xB7FE,   /* the X and Y speed high byte for each of the three guards: (0,-$26), (+$14,-6), (-$14,-6) */
};

void HoldGuardsAroundIt0BFB(void)
{
    call_routine(0xB4B3, 0xB51F);
    call_routine(0xB4B6, 0xB555);
    jsr_push(0xB4B9);
    RiseSteerByState0BFB(cpu.x);
    jsr_pop();
    op_lda(RD8(OBJ->state[cpu.x]));
    op_cmp(0x04);
    if (flag_c()) {
        op_cmp(0x06);
        if (!flag_c()) {
            WR8(ZP->temp00, 0);
            op_txa();
            op_sec();
            op_sbc(0x03);
            op_tay();
            do {
                op_lda(RD8(OBJ->kind[cpu.y]));
                op_cmp(0x2D);
                if (flag_z()) {
                    op_lda(RD8(OBJ->state[cpu.y]));
                    op_cmp(0x02);
                    if (!flag_c()) {
                        jsr_push(0xB4DC);
                        copy_place_exit_shim(CopyObjectPlace(cpu.x, cpu.y));
                        jsr_pop();
                        WR8(ZP->richter_object, cpu.x);
                        op_lda(RD8(ZP->temp00));
                        cpu.a = f_asl(cpu.a);
                        op_tax();
                        op_lda(rd8((uint16_t)(HoldGuardsAroundIt0BFBOffsets + cpu.x)));
                        WR8(OBJ->vx_hi[cpu.y], cpu.a);
                        op_lda(rd8((uint16_t)(HoldGuardsAroundIt0BFBOffsets + 1 + cpu.x)));
                        WR8(OBJ->vy_hi[cpu.y], cpu.a);
                        op_sxy();
                        jsr_push(0xB4F2);
                        axis_move_exit_shim(ApplyVelocity(cpu.x));
                        jsr_pop();
                        op_sxy();
                        op_ldx(RD8(ZP->richter_object));
                    }
                }
                op_iny();
                RMW8(ZP->temp00, v, f_inc(v));
                op_lda(RD8(ZP->temp00));
                op_cmp(0x03);
            } while (!flag_c());
        }
    }
}

void abi_HoldGuardsAroundIt0BFB(void)
{
    HoldGuardsAroundIt0BFB();
}

/*
 * @routine 0BFB:4FF RiseSteerByState0BFB
 * @run 70:B4FF
 * WHAT IT DOES: Runs the step of the object's state: 0 loads its record, 1 RiseAtScreen3_0BFB, 2 RiseToListEnd0BFB,
 * 3 OpenAndStartList0BFB, 4 SteerTowardPlayer0BFB, 5 BackAwayFromPlayer0BFB, 6 DefeatBySubstate0BFB (Stage 07's
 * overlay).
 * HOW: Twice the state indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RiseSteerByState0BFBSteps = 0xB511,   /* the routine for each state, low and high byte */
};

void RiseSteerByState0BFB(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RiseSteerByState0BFBSteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RiseSteerByState0BFBSteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_RiseSteerByState0BFB(void)
{
    RiseSteerByState0BFB(cpu.x);
}

/*
 * @routine 0BFB:56C JustLoadRecord0BFB
 * @run 70:B56C
 * WHAT IT DOES: Loads the object's record (Stage 07's overlay).
 * HOW: LoadObjectRecord.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void JustLoadRecord0BFB(void)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xB56F);
        last = LoadObjectRecord(cpu.x, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
}

void abi_JustLoadRecord0BFB(void)
{
    JustLoadRecord0BFB();
}

/*
 * @routine 0BFB:5C9 OpenAndStartList0BFB
 * @run 70:B5C9
 * WHAT IT DOES: Moves by its Y speed and plays the frame list at $B5F8; when the list ends sounds $57, starts the
 * objects of its list (StartObjectsFromList0BFB), sets OBJ_HITBOX_ON 2 and OBJ_PACE bits 4 and 6, restarts its
 * animation and steps the state on (Stage 07's overlay).
 * HOW: ApplyVelocityY, StepObjectFrames, then stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void OpenAndStartList0BFB(void)
{
    jsr_push(0xB5CC);
    axis_move_exit_shim(ApplyVelocityY(cpu.x));
    jsr_pop();
    op_lda(0xF8);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xB5);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xB5D7);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        op_lda(0x57);
        {
            uint8_t command;

            jsr_push(0xB5DE);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
            jsr_pop();
        }
        call_routine(0xB5E1, 0xB601);
        op_lda(0x02);
        WR8(OBJ->hitbox_on[cpu.x], cpu.a);
        op_lda(RD8(OBJ->pace[cpu.x]));
        op_ora(0x50);
        WR8(OBJ->pace[cpu.x], cpu.a);
        WR8(OBJ->anim_step[cpu.x], 0);
        WR8(OBJ->anim_timer[cpu.x], 0);
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
    }
}

void abi_OpenAndStartList0BFB(void)
{
    OpenAndStartList0BFB();
}

/*
 * @routine 0BFB:6FC DefeatBySubstate0BFB
 * @run 70:B6FC
 * WHAT IT DOES: Runs the step of the object's substate: 0 DefeatSoundsAndRecord0BFB, 1 CountThenFall0BFB, 2
 * MoveThenOpenScroll0BFB (Stage 07's overlay).
 * HOW: Twice the substate indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    DefeatBySubstate0BFBSteps = 0xB70E,   /* the routine for each substate, low and high byte */
};

void DefeatBySubstate0BFB(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(DefeatBySubstate0BFBSteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(DefeatBySubstate0BFBSteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_DefeatBySubstate0BFB(void)
{
    DefeatBySubstate0BFB(cpu.x);
}

/*
 * @routine 0BFB:743 MoveThenOpenScroll0BFB
 * @run 70:B743
 * WHAT IT DOES: Moves by its Y speed, counts OBJ_WORK_H down and drops pieces on its counts (PieceOnCount0BFB);
 * while its Y is $100 to $1BF sets zField8E bit 7, sets the scroll's X range to high bytes 1 to 5 (ScrollXMinHi,
 * ScrollXMaxHi) and removes itself (Stage 07's overlay).
 * HOW: ApplyVelocityY, DEC, two compares of Y, SMB7, ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void MoveThenOpenScroll0BFB(void)
{
    jsr_push(0xB746);
    axis_move_exit_shim(ApplyVelocityY(cpu.x));
    jsr_pop();
    RMW8(OBJ->work_h[cpu.x], v, f_dec(v));
    call_routine(0xB74C, 0xB76A);
    op_lda(RD8(OBJ->y_hi[cpu.x]));
    op_cmp(0x01);
    if (flag_z()) {
        op_lda(RD8(OBJ->y_lo[cpu.x]));
        op_cmp(0xC0);
        if (!flag_c()) {
            RMW8(ZP->field8e, v, v | 0x80);
            op_lda(0x01);
            WR8(SCROLLLIM->x_min_hi, cpu.a);
            op_lda(0x05);
            WR8(SCROLLLIM->x_max.hi, cpu.a);
            ClearObject(cpu.x);
            return;
        }
    }
}

void abi_MoveThenOpenScroll0BFB(void)
{
    MoveThenOpenScroll0BFB();
}

/*
 * @routine 0BFB:7BE AngleFrameByState0BFB
 * @run 70:B7BE
 * WHAT IT DOES: Runs the step of the object's state: 0 loads its record, 1 FrameByAngleUntilHit0BFB, 2
 * FallDroppingPieces0BFC (Stage 07's overlay).
 * HOW: Twice the state indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    AngleFrameByState0BFBSteps = 0xB7D0,   /* the routine for each state, low and high byte */
};

void AngleFrameByState0BFB(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(AngleFrameByState0BFBSteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(AngleFrameByState0BFBSteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_AngleFrameByState0BFB(void)
{
    AngleFrameByState0BFB(cpu.x);
}

/*
 * @routine 0BFC:039 OrbitByState0BFC
 * @run 70:B839
 * WHAT IT DOES: Runs the step of the object's state: 0 StartAngleByArg0BFC, 1 OrbitSlot4C0BFC, 2 FallFromRest0BFC
 * (Stage 07's overlay).
 * HOW: Twice the state indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    OrbitByState0BFCSteps = 0xB84B,   /* the routine for each state, low and high byte */
};

void OrbitByState0BFC(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(OrbitByState0BFCSteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(OrbitByState0BFCSteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_OrbitByState0BFC(void)
{
    OrbitByState0BFC(cpu.x);
}

/*
 * @routine 0BFC:474 LoadRecordWait50_0BFC
 * @run 70:BC74
 * WHAT IT DOES: Loads the object's record and sets OBJ_WORK_D to $50 (Stage 07's overlay).
 * HOW: LoadObjectRecord and a store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LoadRecordWait50_0BFC(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xBC77);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x50);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_LoadRecordWait50_0BFC(void)
{
    LoadRecordWait50_0BFC(cpu.x);
}

/*
 * @routine 0BFC:47D LandThenStateOn0BFC
 * @run 70:BC7D
 * WHAT IT DOES: Falls and lands with height 6, stopping dead; on landing steps the state on (Stage 07's overlay).
 * HOW: StopOnLanding, then INC on carry.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
void LandThenStateOn0BFC(uint8_t slot)
{
    op_lda(0x06);
    jsr_push(0xBC82);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_LandThenStateOn0BFC(void)
{
    LandThenStateOn0BFC(cpu.x);
}

/*
 * @routine 0BFC:488 WalkTowardUntilNear0BFC
 * @run 70:BC88
 * WHAT IT DOES: Walks a step (StepWalkByWorkE0BFC) and keeps its facing in OBJ_WORK_E as the way to walk; within
 * $4C of the player along X goes to state 3; then checks the ground ahead (TurnAtEdgeOrCount0BFC) (Stage 07's
 * overlay).
 * HOW: The walk, DistanceXToPlayer and a compare, then a jump to $BD2F.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void WalkTowardUntilNear0BFC(void)
{
    jsr_push(0xBC8B);
    StepWalkByWorkE0BFC();
    jsr_pop();
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    WR8(OBJ->work_e[cpu.x], cpu.a);
    jsr_push(0xBC94);
    distance_exit_shim(DistanceXToPlayer(cpu.x));
    jsr_pop();
    op_cmp(0x4C);
    if (!flag_c()) {
        op_lda(0x03);
        WR8(OBJ->state[cpu.x], cpu.a);
    }
    TurnAtEdgeOrCount0BFC(cpu.x);
}

void abi_WalkTowardUntilNear0BFC(void)
{
    WalkTowardUntilNear0BFC();
}

/*
 * @routine 0BFC:4A0 StepWalkByWorkE0BFC
 * @run 70:BCA0
 * WHAT IT DOES: Faces the player and plays frame list 0; on the list's steps that report carry moves by an X speed
 * of -4 or +4 by OBJ_WORK_E and moves down one pixel (Stage 07's overlay).
 * HOW: FacePlayerPlayList0BFC with Y 0; a table read; ApplyVelocityX; a 16-bit INC of Y.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    StepWalkByWorkE0BFCSpeeds = 0xBCBB,   /* the X speed high byte by OBJ_WORK_E: -4, +4 */
};

void StepWalkByWorkE0BFC(void)
{
    op_cly();
    call_routine(0xBCA4, 0xBCCE);
    if (flag_c()) {
        op_ldy(RD8(OBJ->work_e[cpu.x]));
        op_lda(rd8((uint16_t)(StepWalkByWorkE0BFCSpeeds + cpu.y)));
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        jsr_push(0xBCB2);
        axis_move_exit_shim(ApplyVelocityX(cpu.x));
        jsr_pop();
        RMW8(OBJ->y_lo[cpu.x], v, f_inc(v));
        if (flag_z()) {
            RMW8(OBJ->y_hi[cpu.x], v, f_inc(v));
        }
    }
}

void abi_StepWalkByWorkE0BFC(void)
{
    StepWalkByWorkE0BFC();
}

/*
 * @routine 0BFC:52F TurnAtEdgeOrCount0BFC
 * @run 70:BD2F
 * WHAT IT DOES: Probes the terrain with height 6: with no bit 0 for request $09 or with bit 1 for request $22,
 * restarts the animation in state 5, substate 0; otherwise counts OBJ_WORK_D down and at 0 restarts it in state 4
 * (Stage 07's overlay).
 * HOW: Two ProbeTerrain calls with zProduct1 6; TileHit and the result tested by bit.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void TurnAtEdgeOrCount0BFC(uint8_t slot)
{
    op_lda(0x06);
    WR8(ZP->product.mid, cpu.a);
    op_lda(0x09);
    jsr_push(0xBD38);
    probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
    jsr_pop();
    op_lda(RD8(LEVEL->tile_hit[0]));
    op_and(0x01);
    if (!flag_z()) {
        op_lda(0x06);
        WR8(ZP->product.mid, cpu.a);
        op_lda(0x22);
        jsr_push(0xBD48);
        probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
        jsr_pop();
        op_and(0x02);
        if (flag_z()) {
            goto at_BD5B;
        }
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->substate[slot], 0);
    op_lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
    return;
at_BD5B:
    RMW8(OBJ->work_d[slot], v, f_dec(v));
    if (flag_z()) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        op_lda(0x04);
        WR8(OBJ->state[slot], cpu.a);
    }
}

void abi_TurnAtEdgeOrCount0BFC(void)
{
    TurnAtEdgeOrCount0BFC(cpu.x);
}

/*
 * @routine 0BFC:56C WalkAwayUntilFar0BFC
 * @run 70:BD6C
 * WHAT IT DOES: Walks a step (StepWalkByWorkE0BFC) with OBJ_WORK_E the opposite of its facing; at $6C or more from
 * the player along X goes to state 2; then checks the ground ahead (TurnAtEdgeOrCount0BFC) (Stage 07's overlay).
 * HOW: The walk, EOR #1, DistanceXToPlayer and a compare, then a jump to $BD2F.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void WalkAwayUntilFar0BFC(void)
{
    jsr_push(0xBD6F);
    StepWalkByWorkE0BFC();
    jsr_pop();
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    op_eor(0x01);
    WR8(OBJ->work_e[cpu.x], cpu.a);
    jsr_push(0xBD7A);
    distance_exit_shim(DistanceXToPlayer(cpu.x));
    jsr_pop();
    op_cmp(0x6C);
    if (flag_c()) {
        op_lda(0x02);
        WR8(OBJ->state[cpu.x], cpu.a);
    }
    TurnAtEdgeOrCount0BFC(cpu.x);
}

void abi_WalkAwayUntilFar0BFC(void)
{
    WalkAwayUntilFar0BFC();
}

/*
 * @routine 0BFC:586 ThrowOnFrame47_0BFC
 * @run 70:BD86
 * WHAT IT DOES: Faces the player and plays frame list 2; on a step with carry at frame $47 starts kind $39 at its
 * place in a free slot from $30; at the list's end restarts its animation in state 3, steps OBJ_WORK_F on and sets
 * OBJ_WORK_D to the wait its low two bits pick (Stage 07's overlay).
 * HOW: FacePlayerPlayList0BFC with Y 2; FindFreeSlotFrom30, StartObjectAtPlace; a table read.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ThrowOnFrame47_0BFCWaits = 0xBCBD,   /* OBJ_WORK_D for each turn of OBJ_WORK_F's low two bits: $80, 8, 8, $40 */
};

void ThrowOnFrame47_0BFC(void)
{
    op_ldy(0x02);
    call_routine(0xBD8B, 0xBCCE);
    if (!flag_z()) {
        if (!flag_c()) {
            goto at_BDBB;
        }
        op_lda(RD8(OBJ->frame[cpu.x]));
        op_cmp(0x47);
        if (!flag_z()) {
            goto at_BDBB;
        }
        {
            uint8_t a;
            bool found;
            uint8_t found_slot;

            jsr_push(0xBD99);
            found_slot = FindFreeSlotFrom30(&a, &found);
            find_slot_exit_shim(found_slot, a, found);
            jsr_pop();
        }
        if (!flag_c()) {
            goto at_BDBB;
        }
        op_lda(0x39);
        jsr_push(0xBDA0);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
        jsr_pop();
        return;
    }
    WR8(OBJ->anim_step[cpu.x], 0);
    WR8(OBJ->anim_timer[cpu.x], 0);
    op_lda(0x03);
    WR8(OBJ->state[cpu.x], cpu.a);
    RMW8(OBJ->work_f[cpu.x], v, f_inc(v));
    op_lda(RD8(OBJ->work_f[cpu.x]));
    op_and(0x03);
    op_tay();
    op_lda(rd8((uint16_t)(ThrowOnFrame47_0BFCWaits + cpu.y)));
    WR8(OBJ->work_d[cpu.x], cpu.a);
at_BDBB:
    ;
}

void abi_ThrowOnFrame47_0BFC(void)
{
    ThrowOnFrame47_0BFC();
}

/*
 * @routine 0BFC:5C9 LeapBySubstate0BFC
 * @run 70:BDC9
 * WHAT IT DOES: Runs the step of the object's substate: 0 LeapAfterList0BFC, 1 LandThenSubstate0BFC, 2
 * LandListThenState3_0BFC (Stage 07's overlay).
 * HOW: Twice the substate indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    LeapBySubstate0BFCSteps = 0xBDDB,   /* the routine for each substate, low and high byte */
};

void LeapBySubstate0BFC(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(LeapBySubstate0BFCSteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(LeapBySubstate0BFCSteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_LeapBySubstate0BFC(void)
{
    LeapBySubstate0BFC(cpu.x);
}

/*
 * @routine 0BFC:5E1 LeapAfterList0BFC
 * @run 70:BDE1
 * WHAT IT DOES: Faces the player and plays frame list 4; at its end leaps: X speed -2 or +2 by OBJ_WORK_E (by its
 * facing turned when OBJ_START_ARG is not 0), Y speed -3, animation restarted and substate stepped on (Stage 07's
 * overlay).
 * HOW: FacePlayerPlayList0BFC with Y 4; table reads; stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    LeapAfterList0BFCSpeeds = 0xBE17,   /* the X speed high byte by direction: -2, +2 */
};

void LeapAfterList0BFC(void)
{
    op_ldy(0x04);
    call_routine(0xBDE6, 0xBCCE);
    if (flag_z()) {
        op_ldy(RD8(OBJ->work_e[cpu.x]));
        op_lda(rd8((uint16_t)(LeapAfterList0BFCSpeeds + cpu.y)));
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        op_lda(0xFD);
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        WR8(OBJ->vx_lo[cpu.x], 0);
        WR8(OBJ->vy_lo[cpu.x], 0);
        WR8(OBJ->anim_step[cpu.x], 0);
        WR8(OBJ->anim_timer[cpu.x], 0);
        RMW8(OBJ->substate[cpu.x], v, f_inc(v));
        op_lda(RD8(OBJ->start_arg[cpu.x]));
        if (!flag_z()) {
            op_lda(RD8(OBJ->facing_left[cpu.x]));
            op_eor(0x01);
            op_tay();
            op_lda(rd8((uint16_t)(LeapAfterList0BFCSpeeds + cpu.y)));
            WR8(OBJ->vx_hi[cpu.x], cpu.a);
        }
    }
}

void abi_LeapAfterList0BFC(void)
{
    LeapAfterList0BFC();
}

/*
 * @routine 0BFC:624 LandListThenState3_0BFC
 * @run 70:BE24
 * WHAT IT DOES: Faces the player and plays frame list 6; at its end restarts its animation in state 3, substate 0
 * (Stage 07's overlay).
 * HOW: FacePlayerPlayList0BFC with Y 6, then stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LandListThenState3_0BFC(void)
{
    op_ldy(0x06);
    call_routine(0xBE29, 0xBCCE);
    if (flag_z()) {
        op_lda(0x03);
        WR8(OBJ->state[cpu.x], cpu.a);
        WR8(OBJ->anim_step[cpu.x], 0);
        WR8(OBJ->anim_timer[cpu.x], 0);
        WR8(OBJ->substate[cpu.x], 0);
    }
}

void abi_LandListThenState3_0BFC(void)
{
    LandListThenState3_0BFC();
}

/*
 * @routine 0BFD:05F LoadAndStartFollowers0BFD
 * @run 71:C05F
 * WHAT IT DOES: Loads its record; with OBJ_START_ARG below $80 starts that many objects of kind $3C at its place in
 * the slots after it (OBJ_START_ARG $81 up, OBJ_WORK_M its slot, OBJ_WORK_N its slot plus the count), counting
 * OBJ_START_ARG down to 0; then clears bit 7 of OBJ_START_ARG, faces left when its bit 0 is clear, and sets its
 * side speed, a random wait and a random OBJ_WORK_F:OBJ_WORK_G (Stage 07's overlay).
 * HOW: A loop of ClearObjectInY and StartObjectAtPlace; SideStepByArg0BFD, RandomWaitDE0BFD, RandomSpeedFG0BFD.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void LoadAndStartFollowers0BFD(void)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xC062);
        last = LoadObjectRecord(cpu.x, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(RD8(OBJ->start_arg[cpu.x]));
    if (!flag_n()) {
        op_txa();
        WR8(OBJ->work_m[cpu.x], cpu.a);
        op_clc();
        op_adc(RD8(OBJ->start_arg[cpu.x]));
        WR8(OBJ->work_n[cpu.x], cpu.a);
        op_txa();
        op_tay();
        op_iny();
        op_lda(0x81);
        WR8(ZP->temp00, cpu.a);
        do {
            jsr_push(0xC07C);
            ClearObjectInY(cpu.y);
            jsr_pop();
            op_lda(0x3C);
            jsr_push(0xC081);
            copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
            jsr_pop();
            op_lda(RD8(ZP->temp00));
            WR8(OBJ->start_arg[cpu.y], cpu.a);
            op_txa();
            WR8(OBJ->work_m[cpu.y], cpu.a);
            op_lda(RD8(OBJ->work_n[cpu.x]));
            WR8(OBJ->work_n[cpu.y], cpu.a);
            RMW8(ZP->temp00, v, f_inc(v));
            op_iny();
            RMW8(OBJ->start_arg[cpu.x], v, f_dec(v));
        } while (!flag_z());
    }
    op_lda(RD8(OBJ->start_arg[cpu.x]));
    op_and(0x7F);
    WR8(OBJ->start_arg[cpu.x], cpu.a);
    op_and(0x01);
    op_eor(0x01);
    WR8(OBJ->facing_left[cpu.x], cpu.a);
    call_routine(0xC0AA, 0xC0B1);
    call_routine(0xC0AD, 0xC0D6);
    call_routine(0xC0B0, 0xC0F2);
}

void abi_LoadAndStartFollowers0BFD(void)
{
    LoadAndStartFollowers0BFD();
}

/*
 * @routine 0BFD:10E CountWaitAndSpeed0BFD
 * @run 71:C10E
 * WHAT IT DOES: Runs both counts of the object: WaitThenState2_0BFD, then CountFGThenState3_0BFD (Stage 07's
 * overlay).
 * HOW: Two calls.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void CountWaitAndSpeed0BFD(void)
{
    jsr_push(0xC111);
    WaitThenState2_0BFD();
    jsr_pop();
    jsr_push(0xC114);
    CountFGThenState3_0BFD();
    jsr_pop();
}

void abi_CountWaitAndSpeed0BFD(void)
{
    CountWaitAndSpeed0BFD();
}

/*
 * @routine 0BFD:115 WaitThenState2_0BFD
 * @run 71:C115
 * WHAT IT DOES: With OBJ_WORK_D:OBJ_WORK_E at 0 takes a new random wait (RandomWaitDE0BFD) and restarts its
 * animation in state 2; otherwise counts it down by 1 (Stage 07's overlay).
 * HOW: An ORA test; a 16-bit subtract.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void WaitThenState2_0BFD(void)
{
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_ora(RD8(OBJ->work_e[cpu.x]));
    if (flag_z()) {
        call_routine(0xC120, 0xC0D6);
        WR8(OBJ->anim_step[cpu.x], 0);
        WR8(OBJ->anim_timer[cpu.x], 0);
        op_lda(0x02);
        WR8(OBJ->state[cpu.x], cpu.a);
        return;
    }
    op_lda(RD8(OBJ->work_e[cpu.x]));
    op_sec();
    op_sbc(0x01);
    WR8(OBJ->work_e[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_sbc(0x00);
    WR8(OBJ->work_d[cpu.x], cpu.a);
}

void abi_WaitThenState2_0BFD(void)
{
    WaitThenState2_0BFD();
}

/*
 * @routine 0BFD:13E CountFGThenState3_0BFD
 * @run 71:C13E
 * WHAT IT DOES: While its X high byte is 0, counts OBJ_WORK_F:OBJ_WORK_G down by 1; at 0 takes a new random value
 * (RandomSpeedFG0BFD) and restarts its animation in state 3, substate 0 (Stage 07's overlay).
 * HOW: A test of X high; an ORA test; a 16-bit subtract.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void CountFGThenState3_0BFD(void)
{
    op_lda(RD8(OBJ->x_hi[cpu.x]));
    if (flag_z()) {
        op_lda(RD8(OBJ->work_f[cpu.x]));
        op_ora(RD8(OBJ->work_g[cpu.x]));
        if (flag_z()) {
            call_routine(0xC14E, 0xC0F2);
            WR8(OBJ->anim_step[cpu.x], 0);
            WR8(OBJ->anim_timer[cpu.x], 0);
            WR8(OBJ->substate[cpu.x], 0);
            op_lda(0x03);
            WR8(OBJ->state[cpu.x], cpu.a);
            return;
        }
        op_lda(RD8(OBJ->work_g[cpu.x]));
        op_sec();
        op_sbc(0x01);
        WR8(OBJ->work_g[cpu.x], cpu.a);
        op_lda(RD8(OBJ->work_f[cpu.x]));
        op_sbc(0x00);
        WR8(OBJ->work_f[cpu.x], cpu.a);
    }
}

void abi_CountFGThenState3_0BFD(void)
{
    CountFGThenState3_0BFD();
}

/*
 * @routine 0BFD:1A4 ThrowBySubstate0BFD
 * @run 71:C1A4
 * WHAT IT DOES: Runs the step of the object's substate: 0 PickThrow0BFD, 1 ThrowKindByPick0BFD, 2
 * ListByPickAndJ0BFD, 3 RepeatOrStand0BFD (Stage 07's overlay).
 * HOW: Twice the substate indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ThrowBySubstate0BFDSteps = 0xC1B6,   /* the routine for each substate, low and high byte */
};

void ThrowBySubstate0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ThrowBySubstate0BFDSteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ThrowBySubstate0BFDSteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ThrowBySubstate0BFD(void)
{
    ThrowBySubstate0BFD(cpu.x);
}

/*
 * @routine 0BFD:330 LaunchFlyByState0BFD
 * @run 71:C330
 * WHAT IT DOES: Runs the step of the object's state: 0 LaunchWithKind3F0BFD, 1 WaitThenBoxOn0BFD, 2
 * FlyByListWorkD0BFD (Stage 07's overlay).
 * HOW: Twice the state indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    LaunchFlyByState0BFDSteps = 0xC342,   /* the routine for each state, low and high byte */
};

void LaunchFlyByState0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(LaunchFlyByState0BFDSteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(LaunchFlyByState0BFDSteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_LaunchFlyByState0BFD(void)
{
    LaunchFlyByState0BFD(cpu.x);
}

/*
 * @routine 0BFD:410 ByStateThenFollowY0BFD
 * @run 71:C410
 * WHAT IT DOES: Runs its state's step (BoxedByState0BFD), then takes its owner's Y (FollowOwnerY0BFD) (Stage 07's
 * overlay).
 * HOW: A call, then a jump to $C3C0.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ByStateThenFollowY0BFD(void)
{
    jsr_push(0xC413);
    BoxedByState0BFD(cpu.x);
    jsr_pop();
    run_routine(0xC3C0);
}

void abi_ByStateThenFollowY0BFD(void)
{
    ByStateThenFollowY0BFD();
}

/*
 * @routine 0BFD:416 BoxedByState0BFD
 * @run 71:C416
 * WHAT IT DOES: Runs the step of the object's state: 0 StartWithKind3FAtOne0BFD, 1 CountSoundBoxOn0BFD, 2
 * AnimateBoxedTimed0BFD (Stage 07's overlay).
 * HOW: Twice the state indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    BoxedByState0BFDSteps = 0xC428,   /* the routine for each state, low and high byte */
};

void BoxedByState0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(BoxedByState0BFDSteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(BoxedByState0BFDSteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_BoxedByState0BFD(void)
{
    BoxedByState0BFD(cpu.x);
}

/*
 * @routine 0BFD:465 CountSoundBoxOn0BFD
 * @run 71:C465
 * WHAT IT DOES: Counts OBJ_WORK_E down; at 0 sounds $26, sets OBJ_WORK_E to $40 and OBJ_HITBOX_ON to 2, and steps
 * the state on (Stage 07's overlay).
 * HOW: DEC, SoundCommand, stores.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void CountSoundBoxOn0BFD(uint8_t slot)
{
    RMW8(OBJ->work_e[slot], v, f_dec(v));
    if (flag_z()) {
        op_lda(0x26);
        {
            uint8_t command;

            jsr_push(0xC46F);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
            jsr_pop();
        }
        op_lda(0x40);
        WR8(OBJ->work_e[slot], cpu.a);
        op_lda(0x02);
        WR8(OBJ->hitbox_on[slot], cpu.a);
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_CountSoundBoxOn0BFD(void)
{
    CountSoundBoxOn0BFD(cpu.x);
}

/*
 * @routine 0BFD:543 ShiftAngleTrail0BFD
 * @run 71:C543
 * WHAT IT DOES: Takes state 4 when hit (State4SoundWhenHit0BFD), marks its parts by its blink
 * (MarkPartsByBlink0BFD) and runs its state's step (ChainByState0BFD); then shifts the overlay's 128-byte trail at
 * $C5F7 back by one and puts OBJ_WORK_D plus $80 in its first byte (Stage 07's overlay).
 * HOW: Three calls; a copy loop by Y from $7F down to 1; an add and a store.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    ShiftAngleTrail0BFDTrail = 0xC5F6,   /* the trail of past angles at $C5F7 to $C676, newest first; read from $C5F6 by Y 1 to $7F */
};

void ShiftAngleTrail0BFD(void)
{
    call_routine(0xC546, 0xC561);
    call_routine(0xC549, 0xC57D);
    jsr_push(0xC54C);
    ChainByState0BFD(cpu.x);
    jsr_pop();
    op_ldy(0x7F);
    do {
        op_lda(rd8((uint16_t)(ShiftAngleTrail0BFDTrail + cpu.y)));
        wr8((uint16_t)(ShiftAngleTrail0BFDTrail + 1 + cpu.y), cpu.a);
        op_dey();
    } while (!flag_z());
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_clc();
    op_adc(0x80);
    wr8(ShiftAngleTrail0BFDTrail + 1, cpu.a);
}

void abi_ShiftAngleTrail0BFD(void)
{
    ShiftAngleTrail0BFD();
}

/*
 * @routine 0BFD:5AA ChainByState0BFD
 * @run 71:C5AA
 * WHAT IT DOES: Runs the step of the object's state: 0 StartNineLinks0BFD, 1 SwayByRowStep0BFD, 2
 * StartKindC2ThenState1_0BFD, 3 DragThenBySubstate0BFD, 4 CollapseAndClear0BFE (Stage 07's overlay).
 * HOW: Twice the state indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ChainByState0BFDSteps = 0xC5BC,   /* the routine for each state, low and high byte */
};

void ChainByState0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ChainByState0BFDSteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ChainByState0BFDSteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ChainByState0BFD(void)
{
    ChainByState0BFD(cpu.x);
}

/*
 * @routine 0BFD:677 SwayByRowStep0BFD
 * @run 71:C677
 * WHAT IT DOES: Takes its next state on its timer, drags its chain, eases its speed and scales the cosine by its
 * row; adds the scaled cosine to OBJ_WORK_D:OBJ_WORK_E and the row's other word to OBJ_WORK_K:OBJ_WORK_L; when that
 * add carries out, picks a row 0 to 3 at random into OBJ_WORK_M and clears OBJ_WORK_D, E, K and L (Stage 07's
 * overlay).
 * HOW: NextStateOnTimer0BFD, DragChainAndMove0BFD, EaseWorkFToTarget0BFD, ScaleByLinkRow0BFD; 16-bit adds of
 * zProduct and zMultiplier; NextRandom.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void SwayByRowStep0BFD(void)
{
    call_routine(0xC67A, 0xC6FE);
    call_routine(0xC67D, 0xC741);
    call_routine(0xC680, 0xC77B);
    call_routine(0xC683, 0xC6D9);
    op_lda(RD8(OBJ->work_e[cpu.x]));
    op_clc();
    op_adc(RD8(ZP->product.mid));
    WR8(OBJ->work_e[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_adc(RD8(ZP->product.hi));
    WR8(OBJ->work_d[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_l[cpu.x]));
    op_clc();
    op_adc(RD8(ZP->multiplier.lo));
    WR8(OBJ->work_l[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_k[cpu.x]));
    op_adc(RD8(ZP->multiplier.hi));
    WR8(OBJ->work_k[cpu.x], cpu.a);
    if (flag_c()) {
        jsr_push(0xC6AA);
        random_exit_shim(NextRandom());
        jsr_pop();
        op_and(0x03);
        WR8(OBJ->work_m[cpu.x], cpu.a);
        WR8(OBJ->work_d[cpu.x], 0);
        WR8(OBJ->work_e[cpu.x], 0);
        WR8(OBJ->work_k[cpu.x], 0);
        WR8(OBJ->work_l[cpu.x], 0);
    }
}

void abi_SwayByRowStep0BFD(void)
{
    SwayByRowStep0BFD();
}

/*
 * @routine 0BFD:7EE DragThenBySubstate0BFD
 * @run 71:C7EE
 * WHAT IT DOES: Drags its chain and eases its speed, then runs the step of its substate: 0 AimOrBackToState1_0BFE,
 * 1 TurnToZeroBy2_0BFE, 2 TimedAimSound0BFE, 3 TurnTowardWorkH0BFE, 4 TurnToZeroThenState1_0BFE (Stage 07's
 * overlay).
 * HOW: DragChainAndMove0BFD, EaseWorkFToTarget0BFD; twice the substate indexes a word table.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    DragThenBySubstate0BFDSteps = 0xC806,   /* the routine for each substate, low and high byte */
};

void DragThenBySubstate0BFD(void)
{
    call_routine(0xC7F1, 0xC741);
    call_routine(0xC7F4, 0xC77B);
    op_lda(RD8(OBJ->substate[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(DragThenBySubstate0BFDSteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(DragThenBySubstate0BFDSteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_DragThenBySubstate0BFD(void)
{
    DragThenBySubstate0BFD();
}

/*
 * @routine 0BFE:010 AimOrBackToState1_0BFE
 * @run 71:C810
 * WHAT IT DOES: With the angle from its ninth link to the player $50 to $AF, puts the nine links in state 2, sets
 * OBJ_WORK_I to $60 and steps the substate on; otherwise goes back to state 1 (Stage 07's overlay).
 * HOW: AngleFromNinthToPlayer0BFD, two compares, SetNineAfterToState2_0BFE.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void AimOrBackToState1_0BFE(void)
{
    call_routine(0xC813, 0xC6CC);
    op_cmp(0x50);
    if (flag_c()) {
        op_cmp(0xB0);
        if (!flag_c()) {
            call_routine(0xC81E, 0xC82D);
            op_lda(0x60);
            WR8(OBJ->work_i[cpu.x], cpu.a);
            RMW8(OBJ->substate[cpu.x], v, f_inc(v));
            return;
        }
    }
    op_lda(0x01);
    WR8(OBJ->state[cpu.x], cpu.a);
}

void abi_AimOrBackToState1_0BFE(void)
{
    AimOrBackToState1_0BFE();
}

/*
 * @routine 0BFE:061 TimedAimSound0BFE
 * @run 71:C861
 * WHAT IT DOES: Counts OBJ_WORK_I down; at 0 aims its links at the player (AimNineLinksAtPlayer0BFE), sounds $B0,
 * sets OBJ_WORK_I to $30 and steps the substate on (Stage 07's overlay).
 * HOW: DEC, the call, SoundCommand, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void TimedAimSound0BFE(void)
{
    RMW8(OBJ->work_i[cpu.x], v, f_dec(v));
    if (flag_z()) {
        jsr_push(0xC869);
        AimNineLinksAtPlayer0BFE();
        jsr_pop();
        op_lda(0xB0);
        {
            uint8_t command;

            jsr_push(0xC86E);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
            jsr_pop();
        }
        op_lda(0x30);
        WR8(OBJ->work_i[cpu.x], cpu.a);
        RMW8(OBJ->substate[cpu.x], v, f_inc(v));
    }
}

void abi_TimedAimSound0BFE(void)
{
    TimedAimSound0BFE();
}

/*
 * @routine 0BFE:077 AimNineLinksAtPlayer0BFE
 * @run 71:C877
 * WHAT IT DOES: Takes the angle from its ninth link to the player, held to $50 to $B0 and rounded down to a
 * multiple of 8, into OBJ_WORK_H of the object and of the nine slots after it (Stage 07's overlay).
 * HOW: AngleFromNinthToPlayer0BFD, ClampA50ToB0_0BFE, AND #$F8; a store loop.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void AimNineLinksAtPlayer0BFE(void)
{
    call_routine(0xC87A, 0xC6CC);
    call_routine(0xC87D, 0xC896);
    op_and(0xF8);
    WR8(OBJ->work_h[cpu.x], cpu.a);
    WR8(ZP->temp00, cpu.a);
    op_txa();
    op_tay();
    op_iny();
    op_lda(0x09);
    WR8(ZP->temp01, cpu.a);
    do {
        op_lda(RD8(ZP->temp00));
        WR8(OBJ->work_h[cpu.y], cpu.a);
        op_iny();
        RMW8(ZP->temp01, v, f_dec(v));
    } while (!flag_z());
}

void abi_AimNineLinksAtPlayer0BFE(void)
{
    AimNineLinksAtPlayer0BFE();
}

/*
 * @routine 0BFE:0EC CollapseAndClear0BFE
 * @run 71:C8EC
 * WHAT IT DOES: In substate 0 puts the nine links in state 3, sets frame $78 and OBJ_WORK_O $60, takes the hit box
 * off and sets X speed 0 and Y speed -$400; then counts OBJ_WORK_O down, dropping pieces on its counts
 * (PieceOnCount0BFE) and falling, and removes itself at 0 (Stage 07's overlay).
 * HOW: SetNineAfterToState3_0BFE and stores; DEC, ApplyGravityUnlimited, ApplyVelocity, ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CollapseAndClear0BFE(void)
{
    op_lda(RD8(OBJ->substate[cpu.x]));
    if (flag_z()) {
        call_routine(0xC8F4, 0xC924);
        op_lda(0x78);
        WR8(OBJ->frame[cpu.x], cpu.a);
        op_lda(0x60);
        WR8(OBJ->work_o[cpu.x], cpu.a);
        WR8(OBJ->hitbox_on[cpu.x], 0);
        WR8(OBJ->vx_hi[cpu.x], 0);
        WR8(OBJ->vx_lo[cpu.x], 0);
        op_lda(0xFC);
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        WR8(OBJ->vy_lo[cpu.x], 0);
        RMW8(OBJ->substate[cpu.x], v, f_inc(v));
        return;
    }
    RMW8(OBJ->work_o[cpu.x], v, f_dec(v));
    if (flag_z()) {
        ClearObject(cpu.x);
        return;
    }
    call_routine(0xC91E, 0xC940);
    jsr_push(0xC921);
    gravity_exit_shim(ApplyGravityUnlimited(cpu.x));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocity(cpu.x));
}

void abi_CollapseAndClear0BFE(void)
{
    CollapseAndClear0BFE();
}

/*
 * @routine 0BFE:169 LinkByState0BFE
 * @run 71:C969
 * WHAT IT DOES: Runs the step of the object's state: 0 loads its record, 1 AimLinkFromBuffer0BFE, 2
 * EaseThenBySubstate0BFE, 3 LaunchLandBySubstate0BFE (Stage 07's overlay).
 * HOW: Twice the state indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    LinkByState0BFESteps = 0xC97B,   /* the routine for each state, low and high byte */
};

void LinkByState0BFE(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(LinkByState0BFESteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(LinkByState0BFESteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_LinkByState0BFE(void)
{
    LinkByState0BFE(cpu.x);
}

/*
 * @routine 0BFE:264 LaunchLandBySubstate0BFE
 * @run 71:CA64
 * WHAT IT DOES: Runs the step of the object's substate: 0 LaunchByStep0BFE, 1 LandOrClearLow0BFE, 2
 * FlickerThenBecomeL0BFE (Stage 07's overlay).
 * HOW: Twice the substate indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    LaunchLandBySubstate0BFESteps = 0xCA76,   /* the routine for each substate, low and high byte */
};

void LaunchLandBySubstate0BFE(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(LaunchLandBySubstate0BFESteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(LaunchLandBySubstate0BFESteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_LaunchLandBySubstate0BFE(void)
{
    LaunchLandBySubstate0BFE(cpu.x);
}

/*
 * @routine 0BFE:313 ShotByState0BFE
 * @run 71:CB13
 * WHAT IT DOES: Runs the step of the object's state: 0 LoadRecordWorkF8E9_0BFE, 1 StepDownOwnerChain0BFE, 2
 * ShotFromOwner0BFE, 3 FlyUntilOutside0BFE (Stage 07's overlay).
 * HOW: Twice the state indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ShotByState0BFESteps = 0xCB25,   /* the routine for each state, low and high byte */
};

void ShotByState0BFE(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ShotByState0BFESteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ShotByState0BFESteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ShotByState0BFE(void)
{
    ShotByState0BFE(cpu.x);
}

/*
 * @routine 0BFE:422 FaceHitThenByState0BFE
 * @run 71:CC22
 * WHAT IT DOES: Steps OBJ_WORK_G on, faces the player in the states that do so, takes state 6 when hit, then runs
 * the step of its state: 0 StartKind47Next0BFE, 1 LandThenNearPlayer0BFE, 2 ListThenBoxOn0BFE, 3
 * AdvanceUntilNear0BFE, 4 BackOffUntilFar0BFE, 5 AttackBySubstate0BFE, 6 WakeTurnBySubstate0BFE (Stage 07's
 * overlay).
 * HOW: INC; FacePlayerInStates0BFE, State6SoundWhenHit0BFE; twice the state indexes a word table.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FaceHitThenByState0BFESteps = 0xCC3D,   /* the routine for each state, low and high byte */
};

void FaceHitThenByState0BFE(void)
{
    RMW8(OBJ->work_g[cpu.x], v, f_inc(v));
    call_routine(0xCC28, 0xCC4B);
    call_routine(0xCC2B, 0xCC64);
    op_lda(RD8(OBJ->state[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(FaceHitThenByState0BFESteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(FaceHitThenByState0BFESteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_FaceHitThenByState0BFE(void)
{
    FaceHitThenByState0BFE();
}

/*
 * @routine 0BFE:4C9 AdvanceUntilNear0BFE
 * @run 71:CCC9
 * WHAT IT DOES: Steps while landed with OBJ_WORK_E its facing; within $54 of the player along X goes to state 4,
 * otherwise goes on to DodgeOrCountToState5_0BFE (Stage 07's overlay).
 * HOW: StepWhileLanded0BFE, DistanceXToPlayer and a compare.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void AdvanceUntilNear0BFE(void)
{
    call_routine(0xCCCC, 0xCCE2);
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    WR8(OBJ->work_e[cpu.x], cpu.a);
    jsr_push(0xCCD5);
    distance_exit_shim(DistanceXToPlayer(cpu.x));
    jsr_pop();
    op_cmp(0x54);
    if (flag_c()) {
        DodgeOrCountToState5_0BFE(cpu.x);
        return;
    }
    op_lda(0x04);
    WR8(OBJ->state[cpu.x], cpu.a);
}

void abi_AdvanceUntilNear0BFE(void)
{
    AdvanceUntilNear0BFE();
}

/*
 * @routine 0BFE:50F DodgeOrCountToState5_0BFE
 * @run 71:CD0F
 * WHAT IT DOES: When the player is in state 3 and the sign of his X speed equals the object's OBJ_FACING_LEFT,
 * restarts its animation in state 5, substate 8; otherwise counts OBJ_WORK_H down and at 0 restarts it in state 5,
 * substate 0 (Stage 07's overlay).
 * HOW: Slot 0's state and X speed bit 7 rolled to bit 0; a compare; DEC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void DodgeOrCountToState5_0BFE(uint8_t slot)
{
    op_lda(RD8(OBJ->state[0x00]));
    op_cmp(0x03);
    if (flag_z()) {
        op_lda(RD8(OBJ->vx_hi[0x00]));
        cpu.a = f_rol(cpu.a);
        cpu.a = f_rol(cpu.a);
        op_and(0x01);
        op_cmp(RD8(OBJ->facing_left[slot]));
        if (flag_z()) {
            WR8(OBJ->anim_step[slot], 0);
            WR8(OBJ->anim_timer[slot], 0);
            op_lda(0x05);
            WR8(OBJ->state[slot], cpu.a);
            op_lda(0x08);
            WR8(OBJ->substate[slot], cpu.a);
            return;
        }
    }
    RMW8(OBJ->work_h[slot], v, f_dec(v));
    if (flag_z()) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        op_lda(0x05);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
    }
}

void abi_DodgeOrCountToState5_0BFE(void)
{
    DodgeOrCountToState5_0BFE(cpu.x);
}

/*
 * @routine 0BFE:547 BackOffUntilFar0BFE
 * @run 71:CD47
 * WHAT IT DOES: Steps while landed with OBJ_WORK_E the opposite of its facing; at $6C or more from the player along
 * X goes to state 3, otherwise goes on to DodgeOrCountToState5_0BFE (Stage 07's overlay).
 * HOW: StepWhileLanded0BFE, EOR #1, DistanceXToPlayer and a compare.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void BackOffUntilFar0BFE(void)
{
    call_routine(0xCD4A, 0xCCE2);
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    op_eor(0x01);
    WR8(OBJ->work_e[cpu.x], cpu.a);
    jsr_push(0xCD55);
    distance_exit_shim(DistanceXToPlayer(cpu.x));
    jsr_pop();
    op_cmp(0x6C);
    if (!flag_c()) {
        DodgeOrCountToState5_0BFE(cpu.x);
        return;
    }
    op_lda(0x03);
    WR8(OBJ->state[cpu.x], cpu.a);
}

void abi_BackOffUntilFar0BFE(void)
{
    BackOffUntilFar0BFE();
}

/*
 * @routine 0BFE:562 AttackBySubstate0BFE
 * @run 71:CD62
 * WHAT IT DOES: Runs the step of the object's substate: 0 PickPhaseOfFour0BFE, 1 SetFrame89Wait28_0BFE, 2
 * HoldFrame89ThenLeapBack0BFE, 3 BackSlideListA0BFE, 4 BackSlideListB0BFE, 5 JumpAtPlayer0BFE, 6
 * FrameBySignLand0BFE, 7 and $0A ListThenState3_0BFE, 8 HopAtPlayer0BFE, 9 LandFrontColumn0BFE (Stage 07's
 * overlay).
 * HOW: Twice the substate indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    AttackBySubstate0BFESteps = 0xCD74,   /* the routine for each substate, low and high byte */
};

void AttackBySubstate0BFE(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(AttackBySubstate0BFESteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(AttackBySubstate0BFESteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_AttackBySubstate0BFE(void)
{
    AttackBySubstate0BFE(cpu.x);
}

/*
 * @routine 0BFE:5B6 HoldFrame89ThenLeapBack0BFE
 * @run 71:CDB6
 * WHAT IT DOES: Shows frame $89, lands in place and counts OBJ_WORK_D down; within $30 of the player takes a speed
 * away from him by its facing into OBJ_WORK_I:OBJ_WORK_J and substate 3 or 4 by bit 1 of its Y; at 0 goes to state
 * 3, substate 0 (Stage 07's overlay).
 * HOW: SlideByFacing0BFE, the X speed cleared, DEC, DistanceXToPlayer; a word table by twice the facing.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    HoldFrame89ThenLeapBack0BFESpeeds = 0xCDF7,   /* the X speed for each facing, low byte (OBJ_WORK_J) then high (OBJ_WORK_I): -$2C0, +$2C0 */
};

void HoldFrame89ThenLeapBack0BFE(void)
{
    op_lda(0x89);
    WR8(OBJ->frame[cpu.x], cpu.a);
    call_routine(0xCDBE, 0xCDFB);
    WR8(OBJ->vx_hi[cpu.x], 0);
    WR8(OBJ->vx_lo[cpu.x], 0);
    RMW8(OBJ->work_d[cpu.x], v, f_dec(v));
    if (!flag_z()) {
        jsr_push(0xCDCC);
        distance_exit_shim(DistanceXToPlayer(cpu.x));
        jsr_pop();
        op_cmp(0x30);
        if (!flag_c()) {
            op_lda(RD8(OBJ->facing_left[cpu.x]));
            cpu.a = f_asl(cpu.a);
            op_tay();
            op_lda(rd8((uint16_t)(HoldFrame89ThenLeapBack0BFESpeeds + 1 + cpu.y)));
            WR8(OBJ->work_i[cpu.x], cpu.a);
            op_lda(rd8((uint16_t)(HoldFrame89ThenLeapBack0BFESpeeds + cpu.y)));
            WR8(OBJ->work_j[cpu.x], cpu.a);
            op_lda(RD8(OBJ->y_lo[cpu.x]));
            cpu.a = f_lsr(cpu.a);
            op_and(0x01);
            op_clc();
            op_adc(0x03);
            WR8(OBJ->substate[cpu.x], cpu.a);
        }
        return;
    }
    op_lda(0x03);
    WR8(OBJ->state[cpu.x], cpu.a);
    WR8(OBJ->substate[cpu.x], 0);
}

void abi_HoldFrame89ThenLeapBack0BFE(void)
{
    HoldFrame89ThenLeapBack0BFE();
}

/*
 * @routine 0BFE:612 BackSlideListA0BFE
 * @run 71:CE12
 * WHAT IT DOES: Slides and slows (SlideAndSlow0BFE) and plays the frame list at $CE2B; at its end goes to state 4,
 * substate 0 (Stage 07's overlay).
 * HOW: The call, StepObjectFrames, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void BackSlideListA0BFE(void)
{
    call_routine(0xCE15, 0xCE34);
    op_lda(0x2B);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xCE);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xCE20);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        op_lda(0x04);
        WR8(OBJ->state[cpu.x], cpu.a);
        WR8(OBJ->substate[cpu.x], 0);
    }
}

void abi_BackSlideListA0BFE(void)
{
    BackSlideListA0BFE();
}

/*
 * @routine 0BFE:66A BackSlideListB0BFE
 * @run 71:CE6A
 * WHAT IT DOES: Slides and slows (SlideAndSlow0BFE) and plays the frame list at $CE83; at its end goes to state 4,
 * substate 0 (Stage 07's overlay).
 * HOW: The call, StepObjectFrames, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void BackSlideListB0BFE(void)
{
    call_routine(0xCE6D, 0xCE34);
    op_lda(0x83);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xCE);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xCE78);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        op_lda(0x04);
        WR8(OBJ->state[cpu.x], cpu.a);
        WR8(OBJ->substate[cpu.x], 0);
    }
}

void abi_BackSlideListB0BFE(void)
{
    BackSlideListB0BFE();
}

/*
 * @routine 0BFE:76B WakeTurnBySubstate0BFE
 * @run 71:CF6B
 * WHAT IT DOES: Runs the step of the object's substate: 0 HitBoxOffWait40_0BFE, 1 LandListThenWakeNext0BFE, 2
 * TurnDownTimed0BFE, 3 TurnUpTimed0BFE (Stage 07's overlay).
 * HOW: Twice the substate indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    WakeTurnBySubstate0BFESteps = 0xCF7D,   /* the routine for each substate, low and high byte */
};

void WakeTurnBySubstate0BFE(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(WakeTurnBySubstate0BFESteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(WakeTurnBySubstate0BFESteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_WakeTurnBySubstate0BFE(void)
{
    WakeTurnBySubstate0BFE(cpu.x);
}

/*
 * @routine 0BFF:084 LoadRecordFrameByArg0BFF
 * @run 71:D084
 * WHAT IT DOES: In state 0 loads the object's record and sets its frame to OBJ_START_ARG and its sprite set to 6
 * (Stage 07's overlay).
 * HOW: A test of the state, LoadObjectRecord, two stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LoadRecordFrameByArg0BFF(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xD08C);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(RD8(OBJ->start_arg[slot]));
        WR8(OBJ->frame[slot], cpu.a);
        op_lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
    }
}

void abi_LoadRecordFrameByArg0BFF(void)
{
    LoadRecordFrameByArg0BFF(cpu.x);
}

/*
 * @routine 0BFF:098 BreakWallByState0BFF
 * @run 71:D098
 * WHAT IT DOES: Runs the step of the object's state: 0 LoadRecordDrawTiles09_0BFF, 1 BreakWallOnHit0BFF (Stage 07's
 * overlay).
 * HOW: Twice the state indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    BreakWallByState0BFFSteps = 0xD0AA,   /* the routine for each state, low and high byte */
};

void BreakWallByState0BFF(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(BreakWallByState0BFFSteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(BreakWallByState0BFFSteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_BreakWallByState0BFF(void)
{
    BreakWallByState0BFF(cpu.x);
}

/*
 * @routine 0BFF:10C LoadRecordInState0_0BFF
 * @run 71:D10C
 * WHAT IT DOES: In state 0 loads the object's record (Stage 07's overlay).
 * HOW: A test of the state, LoadObjectRecord.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LoadRecordInState0_0BFF(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xD114);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
    }
}

void abi_LoadRecordInState0_0BFF(void)
{
    LoadRecordInState0_0BFF(cpu.x);
}

/*
 * @routine 0BFF:28A StoodOnThenSink0BFF
 * @run 71:D28A
 * WHAT IT DOES: In state 0 loads its record with sprite set 6 and frame $11; in state 1, once the player stands on
 * it (StandOnUnlessSide2_0BFF), sets OBJ_WORK_D $20, records its event and steps on; in state 2 counts OBJ_WORK_D
 * down to the next state; in state 3 speeds down by $14 a frame until its Y low is $BA, then moves at Y speed $40;
 * in state 4, from Y low $C2, removes itself when its Y high is not 0, else flips its frame between $11 and 0; from
 * state 1 on it keeps the player's contact each frame (Stage 07's overlay).
 * HOW: A chain of DEC tests of the state; RecordObjectEvent; a 16-bit add to Y speed; ApplyVelocity; ends in
 * ClearCarry0BFF.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void StoodOnThenSink0BFF(void)
{
    op_lda(RD8(OBJ->state[cpu.x]));
    if (!flag_z()) {
        goto at_D29D;
    }
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xD292);
        last = LoadObjectRecord(cpu.x, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x06);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    op_lda(0x11);
    WR8(OBJ->frame[cpu.x], cpu.a);
at_D29C:
    return;
at_D29D:
    cpu.a = f_dec(cpu.a);
    if (flag_z()) {
        call_routine(0xD2A3, 0xD309);
        if (!flag_c()) {
            goto at_D29C;
        }
        op_lda(0x20);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        {
            uint8_t stored;
            uint8_t event;
            uint8_t byte;

            jsr_push(0xD2AD);
            if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
                cpu.a = 0;
                abi_nz(0);
            } else {
                record_event_exit_shim(event, stored, byte);
            }
            jsr_pop();
        }
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
        return;
    }
    cpu.a = f_dec(cpu.a);
    if (flag_z()) {
        call_routine(0xD2B7, 0xD309);
        RMW8(OBJ->work_d[cpu.x], v, f_dec(v));
        if (!flag_z()) {
            goto at_D29C;
        }
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
        return;
    }
    cpu.a = f_dec(cpu.a);
    if (flag_z()) {
        call_routine(0xD2C6, 0xD309);
        op_lda(RD8(OBJ->vy_lo[cpu.x]));
        op_clc();
        op_adc(0x14);
        WR8(OBJ->vy_lo[cpu.x], cpu.a);
        if (flag_c()) {
            RMW8(OBJ->vy_hi[cpu.x], v, f_inc(v));
        }
        jsr_push(0xD2D7);
        axis_move_exit_shim(ApplyVelocity(cpu.x));
        jsr_pop();
        op_lda(RD8(OBJ->y_lo[cpu.x]));
        op_cmp(0xBA);
        if (!flag_c()) {
            goto at_D29C;
        }
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
        op_lda(0x40);
        WR8(OBJ->vy_lo[cpu.x], cpu.a);
        WR8(OBJ->vy_hi[cpu.x], 0);
        return;
    }
    call_routine(0xD2ED, 0xD309);
    jsr_push(0xD2F0);
    axis_move_exit_shim(ApplyVelocity(cpu.x));
    jsr_pop();
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    op_cmp(0xC2);
    if (!flag_c()) {
        goto at_D29C;
    }
    op_lda(RD8(OBJ->y_hi[cpu.x]));
    if (!flag_z()) {
        ClearObject(cpu.x);
        return;
    }
    op_lda(RD8(OBJ->frame[cpu.x]));
    op_eor(0x11);
    WR8(OBJ->frame[cpu.x], cpu.a);
    /* falls into $D307 */
    run_routine(0xD307);
}

void abi_StoodOnThenSink0BFF(void)
{
    StoodOnThenSink0BFF();
}

/*
 * @routine 0BFF:451 SecretWallByState0BFF
 * @run 71:D451
 * WHAT IT DOES: Runs the step of the object's state: 0 LoadRecordScreenY0BFF, 1 SecretWallOnHit0BFF, 2
 * StateOnWhenField85Is2_0BFF, 3 StateOnAndMarkField8C0BFF, 4 StateOnUnlessField8CSet0BFF, 5 steps the state on, 6
 * does nothing (Stage 07's overlay).
 * HOW: Twice the state indexes a word table; a jump through zTemp00/01.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    SecretWallByState0BFFSteps = 0xD463,   /* the routine for each state, low and high byte */
};

void SecretWallByState0BFF(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(SecretWallByState0BFFSteps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(SecretWallByState0BFFSteps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_SecretWallByState0BFF(void)
{
    SecretWallByState0BFF(cpu.x);
}

/*
 * @routine 0BFF:4B7 StateOnWhenField85Is2_0BFF
 * @run 71:D4B7
 * WHAT IT DOES: Steps the object's state on when zField85 is 2 (Stage 07's overlay).
 * HOW: A compare, then INC or a jump to the RTS at $D4CA.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StateOnWhenField85Is2_0BFF(uint8_t slot)
{
    op_lda(RD8(ZP->field85));
    op_cmp(0x02);
    if (!flag_z()) {
        ReturnFromStep0BFF();
        return;
    }
    RMW8(OBJ->state[slot], v, f_inc(v));
}

void abi_StateOnWhenField85Is2_0BFF(void)
{
    StateOnWhenField85Is2_0BFF(cpu.x);
}

/*
 * @routine 0BFF:4CA ReturnFromStep0BFF
 * @run 71:D4CA
 * WHAT IT DOES: Returns at once: the RTS that ends StateOnAndMarkField8C0BFF, which the steps beside it jump to
 * when they do nothing (Stage 07's overlay).
 * HOW: RTS.
 * WAITS: no.
 * Entry: nothing. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ReturnFromStep0BFF(void)
{
}

void abi_ReturnFromStep0BFF(void)
{
    ReturnFromStep0BFF();
}

/*
 * @routine 0BFF:4CB StateOnUnlessField8CSet0BFF
 * @run 71:D4CB
 * WHAT IT DOES: Steps the object's state on unless zField8C bit 7 is set (Stage 07's overlay).
 * HOW: A test of N, then INC or a jump to the RTS at $D4CA.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StateOnUnlessField8CSet0BFF(uint8_t slot)
{
    op_lda(RD8(ZP->field8c));
    if (flag_n()) {
        ReturnFromStep0BFF();
        return;
    }
    RMW8(OBJ->state[slot], v, f_inc(v));
}

void abi_StateOnUnlessField8CSet0BFF(void)
{
    StateOnUnlessField8CSet0BFF(cpu.x);
}

/*
 * @routine 0BFF:4D3 AnimateTilesByScroll0BFF
 * @run 71:D4D3
 * WHAT IT DOES: In state 0 loads its record; each frame takes scroll layer 1's X less this frame's step, and copies
 * the VRAM rows picked by its high byte, rounded by the low byte's bit 7, plus 8 and held to 0 to $1F
 * (CopyEightVramRows0BFF), so the tiles follow the scroll (Stage 07's overlay).
 * HOW: A 16-bit subtract of ScrollStepX from the scroll's X (index 1), ASL and ADC #8, AND #$1F, the call.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void AnimateTilesByScroll0BFF(void)
{
    op_lda(RD8(OBJ->state[cpu.x]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xD4DB);
            last = LoadObjectRecord(cpu.x, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
    }
    op_lda(RD8(SCROLL->x_frac[0x01]));
    op_sec();
    op_sbc(RD8(SCROLLE->step_x_lo[0x01]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(SCROLL->x_lo[0x01]));
    op_sbc(RD8(SCROLLE->step_x_hi[0x01]));
    WR8(ZP->temp01, cpu.a);
    RMW8(ZP->temp00, v, f_asl(v));
    op_lda(RD8(ZP->temp01));
    op_adc(0x08);
    op_and(0x1F);
    call_routine(0xD4F7, 0xD4F8);
}

void abi_AnimateTilesByScroll0BFF(void)
{
    AnimateTilesByScroll0BFF();
}

/*
 * @routine 0BFF:621 WaitPlayerYThenFade0BFF
 * @run 71:D621
 * WHAT IT DOES: In state 0 loads its record with sprite set 6 and frame $29; in state 1, once the player's Y low is
 * $AA, calls the CD fade ($E02D) with $0C and steps on; in state 2 counts OBJ_WORK_D down by 2 and at 0 sets
 * zModeEvents bit 1 and zFieldA3 to $0E (Stage 07's overlay).
 * HOW: DEC tests of the state; vec_cd_fade with X kept on the stack; SMB1.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void WaitPlayerYThenFade0BFF(void)
{
    op_lda(RD8(OBJ->state[cpu.x]));
    if (!flag_z()) {
        goto at_D634;
    }
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xD629);
        last = LoadObjectRecord(cpu.x, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x06);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    op_lda(0x29);
    WR8(OBJ->frame[cpu.x], cpu.a);
at_D633:
    return;
at_D634:
    cpu.a = f_dec(cpu.a);
    if (flag_z()) {
        op_lda(RD8(OBJ->y_lo[0x00]));
        op_cmp(0xAA);
        if (!flag_z()) {
            goto at_D633;
        }
        op_phx();
        op_lda(0x0C);
        jsr_push(0xD644);
        cd_fade_exit_shim(vec_cd_fade(cpu.a));
        jsr_pop();
        op_plx();
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
        return;
    }
    RMW8(OBJ->work_d[cpu.x], v, f_dec(v));
    RMW8(OBJ->work_d[cpu.x], v, f_dec(v));
    if (!flag_z()) {
        goto at_D633;
    }
    RMW8(ZP->mode_events, v, v | 0x02);
    op_lda(0x0E);
    WR8(ZP->fielda3, cpu.a);
}

void abi_WaitPlayerYThenFade0BFF(void)
{
    WaitPlayerYThenFade0BFF();
}
