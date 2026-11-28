/*
 * More routines of Stage 08's overlays: state, substate and argument jump tables, bank switches around them, palette
 * fades, chains of nine parts aimed at the player, walkers and rollers that probe the terrain, and the pieces they fall into.
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
#include "stage08_more_c.h"

/*
 * @routine 0D2D:284 ActByState0D2D_284
 * @run 81:C284
 * WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByState0D2D_284Jumps = 0xC296,   /* the routine for each state, a word each */
};

void ActByState0D2D_284(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByState0D2D_284Jumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByState0D2D_284Jumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByState0D2D_284(void)
{
    ActByState0D2D_284(cpu.x);
}

/*
 * @routine 0D2D:2F4 CountDownOnHitResult0D2D
 * @run 81:C2F4
 * WHAT IT DOES: Runs HitRoutineByArg0D2D; when that returns C set, counts OBJ_WORK_E down and steps the state on at
 * zero, and otherwise reloads OBJ_WORK_E from OBJ_WORK_D (Stage 08's overlay).
 * HOW: A call, then DEC or INC, or a copy.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CountDownOnHitResult0D2D(uint8_t slot)
{
    jsr_push(0xC2F7);
    HitRoutineByArg0D2D(slot);
    jsr_pop();
    if (flag_c()) {
        op_lda(RD8(OBJ->work_e[slot]));
        if (!flag_z()) {
            RMW8(OBJ->work_e[slot], v, f_dec(v));
            return;
        }
        RMW8(OBJ->state[slot], v, f_inc(v));
        return;
    }
    op_lda(RD8(OBJ->work_d[slot]));
    WR8(OBJ->work_e[slot], cpu.a);
}

void abi_CountDownOnHitResult0D2D(void)
{
    CountDownOnHitResult0D2D(cpu.x);
}

/*
 * @routine 0D2D:30D ActByArg0D2D
 * @run 81:C30D
 * WHAT IT DOES: Runs the routine for the object's OBJ_START_ARG (Stage 08's overlay).
 * HOW: A jump table by OBJ_START_ARG.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByArg0D2DJumps = 0xC234,   /* the routine for each OBJ_START_ARG, a word each */
};

void ActByArg0D2D(uint8_t slot)
{
    op_lda(RD8(OBJ->start_arg[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByArg0D2DJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByArg0D2DJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByArg0D2D(void)
{
    ActByArg0D2D(cpu.x);
}

/*
 * @routine 0D2D:31F ActBySubstate0D2D
 * @run 81:C31F
 * WHAT IT DOES: Runs the routine for the object's substate (Stage 08's overlay).
 * HOW: A jump table by the substate.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActBySubstate0D2DJumps = 0xC331,   /* the routine for each substate, a word each */
};

void ActBySubstate0D2D(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActBySubstate0D2DJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActBySubstate0D2DJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActBySubstate0D2D(void)
{
    ActBySubstate0D2D(cpu.x);
}

/*
 * @routine 0D2D:37D HitRoutineByArg0D2D
 * @run 81:C37D
 * WHAT IT DOES: Clears the object's invulnerability timer; when it has taken a hit of 1 to 4 (not a damaging one)
 * this frame, runs the routine a table gives by OBJ_START_ARG, else returns C clear (Stage 08's overlay).
 * HOW: A store, a compare of OBJ_HIT_TAKEN, then a jump table by OBJ_START_ARG or CLC.
 * WAITS: no.
 * Entry: X the object. Exit: C clear when no such hit was taken; else as the routine for OBJ_START_ARG leaves it.
 */
enum {
    HitRoutineByArg0D2DJumps = 0xC25C,   /* the routine for each OBJ_START_ARG, a word each */
};

void HitRoutineByArg0D2D(uint8_t slot)
{
    WR8(OBJ->invuln_timer[slot], 0);
    op_lda(RD8(OBJ->hit_taken[slot]));
    if (!flag_z()) {
        op_cmp(0x05);
        if (!flag_c()) {
            op_lda(RD8(OBJ->start_arg[slot]));
            cpu.a = f_asl(cpu.a);
            op_tay();
            op_lda(rd8((uint16_t)(HitRoutineByArg0D2DJumps + cpu.y)));
            WR8(ZP->temp00, cpu.a);
            op_lda(rd8((uint16_t)(HitRoutineByArg0D2DJumps + 1 + cpu.y)));
            WR8(ZP->temp01, cpu.a);
            run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
            return;
        }
    }
    op_clc();
}

void abi_HitRoutineByArg0D2D(void)
{
    HitRoutineByArg0D2D(cpu.x);
}

/*
 * @routine 0D2D:3BE ReturnCarryClear0D2D
 * @run 81:C3BE
 * WHAT IT DOES: Returns with C clear; the piece CheckPlayerStillNoPad0D2D ends in (Stage 08's overlay).
 * HOW: CLC.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: C clear.
 */
void ReturnCarryClear0D2D(void)
{
    op_clc();
}

void abi_ReturnCarryClear0D2D(void)
{
    ReturnCarryClear0D2D();
}

/*
 * @routine 0D2D:3C0 CheckPlayerStillNoPad0D2D
 * @run 81:C3C0
 * WHAT IT DOES: Says whether the player (slot 0) is in state 0 with no button or direction held (Stage 08's
 * overlay).
 * HOW: A compare of slot 0's state and a test of zPadHeld.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: C set when the player is in state 0 and zPadHeld is 0; else C clear.
 */
void CheckPlayerStillNoPad0D2D(void)
{
    op_lda(RD8(OBJ->state[0x00]));
    op_cmp(0x00);
    if (flag_z()) {
        op_lda(RD8(ZP->pad_held));
        if (!flag_z()) {
            ReturnCarryClear0D2D();
            return;
        }
        op_sec();
        return;
    }
    op_clc();
}

void abi_CheckPlayerStillNoPad0D2D(void)
{
    CheckPlayerStillNoPad0D2D();
}

/*
 * @routine 0D2D:3CF ActByState0D2D_3CF
 * @run 81:C3CF
 * WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByState0D2D_3CFJumps = 0xC3E1,   /* the routine for each state, a word each */
};

void ActByState0D2D_3CF(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByState0D2D_3CFJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByState0D2D_3CFJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByState0D2D_3CF(void)
{
    ActByState0D2D_3CF(cpu.x);
}

/*
 * @routine 0D2D:493 MoveAndSwingByCosine0D2D
 * @run 81:C493
 * WHAT IT DOES: Moves the object by its speed and swings the 16-bit angle OBJ_WORK_D-E by a cosine of OBJ_WORK_F,
 * which steps by 2; when that wraps to 0 the angle goes back to $40; the tail of SwingLinkByCosine0D2D (Stage 08's
 * overlay).
 * HOW: ApplyVelocity, ScaleByCosine of a word from a table by OBJ_START_ARG, a 16-bit add, an add of 2.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    MoveAndSwingByCosine0D2DSwings = 0xC4D0,   /* the swing's size, a word per OBJ_START_ARG */
};

void MoveAndSwingByCosine0D2D(uint8_t slot)
{
    jsr_push(0xC496);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    op_lda(RD8(OBJ->start_arg[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(MoveAndSwingByCosine0D2DSwings + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(MoveAndSwingByCosine0D2DSwings + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_f[slot]));
    jsr_push(0xC4AB);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    op_lda(RD8(OBJ->work_e[slot]));
    op_clc();
    op_adc(RD8(ZP->product.mid));
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    op_adc(RD8(ZP->product.hi));
    WR8(OBJ->work_d[slot], cpu.a);
    op_lda(RD8(OBJ->work_f[slot]));
    op_clc();
    op_adc(0x02);
    WR8(OBJ->work_f[slot], cpu.a);
    if (flag_z()) {
        op_lda(0x40);
        WR8(OBJ->work_d[slot], cpu.a);
        WR8(OBJ->work_e[slot], 0);
    }
}

void abi_MoveAndSwingByCosine0D2D(void)
{
    MoveAndSwingByCosine0D2D(cpu.x);
}

/*
 * @routine 0D2D:4D6 ActByState0D2D_4D6
 * @run 81:C4D6
 * WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByState0D2D_4D6Jumps = 0xC4E8,   /* the routine for each state, a word each */
};

void ActByState0D2D_4D6(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByState0D2D_4D6Jumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByState0D2D_4D6Jumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByState0D2D_4D6(void)
{
    ActByState0D2D_4D6(cpu.x);
}

/*
 * @routine 0D2D:51B FollowLinkOrLeaveScreen0D2D
 * @run 81:C51B
 * WHAT IT DOES: With OBJ_WORK_G 0, clears the object once its X is $160 or more or below -$60; else clears it
 * unless the slot below holds kind $C4, and otherwise puts it at that slot's place, sets a speed of $0800 at angle
 * OBJ_WORK_D, and moves and swings it (MoveAndSwingByCosine0D2D) (Stage 08's overlay).
 * HOW: Compares of the X bytes and ClearObject; a kind compare, CopyObjectPlaceBack, SetVelocityFromAngle, then the
 * piece.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FollowLinkOrLeaveScreen0D2D(uint8_t slot)
{
    op_lda(RD8(OBJ->work_g[slot]));
    if (flag_z()) {
        op_lda(RD8(OBJ->x_hi[slot]));
        if (flag_z()) {
            goto at_C55C;
        }
        if (!flag_n()) {
            op_lda(RD8(OBJ->x_lo[slot]));
            op_cmp(0x60);
            if (flag_c()) {
                ClearObject(slot);
                return;
            }
            return;
        }
        op_lda(RD8(OBJ->x_lo[slot]));
        op_cmp(0xA0);
        if (!flag_c()) {
            ClearObject(slot);
            return;
        }
        return;
    }
    op_lda(rd8((uint16_t)(COL(OBJ->kind) - 1 + slot)));
    op_cmp(0xC4);
    if (!flag_z()) {
        ClearObject(slot);
        return;
    }
    op_txa();
    op_tay();
    op_dey();
    jsr_push(0xC54D);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    WR8(ZP->temp00, 0);
    op_lda(0x08);
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    jsr_push(0xC559);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
    MoveAndSwingByCosine0D2D(slot);
    return;
at_C55C:
    ;
}

void abi_FollowLinkOrLeaveScreen0D2D(void)
{
    FollowLinkOrLeaveScreen0D2D(cpu.x);
}

/*
 * @routine 0D5A:44D RunKindsInBankC0D5A
 * @run 70:AC4D
 * WHAT IT DOES: Maps bank $0C past zRamBaseBank at $C000, runs ActByKind59To5C0D6C there, then maps bank 9 back
 * (Stage 08's overlay).
 * HOW: TAM of MPR6, a call, then the piece.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void RunKindsInBankC0D5A(void)
{
    op_lda(0x0C);
    op_clc();
    op_adc(RD8(ZP->ram_base_bank));
    op_tam(0x40);
    jsr_push(0xAC57);
    ActByKind59To5C0D6C();
    jsr_pop();
    /* falls into $AC57 */
    MapBank9AtC000_0D5A();
}

void abi_RunKindsInBankC0D5A(void)
{
    RunKindsInBankC0D5A();
}

/*
 * @routine 0D5A:457 MapBank9AtC000_0D5A
 * @run 70:AC57
 * WHAT IT DOES: Maps bank 9 past zRamBaseBank at $C000 (Stage 08's overlay).
 * HOW: An add and TAM of MPR6.
 * WAITS: no.
 * Entry: D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void MapBank9AtC000_0D5A(void)
{
    op_lda(0x09);
    op_clc();
    op_adc(RD8(ZP->ram_base_bank));
    op_tam(0x40);
}

void abi_MapBank9AtC000_0D5A(void)
{
    MapBank9AtC000_0D5A();
}

/*
 * @routine 0D5A:460 RunStateTableInBank19_0D5A
 * @run 70:AC60
 * WHAT IT DOES: Maps bank $19 past zRamBaseBank at $C000, runs ActByState0D2D_3CF there, then maps bank 9 back
 * (Stage 08's overlay).
 * HOW: TAM of MPR6, a call, then MapBank9AtC000_0D5A.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void RunStateTableInBank19_0D5A(void)
{
    op_lda(0x19);
    op_clc();
    op_adc(RD8(ZP->ram_base_bank));
    op_tam(0x40);
    jsr_push(0xAC6A);
    ActByState0D2D_3CF(cpu.x);
    jsr_pop();
    MapBank9AtC000_0D5A();
}

void abi_RunStateTableInBank19_0D5A(void)
{
    RunStateTableInBank19_0D5A();
}

/*
 * @routine 0D5A:46C RunOtherStateTableInBank19_0D5A
 * @run 70:AC6C
 * WHAT IT DOES: Maps bank $19 past zRamBaseBank at $C000, runs ActByState0D2D_4D6 there, then maps bank 9 back
 * (Stage 08's overlay).
 * HOW: TAM of MPR6, a call, then MapBank9AtC000_0D5A.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void RunOtherStateTableInBank19_0D5A(void)
{
    op_lda(0x19);
    op_clc();
    op_adc(RD8(ZP->ram_base_bank));
    op_tam(0x40);
    jsr_push(0xAC76);
    ActByState0D2D_4D6(cpu.x);
    jsr_pop();
    MapBank9AtC000_0D5A();
}

void abi_RunOtherStateTableInBank19_0D5A(void)
{
    RunOtherStateTableInBank19_0D5A();
}

/*
 * @routine 0D5B:0F6 BecomeKind9CLowOrByState0D5B
 * @run 70:B0F6
 * WHAT IT DOES: With OBJ_START_ARG not 0 and its Y $BC or more, turns the object into kind $9C in state 0; else
 * runs the routine for its state (Stage 08's overlay).
 * HOW: Two compares and two stores, or a jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    BecomeKind9CLowOrByState0D5BJumps = 0xB11D,   /* the routine for each state, a word each */
};

void BecomeKind9CLowOrByState0D5B(uint8_t slot)
{
    op_lda(RD8(OBJ->start_arg[slot]));
    if (!flag_z()) {
        op_lda(RD8(OBJ->y_lo[slot]));
        op_cmp(0xBC);
        if (flag_c()) {
            op_lda(0x9C);
            WR8(OBJ->kind[slot], cpu.a);
            WR8(OBJ->state[slot], 0);
            return;
        }
    }
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(BecomeKind9CLowOrByState0D5BJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(BecomeKind9CLowOrByState0D5BJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_BecomeKind9CLowOrByState0D5B(void)
{
    BecomeKind9CLowOrByState0D5B(cpu.x);
}

/*
 * @routine 0D5B:180 FacePlayerThenActByDistance0D5B
 * @run 70:B180
 * WHAT IT DOES: Turns the object to face the player, clears OBJ_WORK_D, runs ActByDistanceToPlayer0D5B and steps
 * the state on (Stage 08's overlay).
 * HOW: SideOfPlayer's bit 0 into OBJ_FACING_LEFT, a store, a call, INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FacePlayerThenActByDistance0D5B(void)
{
    jsr_push(0xB183);
    side_of_player_exit_shim(SideOfPlayer(cpu.x));
    jsr_pop();
    op_and(0x01);
    WR8(OBJ->facing_left[cpu.x], cpu.a);
    WR8(OBJ->work_d[cpu.x], 0);
    call_routine(0xB18E, 0xB192);
    RMW8(OBJ->state[cpu.x], v, f_inc(v));
}

void abi_FacePlayerThenActByDistance0D5B(void)
{
    FacePlayerThenActByDistance0D5B();
}

/*
 * @routine 0D5B:2F4 ActAndKeepAngleHistory0D5B
 * @run 70:B2F4
 * WHAT IT DOES: Runs State4WhenHit0D5B, FlashChainWhileInvuln0D5B and ActByState0D5B_356, then shifts a 128-byte
 * history along by one and puts OBJ_WORK_D plus $70 at its start (Stage 08's overlay).
 * HOW: Three calls, a copy loop down from $7F, an add and a store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActAndKeepAngleHistory0D5BHistory = 0xB3A2,   /* the history at $B3A3, newest first: OBJ_WORK_D plus $70 for each frame; read one byte below */
};

void ActAndKeepAngleHistory0D5B(void)
{
    call_routine(0xB2F7, 0xB312);
    jsr_push(0xB2FA);
    FlashChainWhileInvuln0D5B(cpu.x);
    jsr_pop();
    jsr_push(0xB2FD);
    ActByState0D5B_356(cpu.x);
    jsr_pop();
    op_ldy(0x7F);
    do {
        op_lda(rd8((uint16_t)(ActAndKeepAngleHistory0D5BHistory + cpu.y)));
        wr8((uint16_t)(ActAndKeepAngleHistory0D5BHistory + 1 + cpu.y), cpu.a);
        op_dey();
    } while (!flag_z());
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_clc();
    op_adc(0x70);
    wr8(ActAndKeepAngleHistory0D5BHistory + 1, cpu.a);
}

void abi_ActAndKeepAngleHistory0D5B(void)
{
    ActAndKeepAngleHistory0D5B();
}

/*
 * @routine 0D5B:329 FlashChainWhileInvuln0D5B
 * @run 70:B329
 * WHAT IT DOES: In state 1, sets OBJ_WORK_J of the object and the nine slots after it to 2 while its
 * invulnerability timer runs and to 0 otherwise, and halves OBJ_WORK_O when it took a damaging hit (Stage 08's
 * overlay).
 * HOW: A compare, a loop of ten stores, a compare of OBJ_HIT_TAKEN and LSR.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FlashChainWhileInvuln0D5B(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    op_cmp(0x01);
    if (flag_z()) {
        WR8(ZP->temp00, 0);
        op_lda(RD8(OBJ->invuln_timer[slot]));
        if (!flag_z()) {
            op_lda(0x02);
            WR8(ZP->temp00, cpu.a);
        }
        op_txa();
        op_tay();
        op_lda(0x0A);
        WR8(ZP->temp01, cpu.a);
        do {
            op_lda(RD8(ZP->temp00));
            WR8(OBJ->work_j[cpu.y], cpu.a);
            op_iny();
            RMW8(ZP->temp01, v, f_dec(v));
        } while (!flag_z());
        op_lda(RD8(OBJ->hit_taken[slot]));
        op_cmp(0x05);
        if (flag_c()) {
            RMW8(OBJ->work_o[slot], v, f_lsr(v));
        }
    }
}

void abi_FlashChainWhileInvuln0D5B(void)
{
    FlashChainWhileInvuln0D5B(cpu.x);
}

/*
 * @routine 0D5B:356 ActByState0D5B_356
 * @run 70:B356
 * WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByState0D5B_356Jumps = 0xB368,   /* the routine for each state, a word each */
};

void ActByState0D5B_356(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByState0D5B_356Jumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByState0D5B_356Jumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByState0D5B_356(void)
{
    ActByState0D5B_356(cpu.x);
}

/*
 * @routine 0D5B:423 SwingChainThenPickRandom0D5B
 * @run 70:B423
 * WHAT IT DOES: Runs NextStateByPattern0D5B, MoveChainOfNine0D5B, EaseWorkFToTarget0D5B and ScaleByRecordWords0D5B,
 * adds the scaled word to the angle OBJ_WORK_D-E and the record's first word to OBJ_WORK_K-L; when that overflows,
 * puts a random 0 to 3 in OBJ_WORK_M and clears OBJ_WORK_D, E, K and L (Stage 08's overlay).
 * HOW: Four calls, two 16-bit adds, NextRandom and stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SwingChainThenPickRandom0D5B(void)
{
    call_routine(0xB426, 0xB4AA);
    call_routine(0xB429, 0xB4ED);
    call_routine(0xB42C, 0xB527);
    call_routine(0xB42F, 0xB485);
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
        jsr_push(0xB456);
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

void abi_SwingChainThenPickRandom0D5B(void)
{
    SwingChainThenPickRandom0D5B();
}

/*
 * @routine 0D5B:478 AngleToPlayerFromNinthSlot0D5B
 * @run 70:B478
 * WHAT IT DOES: Gives the angle to the player from the slot nine after the object (Stage 08's overlay).
 * HOW: X plus 9, AngleToPlayer, X back from zRichterObject.
 * WAITS: no.
 * Entry: X the object. Exit: A the angle; X the object.
 */
void AngleToPlayerFromNinthSlot0D5B(void)
{
    WR8(ZP->richter_object, cpu.x);
    op_txa();
    op_clc();
    op_adc(0x09);
    op_tax();
    jsr_push(0xB482);
    angle_exit_shim(AngleToPlayer(cpu.x));
    jsr_pop();
    op_ldx(RD8(ZP->richter_object));
}

void abi_AngleToPlayerFromNinthSlot0D5B(void)
{
    AngleToPlayerFromNinthSlot0D5B();
}

/*
 * @routine 0D5B:59A MoveChainThenBySubstate0D5B
 * @run 70:B59A
 * WHAT IT DOES: Runs MoveChainOfNine0D5B and EaseWorkFToTarget0D5B, then the routine for the object's substate
 * (Stage 08's overlay).
 * HOW: Two calls, then a jump table by the substate.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    MoveChainThenBySubstate0D5BJumps = 0xB5B2,   /* the routine for each substate, a word each */
};

void MoveChainThenBySubstate0D5B(void)
{
    call_routine(0xB59D, 0xB4ED);
    call_routine(0xB5A0, 0xB527);
    op_lda(RD8(OBJ->substate[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(MoveChainThenBySubstate0D5BJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(MoveChainThenBySubstate0D5BJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_MoveChainThenBySubstate0D5B(void)
{
    MoveChainThenBySubstate0D5B();
}

/*
 * @routine 0D5B:5BC AimChainIfPlayerBelow0D5B
 * @run 70:B5BC
 * WHAT IT DOES: When the angle from the ninth slot to the player is $50 to $AF, puts the nine slots after the
 * object in state 2, sets OBJ_WORK_I $60 and steps the substate on; else puts the object in state 1 (Stage 08's
 * overlay).
 * HOW: AngleToPlayerFromNinthSlot0D5B, two compares, the piece, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void AimChainIfPlayerBelow0D5B(void)
{
    jsr_push(0xB5BF);
    AngleToPlayerFromNinthSlot0D5B();
    jsr_pop();
    op_cmp(0x50);
    if (flag_c()) {
        op_cmp(0xB0);
        if (!flag_c()) {
            jsr_push(0xB5CA);
            ChainToState2_0D5B();
            jsr_pop();
            op_lda(0x60);
            WR8(OBJ->work_i[cpu.x], cpu.a);
            RMW8(OBJ->substate[cpu.x], v, f_inc(v));
            return;
        }
    }
    op_lda(0x01);
    WR8(OBJ->state[cpu.x], cpu.a);
}

void abi_AimChainIfPlayerBelow0D5B(void)
{
    AimChainIfPlayerBelow0D5B();
}

/*
 * @routine 0D5B:5D9 ChainToState2_0D5B
 * @run 70:B5D9
 * WHAT IT DOES: Puts the nine slots after the object in state 2, substate 0 (Stage 08's overlay).
 * HOW: A loop of two stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ChainToState2_0D5B(void)
{
    op_txa();
    op_tay();
    op_iny();
    op_lda(0x09);
    WR8(ZP->temp01, cpu.a);
    do {
        op_lda(0x02);
        WR8(OBJ->state[cpu.y], cpu.a);
        op_cla();
        WR8(OBJ->substate[cpu.y], cpu.a);
        op_iny();
        RMW8(ZP->temp01, v, f_dec(v));
    } while (!flag_z());
}

void abi_ChainToState2_0D5B(void)
{
    ChainToState2_0D5B();
}

/*
 * @routine 0D5B:5EF TurnAngleToZeroBy2_0D5B
 * @run 70:B5EF
 * WHAT IT DOES: Counts OBJ_WORK_I down and turns the angle OBJ_WORK_D by 2 toward 0, keeping it even; on reaching 0
 * the substate steps on (Stage 08's overlay).
 * HOW: DEC, TurnAngleToward, AND #$FE, a compare, INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void TurnAngleToZeroBy2_0D5B(uint8_t slot)
{
    RMW8(OBJ->work_i[slot], v, f_dec(v));
    WR8(ZP->temp02, 0);
    op_lda(0x02);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xB600);
    turn_angle_exit_shim(TurnAngleToward());
    jsr_pop();
    op_and(0xFE);
    WR8(OBJ->work_d[slot], cpu.a);
    op_cmp(RD8(ZP->temp02));
    if (flag_z()) {
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_TurnAngleToZeroBy2_0D5B(void)
{
    TurnAngleToZeroBy2_0D5B(cpu.x);
}

/*
 * @routine 0D5B:60D AimChainAfterWait0D5B
 * @run 70:B60D
 * WHAT IT DOES: Counts OBJ_WORK_I down; at zero aims the chain at the player (AimChainAtPlayer0D5B), plays sound
 * $C3, sets OBJ_WORK_I $30 and steps the substate on (Stage 08's overlay).
 * HOW: DEC, a call, SoundCommand, stores, INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void AimChainAfterWait0D5B(void)
{
    RMW8(OBJ->work_i[cpu.x], v, f_dec(v));
    if (flag_z()) {
        jsr_push(0xB615);
        AimChainAtPlayer0D5B();
        jsr_pop();
        op_lda(0xC3);
        {
            uint8_t command;

            jsr_push(0xB61A);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
            jsr_pop();
        }
        op_lda(0x30);
        WR8(OBJ->work_i[cpu.x], cpu.a);
        RMW8(OBJ->substate[cpu.x], v, f_inc(v));
    }
}

void abi_AimChainAfterWait0D5B(void)
{
    AimChainAfterWait0D5B();
}

/*
 * @routine 0D5B:623 AimChainAtPlayer0D5B
 * @run 70:B623
 * WHAT IT DOES: Puts the angle from the ninth slot to the player, held to $50 to $B0 and rounded down to a multiple
 * of 8, in OBJ_WORK_H of the object and of the nine slots after it (Stage 08's overlay).
 * HOW: AngleToPlayerFromNinthSlot0D5B, the clamp piece, AND #$F8, a loop of stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void AimChainAtPlayer0D5B(void)
{
    jsr_push(0xB626);
    AngleToPlayerFromNinthSlot0D5B();
    jsr_pop();
    jsr_push(0xB629);
    ClampAngle50ToB0_0D5B();
    jsr_pop();
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

void abi_AimChainAtPlayer0D5B(void)
{
    AimChainAtPlayer0D5B();
}

/*
 * @routine 0D5B:642 ClampAngle50ToB0_0D5B
 * @run 70:B642
 * WHAT IT DOES: Holds the angle in A to $50 to $B0 (Stage 08's overlay).
 * HOW: Two compares.
 * WAITS: no.
 * Entry: A the angle. Exit: A the angle held to $50 to $B0.
 */
void ClampAngle50ToB0_0D5B(void)
{
    op_cmp(0x50);
    if (flag_c()) {
        op_cmp(0xB0);
        if (flag_c()) {
            goto at_B64E;
        }
        return;
    }
    op_lda(0x50);
    return;
at_B64E:
    op_lda(0xB0);
}

void abi_ClampAngle50ToB0_0D5B(void)
{
    ClampAngle50ToB0_0D5B();
}

/*
 * @routine 0D5B:671 TurnToZeroThenState1_0D5B
 * @run 70:B671
 * WHAT IT DOES: Turns the angle OBJ_WORK_D by 8 toward 0; on reaching it puts the object in state 1, substate 0,
 * with OBJ_WORK_E, K and L cleared (Stage 08's overlay).
 * HOW: TurnAngleToward, a compare, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void TurnToZeroThenState1_0D5B(uint8_t slot)
{
    WR8(ZP->temp02, 0);
    op_lda(0x08);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xB67F);
    turn_angle_exit_shim(TurnAngleToward());
    jsr_pop();
    WR8(OBJ->work_d[slot], cpu.a);
    op_cmp(RD8(ZP->temp02));
    if (flag_z()) {
        op_lda(0x01);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
        WR8(OBJ->work_e[slot], 0);
        WR8(OBJ->work_k[slot], 0);
        WR8(OBJ->work_l[slot], 0);
    }
}

void abi_TurnToZeroThenState1_0D5B(void)
{
    TurnToZeroThenState1_0D5B(cpu.x);
}

/*
 * @routine 0D5B:698 DropChainThenFall0D5B
 * @run 70:B698
 * WHAT IT DOES: In substate 0 puts the nine slots after the object in state 3 with its OBJ_START_ARG in their
 * OBJ_WORK_L, gives it frame 6, OBJ_WORK_O $60, no hit box, no X speed and a Y speed of -4, and steps the substate
 * on; later counts OBJ_WORK_O down, clearing it at zero, and meanwhile runs SpawnKind9COnBeat0D5B and falls under
 * gravity (Stage 08's overlay).
 * HOW: The piece and stores; or DEC, ClearObject, a call, ApplyGravityUnlimited and ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void DropChainThenFall0D5B(void)
{
    op_lda(RD8(OBJ->substate[cpu.x]));
    if (flag_z()) {
        jsr_push(0xB6A0);
        ChainToState3_0D5B(cpu.x);
        jsr_pop();
        op_lda(0x06);
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
    call_routine(0xB6CA, 0xB6EC);
    jsr_push(0xB6CD);
    gravity_exit_shim(ApplyGravityUnlimited(cpu.x));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocity(cpu.x));
}

void abi_DropChainThenFall0D5B(void)
{
    DropChainThenFall0D5B();
}

/*
 * @routine 0D5B:6D0 ChainToState3_0D5B
 * @run 70:B6D0
 * WHAT IT DOES: Puts the nine slots after the object in state 3, substate 0, each with the object's OBJ_START_ARG
 * in its OBJ_WORK_L (Stage 08's overlay).
 * HOW: A loop of three stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ChainToState3_0D5B(uint8_t slot)
{
    op_txa();
    op_tay();
    op_iny();
    op_lda(0x09);
    WR8(ZP->temp01, cpu.a);
    do {
        op_lda(0x03);
        WR8(OBJ->state[cpu.y], cpu.a);
        op_lda(RD8(OBJ->start_arg[slot]));
        WR8(OBJ->work_l[cpu.y], cpu.a);
        op_cla();
        WR8(OBJ->substate[cpu.y], cpu.a);
        op_iny();
        RMW8(ZP->temp01, v, f_dec(v));
    } while (!flag_z());
}

void abi_ChainToState3_0D5B(void)
{
    ChainToState3_0D5B(cpu.x);
}

/*
 * @routine 0D5B:715 ActByState0D5B_715
 * @run 70:B715
 * WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByState0D5B_715Jumps = 0xB727,   /* the routine for each state, a word each */
};

void ActByState0D5B_715(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByState0D5B_715Jumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByState0D5B_715Jumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByState0D5B_715(void)
{
    ActByState0D5B_715(cpu.x);
}

/*
 * @routine 0D5B:7F1 TurnToAimThenState1_0D5B
 * @run 70:B7F1
 * WHAT IT DOES: Turns the angle OBJ_WORK_D by 8 toward OBJ_WORK_H and counts OBJ_WORK_I down; at zero clears
 * OBJ_WORK_J and OBJ_WORK_G and puts the object in state 1, substate 0 (Stage 08's overlay).
 * HOW: TurnAngleToward, DEC, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void TurnToAimThenState1_0D5B(uint8_t slot)
{
    op_lda(RD8(OBJ->work_h[slot]));
    WR8(ZP->temp02, cpu.a);
    op_lda(0x08);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xB802);
    turn_angle_exit_shim(TurnAngleToward());
    jsr_pop();
    WR8(OBJ->work_d[slot], cpu.a);
    RMW8(OBJ->work_i[slot], v, f_dec(v));
    if (flag_z()) {
        WR8(OBJ->work_j[slot], 0);
        WR8(OBJ->work_g[slot], 0);
        op_lda(0x01);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
    }
}

void abi_TurnToAimThenState1_0D5B(void)
{
    TurnToAimThenState1_0D5B(cpu.x);
}

/*
 * @routine 0D5C:019 ActBySubstate0D5C_019
 * @run 70:B819
 * WHAT IT DOES: Runs the routine for the object's substate (Stage 08's overlay).
 * HOW: A jump table by the substate.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActBySubstate0D5C_019Jumps = 0xB82B,   /* the routine for each substate, a word each */
};

void ActBySubstate0D5C_019(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActBySubstate0D5C_019Jumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActBySubstate0D5C_019Jumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActBySubstate0D5C_019(void)
{
    ActBySubstate0D5C_019(cpu.x);
}

/*
 * @routine 0D5C:082 FallToFloorOrClear0D5C
 * @run 70:B882
 * WHAT IT DOES: Clears the object once its Y high byte is not 0; else falls and lands with probe height 0, and on
 * landing sets OBJ_WORK_M $20 and steps the substate on (Stage 08's overlay).
 * HOW: A test, ClearObject or StopOnLanding, a store and INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FallToFloorOrClear0D5C(uint8_t slot)
{
    op_lda(RD8(OBJ->y_hi[slot]));
    if (!flag_z()) {
        ClearObject(slot);
        return;
    }
    op_lda(0x00);
    jsr_push(0xB88F);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        op_lda(0x20);
        WR8(OBJ->work_m[slot], cpu.a);
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_FallToFloorOrClear0D5C(void)
{
    FallToFloorOrClear0D5C(cpu.x);
}

/*
 * @routine 0D5C:105 StartWithCounts8And9_0D5C
 * @run 70:B905
 * WHAT IT DOES: Starts the object from its record with OBJ_WORK_F 8 and OBJ_WORK_E 9 (Stage 08's overlay).
 * HOW: LoadObjectRecord and two stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartWithCounts8And9_0D5C(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xB908);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x08);
    WR8(OBJ->work_f[slot], cpu.a);
    op_lda(0x09);
    WR8(OBJ->work_e[slot], cpu.a);
}

void abi_StartWithCounts8And9_0D5C(void)
{
    StartWithCounts8And9_0D5C(cpu.x);
}

/*
 * @routine 0D5C:275 ActBySubstate0D5C_275
 * @run 70:BA75
 * WHAT IT DOES: Runs the routine for the object's substate (Stage 08's overlay).
 * HOW: A jump table by the substate.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActBySubstate0D5C_275Jumps = 0xBA87,   /* the routine for each substate, a word each */
};

void ActBySubstate0D5C_275(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActBySubstate0D5C_275Jumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActBySubstate0D5C_275Jumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActBySubstate0D5C_275(void)
{
    ActBySubstate0D5C_275(cpu.x);
}

/*
 * @routine 0D5C:3ED ActByState0D5C_3ED
 * @run 70:BBED
 * WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByState0D5C_3EDJumps = 0xBBFF,   /* the routine for each state, a word each */
};

void ActByState0D5C_3ED(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByState0D5C_3EDJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByState0D5C_3EDJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByState0D5C_3ED(void)
{
    ActByState0D5C_3ED(cpu.x);
}

/*
 * @routine 0D5C:405 StartFromRecord0D5C
 * @run 70:BC05
 * WHAT IT DOES: Starts the object from its record (Stage 08's overlay).
 * HOW: LoadObjectRecord.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartFromRecord0D5C(void)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xBC08);
        last = LoadObjectRecord(cpu.x, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
}

void abi_StartFromRecord0D5C(void)
{
    StartFromRecord0D5C();
}

/*
 * @routine 0D5C:51C StartFromRecordInState0_0D5C
 * @run 70:BD1C
 * WHAT IT DOES: In state 0, starts the object from its record (Stage 08's overlay).
 * HOW: A test and LoadObjectRecord.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartFromRecordInState0_0D5C(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xBD24);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
    }
}

void abi_StartFromRecordInState0_0D5C(void)
{
    StartFromRecordInState0_0D5C(cpu.x);
}

/*
 * @routine 0D5C:540 ClearTemp00Bit3ThenByState0D5C
 * @run 70:BD40
 * WHAT IT DOES: Clears bit 3 of zTemp00 and falls into ActByState0D5C_542, which overwrites it (Stage 08's
 * overlay).
 * HOW: RMB3, then the next routine.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ClearTemp00Bit3ThenByState0D5C(void)
{
    RMW8(ZP->temp00, v, v & 0xF7);
    /* falls into $BD42 */
    ActByState0D5C_542(cpu.x);
}

void abi_ClearTemp00Bit3ThenByState0D5C(void)
{
    ClearTemp00Bit3ThenByState0D5C();
}

/*
 * @routine 0D5C:542 ActByState0D5C_542
 * @run 70:BD42
 * WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByState0D5C_542Jumps = 0xBD54,   /* the routine for each state, a word each */
};

void ActByState0D5C_542(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByState0D5C_542Jumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByState0D5C_542Jumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByState0D5C_542(void)
{
    ActByState0D5C_542(cpu.x);
}

/*
 * @routine 0D5D:3A9 ActAndThrowPartBelow0D5D
 * @run 71:C3A9
 * WHAT IT DOES: Steps OBJ_WORK_G, runs FacePlayerInStates0D5D, StartDeathFall0D5D, HitBoxWidthByFrame0D5D and
 * ActByState0D5D_45C; then, when the slot below holds kind $38 in a state below 2, puts it at the object's place
 * with the object's facing and the X and Y speeds a table gives by the object's frame less $1E (X turned round when
 * facing left), and moves it once (Stage 08's overlay).
 * HOW: INC, four calls, compares of the slot below, CopyObjectPlace, a pair read by twice the frame less $1E, a
 * negate, ApplyVelocity on that slot.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActAndThrowPartBelow0D5DSpeeds = 0xC8E4,   /* X speed high, Y speed high for each frame from $1E */
};

void ActAndThrowPartBelow0D5D(void)
{
    RMW8(OBJ->work_g[cpu.x], v, f_inc(v));
    call_routine(0xC3AF, 0xC3FB);
    jsr_push(0xC3B2);
    StartDeathFall0D5D(cpu.x);
    jsr_pop();
    call_routine(0xC3B5, 0xC44A);
    jsr_push(0xC3B8);
    ActByState0D5D_45C(cpu.x);
    jsr_pop();
    op_lda(rd8((uint16_t)(COL(OBJ->kind) - 1 + cpu.x)));
    op_cmp(0x38);
    if (flag_z()) {
        op_lda(rd8((uint16_t)(COL(OBJ->state) - 1 + cpu.x)));
        op_cmp(0x02);
        if (!flag_c()) {
            op_txa();
            op_tay();
            op_dey();
            jsr_push(0xC3CC);
            copy_place_exit_shim(CopyObjectPlace(cpu.x, cpu.y));
            jsr_pop();
            WR8(ZP->richter_object, cpu.x);
            op_lda(RD8(OBJ->frame[cpu.x]));
            op_sec();
            op_sbc(0x1E);
            cpu.a = f_asl(cpu.a);
            op_tax();
            op_lda(rd8((uint16_t)(ActAndThrowPartBelow0D5DSpeeds + cpu.x)));
            WR8(OBJ->vx_hi[cpu.y], cpu.a);
            op_lda(rd8((uint16_t)(ActAndThrowPartBelow0D5DSpeeds + 1 + cpu.x)));
            WR8(OBJ->vy_hi[cpu.y], cpu.a);
            op_ldx(RD8(ZP->richter_object));
            op_lda(RD8(OBJ->facing_left[cpu.x]));
            wr8((uint16_t)(COL(OBJ->facing_left) - 1 + cpu.x), cpu.a);
            if (!flag_z()) {
                op_lda(RD8(OBJ->vx_hi[cpu.y]));
                op_eor(0xFF);
                cpu.a = f_inc(cpu.a);
                WR8(OBJ->vx_hi[cpu.y], cpu.a);
            }
            op_sxy();
            jsr_push(0xC3F9);
            axis_move_exit_shim(ApplyVelocity(cpu.x));
            jsr_pop();
            op_sxy();
        }
    }
}

void abi_ActAndThrowPartBelow0D5D(void)
{
    ActAndThrowPartBelow0D5D();
}

/*
 * @routine 0D5D:412 StartDeathFall0D5D
 * @run 71:C412
 * WHAT IT DOES: Once the object is dead (OBJ_HIT_STATUS bit 7) outside state 5, turns its hit box off, clears the
 * slot below it, restarts its animation, and puts it in state 5 with no X speed, a Y speed of -$1C0 and OBJ_WORK_D
 * $40 (Stage 08's overlay).
 * HOW: A compare, a test, ClearObjectInY on X less 1, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartDeathFall0D5D(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    op_cmp(0x05);
    if (!flag_z()) {
        op_lda(RD8(OBJ->hit_status[slot]));
        op_and(0x80);
        if (!flag_z()) {
            WR8(OBJ->hitbox_on[slot], 0);
            op_txa();
            op_tay();
            op_dey();
            jsr_push(0xC429);
            ClearObjectInY(cpu.y);
            jsr_pop();
            WR8(OBJ->anim_step[slot], 0);
            WR8(OBJ->anim_timer[slot], 0);
            WR8(OBJ->vx_hi[slot], 0);
            WR8(OBJ->vx_lo[slot], 0);
            op_lda(0xFE);
            WR8(OBJ->vy_hi[slot], cpu.a);
            op_lda(0x40);
            WR8(OBJ->vy_lo[slot], cpu.a);
            op_lda(0x40);
            WR8(OBJ->work_d[slot], cpu.a);
            op_lda(0x05);
            WR8(OBJ->state[slot], cpu.a);
        }
    }
}

void abi_StartDeathFall0D5D(void)
{
    StartDeathFall0D5D(cpu.x);
}

/*
 * @routine 0D5D:45C ActByState0D5D_45C
 * @run 71:C45C
 * WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByState0D5D_45CJumps = 0xC46E,   /* the routine for each state, a word each */
};

void ActByState0D5D_45C(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByState0D5D_45CJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByState0D5D_45CJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByState0D5D_45C(void)
{
    ActByState0D5D_45C(cpu.x);
}

/*
 * @routine 0D5D:4A2 LandThenNextState0D5D
 * @run 71:C4A2
 * WHAT IT DOES: Falls and lands with probe height 6; on landing steps the state on (Stage 08's overlay).
 * HOW: StopOnLanding and INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LandThenNextState0D5D(uint8_t slot)
{
    op_lda(0x06);
    jsr_push(0xC4A7);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_LandThenNextState0D5D(void)
{
    LandThenNextState0D5D(cpu.x);
}

/*
 * @routine 0D5D:4AD WalkForwardUntilNear0D5D
 * @run 71:C4AD
 * WHAT IT DOES: Walks the way it faces (WalkBySpeedIndex0D5D with OBJ_WORK_F its facing); while waiting on its
 * pattern (WaitThenStepPattern0D5D C clear), goes to state 3 once the player is less than $4E away (Stage 08's
 * overlay).
 * HOW: A call, a store, a call, DistanceXToPlayer and a compare.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void WalkForwardUntilNear0D5D(void)
{
    call_routine(0xC4B0, 0xC4C8);
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    WR8(OBJ->work_f[cpu.x], cpu.a);
    call_routine(0xC4B9, 0xC4FD);
    if (!flag_c()) {
        jsr_push(0xC4BE);
        distance_exit_shim(DistanceXToPlayer(cpu.x));
        jsr_pop();
        op_cmp(0x4E);
        if (!flag_c()) {
            op_lda(0x03);
            WR8(OBJ->state[cpu.x], cpu.a);
        }
    }
}

void abi_WalkForwardUntilNear0D5D(void)
{
    WalkForwardUntilNear0D5D();
}

/*
 * @routine 0D5D:546 WalkBackUntilFar0D5D
 * @run 71:C546
 * WHAT IT DOES: Walks against the way it faces (WalkBySpeedIndex0D5D with OBJ_WORK_F its facing flipped); while
 * waiting on its pattern (WaitThenStepPattern0D5D C clear), goes to state 2 once the player is $64 or more away
 * (Stage 08's overlay).
 * HOW: A call, an EOR and a store, a call, DistanceXToPlayer and a compare.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void WalkBackUntilFar0D5D(void)
{
    call_routine(0xC549, 0xC4C8);
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    op_eor(0x01);
    WR8(OBJ->work_f[cpu.x], cpu.a);
    call_routine(0xC554, 0xC4FD);
    if (!flag_c()) {
        jsr_push(0xC559);
        distance_exit_shim(DistanceXToPlayer(cpu.x));
        jsr_pop();
        op_cmp(0x64);
        if (flag_c()) {
            op_lda(0x02);
            WR8(OBJ->state[cpu.x], cpu.a);
        }
    }
}

void abi_WalkBackUntilFar0D5D(void)
{
    WalkBackUntilFar0D5D();
}

/*
 * @routine 0D5D:563 ActBySubstate0D5D
 * @run 71:C563
 * WHAT IT DOES: Runs the routine for the object's substate (Stage 08's overlay).
 * HOW: A jump table by the substate.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActBySubstate0D5DJumps = 0xC575,   /* the routine for each substate, a word each */
};

void ActBySubstate0D5D(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActBySubstate0D5DJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActBySubstate0D5DJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActBySubstate0D5D(void)
{
    ActBySubstate0D5D(cpu.x);
}

/*
 * @routine 0D5D:5ED PlayListC60FAndShoot0D5D
 * @run 71:C5ED
 * WHAT IT DOES: Plays the frame list at $C60F; when it ends restarts in state 3 (RestartInState3_0D5D), and when
 * the frame changes runs ShootKind37AtStep5_0D5D (Stage 08's overlay).
 * HOW: StepObjectFrames, then a jump or the piece.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void PlayListC60FAndShoot0D5D(void)
{
    op_lda(0x0F);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xC6);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC5F8);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        run_routine(0xC600);
        return;
    }
    if (flag_c()) {
        ShootKind37AtStep5_0D5D(cpu.x);
        return;
    }
}

void abi_PlayListC60FAndShoot0D5D(void)
{
    PlayListC60FAndShoot0D5D();
}

/*
 * @routine 0D5D:61C ShootKind37AtStep5_0D5D
 * @run 71:C61C
 * WHAT IT DOES: At step 5 of the object's frame list, plays sound 6 and starts an object of kind $37 at its place
 * in a free slot from $30, with its facing and the substate less 1 in OBJ_WORK_E (Stage 08's overlay).
 * HOW: A compare, SoundCommand, FindFreeSlotFrom30, StartObjectAtPlace, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ShootKind37AtStep5_0D5D(uint8_t slot)
{
    op_lda(RD8(OBJ->anim_step[slot]));
    op_cmp(0x05);
    if (flag_z()) {
        op_lda(0x06);
        {
            uint8_t command;

            jsr_push(0xC628);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
            jsr_pop();
        }
        {
            uint8_t a;
            bool found;
            uint8_t found_slot;

            jsr_push(0xC62B);
            found_slot = FindFreeSlotFrom30(&a, &found);
            find_slot_exit_shim(found_slot, a, found);
            jsr_pop();
        }
        if (flag_c()) {
            op_lda(0x37);
            jsr_push(0xC632);
            copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
            jsr_pop();
            op_lda(RD8(OBJ->facing_left[slot]));
            WR8(OBJ->facing_left[cpu.y], cpu.a);
            op_lda(RD8(OBJ->substate[slot]));
            cpu.a = f_dec(cpu.a);
            WR8(OBJ->work_e[cpu.y], cpu.a);
        }
    }
}

void abi_ShootKind37AtStep5_0D5D(void)
{
    ShootKind37AtStep5_0D5D(cpu.x);
}

/*
 * @routine 0D5D:640 PlayListC653AndShoot0D5D
 * @run 71:C640
 * WHAT IT DOES: Plays the frame list at $C653; when it ends restarts in state 3 (RestartInState3_0D5D), and when
 * the frame changes runs ShootKind37AtStep5_0D5D (Stage 08's overlay).
 * HOW: StepObjectFrames, then a jump or the piece.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void PlayListC653AndShoot0D5D(void)
{
    op_lda(0x53);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xC6);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC64B);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        run_routine(0xC600);
        return;
    }
    if (flag_c()) {
        ShootKind37AtStep5_0D5D(cpu.x);
        return;
    }
}

void abi_PlayListC653AndShoot0D5D(void)
{
    PlayListC653AndShoot0D5D();
}

/*
 * @routine 0D5D:65E PlayListC671AndShoot0D5D
 * @run 71:C65E
 * WHAT IT DOES: Plays the frame list at $C671; when it ends restarts in state 3 (RestartInState3_0D5D), and when
 * the frame changes runs ShootKind37AtStep5_0D5D (Stage 08's overlay).
 * HOW: StepObjectFrames, then a jump or the piece.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void PlayListC671AndShoot0D5D(void)
{
    op_lda(0x71);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xC6);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC669);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        run_routine(0xC600);
        return;
    }
    if (flag_c()) {
        ShootKind37AtStep5_0D5D(cpu.x);
        return;
    }
}

void abi_PlayListC671AndShoot0D5D(void)
{
    PlayListC671AndShoot0D5D();
}

/*
 * @routine 0D5D:6CD BounceAndShootThenClear0D5D
 * @run 71:C6CD
 * WHAT IT DOES: Plays the frame list at $C71B and counts OBJ_WORK_D down, clearing the object at zero; else runs
 * ShootKind9COnCount0D5D and falls with probe height 6, and on landing with frame $38 bounces at the Y speed a
 * table gives by OBJ_WORK_H, which steps on up to 6 (Stage 08's overlay).
 * HOW: StepObjectFrames, DEC, ClearObject, a call, StopOnLanding, a compare, a word table read, INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    BounceAndShootThenClear0D5DSpeeds = 0xC70D,   /* the bounce's Y speed for each OBJ_WORK_H, a word each */
};

void BounceAndShootThenClear0D5D(void)
{
    op_lda(0xC7);
    WR8(ZP->temp11, cpu.a);
    op_lda(0x1B);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC6D8);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    RMW8(OBJ->work_d[cpu.x], v, f_dec(v));
    if (flag_z()) {
        ClearObject(cpu.x);
        return;
    }
    call_routine(0xC6E3, 0xC724);
    op_lda(0x06);
    jsr_push(0xC6E8);
    mover_exit_shim(StopOnLanding(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        op_lda(RD8(OBJ->frame[cpu.x]));
        op_cmp(0x38);
        if (flag_z()) {
            op_lda(RD8(OBJ->work_h[cpu.x]));
            cpu.a = f_asl(cpu.a);
            op_tay();
            op_lda(rd8((uint16_t)(BounceAndShootThenClear0D5DSpeeds + 1 + cpu.y)));
            WR8(OBJ->vy_hi[cpu.x], cpu.a);
            op_lda(rd8((uint16_t)(BounceAndShootThenClear0D5DSpeeds + cpu.y)));
            WR8(OBJ->vy_lo[cpu.x], cpu.a);
            op_lda(RD8(OBJ->work_h[cpu.x]));
            op_cmp(0x06);
            if (!flag_z()) {
                RMW8(OBJ->work_h[cpu.x], v, f_inc(v));
            }
        }
    }
}

void abi_BounceAndShootThenClear0D5D(void)
{
    BounceAndShootThenClear0D5D();
}

/*
 * @routine 0D5D:776 ActByState0D5D_776
 * @run 71:C776
 * WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByState0D5D_776Jumps = 0xC788,   /* the routine for each state, a word each */
};

void ActByState0D5D_776(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByState0D5D_776Jumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByState0D5D_776Jumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByState0D5D_776(void)
{
    ActByState0D5D_776(cpu.x);
}

/*
 * @routine 0D5D:790 StartWithSpeedByWorkE0D5D
 * @run 71:C790
 * WHAT IT DOES: Starts the object from its record, runs MoveByDirectionIndex0D5D and sets OBJ_WORK_F $48; with
 * OBJ_WORK_E not 2 gives it the X speed a table gives by OBJ_WORK_E and no Y speed, and with 2 the X speed a table
 * gives by the player's distance over 32 and a Y speed of -5; facing left, the X speed is turned round and
 * OBJ_WORK_D set 1 (Stage 08's overlay).
 * HOW: LoadObjectRecord, a call, word table reads, DistanceXToPlayer and LSRs, NegateVelocityX.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    StartWithSpeedByWorkE0D5DSpeeds = 0xC7E1,   /* the X speed for each OBJ_WORK_E, a word each */
    StartWithSpeedByWorkE0D5DLeapSpeeds = 0xC7E5,   /* the X speed for each 32 pixels of distance to the player, a word each */
};

void StartWithSpeedByWorkE0D5D(void)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xC793);
        last = LoadObjectRecord(cpu.x, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    call_routine(0xC796, 0xC7F5);
    op_lda(0x48);
    WR8(OBJ->work_f[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_e[cpu.x]));
    op_cmp(0x02);
    if (flag_z()) {
        goto at_C7C4;
    }
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(StartWithSpeedByWorkE0D5DSpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(StartWithSpeedByWorkE0D5DSpeeds + cpu.y)));
    WR8(OBJ->vx_lo[cpu.x], cpu.a);
    WR8(OBJ->vy_hi[cpu.x], 0);
    WR8(OBJ->vy_lo[cpu.x], 0);
at_C7B6:
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    if (!flag_z()) {
        jsr_push(0xC7BE);
        negate_exit_shim(NegateVelocityX(cpu.x));
        jsr_pop();
        op_lda(0x01);
        WR8(OBJ->work_d[cpu.x], cpu.a);
    }
    return;
at_C7C4:
    jsr_push(0xC7C7);
    distance_exit_shim(DistanceXToPlayer(cpu.x));
    jsr_pop();
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(StartWithSpeedByWorkE0D5DLeapSpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(StartWithSpeedByWorkE0D5DLeapSpeeds + cpu.y)));
    WR8(OBJ->vx_lo[cpu.x], cpu.a);
    op_lda(0xFB);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    goto at_C7B6;
}

void abi_StartWithSpeedByWorkE0D5D(void)
{
    StartWithSpeedByWorkE0D5D();
}

/*
 * @routine 0D5E:0A6 ActByState0D5E_0A6
 * @run 71:C8A6
 * WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByState0D5E_0A6Jumps = 0xC8B8,   /* the routine for each state, a word each */
};

void ActByState0D5E_0A6(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByState0D5E_0A6Jumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByState0D5E_0A6Jumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByState0D5E_0A6(void)
{
    ActByState0D5E_0A6(cpu.x);
}

/*
 * @routine 0D5E:126 FallUnderGravity0D5E
 * @run 71:C926
 * WHAT IT DOES: Falls under gravity and moves by its speed (Stage 08's overlay).
 * HOW: ApplyGravityUnlimited and ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FallUnderGravity0D5E(void)
{
    jsr_push(0xC929);
    gravity_exit_shim(ApplyGravityUnlimited(cpu.x));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocity(cpu.x));
}

void abi_FallUnderGravity0D5E(void)
{
    FallUnderGravity0D5E();
}

/*
 * @routine 0D5E:175 WalkTowardUntilNear0D5E
 * @run 71:C975
 * WHAT IT DOES: Walks (WalkWhileFrameChangesC) with OBJ_WORK_E the side of the player, goes to state 3 once the
 * player is less than $4C away, then checks the ground ahead (TurnAtLedgeOrWall0D5E) (Stage 08's overlay).
 * HOW: A call, SideOfPlayer, DistanceXToPlayer and a compare, then the piece.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void WalkTowardUntilNear0D5E(void)
{
    call_routine(0xC978, 0xC98F);
    jsr_push(0xC97B);
    side_of_player_exit_shim(SideOfPlayer(cpu.x));
    jsr_pop();
    op_and(0x01);
    WR8(OBJ->work_e[cpu.x], cpu.a);
    jsr_push(0xC983);
    distance_exit_shim(DistanceXToPlayer(cpu.x));
    jsr_pop();
    op_cmp(0x4C);
    if (!flag_c()) {
        op_lda(0x03);
        WR8(OBJ->state[cpu.x], cpu.a);
    }
    TurnAtLedgeOrWall0D5E(cpu.x);
}

void abi_WalkTowardUntilNear0D5E(void)
{
    WalkTowardUntilNear0D5E();
}

/*
 * @routine 0D5E:21F TurnAtLedgeOrWall0D5E
 * @run 71:CA1F
 * WHAT IT DOES: Probes the floor; unless all three columns have floor, or when a wall probe finds a wall (then also
 * flipping OBJ_WORK_E), restarts the animation in state 5, substate 0; otherwise counts OBJ_WORK_D down and at zero
 * restarts the animation in state 4 (Stage 08's overlay).
 * HOW: ProbeTerrain request 9, an AND of the three TileHit bytes, ProbeTerrain request $22, stores or DEC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void TurnAtLedgeOrWall0D5E(uint8_t slot)
{
    op_lda(0x0A);
    WR8(ZP->product.mid, cpu.a);
    op_lda(0x09);
    jsr_push(0xCA28);
    probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
    jsr_pop();
    op_lda(RD8(LEVEL->tile_hit[0]));
    op_and(RD8(LEVEL->tile_hit[1]));
    op_and(RD8(LEVEL->tile_hit[2]));
    op_and(0x01);
    if (!flag_z()) {
        op_lda(0x0A);
        WR8(ZP->product.mid, cpu.a);
        op_lda(0x22);
        jsr_push(0xCA3E);
        probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
        jsr_pop();
        op_and(0x02);
        if (flag_z()) {
            goto at_CA59;
        }
        op_lda(RD8(OBJ->work_e[slot]));
        op_eor(0x01);
        WR8(OBJ->work_e[slot], cpu.a);
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->substate[slot], 0);
    op_lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
    return;
at_CA59:
    RMW8(OBJ->work_d[slot], v, f_dec(v));
    if (flag_z()) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        op_lda(0x04);
        WR8(OBJ->state[slot], cpu.a);
    }
}

void abi_TurnAtLedgeOrWall0D5E(void)
{
    TurnAtLedgeOrWall0D5E(cpu.x);
}

/*
 * @routine 0D5E:26A WalkAwayUntilFar0D5E
 * @run 71:CA6A
 * WHAT IT DOES: Walks (WalkWhileFrameChangesC) with OBJ_WORK_E away from the player, goes to state 2 once the
 * player is $6C or more away, then checks the ground ahead (TurnAtLedgeOrWall0D5E) (Stage 08's overlay).
 * HOW: A call, SideOfPlayer flipped, DistanceXToPlayer and a compare, then the piece.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void WalkAwayUntilFar0D5E(void)
{
    call_routine(0xCA6D, 0xC98F);
    jsr_push(0xCA70);
    side_of_player_exit_shim(SideOfPlayer(cpu.x));
    jsr_pop();
    op_and(0x01);
    op_eor(0x01);
    WR8(OBJ->work_e[cpu.x], cpu.a);
    jsr_push(0xCA7A);
    distance_exit_shim(DistanceXToPlayer(cpu.x));
    jsr_pop();
    op_cmp(0x6C);
    if (flag_c()) {
        op_lda(0x02);
        WR8(OBJ->state[cpu.x], cpu.a);
    }
    TurnAtLedgeOrWall0D5E(cpu.x);
}

void abi_WalkAwayUntilFar0D5E(void)
{
    WalkAwayUntilFar0D5E();
}

/*
 * @routine 0D5E:2CB ActBySubstate0D5E
 * @run 71:CACB
 * WHAT IT DOES: Runs the routine for the object's substate (Stage 08's overlay).
 * HOW: A jump table by the substate.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActBySubstate0D5EJumps = 0xCADD,   /* the routine for each substate, a word each */
};

void ActBySubstate0D5E(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActBySubstate0D5EJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActBySubstate0D5EJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActBySubstate0D5E(void)
{
    ActBySubstate0D5E(cpu.x);
}

/*
 * @routine 0D5E:45D ActByStateAfterStruckCheck0D5E
 * @run 71:CC5D
 * WHAT IT DOES: Steps OBJ_WORK_D, runs State5WhenStruck0D5E, then the routine for the object's state (Stage 08's
 * overlay).
 * HOW: INC, a call, then a jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByStateAfterStruckCheck0D5EJumps = 0xCC75,   /* the routine for each state, a word each */
};

void ActByStateAfterStruckCheck0D5E(void)
{
    RMW8(OBJ->work_d[cpu.x], v, f_inc(v));
    call_routine(0xCC63, 0xCC81);
    op_lda(RD8(OBJ->state[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByStateAfterStruckCheck0D5EJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByStateAfterStruckCheck0D5EJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByStateAfterStruckCheck0D5E(void)
{
    ActByStateAfterStruckCheck0D5E();
}

/*
 * @routine 0D5E:53B WalkForwardUntilClose0D5E
 * @run 71:CD3B
 * WHAT IT DOES: Walks (WalkOnFloorOrFall0D5E) the way it faces; every $40 counts of OBJ_WORK_D turns to face the
 * player and goes to state 4 when it is less than 8 away (Stage 08's overlay).
 * HOW: A call, a store, a test of OBJ_WORK_D's low six bits, SideOfPlayer, DistanceXToPlayer and a compare.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void WalkForwardUntilClose0D5E(void)
{
    call_routine(0xCD3E, 0xCD60);
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    WR8(OBJ->work_f[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_and(0x3F);
    if (flag_z()) {
        jsr_push(0xCD4E);
        side_of_player_exit_shim(SideOfPlayer(cpu.x));
        jsr_pop();
        op_and(0x01);
        WR8(OBJ->facing_left[cpu.x], cpu.a);
        jsr_push(0xCD56);
        distance_exit_shim(DistanceXToPlayer(cpu.x));
        jsr_pop();
        op_cmp(0x08);
        if (!flag_c()) {
            op_lda(0x04);
            WR8(OBJ->state[cpu.x], cpu.a);
        }
    }
}

void abi_WalkForwardUntilClose0D5E(void)
{
    WalkForwardUntilClose0D5E();
}

/*
 * @routine 0D5E:5DA WalkBackUntilAway0D5E
 * @run 71:CDDA
 * WHAT IT DOES: Walks (WalkOnFloorOrFall0D5E) against the way it faces; every $40 counts of OBJ_WORK_D turns to
 * face the player and goes to state 3 when it is $30 or more away (Stage 08's overlay).
 * HOW: A call, an EOR and a store, a test of OBJ_WORK_D's low six bits, SideOfPlayer, DistanceXToPlayer and a
 * compare.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void WalkBackUntilAway0D5E(void)
{
    call_routine(0xCDDD, 0xCD60);
    op_lda(RD8(OBJ->facing_left[cpu.x]));
    op_eor(0x01);
    WR8(OBJ->work_f[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_and(0x3F);
    if (flag_z()) {
        jsr_push(0xCDEF);
        side_of_player_exit_shim(SideOfPlayer(cpu.x));
        jsr_pop();
        op_and(0x01);
        WR8(OBJ->facing_left[cpu.x], cpu.a);
        jsr_push(0xCDF7);
        distance_exit_shim(DistanceXToPlayer(cpu.x));
        jsr_pop();
        op_cmp(0x30);
        if (flag_c()) {
            op_lda(0x03);
            WR8(OBJ->state[cpu.x], cpu.a);
        }
    }
}

void abi_WalkBackUntilAway0D5E(void)
{
    WalkBackUntilAway0D5E();
}

/*
 * @routine 0D5E:72E RoamAndKeepSpeedHistory0D5E
 * @run 71:CF2E
 * WHAT IT DOES: Runs StartOrRoamNearPlayer0D5E, then shifts $3F bytes of four speed histories along by one from the
 * place a table gives by OBJ_START_ARG, and puts its X and Y speeds at their start (Stage 08's overlay).
 * HOW: A call, a copy loop of four buffers, a table read less $3F and four stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RoamAndKeepSpeedHistory0D5EVxHi = 0xB2F3,   /* the X speed high byte history at $B2F4; read one byte below */
    RoamAndKeepSpeedHistory0D5EVxLo = 0xB373,   /* the X speed low byte history at $B374; read one byte below */
    RoamAndKeepSpeedHistory0D5EVyHi = 0xB3F3,   /* the Y speed high byte history at $B3F4; read one byte below */
    RoamAndKeepSpeedHistory0D5EVyLo = 0xB473,   /* the Y speed low byte history at $B474; read one byte below */
    RoamAndKeepSpeedHistory0D5EBases = 0xCF7C,   /* the buffers' index, plus $3F, for each OBJ_START_ARG */
};

void RoamAndKeepSpeedHistory0D5E(void)
{
    call_routine(0xCF31, 0xCF7E);
    op_lda(0x3F);
    WR8(ZP->temp00, cpu.a);
    op_ldy(RD8(OBJ->start_arg[cpu.x]));
    op_lda(rd8((uint16_t)(RoamAndKeepSpeedHistory0D5EBases + cpu.y)));
    op_tay();
    do {
        op_lda(rd8((uint16_t)(RoamAndKeepSpeedHistory0D5EVxHi + cpu.y)));
        wr8((uint16_t)(RoamAndKeepSpeedHistory0D5EVxHi + 1 + cpu.y), cpu.a);
        op_lda(rd8((uint16_t)(RoamAndKeepSpeedHistory0D5EVxLo + cpu.y)));
        wr8((uint16_t)(RoamAndKeepSpeedHistory0D5EVxLo + 1 + cpu.y), cpu.a);
        op_lda(rd8((uint16_t)(RoamAndKeepSpeedHistory0D5EVyHi + cpu.y)));
        wr8((uint16_t)(RoamAndKeepSpeedHistory0D5EVyHi + 1 + cpu.y), cpu.a);
        op_lda(rd8((uint16_t)(RoamAndKeepSpeedHistory0D5EVyLo + cpu.y)));
        wr8((uint16_t)(RoamAndKeepSpeedHistory0D5EVyLo + 1 + cpu.y), cpu.a);
        op_dey();
        RMW8(ZP->temp00, v, f_dec(v));
    } while (!flag_z());
    op_ldy(RD8(OBJ->start_arg[cpu.x]));
    op_lda(rd8((uint16_t)(RoamAndKeepSpeedHistory0D5EBases + cpu.y)));
    op_sec();
    op_sbc(0x3F);
    op_tay();
    op_lda(RD8(OBJ->vx_hi[cpu.x]));
    wr8((uint16_t)(RoamAndKeepSpeedHistory0D5EVxHi + 1 + cpu.y), cpu.a);
    op_lda(RD8(OBJ->vx_lo[cpu.x]));
    wr8((uint16_t)(RoamAndKeepSpeedHistory0D5EVxLo + 1 + cpu.y), cpu.a);
    op_lda(RD8(OBJ->vy_hi[cpu.x]));
    wr8((uint16_t)(RoamAndKeepSpeedHistory0D5EVyHi + 1 + cpu.y), cpu.a);
    op_lda(RD8(OBJ->vy_lo[cpu.x]));
    wr8((uint16_t)(RoamAndKeepSpeedHistory0D5EVyLo + 1 + cpu.y), cpu.a);
}

void abi_RoamAndKeepSpeedHistory0D5E(void)
{
    RoamAndKeepSpeedHistory0D5E();
}

/*
 * @routine 0D5F:083 ActByState0D5F_083
 * @run 71:D083
 * WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByState0D5F_083Jumps = 0xD095,   /* the routine for each state, a word each */
};

void ActByState0D5F_083(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByState0D5F_083Jumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByState0D5F_083Jumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByState0D5F_083(void)
{
    ActByState0D5F_083(cpu.x);
}

/*
 * @routine 0D5F:17A BlockActUnlessSlowed0D5F
 * @run 71:D17A
 * WHAT IT DOES: Bars or carries the player as a block $14 wide and $20 high, goes to state 4 once dead, and unless
 * the frame is skipped by the time-slowing, runs FallWithLowerProbe0D5F, HitBoxXByFrame0D5F and the routine for its
 * state (Stage 08's overlay).
 * HOW: Three pieces, two calls, then a jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    BlockActUnlessSlowed0D5FJumps = 0xD19E,   /* the routine for each state, a word each */
};

void BlockActUnlessSlowed0D5F(void)
{
    jsr_push(0xD17D);
    BlockContact14By20_0D5F();
    jsr_pop();
    jsr_push(0xD180);
    State4WhenDead0D5F(cpu.x);
    jsr_pop();
    jsr_push(0xD183);
    SkipFrameWhenSlowed0D5F(cpu.x);
    jsr_pop();
    if (flag_c()) {
        return;
    }
    call_routine(0xD189, 0xD1FD);
    call_routine(0xD18C, 0xD255);
    op_lda(RD8(OBJ->state[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(BlockActUnlessSlowed0D5FJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(BlockActUnlessSlowed0D5FJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_BlockActUnlessSlowed0D5F(void)
{
    BlockActUnlessSlowed0D5F();
}

/*
 * @routine 0D5F:1A8 BlockContact14By20_0D5F
 * @run 71:D1A8
 * WHAT IT DOES: Tests the object as a block $14 wide and $20 high against the player (ContactWithPlayer) (Stage
 * 08's overlay).
 * HOW: Two stores, then a jump.
 * WAITS: no.
 * Entry: X the object. Exit: as ContactWithPlayer: C set when the player stands on it.
 */
void BlockContact14By20_0D5F(void)
{
    op_lda(0x14);
    WR8(ZP->temp08, cpu.a);
    op_lda(0x20);
    WR8(ZP->temp09, cpu.a);
    run_routine(0xAA35);
}

void abi_BlockContact14By20_0D5F(void)
{
    BlockContact14By20_0D5F();
}

/*
 * @routine 0D5F:1B3 State4WhenDead0D5F
 * @run 71:D1B3
 * WHAT IT DOES: Once the object is dead (OBJ_HIT_STATUS bit 7) outside state 4, restarts its animation in state 4
 * (Stage 08's overlay).
 * HOW: A compare, a test, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void State4WhenDead0D5F(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    op_cmp(0x04);
    if (!flag_z()) {
        op_lda(RD8(OBJ->hit_status[slot]));
        op_and(0x80);
        if (!flag_z()) {
            WR8(OBJ->anim_step[slot], 0);
            WR8(OBJ->anim_timer[slot], 0);
            op_lda(0x04);
            WR8(OBJ->state[slot], cpu.a);
        }
    }
}

void abi_State4WhenDead0D5F(void)
{
    State4WhenDead0D5F(cpu.x);
}

/*
 * @routine 0D5F:1CD SkipFrameWhenSlowed0D5F
 * @run 71:D1CD
 * WHAT IT DOES: In states other than 0 and 4 with TimeSlowMode on, decides whether this frame is skipped from the
 * pattern byte for zWaveCounter's low three bits, row $18 of one table in mode 2 and of the other otherwise (Stage
 * 08's overlay).
 * HOW: Compares of the state and the mode; a table read by $18 plus the count.
 * WAITS: no.
 * Entry: X the object. Exit: C set when the frame is skipped; else C clear.
 */
enum {
    SkipFrameWhenSlowed0D5FMode1 = 0xA146,   /* the time-slowing pattern bytes read in the other modes; nonzero skips */
    SkipFrameWhenSlowed0D5FMode2 = 0xA166,   /* the time-slowing pattern bytes read in mode 2; nonzero skips */
};

void SkipFrameWhenSlowed0D5F(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (!flag_z()) {
        op_cmp(0x04);
        if (!flag_z()) {
            op_lda(RD8(PVARS->time_slow_mode));
            if (!flag_z()) {
                op_cmp(0x02);
                if (!flag_z()) {
                    goto at_D1EE;
                }
                op_lda(RD8(ZP->wave_counter));
                op_and(0x07);
                op_clc();
                op_adc(0x18);
                op_tay();
                op_lda(rd8((uint16_t)(SkipFrameWhenSlowed0D5FMode2 + cpu.y)));
                if (flag_z()) {
                    goto at_D1FB;
                }
at_D1EC:
                op_sec();
                return;
at_D1EE:
                op_lda(RD8(ZP->wave_counter));
                op_and(0x07);
                op_clc();
                op_adc(0x18);
                op_tay();
                op_lda(rd8((uint16_t)(SkipFrameWhenSlowed0D5FMode1 + cpu.y)));
                if (!flag_z()) {
                    goto at_D1EC;
                }
            }
        }
    }
at_D1FB:
    op_clc();
}

void abi_SkipFrameWhenSlowed0D5F(void)
{
    SkipFrameWhenSlowed0D5F(cpu.x);
}

/*
 * @routine 0D5F:279 StartWithKind52Followers0D5F
 * @run 71:D279
 * WHAT IT DOES: Starts the object from its record; with OBJ_START_ARG not negative, starts that many objects of
 * kind $52 at its place in the slots after it, with OBJ_START_ARG $81 up, the object's slot in OBJ_WORK_M and its
 * slot plus the count in OBJ_WORK_N; then faces it by OBJ_START_ARG bit 0 and runs ShiftOnceByArg0D5F,
 * RandomWaitDE0D5F and RandomWordFG0D5F (Stage 08's overlay).
 * HOW: LoadObjectRecord, a loop of ClearObjectInY and StartObjectAtPlace counting OBJ_START_ARG down, an AND and
 * EOR, three calls.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartWithKind52Followers0D5F(void)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xD27C);
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
            jsr_push(0xD296);
            ClearObjectInY(cpu.y);
            jsr_pop();
            op_lda(0x52);
            jsr_push(0xD29B);
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
    call_routine(0xD2C4, 0xD2CB);
    call_routine(0xD2C7, 0xD2F0);
    call_routine(0xD2CA, 0xD30C);
}

void abi_StartWithKind52Followers0D5F(void)
{
    StartWithKind52Followers0D5F();
}

/*
 * @routine 0D5F:328 CountBothWaits0D5F
 * @run 71:D328
 * WHAT IT DOES: Runs CountWaitThenState2_0D5F and CountWaitThenState3_0D5F (Stage 08's overlay).
 * HOW: Two calls.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CountBothWaits0D5F(void)
{
    jsr_push(0xD32B);
    CountWaitThenState2_0D5F();
    jsr_pop();
    jsr_push(0xD32E);
    CountWaitThenState3_0D5F();
    jsr_pop();
}

void abi_CountBothWaits0D5F(void)
{
    CountBothWaits0D5F();
}

/*
 * @routine 0D5F:32F CountWaitThenState2_0D5F
 * @run 71:D32F
 * WHAT IT DOES: Counts the 16-bit OBJ_WORK_D-E down; at zero takes a new random wait (RandomWaitDE0D5F) and
 * restarts the animation in state 2 (Stage 08's overlay).
 * HOW: An ORA test, a call and stores, or a 16-bit subtract.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CountWaitThenState2_0D5F(void)
{
    op_lda(RD8(OBJ->work_d[cpu.x]));
    op_ora(RD8(OBJ->work_e[cpu.x]));
    if (flag_z()) {
        call_routine(0xD33A, 0xD2F0);
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

void abi_CountWaitThenState2_0D5F(void)
{
    CountWaitThenState2_0D5F();
}

/*
 * @routine 0D5F:358 CountWaitThenState3_0D5F
 * @run 71:D358
 * WHAT IT DOES: While the object's X high byte is 0, counts the 16-bit OBJ_WORK_F-G down; at zero takes a new
 * random word (RandomWordFG0D5F) and restarts the animation in state 3, substate 0 (Stage 08's overlay).
 * HOW: A test, an ORA test, a call and stores, or a 16-bit subtract.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CountWaitThenState3_0D5F(void)
{
    op_lda(RD8(OBJ->x_hi[cpu.x]));
    if (flag_z()) {
        op_lda(RD8(OBJ->work_f[cpu.x]));
        op_ora(RD8(OBJ->work_g[cpu.x]));
        if (flag_z()) {
            call_routine(0xD368, 0xD30C);
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

void abi_CountWaitThenState3_0D5F(void)
{
    CountWaitThenState3_0D5F();
}

/*
 * @routine 0D5F:3BE ActBySubstate0D5F
 * @run 71:D3BE
 * WHAT IT DOES: Runs the routine for the object's substate (Stage 08's overlay).
 * HOW: A jump table by the substate.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActBySubstate0D5FJumps = 0xD3D0,   /* the routine for each substate, a word each */
};

void ActBySubstate0D5F(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActBySubstate0D5FJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActBySubstate0D5FJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActBySubstate0D5F(void)
{
    ActBySubstate0D5F(cpu.x);
}

/*
 * @routine 0D5F:54A ActByState0D5F_54A
 * @run 71:D54A
 * WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByState0D5F_54AJumps = 0xD55C,   /* the routine for each state, a word each */
};

void ActByState0D5F_54A(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByState0D5F_54AJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByState0D5F_54AJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByState0D5F_54A(void)
{
    ActByState0D5F_54A(cpu.x);
}

/*
 * @routine 0D5F:62A ActThenKeepYOfSlot0D5F
 * @run 71:D62A
 * WHAT IT DOES: Runs ActByState0D5F_630, then KeepYOfWorkFSlot0D5F (Stage 08's overlay).
 * HOW: A call and a jump.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ActThenKeepYOfSlot0D5F(void)
{
    jsr_push(0xD62D);
    ActByState0D5F_630(cpu.x);
    jsr_pop();
    run_routine(0xD5DA);
}

void abi_ActThenKeepYOfSlot0D5F(void)
{
    ActThenKeepYOfSlot0D5F();
}

/*
 * @routine 0D5F:630 ActByState0D5F_630
 * @run 71:D630
 * WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByState0D5F_630Jumps = 0xD642,   /* the routine for each state, a word each */
};

void ActByState0D5F_630(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByState0D5F_630Jumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByState0D5F_630Jumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByState0D5F_630(void)
{
    ActByState0D5F_630(cpu.x);
}

/*
 * @routine 0D5F:67F WaitThenSoundAndHitBox0D5F
 * @run 71:D67F
 * WHAT IT DOES: Counts OBJ_WORK_E down; at zero plays sound $26, sets OBJ_WORK_E $40 and OBJ_HITBOX_ON 2 and steps
 * the state on (Stage 08's overlay).
 * HOW: DEC, SoundCommand, stores, INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void WaitThenSoundAndHitBox0D5F(uint8_t slot)
{
    RMW8(OBJ->work_e[slot], v, f_dec(v));
    if (flag_z()) {
        op_lda(0x26);
        {
            uint8_t command;

            jsr_push(0xD689);
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

void abi_WaitThenSoundAndHitBox0D5F(void)
{
    WaitThenSoundAndHitBox0D5F(cpu.x);
}

/*
 * @routine 0D5F:75D StartWithFrameFromArg0D5F
 * @run 71:D75D
 * WHAT IT DOES: In state 0, starts the object from its record with its frame from OBJ_START_ARG and sprite set 6
 * (Stage 08's overlay).
 * HOW: A test, LoadObjectRecord, two stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartWithFrameFromArg0D5F(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xD765);
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

void abi_StartWithFrameFromArg0D5F(void)
{
    StartWithFrameFromArg0D5F(cpu.x);
}

/*
 * @routine 0D60:1F5 ActByState0D60
 * @run 71:D9F5
 * WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByState0D60Jumps = 0xDA07,   /* the routine for each state, a word each */
};

void ActByState0D60(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByState0D60Jumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByState0D60Jumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByState0D60(void)
{
    ActByState0D60(cpu.x);
}

/*
 * @routine 0D60:213 ProbeFloorThreePoints0D60
 * @run 71:DA13
 * WHAT IT DOES: Reads the map 12 pixels left and 12 right of the object, 9 below it, then (FloorUnderCentre0D60) 16
 * below its middle, stopping at the first that has tile bits $21 (Stage 08's overlay).
 * HOW: 16-bit adds into zTemp00-03 and TerrainTileRead twice, an AND #$21, then the next routine.
 * WAITS: no.
 * Entry: X the object. Exit: A the tile's bits $21, Z clear when a point has them; zTemp00 the column and zTemp02
 * the row in the tile.
 */
void ProbeFloorThreePoints0D60(void)
{
    op_lda(RD8(OBJ->x_lo[cpu.x]));
    op_clc();
    op_adc(0xF4);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->x_hi[cpu.x]));
    op_adc(0xFF);
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    op_clc();
    op_adc(0x09);
    WR8(ZP->temp02, cpu.a);
    op_lda(RD8(OBJ->y_hi[cpu.x]));
    op_adc(0x00);
    WR8(ZP->temp03, cpu.a);
    call_routine(0xDA34, 0x8006);
    op_and(0x21);
    if (!flag_z()) {
        return;
    }
    op_lda(RD8(OBJ->x_lo[cpu.x]));
    op_clc();
    op_adc(0x0C);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->x_hi[cpu.x]));
    op_adc(0x00);
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    op_clc();
    op_adc(0x09);
    WR8(ZP->temp02, cpu.a);
    op_lda(RD8(OBJ->y_hi[cpu.x]));
    op_adc(0x00);
    WR8(ZP->temp03, cpu.a);
    call_routine(0xDA5A, 0x8006);
    op_and(0x21);
    if (flag_z()) {
        FloorUnderCentre0D60();
        return;
    }
}

void abi_ProbeFloorThreePoints0D60(void)
{
    ProbeFloorThreePoints0D60();
}

/*
 * @routine 0D60:25F FloorUnderCentre0D60
 * @run 71:DA5F
 * WHAT IT DOES: Reads the map 16 pixels below the object's place (Stage 08's overlay).
 * HOW: A 16-bit add into zTemp00-03, TerrainTileRead, AND #$21.
 * WAITS: no.
 * Entry: X the object. Exit: A the tile's bits $21, Z clear when it has them; zTemp00 the column and zTemp02 the
 * row in the tile.
 */
void FloorUnderCentre0D60(void)
{
    op_lda(RD8(OBJ->x_lo[cpu.x]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->x_hi[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    op_clc();
    op_adc(0x10);
    WR8(ZP->temp02, cpu.a);
    op_lda(RD8(OBJ->y_hi[cpu.x]));
    op_adc(0x00);
    WR8(ZP->temp03, cpu.a);
    call_routine(0xDA7B, 0x8006);
    op_and(0x21);
}

void abi_FloorUnderCentre0D60(void)
{
    FloorUnderCentre0D60();
}

/*
 * @routine 0D60:29E MoveXAndBounceOffWalls0D60
 * @run 71:DA9E
 * WHAT IT DOES: Moves the object along X; with OBJ_START_ARG not negative, reads the map ahead of it on the side it
 * moves to, $0E below, and at a wall (bit 1) moves it back out of the tile and turns its X speed round (Stage 08's
 * overlay).
 * HOW: ApplyVelocityX, a word table read by the speed's sign, TerrainTileRead, a subtract of the column or an add
 * from a table by it, NegateVelocityX.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    MoveXAndBounceOffWalls0D60Pushes = 0xDB02,   /* the push back right for each column in the tile */
    MoveXAndBounceOffWalls0D60Offsets = 0xDB0A,   /* the probe's X offset, plus 1, moving right then left, a word each */
};

void MoveXAndBounceOffWalls0D60(void)
{
    jsr_push(0xDAA1);
    axis_move_exit_shim(ApplyVelocityX(cpu.x));
    jsr_pop();
    op_lda(RD8(OBJ->start_arg[cpu.x]));
    if (!flag_n()) {
        op_lda(RD8(OBJ->vx_hi[cpu.x]));
        cpu.a = f_asl(cpu.a);
        cpu.a = f_rol(cpu.a);
        op_and(0x01);
        cpu.a = f_asl(cpu.a);
        op_tay();
        op_lda(rd8((uint16_t)(MoveXAndBounceOffWalls0D60Offsets + cpu.y)));
        WR8(ZP->temp08, cpu.a);
        op_lda(rd8((uint16_t)(MoveXAndBounceOffWalls0D60Offsets + 1 + cpu.y)));
        WR8(ZP->temp09, cpu.a);
        RMW8(ZP->temp08, v, f_dec(v));
        op_lda(RD8(OBJ->x_lo[cpu.x]));
        op_clc();
        op_adc(RD8(ZP->temp08));
        WR8(ZP->temp00, cpu.a);
        op_lda(RD8(OBJ->x_hi[cpu.x]));
        op_adc(RD8(ZP->temp09));
        WR8(ZP->temp01, cpu.a);
        op_lda(RD8(OBJ->y_lo[cpu.x]));
        op_clc();
        op_adc(0x0E);
        WR8(ZP->temp02, cpu.a);
        op_lda(RD8(OBJ->y_hi[cpu.x]));
        op_adc(0x00);
        WR8(ZP->temp03, cpu.a);
        call_routine(0xDADC, 0x8006);
        op_and(0x02);
        if (!flag_z()) {
            op_lda(RD8(OBJ->vx_hi[cpu.x]));
            if (flag_n()) {
                goto at_DAF2;
            }
            op_lda(RD8(OBJ->x_lo[cpu.x]));
            op_sec();
            op_sbc(RD8(ZP->temp00));
            WR8(OBJ->x_lo[cpu.x], cpu.a);
            jsr_push(0xDAF1);
            negate_exit_shim(NegateVelocityX(cpu.x));
            jsr_pop();
        }
    }
    return;
at_DAF2:
    op_ldy(RD8(ZP->temp00));
    op_lda(RD8(OBJ->x_lo[cpu.x]));
    op_clc();
    op_adc(rd8((uint16_t)(MoveXAndBounceOffWalls0D60Pushes + cpu.y)));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    jsr_push(0xDB01);
    negate_exit_shim(NegateVelocityX(cpu.x));
    jsr_pop();
}

void abi_MoveXAndBounceOffWalls0D60(void)
{
    MoveXAndBounceOffWalls0D60();
}

/*
 * @routine 0D60:3B3 RideSlotAndCarryPlayer0D60
 * @run 71:DBB3
 * WHAT IT DOES: Puts the object 8 pixels below the slot in OBJ_WORK_H, sets its speed to the move that made
 * (keeping the old place in OBJ_WORK_I to L), and carries the player by it (CarryPlayerByHeightTable0D60) (Stage
 * 08's overlay).
 * HOW: Copies, 16-bit subtracts into the speeds, then a call.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void RideSlotAndCarryPlayer0D60(void)
{
    op_lda(RD8(OBJ->x_frac[cpu.x]));
    WR8(OBJ->work_i[cpu.x], cpu.a);
    op_lda(RD8(OBJ->x_lo[cpu.x]));
    WR8(OBJ->work_j[cpu.x], cpu.a);
    op_lda(RD8(OBJ->y_frac[cpu.x]));
    WR8(OBJ->work_k[cpu.x], cpu.a);
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    WR8(OBJ->work_l[cpu.x], cpu.a);
    op_ldy(RD8(OBJ->work_h[cpu.x]));
    op_lda(RD8(OBJ->x_lo[cpu.y]));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    op_lda(RD8(OBJ->x_hi[cpu.y]));
    WR8(OBJ->x_hi[cpu.x], cpu.a);
    op_lda(RD8(OBJ->y_frac[cpu.y]));
    WR8(OBJ->y_frac[cpu.x], cpu.a);
    op_lda(RD8(OBJ->y_lo[cpu.y]));
    op_clc();
    op_adc(0x08);
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    op_cla();
    op_adc(RD8(OBJ->y_hi[cpu.y]));
    WR8(OBJ->y_hi[cpu.x], cpu.a);
    op_lda(RD8(OBJ->x_frac[cpu.x]));
    op_sec();
    op_sbc(RD8(OBJ->work_i[cpu.x]));
    WR8(OBJ->vx_lo[cpu.x], cpu.a);
    op_lda(RD8(OBJ->x_lo[cpu.x]));
    op_sbc(RD8(OBJ->work_j[cpu.x]));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    op_lda(RD8(OBJ->y_frac[cpu.x]));
    op_sec();
    op_sbc(RD8(OBJ->work_k[cpu.x]));
    WR8(OBJ->vy_lo[cpu.x], cpu.a);
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    op_sbc(RD8(OBJ->work_l[cpu.x]));
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    call_routine(0xDC19, 0xDB23);
}

void abi_RideSlotAndCarryPlayer0D60(void)
{
    RideSlotAndCarryPlayer0D60();
}

/*
 * @routine 0D60:41A RollAndLandOnFloor0D60
 * @run 71:DC1A
 * WHAT IT DOES: Carries the player, moves along X bouncing off walls and moves along Y; finding no floor (or with
 * OBJ_START_ARG negative) falls faster; on floor, when falling at 2 or more and on the first screen goes to state 4
 * with OBJ_WORK_G $10 and sound $2C, turns its X speed round on tile bit 5, sits on the floor with no Y speed and
 * steps the state on; then rolls its frames (Stage 08's overlay).
 * HOW: Calls, ApplyVelocityY, ProbeFloorThreePoints0D60, FallFasterTo4_0D60, compares, SoundCommand,
 * NegateVelocityX, RiseByTemp02_0D60, stores, then the frame piece.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void RollAndLandOnFloor0D60(void)
{
    call_routine(0xDC1D, 0xDB23);
    jsr_push(0xDC20);
    MoveXAndBounceOffWalls0D60();
    jsr_pop();
    jsr_push(0xDC23);
    axis_move_exit_shim(ApplyVelocityY(cpu.x));
    jsr_pop();
    op_lda(RD8(OBJ->start_arg[cpu.x]));
    if (!flag_n()) {
        jsr_push(0xDC2B);
        ProbeFloorThreePoints0D60();
        jsr_pop();
        if (!flag_z()) {
            goto at_DC33;
        }
    }
    call_routine(0xDC30, 0xDA7E);
    RollFrames0D60(cpu.x);
    return;
at_DC33:
    op_pha();
    op_lda(RD8(OBJ->vy_hi[cpu.x]));
    op_cmp(0x02);
    if (flag_c()) {
        op_lda(RD8(OBJ->x_hi[cpu.x]));
        op_ora(RD8(OBJ->y_hi[cpu.x]));
        if (flag_z()) {
            op_lda(0x04);
            WR8(OBJ->state[cpu.x], cpu.a);
            op_lda(0x10);
            WR8(OBJ->work_g[cpu.x], cpu.a);
            op_lda(0x2C);
            {
                uint8_t command;

                jsr_push(0xDC52);
                command = cpu.a;
                sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
                jsr_pop();
            }
        }
    }
    op_pla();
    op_and(0x20);
    if (!flag_z()) {
        op_lda(RD8(OBJ->vy_hi[cpu.x]));
        op_cmp(0x02);
        if (flag_c()) {
            jsr_push(0xDC61);
            negate_exit_shim(NegateVelocityX(cpu.x));
            jsr_pop();
        }
    }
    call_routine(0xDC64, 0xDB0E);
    WR8(OBJ->vy_lo[cpu.x], 0);
    WR8(OBJ->vy_hi[cpu.x], 0);
    RMW8(OBJ->state[cpu.x], v, f_inc(v));
    RollFrames0D60(cpu.x);
}

void abi_RollAndLandOnFloor0D60(void)
{
    RollAndLandOnFloor0D60();
}

/*
 * @routine 0D60:470 RollUntilFloorEnds0D60
 * @run 71:DC70
 * WHAT IT DOES: Carries the player and moves along X bouncing off walls; with OBJ_START_ARG not negative, sits on
 * the floor under its middle until there is none, and with it negative goes on until its X is $A2 (clearing it at X
 * low byte $F0 with the high byte not 0); then clears OBJ_WORK_D and E, keeps its X speed in OBJ_WORK_F and steps
 * the state on; it rolls its frames throughout (Stage 08's overlay).
 * HOW: Calls, FloorUnderCentre0D60, RiseByTemp02_0D60, compares of the X bytes, ClearObject, stores, the frame
 * piece.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void RollUntilFloorEnds0D60(void)
{
    call_routine(0xDC73, 0xDB23);
    jsr_push(0xDC76);
    MoveXAndBounceOffWalls0D60();
    jsr_pop();
    op_lda(RD8(OBJ->start_arg[cpu.x]));
    if (flag_n()) {
        goto at_DC90;
    }
    jsr_push(0xDC7E);
    FloorUnderCentre0D60();
    jsr_pop();
    if (flag_z()) {
        goto at_DC9C;
    }
    call_routine(0xDC83, 0xDB0E);
at_DC83:
    RollFrames0D60(cpu.x);
    return;
at_DC86:
    op_lda(RD8(OBJ->x_lo[cpu.x]));
    op_cmp(0xF0);
    if (!flag_z()) {
        goto at_DC83;
    }
    ClearObject(cpu.x);
    return;
at_DC90:
    op_lda(RD8(OBJ->x_hi[cpu.x]));
    if (!flag_z()) {
        goto at_DC86;
    }
    op_lda(RD8(OBJ->x_lo[cpu.x]));
    op_cmp(0xA2);
    if (!flag_z()) {
        goto at_DC83;
    }
at_DC9C:
    op_lda(0x00);
    WR8(OBJ->work_d[cpu.x], cpu.a);
    op_lda(0x00);
    WR8(OBJ->work_e[cpu.x], cpu.a);
    op_lda(RD8(OBJ->vx_hi[cpu.x]));
    WR8(OBJ->work_f[cpu.x], cpu.a);
    RMW8(OBJ->state[cpu.x], v, f_inc(v));
    RollFrames0D60(cpu.x);
}

void abi_RollUntilFloorEnds0D60(void)
{
    RollUntilFloorEnds0D60();
}

/*
 * @routine 0D60:4B2 CarryThenState2_0D60
 * @run 71:DCB2
 * WHAT IT DOES: Carries the player, puts the object in state 2 and rolls its frames (Stage 08's overlay).
 * HOW: A call, a store, the frame piece.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CarryThenState2_0D60(void)
{
    call_routine(0xDCB5, 0xDB23);
    op_lda(0x02);
    WR8(OBJ->state[cpu.x], cpu.a);
    RollFrames0D60(cpu.x);
}

void abi_CarryThenState2_0D60(void)
{
    CarryThenState2_0D60();
}

/*
 * @routine 0D60:4BD CarryAndCountToState3_0D60
 * @run 71:DCBD
 * WHAT IT DOES: Carries the player and counts OBJ_WORK_G down; at zero puts the object in state 3 (Stage 08's
 * overlay).
 * HOW: A call, DEC, a store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CarryAndCountToState3_0D60(void)
{
    call_routine(0xDCC0, 0xDB23);
    RMW8(OBJ->work_g[cpu.x], v, f_dec(v));
    if (flag_z()) {
        op_lda(0x03);
        WR8(OBJ->state[cpu.x], cpu.a);
    }
}

void abi_CarryAndCountToState3_0D60(void)
{
    CarryAndCountToState3_0D60();
}

/*
 * @routine 0D60:4CB RollFrames0D60
 * @run 71:DCCB
 * WHAT IT DOES: Every $10 frames steps the object's frame through $3B to $42, up while it moves left and down
 * otherwise, wrapping round (Stage 08's overlay).
 * HOW: INC and a compare of OBJ_ANIM_TIMER, INC or DEC of the frame, compares, then ReturnOnly0D60.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void RollFrames0D60(uint8_t slot)
{
    op_lda(0x10);
    RMW8(OBJ->anim_timer[slot], v, f_inc(v));
    op_cmp(RD8(OBJ->anim_timer[slot]));
    if (!flag_z()) {
        run_routine(0xDCFA);
        return;
    }
    WR8(OBJ->anim_timer[slot], 0);
    op_lda(RD8(OBJ->vx_hi[slot]));
    if (flag_n()) {
        RMW8(OBJ->frame[slot], v, f_inc(v));
        op_lda(RD8(OBJ->frame[slot]));
        op_cmp(0x43);
        if (!flag_z()) {
            run_routine(0xDCFA);
            return;
        }
        op_lda(0x3B);
    } else {
        RMW8(OBJ->frame[slot], v, f_dec(v));
        op_lda(RD8(OBJ->frame[slot]));
        op_cmp(0x3A);
        if (!flag_z()) {
            run_routine(0xDCFA);
            return;
        }
        op_lda(0x42);
    }
    WR8(OBJ->frame[slot], cpu.a);
    /* falls into $DCFA */
    run_routine(0xDCFA);
}

void abi_RollFrames0D60(void)
{
    RollFrames0D60(cpu.x);
}

/*
 * @routine 0D6C:200 ActByKind59To5C0D6C
 * @run 74:DA00
 * WHAT IT DOES: Runs the object by its kind: $59 by the routine for its state, $5A nothing, $5B by
 * ActByStateKind5B0D6C, $5C by FlyAndFallKind5C0D6C; other kinds nothing (Stage 08's overlay).
 * HOW: Kind compares, then a jump table by the state or a piece.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByKind59To5C0D6CJumps = 0xDA2F,   /* the routine for each state of kind $59, a word each */
};

void ActByKind59To5C0D6C(void)
{
    op_lda(RD8(OBJ->kind[cpu.x]));
    op_cmp(0x59);
    if (!flag_z()) {
        op_cmp(0x5A);
        if (flag_z()) {
            ReturnForKind5A0D6C();
            return;
        }
        op_cmp(0x5B);
        if (flag_z()) {
            ActByStateKind5B0D6C(cpu.x);
            return;
        }
        op_cmp(0x5C);
        if (flag_z()) {
            FlyAndFallKind5C0D6C(cpu.x);
            return;
        }
        return;
    }
    op_lda(RD8(OBJ->state[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByKind59To5C0D6CJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByKind59To5C0D6CJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByKind59To5C0D6C(void)
{
    ActByKind59To5C0D6C();
}

/*
 * @routine 0D6C:237 DrawList6AndSetScroll0D6C
 * @run 74:DA37
 * WHAT IT DOES: Draws block list 6 (DrawBlockListToVram0D6C), puts the object at Y $C0, sets StageVramBuffer+$13 to
 * $31 and the scroll's X limit to $0280, and steps the state on (Stage 08's overlay).
 * HOW: A call with X kept on the stack, stores, INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void DrawList6AndSetScroll0D6C(void)
{
    op_phx();
    op_lda(0x06);
    jsr_push(0xDA3D);
    DrawBlockListToVram0D6C();
    jsr_pop();
    op_plx();
    op_lda(0xC0);
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    op_lda(0x31);
    WR8(STAGEVRAM->buffer[19], cpu.a);
    op_lda(0x02);
    WR8(SCROLLLIM->x_max.hi, cpu.a);
    op_lda(0x80);
    WR8(SCROLLLIM->x_max.lo, cpu.a);
    RMW8(OBJ->state[cpu.x], v, f_inc(v));
}

void abi_DrawList6AndSetScroll0D6C(void)
{
    DrawList6AndSetScroll0D6C();
}

/*
 * @routine 0D6C:269 CountDownAndDrawLists0D6C
 * @run 74:DA69
 * WHAT IT DOES: Sets StageVramBuffer+$13 to $23, the player's OBJ_WORK_E $80 and OBJ_WORK_F $10 and turns the
 * player's hit box off; runs LevelWordByStep0D6C, spawns kind $9E (SpawnKind9EScattered0D6C) once OBJ_WORK_F is
 * below $A0, and counts OBJ_WORK_F down; below $60 also counts OBJ_WORK_E down and each time it ends draws block
 * list OBJ_WORK_D, steps that on, and after list 5 sets OBJ_WORK_F $80 and steps the state on, else OBJ_WORK_E $0C
 * (Stage 08's overlay).
 * HOW: Stores, two calls, compares, DECs, DrawBlockListToVram0D6C, INC and compares.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CountDownAndDrawLists0D6C(void)
{
    op_lda(0x23);
    WR8(STAGEVRAM->buffer[19], cpu.a);
    op_lda(0x80);
    WR8(OBJ->work_e[0x00], cpu.a);
    op_lda(0x10);
    WR8(OBJ->work_f[0x00], cpu.a);
    WR8(OBJ->hitbox_on[0x00], 0);
    call_routine(0xDA7E, 0xDAE9);
    op_lda(RD8(OBJ->work_f[cpu.x]));
    op_cmp(0xA0);
    if (!flag_c()) {
        call_routine(0xDA88, 0xDB77);
        op_lda(RD8(OBJ->work_f[cpu.x]));
        op_cmp(0x60);
        if (!flag_c()) {
            goto at_DA93;
        }
    }
    RMW8(OBJ->work_f[cpu.x], v, f_dec(v));
    return;
at_DA93:
    RMW8(OBJ->work_f[cpu.x], v, f_dec(v));
    RMW8(OBJ->work_e[cpu.x], v, f_dec(v));
    if (flag_z()) {
        op_lda(RD8(OBJ->work_d[cpu.x]));
        op_phx();
        jsr_push(0xDAA2);
        DrawBlockListToVram0D6C();
        jsr_pop();
        op_plx();
        op_lda(RD8(OBJ->work_d[cpu.x]));
        cpu.a = f_inc(cpu.a);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        op_cmp(0x06);
        if (flag_z()) {
            goto at_DAB4;
        }
        op_lda(0x0C);
        WR8(OBJ->work_e[cpu.x], cpu.a);
    }
    return;
at_DAB4:
    op_lda(0x80);
    WR8(OBJ->work_f[cpu.x], cpu.a);
    RMW8(OBJ->state[cpu.x], v, f_inc(v));
}

void abi_CountDownAndDrawLists0D6C(void)
{
    CountDownAndDrawLists0D6C();
}

/*
 * @routine 0D6C:2BD CountUpThenRestoreScroll0D6C
 * @run 74:DABD
 * WHAT IT DOES: Runs SpawnKind9EEvery8_0D6C and LevelWordByStep0D6C and steps OBJ_WORK_F on; when it wraps to 0,
 * clears the level word at $3EB8, records the object's event, sets StageVramBuffer+$13 to $23, the scroll's X limit
 * to $0700 and the player's OBJ_HITBOX_ON 1, and clears the object (Stage 08's overlay).
 * HOW: Two calls, INC, stores, RecordObjectEvent, ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CountUpThenRestoreScroll0D6C(void)
{
    call_routine(0xDAC0, 0xDB72);
    call_routine(0xDAC3, 0xDAE9);
    RMW8(OBJ->work_f[cpu.x], v, f_inc(v));
    if (!flag_z()) {
        return;
    }
    WR8(LEVELW->y.lo, 0);
    WR8(LEVELW->y.hi, 0);
    {
        uint8_t stored;
        uint8_t event;
        uint8_t byte;

        jsr_push(0xDAD2);
        if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
            cpu.a = 0;
            abi_nz(0);
        } else {
            record_event_exit_shim(event, stored, byte);
        }
        jsr_pop();
    }
    op_lda(0x23);
    WR8(STAGEVRAM->buffer[19], cpu.a);
    op_lda(0x07);
    WR8(SCROLLLIM->x_max.hi, cpu.a);
    op_lda(0x00);
    WR8(SCROLLLIM->x_max.lo, cpu.a);
    op_lda(0x01);
    WR8(OBJ->hitbox_on[0x00], cpu.a);
    ClearObject(cpu.x);
}

void abi_CountUpThenRestoreScroll0D6C(void)
{
    CountUpThenRestoreScroll0D6C();
}

/*
 * @routine 0D6C:3C6 DrawBlockListToVram0D6C
 * @run 74:DBC6
 * WHAT IT DOES: Draws the block list whose index is in A: from each VRAM address in it, copies each block it names
 * to VRAM (CopyBlockToVram0D6C), 4 on per block, until $FE gives a new address or $FF ends it (Stage 08's overlay).
 * HOW: A word table read by twice A, a loop of reads through zTemp00 with calls, and adds of 4.
 * WAITS: no.
 * Entry: A the list's index; X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    DrawBlockListToVram0D6CLists = 0xDC5D,   /* the address of each block list, a word each */
};

void DrawBlockListToVram0D6C(void)
{
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(DrawBlockListToVram0D6CLists + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(DrawBlockListToVram0D6CLists + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    op_cly();
at_DBD3:
    op_lda(RD8_PTR_Y(ZP->temp00, cpu.y));
    WR8(ZP->temp10, cpu.a);
    WR8(ZP->temp08, cpu.a);
    op_iny();
    op_lda(RD8_PTR_Y(ZP->temp00, cpu.y));
    WR8(ZP->temp11, cpu.a);
    WR8(ZP->temp09, cpu.a);
    op_iny();
at_DBE1:
    op_lda(RD8_PTR_Y(ZP->temp00, cpu.y));
    op_iny();
    op_cmp(0xFF);
    if (!flag_z()) {
        op_cmp(0xFE);
        if (flag_z()) {
            goto at_DBD3;
        }
        op_phy();
        call_routine(0xDBF0, 0xDC01);
        op_ply();
        op_lda(RD8(ZP->temp08));
        op_clc();
        op_adc(0x04);
        WR8(ZP->temp08, cpu.a);
        WR8(ZP->temp10, cpu.a);
        op_lda(RD8(ZP->temp09));
        WR8(ZP->temp11, cpu.a);
        goto at_DBE1;
    }
}

void abi_DrawBlockListToVram0D6C(void)
{
    DrawBlockListToVram0D6C();
}

/*
 * @routine 0D6C:519 ReturnForKind5A0D6C
 * @run 74:DD19
 * WHAT IT DOES: Returns; what ActByKind59To5C0D6C runs for kind $5A (Stage 08's overlay).
 * HOW: RTS.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ReturnForKind5A0D6C(void)
{
}

void abi_ReturnForKind5A0D6C(void)
{
    ReturnForKind5A0D6C();
}

/*
 * @routine 0D6C:52A ActByStateKind5B0D6C
 * @run 74:DD2A
 * WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
 * HOW: A jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ActByStateKind5B0D6CJumps = 0xDD3C,   /* the routine for each state, a word each */
};

void ActByStateKind5B0D6C(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ActByStateKind5B0D6CJumps + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ActByStateKind5B0D6CJumps + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ActByStateKind5B0D6C(void)
{
    ActByStateKind5B0D6C(cpu.x);
}

/*
 * @routine 0D6C:5B2 StartKind5CRowsThenMask0D6C
 * @run 74:DDB2
 * WHAT IT DOES: While slot $1E does not hold kind $9B, counts OBJ_WORK_M down; at zero starts three objects of kind
 * $5C (StartKind5CByWorkL0D6C), and with all three started masks tiles by the record (MaskTilesByRecord0D6C), sets
 * OBJ_WORK_M 4 and steps OBJ_WORK_L on, and at $12 sets OBJ_WORK_M $81 and steps the state on (Stage 08's overlay).
 * HOW: A kind compare, DEC, three calls each needing C set, a call with X kept on the stack, stores, INC and a
 * compare.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartKind5CRowsThenMask0D6C(void)
{
    op_lda(RD8(OBJ->kind[0x1E]));
    op_cmp(0x9B);
    if (!flag_z()) {
        op_lda(RD8(OBJ->work_m[cpu.x]));
        if (!flag_z()) {
            RMW8(OBJ->work_m[cpu.x], v, f_dec(v));
            return;
        }
        call_routine(0xDDC5, 0xDE2A);
        if (flag_c()) {
            call_routine(0xDDCA, 0xDE2A);
            if (flag_c()) {
                call_routine(0xDDCF, 0xDE2A);
                if (flag_c()) {
                    op_phx();
                    call_routine(0xDDD5, 0xDEE6);
                    op_plx();
                    op_lda(0x04);
                    WR8(OBJ->work_m[cpu.x], cpu.a);
                    RMW8(OBJ->work_l[cpu.x], v, f_inc(v));
                    op_lda(RD8(OBJ->work_l[cpu.x]));
                    op_cmp(0x12);
                    if (flag_c()) {
                        op_lda(0x81);
                        WR8(OBJ->work_m[cpu.x], cpu.a);
                        RMW8(OBJ->state[cpu.x], v, f_inc(v));
                    }
                }
            }
        }
    }
}

void abi_StartKind5CRowsThenMask0D6C(void)
{
    StartKind5CRowsThenMask0D6C();
}

/*
 * @routine 0D6C:5EE SpawnKind9EEvery8Counts0D6C
 * @run 74:DDEE
 * WHAT IT DOES: Counts OBJ_WORK_M down, clearing the object at zero; every 8 counts starts an object of kind $9E in
 * a free slot from $20 (StartKind9EInY0D6C), plays sound $C1 and sets OBJ_WORK_H of slot $19 to 1 (Stage 08's
 * overlay).
 * HOW: DEC, ClearObject, TST #$07, FindFreeSlotFrom20, a call, SoundCommand, a store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SpawnKind9EEvery8Counts0D6C(void)
{
    RMW8(OBJ->work_m[cpu.x], v, f_dec(v));
    if (flag_z()) {
        ClearObject(cpu.x);
        return;
    }
    op_tst(0x07, RD8(OBJ->work_m[cpu.x]));
    if (flag_z()) {
        {
            uint8_t a;
            bool found;
            uint8_t found_slot;

            jsr_push(0xDDFF);
            found_slot = FindFreeSlotFrom20(&a, &found);
            find_slot_exit_shim(found_slot, a, found);
            jsr_pop();
        }
        if (flag_c()) {
            call_routine(0xDE04, 0xDE0F);
            op_lda(0xC1);
            {
                uint8_t command;

                jsr_push(0xDE09);
                command = cpu.a;
                sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
                jsr_pop();
            }
            op_lda(0x01);
            WR8(OBJ->work_h[0x19], cpu.a);
        }
    }
}

void abi_SpawnKind9EEvery8Counts0D6C(void)
{
    SpawnKind9EEvery8Counts0D6C();
}

/*
 * @routine 0D6C:67D FlyAndFallKind5C0D6C
 * @run 74:DE7D
 * WHAT IT DOES: In state 0 gives the object sprite set 3, frame $6A, attribute $10, OBJ_WORK_D 6 and the X and Y
 * speeds two tables give by its slot's low two bits, and steps the state on; later counts OBJ_WORK_D down, clearing
 * it at zero, moves by its speed and adds $10 to its Y speed (Stage 08's overlay).
 * HOW: Stores and word table reads by twice the slot's low bits; or DEC, ClearObject, ApplyVelocityX and
 * ApplyVelocityY, a 16-bit add.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FlyAndFallKind5C0D6CSpeedsX = 0xDED6,   /* the X speed for each of the slot's low two bits, a word each */
    FlyAndFallKind5C0D6CSpeedsY = 0xDEDE,   /* the Y speed for each of the slot's low two bits, a word each */
};

void FlyAndFallKind5C0D6C(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        op_lda(0x03);
        WR8(OBJ->sprite_set[slot], cpu.a);
        op_lda(0x6A);
        WR8(OBJ->frame[slot], cpu.a);
        op_txa();
        op_and(0x03);
        cpu.a = f_asl(cpu.a);
        op_tay();
        op_lda(rd8((uint16_t)(FlyAndFallKind5C0D6CSpeedsX + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        op_lda(rd8((uint16_t)(FlyAndFallKind5C0D6CSpeedsX + 1 + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        op_lda(rd8((uint16_t)(FlyAndFallKind5C0D6CSpeedsY + cpu.y)));
        WR8(OBJ->vy_lo[slot], cpu.a);
        op_lda(rd8((uint16_t)(FlyAndFallKind5C0D6CSpeedsY + 1 + cpu.y)));
        WR8(OBJ->vy_hi[slot], cpu.a);
        op_lda(0x10);
        WR8(OBJ->sprite_attr[slot], cpu.a);
        op_lda(0x06);
        WR8(OBJ->work_d[slot], cpu.a);
        RMW8(OBJ->state[slot], v, f_inc(v));
        return;
    }
    RMW8(OBJ->work_d[slot], v, f_dec(v));
    if (flag_z()) {
        ClearObject(slot);
        return;
    }
    jsr_push(0xDEC2);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
    jsr_push(0xDEC5);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
    op_lda(RD8(OBJ->vy_lo[slot]));
    op_clc();
    op_adc(0x10);
    WR8(OBJ->vy_lo[slot], cpu.a);
    op_cla();
    op_adc(RD8(OBJ->vy_hi[slot]));
    WR8(OBJ->vy_hi[slot], cpu.a);
}

void abi_FlyAndFallKind5C0D6C(void)
{
    FlyAndFallKind5C0D6C(cpu.x);
}
