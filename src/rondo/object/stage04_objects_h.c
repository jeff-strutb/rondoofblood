/*
 * Stage 04's objects, runs D029-D9DC.
 */
#include "stage_obj_ops.h"
#include "stage04_objects_h.h"
#include "calls_b.h"

/*
 * @routine 07DF:029 BatSwoop07DF
 * @run 71:D029
 * WHAT IT DOES: Steps the list at $D08F, bobbing on the cosine and flying. Within $60 of the player, one time in 128, takes a
 * random dive speed (turned by facing) and goes on.
 * HOW: ScaleByCosine, NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BatSwoop07DF(uint8_t slot)
{
    list_to_temp10_high_first(0xD08F);
    JSR(0xD034, k_step_frames());
    lda(0x40);
    WR8(ZP->temp00, cpu.a);
    lda(0x00);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_d[cpu.x]));
    JSR(0xD042, k_cosine());
    lda(RD8(ZP->plane_rows_left));
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    lda(RD8(ZP->sprite_flip_mask));
    WR8(OBJ->vy_lo[cpu.x], cpu.a);
    JSR(0xD04F, k_move_y());
    abi_nz(INC8(OBJ->work_d[cpu.x]));
    JSR(0xD055, k_move_x());
    JSR(0xD058, k_distance_x());
    cmp(cpu.a, 0x60);
    if (CS) {
        return;
    }
    JSR(0xD05F, k_random());
    cmp(cpu.a, 0x02);
    if (CS) {
        return;
    }
    JSR(0xD066, k_random());
    and_a(0x07);
    asl_a();
    asl_a();
    tay();
    slot = cpu.x;
    lda(rd8((uint16_t)(0xD098 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xD099 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xD09A + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xD09B + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(RD8(OBJ->facing_left[slot]));
    if (NE) {
        JSR(0xD08B, k_negate_x());
    }
    abi_nz(INC8(OBJ->state[cpu.x]));
}

void abi_BatSwoop07DF(void)
{
    BatSwoop07DF(cpu.x);
}

/*
 * @routine 07DF:0B8 FallBurstAtLine07DF
 * @run 71:D0B8
 * WHAT IT DOES: Steps the list at $D08F and falls, bursting debris at Y $D0-$DF.
 * HOW: DebrisWithSound07DE, ApplyGravityUnlimited.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FallBurstAtLine07DF(uint8_t slot)
{
    (void)slot;
    list_to_temp10_high_first(0xD08F);
    JSR(0xD0C3, k_step_frames());
    lda(RD8(OBJ->y_lo[cpu.x]));
    and_a(0xF0);
    cmp(cpu.a, 0xD0);
    if (EQ) {
        call_routine(0xD0CF, 0xCCE7);
    }
    JSR(0xD0D2, k_gravity_unlimited());
    k_move();
}

void abi_FallBurstAtLine07DF(void)
{
    FallBurstAtLine07DF(cpu.x);
}

/*
 * @routine 07DF:2E4 CandleBox07DF
 * @run 71:D2E4
 * WHAT IT DOES: State 0 loads the record as frame 1; later steps the list at $D344 and checks for breaking ($D2FD).
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CandleBox07DF(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xD2EC, k_load_record());
        lda(0x01);
        WR8(OBJ->frame[cpu.x], cpu.a);
        return;
    }
    list_to_temp10_high_first(0xD344);
    JSR(0xD2FD, k_step_frames());
    run_routine(0xD2FD);
}

void abi_CandleBox07DF(void)
{
    CandleBox07DF(cpu.x);
}

/*
 * @routine 07DF:2FD BreakWhenHitOrPassed07DF
 * @run 71:D2FD
 * WHAT IT DOES: Passed by the object in slot $4D within $50, or five hits taken: records the event, becomes the kind
 * OBJ_START_ARG holds, and starts in a free slot from $50 a kind $9C rising (kind $38) or a kind $9B.
 * HOW: RightOfObject4DWithin, FindFreeSlotFrom50.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BreakWhenHitOrPassed07DF(uint8_t slot)
{
    lda(0x50);
    call_routine(0xD302, 0xD349);
    slot = cpu.x;
    if (CC) {
        lda(RD8(OBJ->hit_taken[slot]));
        cmp(cpu.a, 0x05);
        if (CC) {
            return;
        }
    }
    JSR(0xD30E, k_record_event());
    slot = cpu.x;
    lda(RD8(OBJ->kind[slot]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    WR8(OBJ->kind[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->invuln_timer[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
    JSR(0xD324, k_free_from50());
    if (CC) {
        return;
    }
    lda(RD8(ZP->temp00));
    cmp(cpu.a, 0x38);
    if (EQ) {
        run_routine(0xD331);
        return;
    }
    lda(0x9B);
    k_start_at_place();
}

void abi_BreakWhenHitOrPassed07DF(void)
{
    BreakWhenHitOrPassed07DF(cpu.x);
}

/*
 * @routine 07DF:36D StandWithBox07DF
 * @run 71:D36D
 * WHAT IT DOES: State 0 loads the record as sprite set 6, half height $14; later steps the list at $D38E and checks for
 * breaking ($D2FD).
 * HOW: StepObjectFrames, then into BreakWhenHitOrPassed07DF.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StandWithBox07DF(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xD375, k_load_record());
        lda(0x06);
        WR8(OBJ->sprite_set[cpu.x], cpu.a);
        lda(0x14);
        WR8(OBJ->hitbox_half_h[cpu.x], cpu.a);
        return;
    }
    list_to_temp10_high_first(0xD38E);
    JSR(0xD38B, k_step_frames());
    run_routine(0xD2FD);
}

void abi_StandWithBox07DF(void)
{
    StandWithBox07DF(cpu.x);
}

/*
 * @routine 07DF:402 ByState07DFD402
 * @run 71:D402
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $D414.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState07DFD402(uint8_t slot)
{
    jump_by_state(slot, 0xD414);
}

void abi_ByState07DFD402(void)
{
    ByState07DFD402(cpu.x);
}

/*
 * @routine 07DF:41A LoadBreakable07DF
 * @run 71:D41A
 * WHAT IT DOES: Loads the record as frame 5, the hit box and wait the argument picks.
 * HOW: Tables by argument.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LoadBreakable07DF(uint8_t slot)
{
    JSR(0xD41D, k_load_record());
    slot = cpu.x;
    lda(0x05);
    WR8(OBJ->frame[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xD3F2 + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(rd8((uint16_t)(0xD3F3 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
    lda(rd8((uint16_t)(0xD3F6 + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(rd8((uint16_t)(0xD3F7 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    ldy(RD8(OBJ->start_arg[slot]));
    lda(rd8((uint16_t)(0xD3EE + cpu.y)));
    WR8(OBJ->work_d[slot], cpu.a);
    WR8(OBJ->work_e[slot], cpu.a);
}

void abi_LoadBreakable07DF(void)
{
    LoadBreakable07DF(cpu.x);
}

/*
 * @routine 07DF:44C CountWhileHit07DF
 * @run 71:D44C
 * WHAT IT DOES: While hit ($D4E1), counts OBJ_PIECE_PLACE down and at zero goes on; otherwise reloads it.
 * HOW: TakeHitByArgument07DF.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountWhileHit07DF(uint8_t slot)
{
    call_routine(0xD44F, 0xD4E1);
    slot = cpu.x;
    if (CS) {
        lda(RD8(OBJ->work_e[slot]));
        if (NE) {
            abi_nz(DEC8(OBJ->work_e[slot]));
            return;
        }
        abi_nz(INC8(OBJ->state[slot]));
        return;
    }
    lda(RD8(OBJ->work_d[slot]));
    WR8(OBJ->work_e[slot], cpu.a);
}

void abi_CountWhileHit07DF(void)
{
    CountWhileHit07DF(cpu.x);
}

/*
 * @routine 07DF:465 ByArgument07DFD465
 * @run 71:D465
 * WHAT IT DOES: Goes to the handler for OBJ_START_ARG.
 * HOW: A table of handlers at $D3FA.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByArgument07DFD465(uint8_t slot)
{
    lda(RD8(OBJ->start_arg[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xD3FA + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    lda(rd8((uint16_t)(0xD3FB + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ByArgument07DFD465(void)
{
    ByArgument07DFD465(cpu.x);
}

/*
 * @routine 07DF:477 BySubstate07DFD477
 * @run 71:D477
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $D489.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate07DFD477(uint8_t slot)
{
    jump_by_substate(slot, 0xD489);
}

void abi_BySubstate07DFD477(void)
{
    BySubstate07DFD477(cpu.x);
}

/*
 * @routine 07DF:48D StartBreakingByArgument07DF
 * @run 71:D48D
 * WHAT IT DOES: The frame and wait the argument picks, sound $3A, next substate.
 * HOW: Tables.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StartBreakingByArgument07DF(uint8_t slot)
{
    ldy(RD8(OBJ->start_arg[slot]));
    lda(rd8((uint16_t)(0xD3EA + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
    lda(rd8((uint16_t)(0xD3EC + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
    lda(0x3A);
    JSR(0xD4A1, k_sound());
    abi_nz(INC8(OBJ->substate[cpu.x]));
}

void abi_StartBreakingByArgument07DF(void)
{
    StartBreakingByArgument07DF(cpu.x);
}

/*
 * @routine 07DF:4A5 RiseThenBecomeByArgument07DF
 * @run 71:D4A5
 * WHAT IT DOES: Kind $88 results step the list at $D524. Rises a dot a frame while the wait counts, then records the event and
 * becomes the kind the argument picks at rest.
 * HOW: A table of kinds.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void RiseThenBecomeByArgument07DF(uint8_t slot)
{
    ldy(RD8(OBJ->start_arg[slot]));
    lda(rd8((uint16_t)(0xD3F0 + cpu.y)));
    cmp(cpu.a, 0x88);
    if (EQ) {
        list_to_temp10_high_first(0xD524);
        JSR(0xD4BA, k_step_frames());
        slot = cpu.x;
    }
    abi_nz(DEC8(OBJ->y_lo[slot]));
    abi_nz(DEC8(OBJ->work_e[slot]));
    if (NE) {
        return;
    }
    JSR(0xD4C5, k_record_event());
    slot = cpu.x;
    ldy(RD8(OBJ->start_arg[slot]));
    lda(rd8((uint16_t)(0xD3F0 + cpu.y)));
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
    WR8(OBJ->substate[slot], 0);
    WR8(OBJ->hitbox_dx[slot], 0);
    WR8(OBJ->hitbox_dy[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
}

void abi_RiseThenBecomeByArgument07DF(void)
{
    RiseThenBecomeByArgument07DF(cpu.x);
}

/*
 * @routine 07DF:4E1 TakeHitByArgument07DF
 * @run 71:D4E1
 * WHAT IT DOES: Clears the invulnerability; a hit type of 1-4 goes to the handler for OBJ_START_ARG, else carry clear.
 * HOW: A table of handlers at $D3FE.
 * WAITS: no.
 * Entry: X object. Exit: C as the handler or clear.
 */
void TakeHitByArgument07DF(uint8_t slot)
{
    WR8(OBJ->invuln_timer[slot], 0);
    lda(RD8(OBJ->hit_taken[slot]));
    if (NE) {
        cmp(cpu.a, 0x05);
        if (CC) {
            lda(RD8(OBJ->start_arg[slot]));
            asl_a();
            tay();
            lda(rd8((uint16_t)(0xD3FE + cpu.y)));
            WR8(ZP->temp00, cpu.a);
            lda(rd8((uint16_t)(0xD3FF + cpu.y)));
            WR8(ZP->temp01, cpu.a);
            run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
            return;
        }
    }
    clc();
}

void abi_TakeHitByArgument07DF(void)
{
    TakeHitByArgument07DF(cpu.x);
}

/*
 * @routine 07DF:501 PlayerInState2_07DF
 * @run 71:D501
 * WHAT IT DOES: Carry set when the player is in state 2.
 * HOW: A compare of slot 0's state.
 * WAITS: no.
 * Entry: none. Exit: C the answer, A the state.
 */
void PlayerInState2_07DF(uint8_t slot)
{
    (void)slot;
    lda(RD8(OBJ->state[0]));
    cmp(cpu.a, 0x02);
    if (EQ) {
        sec();
        return;
    }
    clc();
}

void abi_PlayerInState2_07DF(void)
{
    PlayerInState2_07DF(cpu.x);
}

/*
 * @routine 07DF:52D ByState07DFD52D
 * @run 71:D52D
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $D53F.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState07DFD52D(uint8_t slot)
{
    jump_by_state(slot, 0xD53F);
}

void abi_ByState07DFD52D(void)
{
    ByState07DFD52D(cpu.x);
}

/*
 * @routine 07DF:549 LeaveWhenClearedAtEnd07DF
 * @run 71:D549
 * WHAT IT DOES: With the door ready, the camera past $06B0 and no object of kind $25 in slots $10-$57: sub-step 3, attack flags
 * 2, and on.
 * HOW: A walk of the kinds.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LeaveWhenClearedAtEnd07DF(uint8_t slot)
{
    lda(RD8(PVARS->door_ready));
    if (EQ) {
        return;
    }
    lda(RD8(SCROLL->x_hi[0]));
    cmp(cpu.a, 0x06);
    if (CC) {
        return;
    }
    if (EQ) {
        lda(RD8(SCROLL->x_lo[0]));
        cmp(cpu.a, 0xB0);
        if (CC) {
            return;
        }
    }
    ldy(0x10);
    do {
        lda(RD8(OBJ->kind[cpu.y]));
        cmp(cpu.a, 0x25);
        if (EQ) {
            return;
        }
        iny();
        cmp(cpu.y, 0x58);
    } while (CC);
    lda(0x03);
    WR8(ZP->sub_step, cpu.a);
    lda(0x02);
    wr8(HitAttackFlags, cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_LeaveWhenClearedAtEnd07DF(void)
{
    LeaveWhenClearedAtEnd07DF(cpu.x);
}

/*
 * @routine 07E0:065 FallingSpikeTrap07E0
 * @run 71:D865
 * WHAT IT DOES: State 0 loads the record. State 1 waits for the camera, then counts $20-$3F; state 2 counts and starts script
 * $82 in the slot OBJ_START_ARG names with a count of $10; state 3 counts back to state 1 with sound $4F.
 * HOW: Branches by state.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FallingSpikeTrap07E0(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        k_load_record();
        return;
    }
    dec_a();
    if (EQ) {
        lda(RD8(OBJ->x_lo[slot]));
        if (NE) {
            return;
        }
        lda(RD8(ZP->wave_counter));
        and_a(0x1F);
        clc();
        adc_a(0x20);
        WR8(OBJ->work_d[slot], cpu.a);
        abi_nz(INC8(OBJ->state[slot]));
        return;
    }
    dec_a();
    if (EQ) {
        abi_nz(DEC8(OBJ->work_d[slot]));
        if (NE) {
            return;
        }
        phx();
        lda(RD8(OBJ->start_arg[slot]));
        tax();
        lda(0x82);
        JSR(0xD895, k_script1());
        plx();
        lda(0x10);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        abi_nz(INC8(OBJ->state[cpu.x]));
        return;
    }
    abi_nz(DEC8(OBJ->work_d[slot]));
    if (NE) {
        return;
    }
    lda(0x01);
    WR8(OBJ->state[slot], cpu.a);
    lda(0x4F);
    k_sound();
}

void abi_FallingSpikeTrap07E0(void)
{
    FallingSpikeTrap07E0(cpu.x);
}

/*
 * @routine 07E0:0AE FlickeringTorch07E0
 * @run 71:D8AE
 * WHAT IT DOES: State 0 loads the record as sprite set 6. Then keeps the script of its slot running (one of two by the wave
 * counter) and steps the list at $D8DF.
 * HOW: StartScriptSlotKind1 when the slot is empty.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlickeringTorch07E0(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xD8B6, k_load_record());
        lda(0x06);
        WR8(OBJ->sprite_set[cpu.x], cpu.a);
        return;
    }
    phx();
    lda(RD8(OBJ->start_arg[slot]));
    tax();
    lda(RD8(SLOTS->kind[cpu.x]));
    if (EQ) {
        lda(RD8(ZP->wave_counter));
        lsr_a();
        lsr_a();
        and_a(0x01);
        tay();
        lda(rd8((uint16_t)(0xD8EA + cpu.y)));
        JSR(0xD8D3, k_script1());
    }
    plx();
    list_to_temp10_high_first(0xD8DF);
    k_step_frames();
}

void abi_FlickeringTorch07E0(void)
{
    FlickeringTorch07E0(cpu.x);
}

/*
 * @routine 07E0:0EC RaisedWhenFlag07E0
 * @run 71:D8EC
 * WHAT IT DOES: State 0 loads the record as sprite set 6, frame $1D. Once zField8C is set, rises 2 a frame until off the top.
 * HOW: Branches by state.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void RaisedWhenFlag07E0(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xD8F4, k_load_record());
        lda(0x06);
        WR8(OBJ->sprite_set[cpu.x], cpu.a);
        lda(0x1D);
        WR8(OBJ->frame[cpu.x], cpu.a);
        return;
    }
    dec_a();
    if (EQ) {
        lda(RD8(ZP->field8c));
        if (EQ) {
            return;
        }
        abi_nz(INC8(OBJ->state[slot]));
        return;
    }
    abi_nz(DEC8(OBJ->y_lo[slot]));
    abi_nz(DEC8(OBJ->y_lo[slot]));
    if (PL) {
        return;
    }
    k_clear();
}

void abi_RaisedWhenFlag07E0(void)
{
    RaisedWhenFlag07E0(cpu.x);
}

/*
 * @routine 07E0:115 PictureByArgument07E0
 * @run 71:D915
 * WHAT IT DOES: State 0 loads the record with the frame OBJ_START_ARG, sprite set 6.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void PictureByArgument07E0(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (NE) {
        return;
    }
    JSR(0xD91D, k_load_record());
    lda(RD8(OBJ->start_arg[cpu.x]));
    WR8(OBJ->frame[cpu.x], cpu.a);
    lda(0x06);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
}

void abi_PictureByArgument07E0(void)
{
    PictureByArgument07E0(cpu.x);
}

/*
 * @routine 07E0:129 ByState07E0D929
 * @run 71:D929
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $D93B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState07E0D929(uint8_t slot)
{
    jump_by_state(slot, 0xD93B);
}

void abi_ByState07E0D929(void)
{
    ByState07E0D929(cpu.x);
}

/*
 * @routine 07E0:147 ExitDoorWhenHit07E0
 * @run 71:D947
 * WHAT IT DOES: A first hit with kind 7 in slot 5 marks it caught. With the door ready and not held, a hit of 1 or 2 takes the
 * exit OBJ_START_ARG's low digit plus one: sub-step 3, sound $90, a count of 4.
 * HOW: Tests of the door and hit.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ExitDoorWhenHit07E0(uint8_t slot)
{
    lda(RD8(OBJ->work_f[slot]));
    if (EQ) {
        lda(RD8(OBJ->hit_taken[slot]));
        if (EQ) {
            return;
        }
        lda(RD8(OBJ->kind[5]));
        cmp(cpu.a, 0x07);
        if (NE) {
            return;
        }
        lda(0x01);
        WR8(PVARS2->kind7_caught, cpu.a);
        abi_nz(INC8(OBJ->work_f[slot]));
    }
    lda(rd8(StageDoorHeld));
    eor_a(0x01);
    and_a(RD8(PVARS->door_ready));
    if (EQ) {
        return;
    }
    lda(RD8(OBJ->hit_taken[slot]));
    if (EQ) {
        return;
    }
    cmp(cpu.a, 0x03);
    if (CS) {
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    and_a(0x0F);
    inc_a();
    WR8(ZP->field8c, cpu.a);
    lda(0x03);
    WR8(ZP->sub_step, cpu.a);
    lda(0x90);
    JSR(0xD984, k_sound());
    abi_nz(INC8(OBJ->state[cpu.x]));
    WR8(OBJ->work_e[cpu.x], 0);
    lda(0x04);
    WR8(OBJ->work_d[cpu.x], cpu.a);
}

void abi_ExitDoorWhenHit07E0(void)
{
    ExitDoorWhenHit07E0(cpu.x);
}

/*
 * @routine 07E0:190 NextWhenPart2_07E0
 * @run 71:D990
 * WHAT IT DOES: Goes on once zField85 is 2.
 * HOW: A compare.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void NextWhenPart2_07E0(uint8_t slot)
{
    lda(RD8(ZP->field85));
    cmp(cpu.a, 0x02);
    if (NE) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_NextWhenPart2_07E0(void)
{
    NextWhenPart2_07E0(cpu.x);
}

/*
 * @routine 07E0:19A FlagAfterFourWaits07E0
 * @run 71:D99A
 * WHAT IT DOES: After four waits of OBJ_PIECE_NEXT, goes on and sets zField8C bit 7.
 * HOW: A count in OBJ_PIECE_PLACE.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlagAfterFourWaits07E0(uint8_t slot)
{
    abi_nz(DEC8(OBJ->work_d[slot]));
    if (NE) {
        return;
    }
    abi_nz(INC8(OBJ->work_e[slot]));
    lda(RD8(OBJ->work_e[slot]));
    cmp(cpu.a, 0x04);
    if (EQ) {
        abi_nz(INC8(OBJ->state[slot]));
        lda(RD8(ZP->field8c));
        ora_a(0x80);
        WR8(ZP->field8c, cpu.a);
        return;
    }
    lda(0x04);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_FlagAfterFourWaits07E0(void)
{
    FlagAfterFourWaits07E0(cpu.x);
}

/*
 * @routine 07E0:1B9 SetExitWhenFlagCleared07E0
 * @run 71:D9B9
 * WHAT IT DOES: Once zField8C bit 7 is clear, goes on; Richter's progress bit 0 sets this argument's high digit to $A. zField8C
 * becomes the argument plus one.
 * HOW: Reads of ProgressCharacter.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SetExitWhenFlagCleared07E0(uint8_t slot)
{
    lda(RD8(ZP->field8c));
    if (MI) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
    lda(0x04);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(rd8(ProgressCharacter));
    and_a(0x01);
    if (NE) {
        lda(RD8(OBJ->start_arg[slot]));
        and_a(0x0F);
        ora_a(0xA0);
        WR8(OBJ->start_arg[slot], cpu.a);
    }
    lda(RD8(OBJ->start_arg[slot]));
    inc_a();
    WR8(ZP->field8c, cpu.a);
}

void abi_SetExitWhenFlagCleared07E0(void)
{
    SetExitWhenFlagCleared07E0(cpu.x);
}
