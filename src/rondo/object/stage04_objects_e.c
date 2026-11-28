/*
 * Stage 04's objects, runs C552-C828.
 */
#include "stage_obj_ops.h"
#include "stage04_objects_e.h"
#include "calls_b.h"

#include "stage02_objects_b.h"

/*
 * @routine 07DD:552 SparkOnFirstScreen07DD
 * @run 71:C552
 * WHAT IT DOES: Clears the object off the first screen or without an argument, and when the list at $C573 ends.
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SparkOnFirstScreen07DD(uint8_t slot)
{
    lda(RD8(OBJ->x_hi[slot]));
    if (NE) {
        k_clear();
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    if (EQ) {
        k_clear();
        return;
    }
    list_to_temp10(0xC573);
    JSR(0xC56D, k_step_frames());
    if (EQ) {
        k_clear();
    }
}

void abi_SparkOnFirstScreen07DD(void)
{
    SparkOnFirstScreen07DD(cpu.x);
}

/*
 * @routine 07DD:586 EmitAfterWait07DD
 * @run 71:C586
 * WHAT IT DOES: State 0 loads the record with a wait of $40. When the 16-bit wait is out, starts a kind $31 object (number 0 in a
 * free slot, others in an empty one of $43-$4D) and picks the next wait.
 * HOW: A 16-bit countdown.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void EmitAfterWait07DD(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xC58E, k_load_record());
        lda(0x40);
        WR8(OBJ->work_e[cpu.x], cpu.a);
        return;
    }
    lda(RD8(OBJ->work_e[slot]));
    ora_a(RD8(OBJ->work_d[slot]));
    if (NE) {
        lda(RD8(OBJ->work_e[slot]));
        sec();
        sbc_a(0x01);
        WR8(OBJ->work_e[slot], cpu.a);
        lda(RD8(OBJ->work_d[slot]));
        sbc_a(0x00);
        WR8(OBJ->work_d[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    if (NE) {
        ldy(0x43);
        for (;;) {
            lda(RD8(OBJ->kind[cpu.y]));
            if (EQ) {
                break;
            }
            iny();
            cmp(cpu.y, 0x4E);
            if (CS) {
                return;
            }
        }
    } else {
        JSR(0xC5C3, k_free_from30());
        if (CC) {
            return;
        }
    }
    JSR(0xC5C8, k_clear_in_y());
    lda(0x31);
    JSR(0xC5CD, k_start_at_place());
    slot = cpu.x;
    lda(RD8(OBJ->start_arg[slot]));
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    and_a(0x03);
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xC5E7 + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
    lda(rd8((uint16_t)(0xC5E8 + cpu.y)));
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_EmitAfterWait07DD(void)
{
    EmitAfterWait07DD(cpu.x);
}

/*
 * @routine 07DD:5EB StepThenByState07DDC5EB
 * @run 71:C5EB
 * WHAT IT DOES: Steps the list at $C610 ($C604) and goes to the state's handler.
 * HOW: A table of handlers at $C600.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StepThenByState07DDC5EB(uint8_t slot)
{
    (void)slot;
    call_routine(0xC5EE, 0xC604);
    jump_by_state(cpu.x, 0xC600);
}

void abi_StepThenByState07DDC5EB(void)
{
    StepThenByState07DDC5EB(cpu.x);
}

/*
 * @routine 07DD:604 StepList07DDC610
 * @run 71:C604
 * WHAT IT DOES: Steps the list at $C610.
 * HOW: A tail jump to StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StepList07DDC610(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xC610);
    k_step_frames();
}

void abi_StepList07DDC610(void)
{
    StepList07DDC610(cpu.x);
}

/*
 * @routine 07DD:619 ScrollShortOfRightEnd07DD
 * @run 71:C619
 * WHAT IT DOES: As ScrollShortOfRightEnd051B: carry set when the camera is two screens or more short of the right end, or one
 * short with its low byte under $E0.
 * HOW: The same instructions at this address.
 * WAITS: no.
 * Entry: none. Exit: C the answer, A the difference or ScrollXLo.
 */
void ScrollShortOfRightEnd07DD(uint8_t slot)
{
    ScrollShortOfRightEnd051B(slot);
}

void abi_ScrollShortOfRightEnd07DD(void)
{
    ScrollShortOfRightEnd07DD(cpu.x);
}

/*
 * @routine 07DD:633 EnterBySideOrFloat07DD
 * @run 71:C633
 * WHAT IT DOES: With an argument: placed off the right ($C696), then hidden at a random height ($48-$96) with loop flags $20.
 * Without: placed at the right or the left of the screen by the player's side, as EnterFromEdgeBySide051B.
 * HOW: LoadObjectRecord, two NextRandom, ScrollShortOfRightEnd07DD.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void EnterBySideOrFloat07DD(uint8_t slot)
{
    JSR(0xC636, k_load_record());
    slot = cpu.x;
    lda(RD8(OBJ->start_arg[slot]));
    if (NE) {
        call_routine(0xC63E, 0xC696);
        slot = cpu.x;
        WR8(OBJ->hitbox_on[slot], 0);
        lda(0x80);
        WR8(OBJ->sprite_flip[slot], cpu.a);
        WR8(OBJ->y_hi[slot], 0);
        JSR(0xC64C, k_random());
        and_a(0x3F);
        clc();
        adc_a(0x48);
        WR8(ZP->temp00, cpu.a);
        JSR(0xC656, k_random());
        and_a(0x0F);
        clc();
        adc_a(RD8(ZP->temp00));
        WR8(OBJ->y_lo[cpu.x], cpu.a);
        lda(0x20);
        WR8(OBJ->loop_flags[cpu.x], cpu.a);
        WR8(OBJ->hit_reaction[cpu.x], 0);
        return;
    }
    lda(RD8(OBJ->x_lo[0]));
    cmp(cpu.a, 0x40);
    if (CC) {
        run_routine(0xC696);
        return;
    }
    cmp(cpu.a, 0xC0);
    if (CC) {
        call_routine(0xC674, 0xC619);
        if (CS) {
            lda(RD8(OBJ->facing_left[0]));
            if (EQ) {
                run_routine(0xC696);
                return;
            }
        }
    }
    lda(0x01);
    WR8(OBJ->facing_left[slot], cpu.a);
    lda(0xFF);
    WR8(OBJ->x_hi[slot], cpu.a);
    lda(0xF0);
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(0x01);
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->vx_lo[slot], cpu.a);
    run_routine(0xC6AA);
}

void abi_EnterBySideOrFloat07DD(void)
{
    EnterBySideOrFloat07DD(cpu.x);
}

/*
 * @routine 07DD:696 PlaceRightWalkingLeft07DD
 * @run 71:C696
 * WHAT IT DOES: At X $0110 with X speed $FEC0, then Y from the player's ($C6AA).
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void PlaceRightWalkingLeft07DD(uint8_t slot)
{
    lda(0x01);
    WR8(OBJ->x_hi[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(0xFE);
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0xC0);
    WR8(OBJ->vx_lo[slot], cpu.a);
    run_routine(0xC6AA);
}

void abi_PlaceRightWalkingLeft07DD(void)
{
    PlaceRightWalkingLeft07DD(cpu.x);
}

/*
 * @routine 07DD:6AA PlaceAbovePlayer07DD
 * @run 71:C6AA
 * WHAT IT DOES: Y the player's less $10.
 * HOW: A 16-bit subtraction.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void PlaceAbovePlayer07DD(uint8_t slot)
{
    lda(RD8(OBJ->y_lo[0]));
    sec();
    sbc_a(0x10);
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(RD8(OBJ->y_hi[0]));
    sbc_a(0x00);
    WR8(OBJ->y_hi[slot], cpu.a);
}

void abi_PlaceAbovePlayer07DD(void)
{
    PlaceAbovePlayer07DD(cpu.x);
}

/*
 * @routine 07DD:6BC BobOnCosine07DD
 * @run 71:C6BC
 * WHAT IT DOES: As BobOnCosine051B: Y speed from the cosine of OBJ_WORK_D, magnitude 1.0; moves, and advances the angle by 2.
 * HOW: ScaleByCosine and ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BobOnCosine07DD(uint8_t slot)
{
    lda(0x00);
    WR8(ZP->temp00, cpu.a);
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    JSR(0xC6CA, k_cosine());
    lda(RD8(ZP->plane_rows_left));
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    lda(RD8(ZP->sprite_flip_mask));
    WR8(OBJ->vy_lo[cpu.x], cpu.a);
    JSR(0xC6D7, k_move());
    lda(RD8(OBJ->work_d[cpu.x]));
    clc();
    adc_a(0x02);
    WR8(OBJ->work_d[cpu.x], cpu.a);
}

void abi_BobOnCosine07DD(void)
{
    BobOnCosine07DD(cpu.x);
}

/*
 * @routine 07DD:6E1 HitThenByState07DDC6E1
 * @run 71:C6E1
 * WHAT IT DOES: Breaks the chain on a hit ($C702) and goes to the state's handler.
 * HOW: A table of handlers at $C6F6.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void HitThenByState07DDC6E1(uint8_t slot)
{
    (void)slot;
    call_routine(0xC6E4, 0xC702);
    jump_by_state(cpu.x, 0xC6F6);
}

void abi_HitThenByState07DDC6E1(void)
{
    HitThenByState07DDC6E1(cpu.x);
}

/*
 * @routine 07DD:702 BreakChainOnHit07DD
 * @run 71:C702
 * WHAT IT DOES: A hit outside state 4: the next slot becomes kind $9C, the six after it lose their hit boxes and pace bits and
 * take attack powers from the table; this one goes to state 4 without a hit box or picture.
 * HOW: A loop over slots X+2 to X+7.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BreakChainOnHit07DD(uint8_t slot)
{
    lda(RD8(OBJ->hit_status[slot]));
    and_a(0x80);
    if (EQ) {
        return;
    }
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x04);
    if (EQ) {
        return;
    }
    phx();
    inx();
    lda(0x9C);
    WR8(OBJ->kind[cpu.x], cpu.a);
    WR8(OBJ->state[cpu.x], 0);
    inx();
    cpu.y = 0;
    do {
        lda(rd8((uint16_t)(0xC740 + cpu.y)));
        WR8(OBJ->work_n[cpu.x], cpu.a);
        WR8(OBJ->hitbox_on[cpu.x], 0);
        lda(RD8(OBJ->pace[cpu.x]));
        and_a(0x87);
        WR8(OBJ->pace[cpu.x], cpu.a);
        inx();
        iny();
        cmp(cpu.y, 0x06);
    } while (CC);
    plx();
    WR8(OBJ->hitbox_on[cpu.x], 0);
    WR8(OBJ->frame[cpu.x], 0);
    lda(0x04);
    WR8(OBJ->state[cpu.x], cpu.a);
}

void abi_BreakChainOnHit07DD(void)
{
    BreakChainOnHit07DD(cpu.x);
}

/*
 * @routine 07DD:746 LoadWithRider07DD
 * @run 71:C746
 * WHAT IT DOES: Loads the record with a wait of $60 and starts a kind $34 object in the next slot; argument bit 7 goes to state 5,
 * and $FF to substate 2.
 * HOW: ClearObjectInY and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LoadWithRider07DD(uint8_t slot)
{
    JSR(0xC749, k_load_record());
    lda(0x60);
    WR8(OBJ->work_e[cpu.x], cpu.a);
    txa();
    tay();
    iny();
    JSR(0xC754, k_clear_in_y());
    lda(0x34);
    JSR(0xC759, k_start_at_place());
    slot = cpu.x;
    lda(RD8(OBJ->start_arg[slot]));
    if (PL) {
        return;
    }
    lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    cmp(cpu.a, 0xFF);
    if (NE) {
        return;
    }
    lda(0x02);
    WR8(OBJ->substate[slot], cpu.a);
}

void abi_LoadWithRider07DD(void)
{
    LoadWithRider07DD(cpu.x);
}

/*
 * @routine 07DD:770 BySubstate07DDC770
 * @run 71:C770
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $C782.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate07DDC770(uint8_t slot)
{
    jump_by_substate(slot, 0xC782);
}

void abi_BySubstate07DDC770(void)
{
    BySubstate07DDC770(cpu.x);
}

/*
 * @routine 07DD:788 StopAtPlaceOfRider07DD
 * @run 71:C788
 * WHAT IT DOES: On the first screen, once left of the X the table gives for slot Y's argument, slows to $0020 and goes on.
 * HOW: Y is the caller's; the argument's low seven bits index the table.
 * WAITS: no.
 * Entry: X object, Y a slot. Exit: as the last instruction left them.
 */
void StopAtPlaceOfRider07DD(uint8_t slot)
{
    lda(RD8(OBJ->x_hi[slot]));
    if (NE) {
        return;
    }
    lda(RD8(OBJ->start_arg[cpu.y]));
    and_a(0x7F);
    tay();
    lda(RD8(OBJ->x_lo[slot]));
    cmp(cpu.a, rd8((uint16_t)(0xC7A9 + cpu.y)));
    if (CS) {
        return;
    }
    lda(0x00);
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0x20);
    WR8(OBJ->vx_lo[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_StopAtPlaceOfRider07DD(void)
{
    StopAtPlaceOfRider07DD(cpu.x);
}

/*
 * @routine 07DD:7AD BounceToRest07DD
 * @run 71:C7AD
 * WHAT IT DOES: Falls; landing at Y $B8 on the first screen, bounces up to three times by the table, then goes to state 2.
 * HOW: ApplyGravity and ApplyVelocity; OBJ_WORK_J counts the bounces.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BounceToRest07DD(uint8_t slot)
{
    JSR(0xC7B0, k_gravity());
    JSR(0xC7B3, k_move());
    slot = cpu.x;
    lda(RD8(OBJ->y_hi[slot]));
    if (NE) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xB8);
    if (CC) {
        return;
    }
    lda(0xB8);
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(RD8(OBJ->work_j[slot]));
    cmp(cpu.a, 0x03);
    if (CS) {
        run_routine(0xC7DD);
        return;
    }
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xC7E3 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xC7E4 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    abi_nz(INC8(OBJ->work_j[slot]));
}

void abi_BounceToRest07DD(void)
{
    BounceToRest07DD(cpu.x);
}

/*
 * @routine 07DD:7DD StartState2_07DD
 * @run 71:C7DD
 * WHAT IT DOES: Goes to state 2.
 * HOW: A store.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StartState2_07DD(uint8_t slot)
{
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_StartState2_07DD(void)
{
    StartState2_07DD(cpu.x);
}

/*
 * @routine 07DD:7EB DriftAwayWhenFar07DD
 * @run 71:C7EB
 * WHAT IT DOES: Within $30 of the player, state 2; else drifts left at $FFC0 and bobs ($C7FF).
 * HOW: DistanceXToPlayer, ApplyVelocityX.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DriftAwayWhenFar07DD(uint8_t slot)
{
    (void)slot;
    JSR(0xC7EE, k_distance_x());
    cmp(cpu.a, 0x30);
    if (CC) {
        run_routine(0xC7DD);
        return;
    }
    lda(0xFF);
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    lda(0xC0);
    WR8(OBJ->vx_lo[cpu.x], cpu.a);
    JSR(0xC7FF, k_move_x());
    run_routine(0xC7FF);
}

void abi_DriftAwayWhenFar07DD(void)
{
    DriftAwayWhenFar07DD(cpu.x);
}

/*
 * @routine 07DD:7FF BobSlowlyOnCosine07DD
 * @run 71:C7FF
 * WHAT IT DOES: Y speed from the cosine of OBJ_WORK_D at $20, moves down it and advances the angle by 2; each turn resets Y to $CE.
 * HOW: ScaleByCosine and ApplyVelocityY.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BobSlowlyOnCosine07DD(uint8_t slot)
{
    lda(0x20);
    WR8(ZP->temp00, cpu.a);
    WR8(ZP->temp01, 0);
    lda(RD8(OBJ->work_d[slot]));
    JSR(0xC80B, k_cosine());
    slot = cpu.x;
    lda(RD8(ZP->sprite_flip_mask));
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(ZP->plane_rows_left));
    WR8(OBJ->vy_hi[slot], cpu.a);
    JSR(0xC818, k_move_y());
    slot = cpu.x;
    lda(RD8(OBJ->work_d[slot]));
    clc();
    adc_a(0x02);
    WR8(OBJ->work_d[slot], cpu.a);
    if (CC) {
        return;
    }
    lda(0xCE);
    WR8(OBJ->y_lo[slot], cpu.a);
}

void abi_BobSlowlyOnCosine07DD(void)
{
    BobSlowlyOnCosine07DD(cpu.x);
}
