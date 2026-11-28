/*
 * Stage 05's objects, runs B82F-C8EA.
 */
#include "stage_obj_ops.h"
#include "stage05_objects_b.h"
#include "calls_b.h"

/*
 * @routine 093C:02F ItemMarkThenByState093C
 * @run 70:B82F
 * WHAT IT DOES: Checks contact ($B85D), the flag ($B868) and the mark ($B882); touching it ends here, else clears slot 5, sets the
 * hit box offset and goes to the state's handler.
 * HOW: A table of handlers at $B853.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ItemMarkThenByState093C(uint8_t slot)
{
    (void)slot;
    call_routine(0xB832, 0xB85D);
    call_routine(0xB835, 0xB868);
    call_routine(0xB838, 0xB882);
    if (CS) {
        return;
    }
    call_routine(0xB83E, 0xB8B2);
    call_routine(0xB841, 0xB90A);
    jump_by_state(cpu.x, 0xB853);
}

void abi_ItemMarkThenByState093C(void)
{
    ItemMarkThenByState093C(cpu.x);
}

/*
 * @routine 093C:0B2 ProbeFromBelow093C
 * @run 70:B8B2
 * WHAT IT DOES: On the first screen past state 0: probes $10 times OBJ_START_ARG below, falling, and stops on ground there.
 * HOW: Y moved down for the probe and back.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ProbeFromBelow093C(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        return;
    }
    lda(RD8(OBJ->y_hi[slot]));
    if (NE) {
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    asl_a();
    asl_a();
    asl_a();
    asl_a();
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->y_lo[slot]));
    clc();
    adc_a(RD8(ZP->temp00));
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(RD8(OBJ->y_hi[slot]));
    adc_a(0x00);
    WR8(OBJ->y_hi[slot], cpu.a);
    JSR(0xB8D9, k_move_y());
    JSR(0xB8DC, k_gravity());
    lda(0x04);
    WR8(ZP->sprite_flip_mask, cpu.a);
    lda(0x09);
    JSR(0xB8E5, k_probe());
    slot = cpu.x;
    and_a(0x01);
    if (NE) {
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->vy_lo[slot], 0);
    }
    lda(RD8(OBJ->start_arg[slot]));
    asl_a();
    asl_a();
    asl_a();
    asl_a();
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->y_lo[slot]));
    sec();
    sbc_a(RD8(ZP->temp00));
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(RD8(OBJ->y_hi[slot]));
    sbc_a(0x00);
    WR8(OBJ->y_hi[slot], cpu.a);
}

void abi_ProbeFromBelow093C(void)
{
    ProbeFromBelow093C(cpu.x);
}

/*
 * @routine 093C:273 BySubstate093CBA73
 * @run 70:BA73
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $BA85.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate093CBA73(uint8_t slot)
{
    jump_by_substate(slot, 0xBA85);
}

void abi_BySubstate093CBA73(void)
{
    BySubstate093CBA73(cpu.x);
}

/*
 * @routine 093C:2F3 TossObjectByCount093C
 * @run 70:BAF3
 * WHAT IT DOES: In a free slot from $30 (none: $BB85) starts the kind the table gives for OBJ_RAIN_COUNT, thrown left at -8 or,
 * with OBJ_WORK_J, at -18 facing the other way; goes to the next substate.
 * HOW: The count and parent kept in the new object.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void TossObjectByCount093C(uint8_t slot)
{
    JSR(0xBAF6, k_free_from30());
    if (CC) {
        run_routine(0xBB85);
        return;
    }
    WR8(ZP->burst_count, cpu.y);
    ldy(RD8(OBJ->work_i[cpu.x]));
    lda(rd8((uint16_t)(0xBB35 + cpu.y)));
    ldy(RD8(ZP->burst_count));
    JSR(0xBB08, k_start_at_place());
    slot = cpu.x;
    lda(RD8(OBJ->work_i[slot]));
    WR8(OBJ->work_d[cpu.y], cpu.a);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    txa();
    WR8(OBJ->work_f[cpu.y], cpu.a);
    lda(RD8(OBJ->work_j[slot]));
    if (EQ) {
        lda(0xF8);
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
    } else {
        lda(0xEE);
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        lda(RD8(OBJ->facing_left[cpu.y]));
        eor_a(0x01);
        WR8(OBJ->facing_left[cpu.y], cpu.a);
    }
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_TossObjectByCount093C(void)
{
    TossObjectByCount093C(cpu.x);
}

/*
 * @routine 093C:385 State1ByWorkJ093C
 * @run 70:BB85
 * WHAT IT DOES: State 1 with frame 1, or 4 with OBJ_WORK_J.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void State1ByWorkJ093C(uint8_t slot)
{
    lda(0x01);
    WR8(OBJ->state[slot], cpu.a);
    lda(0x01);
    WR8(OBJ->frame[slot], cpu.a);
    lda(RD8(OBJ->work_j[slot]));
    if (EQ) {
        return;
    }
    lda(0x04);
    WR8(OBJ->frame[slot], cpu.a);
}

void abi_State1ByWorkJ093C(void)
{
    State1ByWorkJ093C(cpu.x);
}

/*
 * @routine 093C:4DF TableThenFollow093C
 * @run 70:BCDF
 * WHAT IT DOES: Dispatches by state ($BCE5) and follows the parent ($BC8F).
 * HOW: Two routines.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void TableThenFollow093C(uint8_t slot)
{
    (void)slot;
    call_routine(0xBCE2, 0xBCE5);
    run_routine(0xBC8F);
}

void abi_TableThenFollow093C(void)
{
    TableThenFollow093C(cpu.x);
}

/*
 * @routine 093C:612 CountHitThenByState093C
 * @run 70:BE12
 * WHAT IT DOES: Counts OBJ_FALL_LO, bursts when hit ($BE38) and goes to the state's handler.
 * HOW: A table of handlers at $BE2A.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountHitThenByState093C(uint8_t slot)
{
    abi_nz(INC8(OBJ->work_h[slot]));
    call_routine(0xBE18, 0xBE38);
    jump_by_state(cpu.x, 0xBE2A);
}

void abi_CountHitThenByState093C(void)
{
    CountHitThenByState093C(cpu.x);
}

/*
 * @routine 093C:638 BurstWhenHit093C
 * @run 70:BE38
 * WHAT IT DOES: A hit outside state 6 sounds $93, bursts ($BE5D) and goes to state 6 without a hit box.
 * HOW: A call and stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BurstWhenHit093C(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x06);
    if (EQ) {
        return;
    }
    lda(RD8(OBJ->hit_status[slot]));
    and_a(0x80);
    if (EQ) {
        return;
    }
    lda(0x93);
    JSR(0xBE4B, k_sound());
    call_routine(0xBE4E, 0xBE5D);
    slot = cpu.x;
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x06);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_BurstWhenHit093C(void)
{
    BurstWhenHit093C(cpu.x);
}

/*
 * @routine 093C:65D ScatterSix093C
 * @run 70:BE5D
 * WHAT IT DOES: Starts up to six kind $2B pieces in free slots, each with the row's waits and speeds.
 * HOW: The row in zBurstCount, X kept in zClimbStepBefore.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ScatterSix093C(uint8_t slot)
{
    (void)slot;
    cpu.y = 0;
    do {
        WR8(ZP->burst_count, cpu.y);
        JSR(0xBE63, k_free_from30());
        if (CC) {
            return;
        }
        lda(0x2B);
        JSR(0xBE6A, k_start_at_place());
        WR8(ZP->climb_step_before, cpu.x);
        lda(RD8(ZP->burst_count));
        asl_a();
        asl_a();
        tax();
        lda(rd8((uint16_t)(0xBE93 + cpu.x)));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xBE94 + cpu.x)));
        WR8(OBJ->work_e[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xBE95 + cpu.x)));
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xBE96 + cpu.x)));
        WR8(OBJ->vy_hi[cpu.y], cpu.a);
        ldx(RD8(ZP->climb_step_before));
        ldy(RD8(ZP->burst_count));
        iny();
        cmp(cpu.y, 0x06);
    } while (CC);
}

void abi_ScatterSix093C(void)
{
    ScatterSix093C(cpu.x);
}

/*
 * @routine 093C:767 BySubstate093CBF67
 * @run 70:BF67
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $BF79.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate093CBF67(uint8_t slot)
{
    jump_by_substate(slot, 0xBF79);
}

void abi_BySubstate093CBF67(void)
{
    BySubstate093CBF67(cpu.x);
}

/*
 * @routine 093C:781 SpringAway093C
 * @run 70:BF81
 * WHAT IT DOES: As SpringAway07DB with this stage's speeds ($BFAA) and halving ($C016).
 * HOW: PlayerDistanceSum, NegateVelocityX and Y.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SpringAway093C(uint8_t slot)
{
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->work_k[slot], cpu.a);
    JSR(0xBF8A, k_distance_sum());
    and_a(0x0F);
    WR8(OBJ->work_i[cpu.x], cpu.a);
    call_routine(0xBF92, 0xBFAA);
    JSR(0xBF95, k_negate_x());
    JSR(0xBF98, k_negate_y());
    call_routine(0xBF9B, 0xC016);
    slot = cpu.x;
    lda(0x0C);
    WR8(OBJ->work_d[slot], cpu.a);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SpringAway093C(void)
{
    SpringAway093C(cpu.x);
}

/*
 * @routine 093D:016 HalveVelocity093D
 * @run 71:C016
 * WHAT IT DOES: Halves both speeds, keeping their signs.
 * HOW: ASL for the sign into carry, then ROR of the high and low bytes.
 * WAITS: no.
 * Entry: X object. Exit: C the last bit out, N and Z from VY low.
 */
void HalveVelocity093D(uint8_t slot)
{
    lda(RD8(OBJ->vx_hi[slot]));
    asl_a();
    ROR_MEM(OBJ->vx_hi[slot]);
    ROR_MEM(OBJ->vx_lo[slot]);
    lda(RD8(OBJ->vy_hi[slot]));
    asl_a();
    ROR_MEM(OBJ->vy_hi[slot]);
    ROR_MEM(OBJ->vy_lo[slot]);
}

void abi_HalveVelocity093D(void)
{
    HalveVelocity093D(cpu.x);
}

/*
 * @routine 093D:055 ChaseAndRise093D
 * @run 71:C055
 * WHAT IT DOES: As ChaseAndRise07DB with this stage's clamp ($C0C0), pictures ($BEE9), throw ($C0E5) and sound ($C0F5).
 * HOW: Calls and 16-bit adds.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ChaseAndRise093D(uint8_t slot)
{
    JSR(0xC058, k_move());
    JSR(0xC05B, k_side());
    slot = cpu.x;
    and_a(0x01);
    WR8(ZP->temp00, cpu.a);
    asl_a();
    tay();
    lda(RD8(OBJ->vx_lo[slot]));
    clc();
    adc_a(rd8((uint16_t)(0xC0BC + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    adc_a(rd8((uint16_t)(0xC0BD + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    call_routine(0xC077, 0xC0C0);
    slot = cpu.x;
    lda(RD8(OBJ->vy_lo[slot]));
    sec();
    sbc_a(0x10);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    sbc_a(0x00);
    WR8(OBJ->vy_hi[slot], cpu.a);
    if (PL) {
        ldy(0x04);
        call_routine(0xC08F, 0xBEE9);
        slot = cpu.x;
        lda(RD8(OBJ->vx_hi[slot]));
        rol_a();
        rol_a();
        and_a(0x01);
        eor_a(RD8(ZP->temp00));
        if (EQ) {
            return;
        }
        lda(0x20);
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    call_routine(0xC0A3, 0xC0E5);
    ldy(0x04);
    call_routine(0xC0A8, 0xBEE9);
    call_routine(0xC0AB, 0xC0F5);
    slot = cpu.x;
    lda(RD8(OBJ->y_hi[slot]));
    if (NE) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, RD8(OBJ->work_j[slot]));
    if (CS) {
        return;
    }
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ChaseAndRise093D(void)
{
    ChaseAndRise093D(cpu.x);
}

/*
 * @routine 093D:0C0 ClampSpeedXToTwo093D
 * @run 71:C0C0
 * WHAT IT DOES: Keeps the X speed within -2 and 2.
 * HOW: Compares of the high byte.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ClampSpeedXToTwo093D(uint8_t slot)
{
    lda(RD8(OBJ->vx_hi[slot]));
    if (PL) {
        lda(RD8(OBJ->vx_hi[slot]));
        cmp(cpu.a, 0x02);
        if (CC) {
            return;
        }
        lda(0x02);
        WR8(OBJ->vx_hi[slot], cpu.a);
        WR8(OBJ->vx_lo[slot], 0);
        return;
    }
    lda(RD8(OBJ->vx_hi[slot]));
    cmp(cpu.a, 0xFE);
    if (CS) {
        return;
    }
    lda(0xFE);
    WR8(OBJ->vx_hi[slot], cpu.a);
    WR8(OBJ->vx_lo[slot], 0);
}

void abi_ClampSpeedXToTwo093D(void)
{
    ClampSpeedXToTwo093D(cpu.x);
}

/*
 * @routine 093D:0E5 ThrowUpward093D
 * @run 71:C0E5
 * WHAT IT DOES: Unless already rising at -2 or less, rises at -3.
 * HOW: A compare of the Y speed.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ThrowUpward093D(uint8_t slot)
{
    lda(RD8(OBJ->vy_hi[slot]));
    cmp(cpu.a, 0xFE);
    if (CS) {
        return;
    }
    lda(0xFD);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->vy_lo[slot], 0);
}

void abi_ThrowUpward093D(void)
{
    ThrowUpward093D(cpu.x);
}

/*
 * @routine 093D:102 HoverNearTop093D
 * @run 71:C102
 * WHAT IT DOES: As HoverNearTop07DB with this stage's pictures, sound and halving.
 * HOW: HalveVelocity093D.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void HoverNearTop093D(uint8_t slot)
{
    ldy(0x04);
    call_routine(0xC107, 0xBEE9);
    call_routine(0xC10A, 0xC0F5);
    JSR(0xC10D, k_move());
    slot = cpu.x;
    lda(RD8(OBJ->y_hi[slot]));
    if (NE) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0x10);
    if (CS) {
        lda(RD8(OBJ->work_h[slot]));
        and_a(0x01);
        if (NE) {
            return;
        }
        call_routine(0xC123, 0xC016);
        slot = cpu.x;
        lda(RD8(OBJ->vy_hi[slot]));
        ora_a(RD8(OBJ->vy_lo[slot]));
        if (NE) {
            lda(RD8(OBJ->vy_hi[slot]));
            and_a(RD8(OBJ->vy_lo[slot]));
            cmp(cpu.a, 0xFF);
            if (NE) {
                return;
            }
        }
    }
    lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_HoverNearTop093D(void)
{
    HoverNearTop093D(cpu.x);
}

/*
 * @routine 093D:1AF BecomeType9CAtListEnd093D
 * @run 71:C1AF
 * WHAT IT DOES: Steps list 8 ($BEE9) and becomes kind $9C when it ends.
 * HOW: StepFramesFacingPlayerC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BecomeType9CAtListEnd093D(uint8_t slot)
{
    (void)slot;
    ldy(0x08);
    call_routine(0xC1B4, 0xBEE9);
    if (NE) {
        return;
    }
    lda(0x9C);
    WR8(OBJ->kind[cpu.x], cpu.a);
    WR8(OBJ->state[cpu.x], 0);
}

void abi_BecomeType9CAtListEnd093D(void)
{
    BecomeType9CAtListEnd093D(cpu.x);
}

/*
 * @routine 093D:1BF CountThenByState093D1BF
 * @run 71:C1BF
 * WHAT IT DOES: Counts down to state 4 ($C1DE) and goes to the state's handler.
 * HOW: A table of handlers at $C1D4.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountThenByState093D1BF(uint8_t slot)
{
    (void)slot;
    call_routine(0xC1C2, 0xC1DE);
    jump_by_state(cpu.x, 0xC1D4);
}

void abi_CountThenByState093D1BF(void)
{
    CountThenByState093D1BF(cpu.x);
}

/*
 * @routine 093D:1DE CountdownThenState4_093D
 * @run 71:C1DE
 * WHAT IT DOES: Counts OBJ_PIECE_PLACE out and goes to state 4.
 * HOW: A countdown.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountdownThenState4_093D(uint8_t slot)
{
    abi_nz(DEC8(OBJ->work_e[slot]));
    if (NE) {
        return;
    }
    lda(0x04);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_CountdownThenState4_093D(void)
{
    CountdownThenState4_093D(cpu.x);
}

/*
 * @routine 093D:1E9 LoadRecordStartAngle40_093D
 * @run 71:C1E9
 * WHAT IT DOES: Loads the record, moves, and starts the angle at $40.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LoadRecordStartAngle40_093D(uint8_t slot)
{
    (void)slot;
    JSR(0xC1EC, k_load_record());
    JSR(0xC1EF, k_move());
    lda(0x40);
    WR8(OBJ->work_f[cpu.x], cpu.a);
}

void abi_LoadRecordStartAngle40_093D(void)
{
    LoadRecordStartAngle40_093D(cpu.x);
}

/*
 * @routine 093D:1FE OrbitStepBack093D
 * @run 71:C1FE
 * WHAT IT DOES: Moves along the angle at speed 1 and turns it back by 4; at $E0 starts again at $40 and goes on.
 * HOW: SetVelocityFromAngle and ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void OrbitStepBack093D(uint8_t slot)
{
    WR8(ZP->temp00, 0);
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_f[slot]));
    JSR(0xC20A, k_velocity_from_angle());
    JSR(0xC20D, k_move());
    slot = cpu.x;
    lda(RD8(OBJ->work_f[slot]));
    sec();
    sbc_a(0x04);
    WR8(OBJ->work_f[slot], cpu.a);
    cmp(cpu.a, 0xE0);
    if (NE) {
        return;
    }
    lda(0x40);
    WR8(OBJ->work_f[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_OrbitStepBack093D(void)
{
    OrbitStepBack093D(cpu.x);
}

/*
 * @routine 093D:223 OrbitStepForward093D
 * @run 71:C223
 * WHAT IT DOES: Moves along the angle at speed 1 and turns it on by 4; at $A0 starts again at $40 and goes back.
 * HOW: SetVelocityFromAngle and ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void OrbitStepForward093D(uint8_t slot)
{
    WR8(ZP->temp00, 0);
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_f[slot]));
    JSR(0xC22F, k_velocity_from_angle());
    JSR(0xC232, k_move());
    slot = cpu.x;
    lda(RD8(OBJ->work_f[slot]));
    clc();
    adc_a(0x04);
    WR8(OBJ->work_f[slot], cpu.a);
    cmp(cpu.a, 0xA0);
    if (NE) {
        return;
    }
    lda(0x40);
    WR8(OBJ->work_f[slot], cpu.a);
    abi_nz(DEC8(OBJ->state[slot]));
}

void abi_OrbitStepForward093D(void)
{
    OrbitStepForward093D(cpu.x);
}

/*
 * @routine 093D:251 ChecksThenByState093D251
 * @run 71:C251
 * WHAT IT DOES: Counts, faces the player in some states, takes damage ($C29B), the flag ($C2B7) and the hit box ($C2D7), then goes
 * to the state's handler.
 * HOW: A table of handlers at $C272.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ChecksThenByState093D251(uint8_t slot)
{
    abi_nz(INC8(OBJ->work_h[slot]));
    call_routine(0xC257, 0xC282);
    call_routine(0xC25A, 0xC29B);
    call_routine(0xC25D, 0xC2B7);
    call_routine(0xC260, 0xC2D7);
    jump_by_state(cpu.x, 0xC272);
}

void abi_ChecksThenByState093D251(void)
{
    ChecksThenByState093D251(cpu.x);
}

/*
 * @routine 093D:29B DamageToState6_093D
 * @run 71:C29B
 * WHAT IT DOES: With OBJ_RAIN_COUNT set and under state 6, damage that leaves zTemp00 goes to state 6.
 * HOW: HitDamage0939.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DamageToState6_093D(uint8_t slot)
{
    lda(RD8(OBJ->work_i[slot]));
    if (EQ) {
        return;
    }
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x06);
    if (CS) {
        return;
    }
    call_routine(0xC2AA, 0xA28D);
    slot = cpu.x;
    lda(RD8(ZP->temp00));
    if (EQ) {
        return;
    }
    lda(0x06);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_DamageToState6_093D(void)
{
    DamageToState6_093D(cpu.x);
}

/*
 * @routine 093D:316 NextStateOnFrontLanding093D
 * @run 71:C316
 * WHAT IT DOES: Goes to the next state on landing at the front column, height 6.
 * HOW: StopOnLandingFrontColumn.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void NextStateOnFrontLanding093D(uint8_t slot)
{
    (void)slot;
    lda(0x06);
    JSR(0xC31B, k_stop_on_landing_front());
    if (CC) {
        return;
    }
    abi_nz(INC8(OBJ->state[cpu.x]));
}

void abi_NextStateOnFrontLanding093D(void)
{
    NextStateOnFrontLanding093D(cpu.x);
}

/*
 * @routine 093D:321 WalkNearOrAttack093D
 * @run 71:C321
 * WHAT IT DOES: Walks ($C33A with Y 0), keeps the facing, goes to state 3 within $4C and checks the ground ($C3B3).
 * HOW: DistanceXToPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WalkNearOrAttack093D(uint8_t slot)
{
    cpu.y = 0;
    call_routine(0xC325, 0xC33A);
    slot = cpu.x;
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->work_e[slot], cpu.a);
    JSR(0xC32E, k_distance_x());
    cmp(cpu.a, 0x4C);
    if (CC) {
        lda(0x03);
        WR8(OBJ->state[cpu.x], cpu.a);
    }
    run_routine(0xC3B3);
}

void abi_WalkNearOrAttack093D(void)
{
    WalkNearOrAttack093D(cpu.x);
}

/*
 * @routine 093D:36E MoveTurningAtWalls093D
 * @run 71:C36E
 * WHAT IT DOES: Moves sideways; when the terrain ahead is solid at probes 1 and 2, turns back and moves. Carry clear when it
 * turned, set when not.
 * HOW: ApplyVelocityX and two ProbeTerrain with SpriteFlipMask 6.
 * WAITS: no.
 * Entry: X object, Y and C for the probe. Exit: C.
 */
void MoveTurningAtWalls093D(uint8_t slot)
{
    JSR(0xC371, k_move_x());
    lda(0x06);
    WR8(ZP->sprite_flip_mask, cpu.a);
    lda(0x01);
    JSR(0xC37A, k_probe());
    slot = cpu.x;
    lda(RD8(OBJ->vx_hi[slot]));
    rol_a();
    rol_a();
    and_a(0x01);
    eor_a(0x01);
    tay();
    lda(rd8((uint16_t)(TileHit + 1 + cpu.y)));
    and_a(0x01);
    if (NE) {
        lda(0x06);
        WR8(ZP->sprite_flip_mask, cpu.a);
        lda(0x02);
        JSR(0xC394, k_probe());
        slot = cpu.x;
        lda(RD8(OBJ->vx_hi[slot]));
        rol_a();
        rol_a();
        and_a(0x01);
        eor_a(0x01);
        asl_a();
        tay();
        lda(rd8((uint16_t)(TileHit + cpu.y)));
        ora_a(rd8((uint16_t)(TileHit + 1 + cpu.y)));
        and_a(0x02);
        if (EQ) {
            sec();
            return;
        }
    }
    JSR(0xC3AE, k_negate_x());
    JSR(0xC3B1, k_move_x());
    clc();
}

void abi_MoveTurningAtWalls093D(void)
{
    MoveTurningAtWalls093D(cpu.x);
}

/*
 * @routine 093D:3B3 CheckGroundAhead093D
 * @run 71:C3B3
 * WHAT IT DOES: Without solid ground below, or with a wall ahead on its side, goes to state 4; otherwise counts OBJ_WORK_D down
 * to state 5.
 * HOW: Two ProbeTerrain with SpriteFlipMask 6.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CheckGroundAhead093D(uint8_t slot)
{
    bool turn;

    lda(0x06);
    WR8(ZP->sprite_flip_mask, cpu.a);
    lda(0x09);
    JSR(0xC3BC, k_probe());
    slot = cpu.x;
    lda(rd8(TileHit));
    and_a(rd8((uint16_t)(TileHit + 1)));
    and_a(rd8((uint16_t)(TileHit + 2)));
    and_a(0x01);
    turn = EQ;
    if (!turn) {
        lda(0x06);
        WR8(ZP->sprite_flip_mask, cpu.a);
        lda(0x22);
        JSR(0xC3D2, k_probe());
        slot = cpu.x;
        lda(RD8(OBJ->work_e[slot]));
        asl_a();
        tay();
        lda(rd8((uint16_t)(TileHit + cpu.y)));
        ora_a(rd8((uint16_t)(TileHit + 1 + cpu.y)));
        and_a(0x02);
        turn = EQ;
    }
    if (turn) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        lda(0x04);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
        return;
    }
    abi_nz(DEC8(OBJ->work_d[slot]));
    if (NE) {
        return;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_CheckGroundAhead093D(void)
{
    CheckGroundAhead093D(cpu.x);
}

/*
 * @routine 093D:404 WalkFarOrRetreat093D
 * @run 71:C404
 * WHAT IT DOES: Walks ($C33A with Y 2), keeps the facing reversed, goes to state 2 from $6C away and checks the ground.
 * HOW: DistanceXToPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WalkFarOrRetreat093D(uint8_t slot)
{
    ldy(0x02);
    call_routine(0xC409, 0xC33A);
    slot = cpu.x;
    lda(RD8(OBJ->facing_left[slot]));
    eor_a(0x01);
    WR8(OBJ->work_e[slot], cpu.a);
    JSR(0xC414, k_distance_x());
    cmp(cpu.a, 0x6C);
    if (CS) {
        lda(0x02);
        WR8(OBJ->state[cpu.x], cpu.a);
    }
    run_routine(0xC3B3);
}

void abi_WalkFarOrRetreat093D(void)
{
    WalkFarOrRetreat093D(cpu.x);
}

/*
 * @routine 093D:420 BySubstate093DC420
 * @run 71:C420
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $C432.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate093DC420(uint8_t slot)
{
    jump_by_substate(slot, 0xC432);
}

void abi_BySubstate093DC420(void)
{
    BySubstate093DC420(cpu.x);
}

/*
 * @routine 093D:467 LandThenFrame39_093D
 * @run 71:C467
 * WHAT IT DOES: On landing at the front column: frame $39, a wait of $C, next substate.
 * HOW: StopOnLandingFrontColumn.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LandThenFrame39_093D(uint8_t slot)
{
    lda(0x06);
    JSR(0xC46C, k_stop_on_landing_front());
    if (CC) {
        return;
    }
    slot = cpu.x;
    lda(0x0C);
    WR8(OBJ->work_g[slot], cpu.a);
    lda(0x39);
    WR8(OBJ->frame[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_LandThenFrame39_093D(void)
{
    LandThenFrame39_093D(cpu.x);
}

/*
 * @routine 093D:47C CountThenState2_093D
 * @run 71:C47C
 * WHAT IT DOES: Counts OBJ_SWOOP_SPEED_HI out and goes to state 2 from its start.
 * HOW: A countdown.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountThenState2_093D(uint8_t slot)
{
    abi_nz(DEC8(OBJ->work_g[slot]));
    if (NE) {
        return;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_CountThenState2_093D(void)
{
    CountThenState2_093D(cpu.x);
}

/*
 * @routine 093D:48D BySubstate093DC48D
 * @run 71:C48D
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $C49F.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate093DC48D(uint8_t slot)
{
    jump_by_substate(slot, 0xC49F);
}

void abi_BySubstate093DC48D(void)
{
    BySubstate093DC48D(cpu.x);
}

/*
 * @routine 093D:4D2 SlashAlongList093D
 * @run 71:C4D2
 * @stop C50A
 * WHAT IT DOES: Steps the list at $C50C; at its end sets OBJ_RAIN_COUNT and goes to state 3. On a new frame, frame $2B sounds
 * $46; moves at the frame's speed and, turned at a wall, throws ($C51F).
 * HOW: MoveTurningAtWalls093D.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SlashAlongList093D(uint8_t slot)
{
    WR8(OBJ->work_i[slot], 0);
    list_to_temp10_high_first(0xC50C);
    JSR(0xC4E0, k_step_frames());
    slot = cpu.x;
    if (EQ) {
        lda(0x01);
        WR8(OBJ->work_i[slot], cpu.a);
        lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
        return;
    }
    if (CC) {
        return;
    }
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x2B);
    if (EQ) {
        lda(0x46);
        JSR(0xC4F0, k_sound());
        slot = cpu.x;
    }
    ldy(RD8(OBJ->work_e[slot]));
    lda(rd8((uint16_t)(0xC50A + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    call_routine(0xC4FC, 0xC36E);
    if (CS) {
        run_routine(0xC51F);
    }
}

void abi_SlashAlongList093D(void)
{
    SlashAlongList093D(cpu.x);
}

/*
 * @routine 093D:595 LandThenWait093DC595
 * @run 71:C595
 * WHAT IT DOES: Steps the list at $C5BA; on landing: sound 2, a wait of $18, frame $39, next substate.
 * HOW: StopOnLandingFrontColumn.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LandThenWait093DC595(uint8_t slot)
{
    list_to_temp10_high_first(0xC5BA);
    JSR(0xC5A0, k_step_frames());
    lda(0x06);
    JSR(0xC5A5, k_stop_on_landing_front());
    if (CC) {
        return;
    }
    lda(0x02);
    JSR(0xC5AC, k_sound());
    slot = cpu.x;
    lda(0x18);
    WR8(OBJ->work_g[slot], cpu.a);
    lda(0x39);
    WR8(OBJ->frame[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_LandThenWait093DC595(void)
{
    LandThenWait093DC595(cpu.x);
}

/*
 * @routine 093D:5D4 HopAtPlayer093D
 * @run 71:C5D4
 * @stop C602
 * WHAT IT DOES: Substate 0 hops toward the player with frame $36; later lands ($C606).
 * HOW: Speeds by side.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void HopAtPlayer093D(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (NE) {
        run_routine(0xC606);
        return;
    }
    JSR(0xC5DC, k_side());
    slot = cpu.x;
    and_a(0x01);
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xC603 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC602 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(0xFD);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->vy_lo[slot], cpu.a);
    WR8(OBJ->vy_lo[slot], 0);
    lda(0x36);
    WR8(OBJ->frame[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_HopAtPlayer093D(void)
{
    HopAtPlayer093D(cpu.x);
}

/*
 * @routine 093D:606 LandThenState3_093D
 * @run 71:C606
 * WHAT IT DOES: On landing at the front column, state 3 from its start.
 * HOW: StopOnLandingFrontColumn.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LandThenState3_093D(uint8_t slot)
{
    lda(0x06);
    JSR(0xC60B, k_stop_on_landing_front());
    if (CC) {
        return;
    }
    slot = cpu.x;
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_LandThenState3_093D(void)
{
    LandThenState3_093D(cpu.x);
}

/*
 * @routine 093D:7C7 ChecksThenByState093D7C7
 * @run 71:C7C7
 * WHAT IT DOES: Counts, faces the player in some states, runs three checks and goes to the state's handler.
 * HOW: A table of handlers at $C7E8.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ChecksThenByState093D7C7(uint8_t slot)
{
    abi_nz(INC8(OBJ->work_d[slot]));
    call_routine(0xC7CD, 0xC7FC);
    call_routine(0xC7D0, 0xC817);
    call_routine(0xC7D3, 0xC89C);
    call_routine(0xC7D6, 0xC8BC);
    jump_by_state(cpu.x, 0xC7E8);
}

void abi_ChecksThenByState093D7C7(void)
{
    ChecksThenByState093D7C7(cpu.x);
}

/*
 * @routine 093D:7FC FacePlayerInSomeStates093D
 * @run 71:C7FC
 * WHAT IT DOES: Faces the player in the states the table at $C80D marks.
 * HOW: SideOfPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FacePlayerInSomeStates093D(uint8_t slot)
{
    ldy(RD8(OBJ->state[slot]));
    lda(rd8((uint16_t)(0xC80D + cpu.y)));
    if (EQ) {
        return;
    }
    JSR(0xC807, k_side());
    and_a(0x01);
    WR8(OBJ->facing_left[cpu.x], cpu.a);
}

void abi_FacePlayerInSomeStates093D(void)
{
    FacePlayerInSomeStates093D(cpu.x);
}

/*
 * @routine 093D:1F5 CountdownNextState093D
 * @run 71:C1F5
 * WHAT IT DOES: Counts OBJ_PIECE_NEXT out and goes on.
 * HOW: A countdown.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountdownNextState093D(uint8_t slot)
{
    abi_nz(DEC8(OBJ->work_d[slot]));
    if (EQ) {
        abi_nz(INC8(OBJ->state[slot]));
    }
}

void abi_CountdownNextState093D(void)
{
    CountdownNextState093D(cpu.x);
}

/*
 * @routine 093D:248 ChangeToType9B_093D
 * @run 71:C248
 * WHAT IT DOES: Becomes kind $9B.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ChangeToType9B_093D(uint8_t slot)
{
    lda(0x9B);
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
}

void abi_ChangeToType9B_093D(void)
{
    ChangeToType9B_093D(cpu.x);
}

/*
 * @routine 093E:017 GrabWhenStruckOrIdle093E
 * @run 71:C817
 * WHAT IT DOES: With five hits, or in a state the table at $C842 marks on the first screen: once an object to grab is near
 * ($C84C), goes to state 8.
 * HOW: FindNearby093E.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void GrabWhenStruckOrIdle093E(uint8_t slot)
{
    lda(RD8(OBJ->hit_taken[slot]));
    cmp(cpu.a, 0x05);
    if (CC) {
        ldy(RD8(OBJ->state[slot]));
        lda(rd8((uint16_t)(0xC842 + cpu.y)));
        if (EQ) {
            return;
        }
        lda(RD8(OBJ->x_hi[slot]));
        ora_a(RD8(OBJ->y_hi[slot]));
        if (NE) {
            return;
        }
    }
    call_routine(0xC831, 0xC84C);
    if (CC) {
        return;
    }
    slot = cpu.x;
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x08);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_GrabWhenStruckOrIdle093E(void)
{
    GrabWhenStruckOrIdle093E(cpu.x);
}

/*
 * @routine 093E:04C FindNearby093E
 * @run 71:C84C
 * WHAT IT DOES: Looks for an on-screen object in slots 1-15 within $3C across (from its hit box centre) and $30 down; keeps its
 * slot in OBJ_WORK_L and returns carry set, else clear.
 * HOW: A walk of the slots with absolute differences.
 * WAITS: no.
 * Entry: X object. Exit: C found.
 */
void FindNearby093E(uint8_t slot)
{
    ldy(0x01);
    do {
        lda(RD8(OBJ->kind[cpu.y]));
        if (NE) {
            lda(RD8(OBJ->x_hi[cpu.y]));
            ora_a(RD8(OBJ->y_hi[cpu.y]));
            if (EQ) {
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
                sec();
                sbc_a(RD8(OBJ->x_lo[slot]));
                if (CC) {
                    eor_a(0xFF);
                    inc_a();
                }
                cmp(cpu.a, 0x3C);
                if (CC) {
                    lda(RD8(OBJ->y_lo[cpu.y]));
                    sec();
                    sbc_a(RD8(OBJ->y_lo[slot]));
                    if (CC) {
                        eor_a(0xFF);
                        inc_a();
                    }
                    cmp(cpu.a, 0x30);
                    if (CC) {
                        tya();
                        WR8(OBJ->work_l[slot], cpu.a);
                        sec();
                        return;
                    }
                }
            }
        }
        iny();
        cmp(cpu.y, 0x10);
    } while (CC);
    clc();
}

void abi_FindNearby093E(void)
{
    FindNearby093E(cpu.x);
}

/*
 * @routine 093E:09C State9WhenHit093E
 * @run 71:C89C
 * WHAT IT DOES: Under state 9, a hit goes to state 9 without a hit box.
 * HOW: Two tests.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void State9WhenHit093E(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x09);
    if (CS) {
        return;
    }
    lda(RD8(OBJ->hit_status[slot]));
    and_a(0x80);
    if (EQ) {
        return;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x09);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_State9WhenHit093E(void)
{
    State9WhenHit093E(cpu.x);
}

/*
 * @routine 093E:0BC SetHitBoxByFrame093E
 * @run 71:C8BC
 * WHAT IT DOES: From frame $59 the hit box the frame's row names (offsets and half size); below it no size.
 * HOW: A row index table.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SetHitBoxByFrame093E(uint8_t slot)
{
    lda(RD8(OBJ->frame[slot]));
    sec();
    sbc_a(0x59);
    if (CC) {
        WR8(OBJ->hitbox_half_w[slot], 0);
        WR8(OBJ->hitbox_half_h[slot], 0);
        return;
    }
    tay();
    lda(rd8((uint16_t)(0xCF4A + cpu.y)));
    asl_a();
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xCF61 + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(rd8((uint16_t)(0xCF62 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    lda(rd8((uint16_t)(0xCF63 + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(rd8((uint16_t)(0xCF64 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_SetHitBoxByFrame093E(void)
{
    SetHitBoxByFrame093E(cpu.x);
}
