/*
 * Stage 05's objects, runs C8EB-D082.
 */
#include "stage_obj_ops.h"
#include "stage05_objects_c.h"
#include "calls_b.h"

/*
 * @routine 093E:0EB LoadPaleEnemy093E
 * @run 71:C8EB
 * WHAT IT DOES: Loads the record with attribute $8D.
 * HOW: LoadObjectRecord.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LoadPaleEnemy093E(uint8_t slot)
{
    (void)slot;
    JSR(0xC8EE, k_load_record());
    lda(0x8D);
    WR8(OBJ->sprite_attr[cpu.x], cpu.a);
}

void abi_LoadPaleEnemy093E(void)
{
    LoadPaleEnemy093E(cpu.x);
}

/*
 * @routine 093E:0F4 WakeWhenNear093E
 * @run 71:C8F4
 * WHAT IT DOES: Argument 0: within $74 of the player, drops the attribute, sets loop flags $60 and a hit box, and goes on.
 * HOW: DistanceXToPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WakeWhenNear093E(uint8_t slot)
{
    lda(RD8(OBJ->start_arg[slot]));
    if (NE) {
        return;
    }
    JSR(0xC8FC, k_distance_x());
    cmp(cpu.a, 0x74);
    if (CS) {
        return;
    }
    slot = cpu.x;
    WR8(OBJ->sprite_attr[slot], 0);
    lda(RD8(OBJ->loop_flags[slot]));
    ora_a(0x60);
    WR8(OBJ->loop_flags[slot], cpu.a);
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_WakeWhenNear093E(void)
{
    WakeWhenNear093E(cpu.x);
}

/*
 * @routine 093E:114 StepThenNext093E
 * @run 71:C914
 * WHAT IT DOES: Steps the list at $C92A; at its end keeps $20 in OBJ_WORK_J and goes on.
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StepThenNext093E(uint8_t slot)
{
    (void)slot;
    list_to_temp10_high_first(0xC92A);
    JSR(0xC91F, k_step_frames());
    if (NE) {
        return;
    }
    lda(0x20);
    WR8(OBJ->work_j[cpu.x], cpu.a);
    abi_nz(INC8(OBJ->state[cpu.x]));
}

void abi_StepThenNext093E(void)
{
    StepThenNext093E(cpu.x);
}

/*
 * @routine 093E:139 ChooseByDistance093E
 * @run 71:C939
 * WHAT IT DOES: Restarts the animation and substate. Far from the player, state 6; near, a random state from two tables.
 * HOW: DistanceXToPlayer, DistanceYToPlayer, NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ChooseByDistance093E(uint8_t slot)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->substate[slot], 0);
    JSR(0xC945, k_distance_x());
    cmp(cpu.a, 0x60);
    if (CC) {
        JSR(0xC94C, k_distance_y());
        cmp(cpu.a, 0x60);
        if (CC) {
            JSR(0xC959, k_random());
            and_a(0x0F);
            tay();
            lda(rd8((uint16_t)(0xC967 + cpu.y)));
            tay();
            lda(rd8((uint16_t)(0xC977 + cpu.y)));
            WR8(OBJ->state[cpu.x], cpu.a);
            return;
        }
    }
    lda(0x06);
    WR8(OBJ->state[cpu.x], cpu.a);
}

void abi_ChooseByDistance093E(void)
{
    ChooseByDistance093E(cpu.x);
}

/*
 * @routine 093E:179 PickWaitThenMove093E
 * @run 71:C979
 * @stop C994
 * WHAT IT DOES: Substate 0 takes a random wait and restarts the animation; substate 1 moves ($C998).
 * HOW: NextRandom and a table.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void PickWaitThenMove093E(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (NE) {
        run_routine(0xC998);
        return;
    }
    JSR(0xC981, k_random());
    slot = cpu.x;
    and_a(0x03);
    tay();
    lda(rd8((uint16_t)(0xC994 + cpu.y)));
    WR8(OBJ->work_k[slot], cpu.a);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_PickWaitThenMove093E(void)
{
    PickWaitThenMove093E(cpu.x);
}

/*
 * @routine 093E:198 StepWaiting093E
 * @run 71:C998
 * WHAT IT DOES: Steps the list at $C9CE, then counts ($C9A3).
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StepWaiting093E(uint8_t slot)
{
    (void)slot;
    list_to_temp10_high_first(0xC9CE);
    JSR(0xC9A3, k_step_frames());
    run_routine(0xC9A3);
}

void abi_StepWaiting093E(void)
{
    StepWaiting093E(cpu.x);
}

/*
 * @routine 093E:1A3 CountToTurn093E
 * @run 71:C9A3
 * WHAT IT DOES: Counts OBJ_WORK_K to state 3, else OBJ_WORK_J to state 7 with a new random count.
 * HOW: Two countdowns.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountToTurn093E(uint8_t slot)
{
    abi_nz(DEC8(OBJ->work_k[slot]));
    if (EQ) {
        lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
        return;
    }
    abi_nz(DEC8(OBJ->work_j[slot]));
    if (NE) {
        return;
    }
    JSR(0xC9B6, k_random());
    slot = cpu.x;
    and_a(0x03);
    tay();
    lda(rd8((uint16_t)(0xC9D5 + cpu.y)));
    WR8(OBJ->work_j[slot], cpu.a);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x07);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_CountToTurn093E(void)
{
    CountToTurn093E(cpu.x);
}

/*
 * @routine 093E:1D9 AimNearPlayer093E
 * @run 71:C9D9
 * @stop CA09
 * WHAT IT DOES: Substate 0 takes the angle to the player plus the row's offset, a wait, a turn of $0200 and counts the row on; substate 1 curves ($CA19).
 * HOW: AngleToPlayer and tables.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void AimNearPlayer093E(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (NE) {
        run_routine(0xCA19);
        return;
    }
    JSR(0xC9E1, k_angle_to_player());
    slot = cpu.x;
    WR8(OBJ->work_e[slot], cpu.a);
    lda(RD8(OBJ->work_h[slot]));
    and_a(0x07);
    tay();
    lda(RD8(OBJ->work_e[slot]));
    clc();
    adc_a(rd8((uint16_t)(0xCA09 + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
    lda(rd8((uint16_t)(0xCA11 + cpu.y)));
    WR8(OBJ->work_k[slot], cpu.a);
    lda(0x02);
    WR8(OBJ->work_f[slot], cpu.a);
    WR8(OBJ->work_g[slot], 0);
    abi_nz(INC8(OBJ->work_h[slot]));
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_AimNearPlayer093E(void)
{
    AimNearPlayer093E(cpu.x);
}

/*
 * @routine 093E:219 CurveSlowing093E
 * @run 71:CA19
 * WHAT IT DOES: Steps the list at $CA53 and moves at the angle, slowing the speed by 8; then counts ($C9A3).
 * HOW: SetVelocityFromAngle with the speed word.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CurveSlowing093E(uint8_t slot)
{
    list_to_temp10_high_first(0xCA53);
    JSR(0xCA24, k_step_frames());
    slot = cpu.x;
    lda(RD8(OBJ->work_g[slot]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_f[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    JSR(0xCA34, k_velocity_from_angle());
    JSR(0xCA37, k_move());
    slot = cpu.x;
    lda(RD8(OBJ->work_f[slot]));
    ora_a(RD8(OBJ->work_g[slot]));
    if (NE) {
        lda(RD8(OBJ->work_g[slot]));
        sec();
        sbc_a(0x08);
        WR8(OBJ->work_g[slot], cpu.a);
        lda(RD8(OBJ->work_f[slot]));
        sbc_a(0x00);
        WR8(OBJ->work_f[slot], cpu.a);
    }
    run_routine(0xC9A3);
}

void abi_CurveSlowing093E(void)
{
    CurveSlowing093E(cpu.x);
}

/*
 * @routine 093E:25A DriftToPlayer093E
 * @run 71:CA5A
 * @stop CAA7
 * WHAT IT DOES: Substate 0 stops, then drifts toward the player on each axis $60 or more away, with a wait of $20; substate 1 moves ($CAAB).
 * HOW: Speed tables by side.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DriftToPlayer093E(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (NE) {
        run_routine(0xCAAB);
        return;
    }
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    JSR(0xCA6E, k_distance_x());
    cmp(cpu.a, 0x60);
    if (CS) {
        JSR(0xCA75, k_side());
        and_a(0x01);
        asl_a();
        tay();
        lda(rd8((uint16_t)(0xCAA8 + cpu.y)));
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        lda(rd8((uint16_t)(0xCAA7 + cpu.y)));
        WR8(OBJ->vx_lo[cpu.x], cpu.a);
    }
    JSR(0xCA88, k_distance_y());
    cmp(cpu.a, 0x60);
    if (CS) {
        JSR(0xCA8F, k_side());
        and_a(0x02);
        tay();
        lda(rd8((uint16_t)(0xCAA8 + cpu.y)));
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        lda(rd8((uint16_t)(0xCAA7 + cpu.y)));
        WR8(OBJ->vy_lo[cpu.x], cpu.a);
    }
    lda(0x20);
    WR8(OBJ->work_k[cpu.x], cpu.a);
    abi_nz(INC8(OBJ->substate[cpu.x]));
}

void abi_DriftToPlayer093E(void)
{
    DriftToPlayer093E(cpu.x);
}

/*
 * @routine 093E:2AB DriftUntilWait093E
 * @run 71:CAAB
 * WHAT IT DOES: Steps the list at $CA53 and moves; the wait over, state 3.
 * HOW: A countdown.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DriftUntilWait093E(uint8_t slot)
{
    (void)slot;
    list_to_temp10_high_first(0xCA53);
    JSR(0xCAB6, k_step_frames());
    JSR(0xCAB9, k_move());
    abi_nz(DEC8(OBJ->work_k[cpu.x]));
    if (NE) {
        return;
    }
    lda(0x03);
    WR8(OBJ->state[cpu.x], cpu.a);
}

void abi_DriftUntilWait093E(void)
{
    DriftUntilWait093E(cpu.x);
}

/*
 * @routine 093E:2C4 DriftByHeight093E
 * @run 71:CAC4
 * WHAT IT DOES: Takes the Y speed for being above or below the player, a wait of $20 and the next substate.
 * HOW: SideOfPlayer bit 1.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DriftByHeight093E(uint8_t slot)
{
    (void)slot;
    JSR(0xCAC7, k_side());
    and_a(0x02);
    tay();
    lda(rd8((uint16_t)(0xCAA8 + cpu.y)));
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    lda(rd8((uint16_t)(0xCAA7 + cpu.y)));
    WR8(OBJ->vy_lo[cpu.x], cpu.a);
    lda(0x20);
    WR8(OBJ->work_k[cpu.x], cpu.a);
    abi_nz(INC8(OBJ->substate[cpu.x]));
}

void abi_DriftByHeight093E(void)
{
    DriftByHeight093E(cpu.x);
}

/*
 * @routine 093E:2DF BySubstate093ECADF
 * @run 71:CADF
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $CAF1.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate093ECADF(uint8_t slot)
{
    jump_by_substate(slot, 0xCAF1);
}

void abi_BySubstate093ECADF(void)
{
    BySubstate093ECADF(cpu.x);
}

/*
 * @routine 093E:305 ChooseAttack093E
 * @run 71:CB05
 * WHAT IT DOES: One time in four substate 7; else level with the player substate 1, below it substate 4.
 * HOW: NextRandom, DistanceYToPlayer, SideOfPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ChooseAttack093E(uint8_t slot)
{
    (void)slot;
    JSR(0xCB08, k_random());
    and_a(0x03);
    if (NE) {
        JSR(0xCB15, k_distance_y());
        cmp(cpu.a, 0x18);
        if (CC) {
            lda(0x01);
            WR8(OBJ->substate[cpu.x], cpu.a);
            return;
        }
        JSR(0xCB22, k_side());
        and_a(0x02);
        if (NE) {
            lda(0x04);
            WR8(OBJ->substate[cpu.x], cpu.a);
            return;
        }
    }
    lda(0x07);
    WR8(OBJ->substate[cpu.x], cpu.a);
}

void abi_ChooseAttack093E(void)
{
    ChooseAttack093E(cpu.x);
}

/*
 * @routine 093E:33C DashByFacing093E
 * @run 71:CB3C
 * WHAT IT DOES: The X speed for the facing and the next substate.
 * HOW: A table.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DashByFacing093E(uint8_t slot)
{
    lda(RD8(OBJ->facing_left[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xCB52 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xCB51 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_DashByFacing093E(void)
{
    DashByFacing093E(cpu.x);
}

/*
 * @routine 093E:355 DashThenAim093E
 * @run 71:CB55
 * WHAT IT DOES: Moves while the list at $CB8C steps; at its end aims at the player at speed $0300, faces them and goes on with a wait of $38.
 * HOW: AngleToPlayer and SetVelocityFromAngle.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DashThenAim093E(uint8_t slot)
{
    (void)slot;
    JSR(0xCB58, k_move_x());
    list_to_temp10_high_first(0xCB8C);
    JSR(0xCB63, k_step_frames());
    if (NE) {
        return;
    }
    JSR(0xCB68, k_angle_to_player());
    pha();
    lda(0x03);
    WR8(ZP->temp01, cpu.a);
    lda(0x00);
    WR8(ZP->temp00, cpu.a);
    pla();
    JSR(0xCB75, k_velocity_from_angle());
    JSR(0xCB78, k_side());
    and_a(0x01);
    WR8(OBJ->facing_left[cpu.x], cpu.a);
    WR8(OBJ->anim_step[cpu.x], 0);
    WR8(OBJ->anim_timer[cpu.x], 0);
    lda(0x38);
    WR8(OBJ->work_k[cpu.x], cpu.a);
    abi_nz(INC8(OBJ->substate[cpu.x]));
}

void abi_DashThenAim093E(void)
{
    DashThenAim093E(cpu.x);
}

/*
 * @routine 093E:3A5 FlyThenState4_093E
 * @run 71:CBA5
 * WHAT IT DOES: Steps the list at $CBC7 and moves; the wait over, state 4.
 * HOW: A countdown.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlyThenState4_093E(uint8_t slot)
{
    (void)slot;
    list_to_temp10_high_first(0xCBC7);
    JSR(0xCBB0, k_step_frames());
    JSR(0xCBB3, k_move());
    abi_nz(DEC8(OBJ->work_k[cpu.x]));
    if (NE) {
        return;
    }
    WR8(OBJ->anim_step[cpu.x], 0);
    WR8(OBJ->anim_timer[cpu.x], 0);
    lda(0x04);
    WR8(OBJ->state[cpu.x], cpu.a);
    WR8(OBJ->substate[cpu.x], 0);
}

void abi_FlyThenState4_093E(void)
{
    FlyThenState4_093E(cpu.x);
}

/*
 * @routine 093E:3D6 AimBesidePlayer093E
 * @run 71:CBD6
 * WHAT IT DOES: Aims at a point beside the player at speed $0140, with a wait of $30.
 * HOW: AngleToPlayerOffset.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void AimBesidePlayer093E(uint8_t slot)
{
    lda(RD8(OBJ->facing_left[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xCC02 + cpu.y)));
    WR8(ZP->temp08, cpu.a);
    lda(rd8((uint16_t)(0xCC03 + cpu.y)));
    WR8(ZP->temp09, cpu.a);
    WR8(ZP->temp0a, 0);
    WR8(ZP->temp0b, 0);
    JSR(0xCBEC, k_angle_to_player_offset());
    pha();
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    lda(0x40);
    WR8(ZP->temp00, cpu.a);
    pla();
    JSR(0xCBF9, k_velocity_from_angle());
    lda(0x30);
    WR8(OBJ->work_k[cpu.x], cpu.a);
    abi_nz(INC8(OBJ->substate[cpu.x]));
}

void abi_AimBesidePlayer093E(void)
{
    AimBesidePlayer093E(cpu.x);
}

/*
 * @routine 093E:406 FlyAtPoint093E
 * @run 71:CC06
 * @stop CC3B
 * WHAT IT DOES: Steps the list at $CC3D, moving and facing the player. The wait over, state 3 ($CC6B); below the player at $20-$2F down, next substate.
 * HOW: DistanceYToPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlyAtPoint093E(uint8_t slot)
{
    (void)slot;
    list_to_temp10_high_first(0xCC3D);
    JSR(0xCC11, k_step_frames());
    JSR(0xCC14, k_move());
    JSR(0xCC17, k_side());
    and_a(0x01);
    WR8(OBJ->facing_left[cpu.x], cpu.a);
    abi_nz(DEC8(OBJ->work_k[cpu.x]));
    if (EQ) {
        run_routine(0xCC6B);
        return;
    }
    JSR(0xCC24, k_side());
    and_a(0x02);
    if (EQ) {
        return;
    }
    JSR(0xCC2B, k_distance_y());
    and_a(0xF0);
    cmp(cpu.a, 0x20);
    if (NE) {
        return;
    }
    WR8(OBJ->anim_step[cpu.x], 0);
    WR8(OBJ->anim_timer[cpu.x], 0);
    abi_nz(INC8(OBJ->substate[cpu.x]));
}

void abi_FlyAtPoint093E(void)
{
    FlyAtPoint093E(cpu.x);
}

/*
 * @routine 093E:444 ThrowAtFrame6C_093E
 * @run 71:CC44
 * WHAT IT DOES: Steps the list at $CC7A; at its end, or with no free slot, state 3 ($CC6B). At frame $6C starts a kind $2F object.
 * HOW: FindFreeSlotFrom50.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ThrowAtFrame6C_093E(uint8_t slot)
{
    (void)slot;
    list_to_temp10_high_first(0xCC7A);
    JSR(0xCC4F, k_step_frames());
    if (EQ) {
        run_routine(0xCC6B);
        return;
    }
    if (CC) {
        return;
    }
    lda(RD8(OBJ->frame[cpu.x]));
    cmp(cpu.a, 0x6C);
    if (NE) {
        return;
    }
    JSR(0xCC5D, k_free_from50());
    if (CC) {
        run_routine(0xCC6B);
        return;
    }
    lda(0x2F);
    JSR(0xCC64, k_start_at_place());
    lda(RD8(OBJ->facing_left[cpu.x]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
}

void abi_ThrowAtFrame6C_093E(void)
{
    ThrowAtFrame6C_093E(cpu.x);
}

/*
 * @routine 093E:46B RestartState3_093E
 * @run 71:CC6B
 * WHAT IT DOES: State 3 from its start.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void RestartState3_093E(uint8_t slot)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_RestartState3_093E(void)
{
    RestartState3_093E(cpu.x);
}

/*
 * @routine 093E:499 AimAtPoint50_093E
 * @run 71:CC99
 * WHAT IT DOES: Aims at the point the facing picks (Y $50) at speed $0140 and goes on.
 * HOW: AngleToPoint.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void AimAtPoint50_093E(uint8_t slot)
{
    ldy(RD8(OBJ->facing_left[slot]));
    lda(rd8((uint16_t)(0xCCBD + cpu.y)));
    WR8(ZP->temp08, cpu.a);
    WR8(ZP->temp09, 0);
    lda(0x50);
    WR8(ZP->temp0a, cpu.a);
    WR8(ZP->temp0b, 0);
    JSR(0xCCAC, k_angle_to_point());
    pha();
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    lda(0x40);
    WR8(ZP->temp00, cpu.a);
    pla();
    JSR(0xCCB9, k_velocity_from_angle());
    abi_nz(INC8(OBJ->substate[cpu.x]));
}

void abi_AimAtPoint50_093E(void)
{
    AimAtPoint50_093E(cpu.x);
}

/*
 * @routine 093E:4BF FlyUntilEdge093E
 * @run 71:CCBF
 * WHAT IT DOES: Moves and steps the list at $CCE4; at its end or within $10 of a screen side, next substate.
 * HOW: Compares of X low.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlyUntilEdge093E(uint8_t slot)
{
    (void)slot;
    JSR(0xCCC2, k_move());
    list_to_temp10(0xCCE4);
    JSR(0xCCCD, k_step_frames());
    if (NE) {
        lda(RD8(OBJ->x_lo[cpu.x]));
        cmp(cpu.a, 0xF0);
        if (CC) {
            cmp(cpu.a, 0x10);
            if (CS) {
                return;
            }
        }
    }
    WR8(OBJ->anim_step[cpu.x], 0);
    WR8(OBJ->anim_timer[cpu.x], 0);
    abi_nz(INC8(OBJ->substate[cpu.x]));
}

void abi_FlyUntilEdge093E(void)
{
    FlyUntilEdge093E(cpu.x);
}

/*
 * @routine 093E:4FD SpitAtFrame6C_093E
 * @run 71:CCFD
 * WHAT IT DOES: Steps the list at $CD25; at frame $6C starts the six flames ($CD44), at its end state 3.
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SpitAtFrame6C_093E(uint8_t slot)
{
    (void)slot;
    list_to_temp10_high_first(0xCD25);
    JSR(0xCD08, k_step_frames());
    if (EQ) {
        WR8(OBJ->anim_step[cpu.x], 0);
        WR8(OBJ->anim_timer[cpu.x], 0);
        lda(0x03);
        WR8(OBJ->state[cpu.x], cpu.a);
        WR8(OBJ->substate[cpu.x], 0);
        return;
    }
    if (CC) {
        return;
    }
    lda(RD8(OBJ->frame[cpu.x]));
    cmp(cpu.a, 0x6C);
    if (NE) {
        return;
    }
    run_routine(0xCD44);
}

void abi_SpitAtFrame6C_093E(void)
{
    SpitAtFrame6C_093E(cpu.x);
}

/*
 * @routine 093E:544 StartSevenFlames093E
 * @run 71:CD44
 * WHAT IT DOES: Sound $25 and up to seven kind $36 flames in free slots, facing this way and numbered.
 * HOW: FindFreeSlotFrom30 and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StartSevenFlames093E(uint8_t slot)
{
    (void)slot;
    lda(0x25);
    JSR(0xCD49, k_sound());
    lda(0x07);
    WR8(ZP->temp00, cpu.a);
    WR8(ZP->temp01, 0);
    do {
        JSR(0xCD52, k_free_from30());
        if (CC) {
            return;
        }
        lda(0x36);
        JSR(0xCD59, k_start_at_place());
        lda(RD8(OBJ->facing_left[cpu.x]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        lda(RD8(ZP->temp01));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        abi_nz(INC8(ZP->temp01));
        abi_nz(DEC8(ZP->temp00));
    } while (NE);
}

void abi_StartSevenFlames093E(void)
{
    StartSevenFlames093E(cpu.x);
}

/*
 * @routine 093E:56B BySubstate093ECD6B
 * @run 71:CD6B
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $CD7D.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate093ECD6B(uint8_t slot)
{
    jump_by_substate(slot, 0xCD7D);
}

void abi_BySubstate093ECD6B(void)
{
    BySubstate093ECD6B(cpu.x);
}

/*
 * @routine 093E:583 LungeFromHolder093E
 * @run 71:CD83
 * WHAT IT DOES: Aims from the holder's hit box (OBJ_WORK_L) at speed $0400, a random half-angle off, with a wait of $14.
 * HOW: AngleToPoint and SetVelocityFromAngle.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LungeFromHolder093E(uint8_t slot)
{
    ldy(RD8(OBJ->work_l[slot]));
    lda(RD8(OBJ->hitbox_dx[cpu.y]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->facing_left[cpu.y]));
    if (NE) {
        lda(RD8(ZP->temp00));
        eor_a(0xFF);
        inc_a();
        WR8(ZP->temp00, cpu.a);
    }
    lda(RD8(OBJ->x_lo[cpu.y]));
    clc();
    adc_a(RD8(ZP->temp00));
    WR8(ZP->temp08, cpu.a);
    lda(RD8(OBJ->x_hi[cpu.y]));
    adc_a(0x00);
    WR8(ZP->temp09, cpu.a);
    lda(RD8(OBJ->y_hi[cpu.y]));
    WR8(ZP->temp0b, cpu.a);
    lda(RD8(OBJ->y_lo[cpu.y]));
    WR8(ZP->temp0a, cpu.a);
    JSR(0xCDB3, k_angle_to_point());
    WR8(ZP->temp02, cpu.a);
    lda(0x04);
    WR8(ZP->temp01, cpu.a);
    lda(0x00);
    WR8(ZP->temp00, cpu.a);
    JSR(0xCDC0, k_random());
    and_a(0x01);
    tay();
    lda(RD8(ZP->temp02));
    clc();
    adc_a(rd8((uint16_t)(0xCDD5 + cpu.y)));
    JSR(0xCDCC, k_velocity_from_angle());
    lda(0x14);
    WR8(OBJ->work_k[cpu.x], cpu.a);
    abi_nz(INC8(OBJ->substate[cpu.x]));
}

void abi_LungeFromHolder093E(void)
{
    LungeFromHolder093E(cpu.x);
}

/*
 * @routine 093E:5D7 LungeAway093E
 * @run 71:CDD7
 * WHAT IT DOES: Steps the list at $CDF3 and moves; the wait over, reloads $14 and goes on.
 * HOW: A countdown.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LungeAway093E(uint8_t slot)
{
    (void)slot;
    list_to_temp10_high_first(0xCDF3);
    JSR(0xCDE2, k_step_frames());
    JSR(0xCDE5, k_move());
    abi_nz(DEC8(OBJ->work_k[cpu.x]));
    if (NE) {
        return;
    }
    lda(0x14);
    WR8(OBJ->work_k[cpu.x], cpu.a);
    abi_nz(INC8(OBJ->substate[cpu.x]));
}

void abi_LungeAway093E(void)
{
    LungeAway093E(cpu.x);
}

/*
 * @routine 093E:602 CountThenState3_093E
 * @run 71:CE02
 * WHAT IT DOES: The wait over, state 3 from its start.
 * HOW: A countdown.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountThenState3_093E(uint8_t slot)
{
    abi_nz(DEC8(OBJ->work_k[slot]));
    if (NE) {
        return;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_CountThenState3_093E(void)
{
    CountThenState3_093E(cpu.x);
}

/*
 * @routine 093E:613 DieWithSparks093E
 * @run 71:CE13
 * WHAT IT DOES: Substate 0: frame $43 and a count of $40. Then falls, cleared at the count's end, sparking at eight counts.
 * HOW: SparkOfRow093E.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DieWithSparks093E(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (EQ) {
        lda(0x43);
        WR8(OBJ->frame[slot], cpu.a);
        lda(0x40);
        WR8(OBJ->work_i[slot], cpu.a);
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    JSR(0xCE29, k_gravity_unlimited());
    JSR(0xCE2C, k_move_y());
    abi_nz(DEC8(OBJ->work_i[cpu.x]));
    if (EQ) {
        k_clear();
        return;
    }
    cpu.y = 0;
    do {
        lda(RD8(OBJ->work_i[cpu.x]));
        cmp(cpu.a, rd8((uint16_t)(0xCE68 + cpu.y)));
        if (EQ) {
            run_routine(0xCE43);
            return;
        }
        iny();
        cmp(cpu.y, 0x08);
    } while (CC);
}

void abi_DieWithSparks093E(void)
{
    DieWithSparks093E(cpu.x);
}

/*
 * @routine 093E:690 FlameBurst093E
 * @run 71:CE90
 * WHAT IT DOES: State 0 loads it with sound $26; later steps the list at $CED6 and takes the hit box the frame names, cleared at the end.
 * HOW: A box table by frame from $70.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlameBurst093E(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xCE98, k_load_record());
        lda(0x26);
        JSR(0xCE9D, k_sound());
        return;
    }
    list_to_temp10(0xCED6);
    JSR(0xCEA9, k_step_frames());
    if (EQ) {
        k_clear();
        return;
    }
    slot = cpu.x;
    lda(RD8(OBJ->frame[slot]));
    sec();
    sbc_a(0x70);
    cmp(cpu.a, 0x05);
    if (CS) {
        lda(0x05);
    }
    asl_a();
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xCF81 + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(rd8((uint16_t)(0xCF82 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    lda(rd8((uint16_t)(0xCF83 + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(rd8((uint16_t)(0xCF84 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_FlameBurst093E(void)
{
    FlameBurst093E(cpu.x);
}

/*
 * @routine 093E:6ED LaunchFlame093E
 * @run 71:CEED
 * @stop CF1D
 * WHAT IT DOES: State 0: loads it, steps once at -$18 upward with the facing's X, then takes an angle from the facing and its number at $0140; later flies ($CF28).
 * HOW: SetVelocityFromAngle.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LaunchFlame093E(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (NE) {
        run_routine(0xCF28);
        return;
    }
    JSR(0xCEF5, k_load_record());
    slot = cpu.x;
    ldy(RD8(OBJ->facing_left[slot]));
    lda(rd8((uint16_t)(0xCF1D + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0xE8);
    WR8(OBJ->vy_hi[slot], cpu.a);
    JSR(0xCF06, k_move());
    lda(rd8((uint16_t)(0xCF1F + cpu.y)));
    ldy(RD8(OBJ->work_d[cpu.x]));
    clc();
    adc_a(rd8((uint16_t)(0xCF21 + cpu.y)));
    pha();
    lda(0x40);
    WR8(ZP->temp00, cpu.a);
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    pla();
    k_velocity_from_angle();
}

void abi_LaunchFlame093E(void)
{
    LaunchFlame093E(cpu.x);
}

/*
 * @routine 093E:728 FlyFlame093E
 * @run 71:CF28
 * WHAT IT DOES: Steps the list at $CF41; on the first screen moves, else cleared.
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlyFlame093E(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xCF41);
    JSR(0xCF33, k_step_frames());
    lda(RD8(OBJ->x_hi[cpu.x]));
    ora_a(RD8(OBJ->y_hi[cpu.x]));
    if (NE) {
        k_clear();
        return;
    }
    k_move();
}

void abi_FlyFlame093E(void)
{
    FlyFlame093E(cpu.x);
}

/*
 * @routine 093E:799 HeadChecksThenBody093E
 * @run 71:CF99
 * WHAT IT DOES: Counts, faces the player in some states, checks for a hit, sets the hit box, goes to the state's handler and drives the body ($D032).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void HeadChecksThenBody093E(uint8_t slot)
{
    abi_nz(INC8(OBJ->work_i[slot]));
    call_routine(0xCF9F, 0xCFC9);
    call_routine(0xCFA2, 0xCFEA);
    call_routine(0xCFA5, 0xD007);
    call_routine(0xCFA8, 0xCFAB);
    run_routine(0xD032);
}

void abi_HeadChecksThenBody093E(void)
{
    HeadChecksThenBody093E(cpu.x);
}

/*
 * @routine 093E:7AB ByState093ECFAB
 * @run 71:CFAB
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $CFBD.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState093ECFAB(uint8_t slot)
{
    jump_by_state(slot, 0xCFBD);
}

void abi_ByState093ECFAB(void)
{
    ByState093ECFAB(cpu.x);
}

/*
 * @routine 093E:7C9 FacePlayerInSomeStates093E
 * @run 71:CFC9
 * WHAT IT DOES: Faces the player in the states the table at $CFDA marks.
 * HOW: SideOfPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FacePlayerInSomeStates093E(uint8_t slot)
{
    ldy(RD8(OBJ->state[slot]));
    lda(rd8((uint16_t)(0xCFDA + cpu.y)));
    if (EQ) {
        return;
    }
    JSR(0xCFD4, k_side());
    and_a(0x01);
    WR8(OBJ->facing_left[cpu.x], cpu.a);
}

void abi_FacePlayerInSomeStates093E(void)
{
    FacePlayerInSomeStates093E(cpu.x);
}

/*
 * @routine 093E:7EA State5WhenHit093E
 * @run 71:CFEA
 * WHAT IT DOES: Under state 5, a hit goes to state 5 from its start.
 * HOW: Two tests.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void State5WhenHit093E(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x05);
    if (CS) {
        return;
    }
    lda(RD8(OBJ->hit_status[slot]));
    and_a(0x80);
    if (EQ) {
        return;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_State5WhenHit093E(void)
{
    State5WhenHit093E(cpu.x);
}

/*
 * @routine 093F:007 SetHitBoxByFrame093F
 * @run 71:D007
 * WHAT IT DOES: The hit box the frame names (up to $23), its half width widened by 8.
 * HOW: A four-byte table.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SetHitBoxByFrame093F(uint8_t slot)
{
    lda(RD8(OBJ->frame[slot]));
    sec();
    sbc_a(0x01);
    cmp(cpu.a, 0x22);
    if (CS) {
        lda(0x22);
    }
    asl_a();
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xD605 + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(rd8((uint16_t)(0xD606 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    lda(rd8((uint16_t)(0xD607 + cpu.y)));
    clc();
    adc_a(0x08);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(rd8((uint16_t)(0xD608 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_SetHitBoxByFrame093F(void)
{
    SetHitBoxByFrame093F(cpu.x);
}

/*
 * @routine 093F:032 DriveBodyBelow093F
 * @run 71:D032
 * WHAT IT DOES: With health under $17, once: puts the slot below in its next state and clears its pace bits. Else the slot below takes this place, facing and attribute, thrown at the frame's speed turned by facing, and moves one step.
 * HOW: CopyObjectPlace and ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DriveBodyBelow093F(uint8_t slot)
{
    lda(RD8(OBJ->work_j[slot]));
    if (NE) {
        return;
    }
    lda(RD8(OBJ->health[slot]));
    cmp(cpu.a, 0x17);
    if (CC) {
        unsigned below = slot - 1u;

        abi_nz(INC8(OBJ->state[below]));
        abi_nz(INC8(OBJ->work_j[slot]));
        lda(RD8(OBJ->pace[slot]));
        and_a(0xFC);
        WR8(OBJ->pace[slot], cpu.a);
        return;
    }
    txa();
    tay();
    dey();
    JSR(0xD053, k_copy_place());
    lda(RD8(OBJ->facing_left[cpu.x]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    lda(RD8(OBJ->sprite_attr[cpu.x]));
    WR8(OBJ->sprite_attr[cpu.y], cpu.a);
    phx();
    lda(RD8(OBJ->frame[cpu.x]));
    sec();
    sbc_a(0x01);
    asl_a();
    tax();
    lda(rd8((uint16_t)(0xD56F + cpu.x)));
    WR8(OBJ->vx_hi[cpu.y], cpu.a);
    lda(rd8((uint16_t)(0xD570 + cpu.x)));
    WR8(OBJ->vy_hi[cpu.y], cpu.a);
    plx();
    sxy();
    lda(RD8(OBJ->facing_left[cpu.x]));
    if (NE) {
        JSR(0xD07E, k_negate_x());
    }
    JSR(0xD081, k_move());
    sxy();
}

void abi_DriveBodyBelow093F(void)
{
    DriveBodyBelow093F(cpu.x);
}
