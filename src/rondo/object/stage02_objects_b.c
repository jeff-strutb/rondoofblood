/*
 * Stage 02's objects, runs BA29-BDE4.
 */
#include "stage_obj_ops.h"
#include "stage02_objects_b.h"
#include "calls_b.h"

/*
 * @routine 051C:229 SoundThenNextSubstate051B
 * @run 70:BA29
 * WHAT IT DOES: Sound $1F when on the first screen, then the next substate.
 * HOW: SoundIfOnFirstScreen.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SoundThenNextSubstate051B(uint8_t slot)
{
    (void)slot;
    lda(0x1F);
    JSR(0xBA2E, k_sound_first_screen());
    abi_nz(INC8(OBJ->substate[cpu.x]));
}

void abi_SoundThenNextSubstate051B(void)
{
    SoundThenNextSubstate051B(cpu.x);
}

/*
 * @routine 051C:232 SoundAtStepNine051B
 * @run 70:BA32
 * WHAT IT DOES: Steps the list at $BA59; at its end goes to state 1; at step 9 sounds $1A.
 * HOW: StepObjectFrames and a test of OBJ_ANIM_STEP.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SoundAtStepNine051B(uint8_t slot)
{
    list_to_temp10(0xBA59);
    JSR(0xBA3D, k_step_frames());
    if (EQ) {
        WR8(OBJ->piece_place[slot], 0);
        lda(0x01);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
        return;
    }
    if (CC) {
        return;
    }
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x09);
    if (NE) {
        return;
    }
    lda(0x1A);
    k_sound();
}

void abi_SoundAtStepNine051B(void)
{
    SoundAtStepNine051B(cpu.x);
}

/*
 * @routine 051C:27A BySubstate051BBA7A
 * @run 70:BA7A
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $BA8C.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate051BBA7A(uint8_t slot)
{
    jump_by_substate(slot, 0xBA8C);
}

void abi_BySubstate051BBA7A(void)
{
    BySubstate051BBA7A(cpu.x);
}

/*
 * @routine 051C:290 StartFortyCountAndCountNext051B
 * @run 70:BA90
 * WHAT IT DOES: No hit box, the animation from its start, a count of $40, the state of the slot two on counted up, next substate.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StartFortyCountAndCountNext051B(uint8_t slot)
{
    unsigned two = slot + 2u;

    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x40);
    WR8(OBJ->work_d[slot], cpu.a);
    abi_nz(INC8(OBJ->state[two]));
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_StartFortyCountAndCountNext051B(void)
{
    StartFortyCountAndCountNext051B(cpu.x);
}

/*
 * @routine 051C:2A5 SinkWhileCounting051B
 * @run 70:BAA5
 * WHAT IT DOES: Counts OBJ_WORK_D out and clears the object at zero. Until then draws it with attribute bit 4, moves it down, adds
 * 2 to its Y speed, moves it again and drops water drops ($BAD1).
 * HOW: ApplyVelocityY twice around the speed's increase.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SinkWhileCounting051B(uint8_t slot)
{
    abi_nz(DEC8(OBJ->work_d[slot]));
    if (EQ) {
        JSR(0xBAAD, k_clear());
        return;
    }
    lda(RD8(OBJ->sprite_attr[slot]));
    ora_a(0x10);
    WR8(OBJ->sprite_attr[slot], cpu.a);
    JSR(0xBAB9, k_move_y());
    lda(RD8(OBJ->vy_lo[slot]));
    clc();
    adc_a(0x02);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    adc_a(0x00);
    WR8(OBJ->vy_hi[slot], cpu.a);
    JSR(0xBACD, k_move_y());
    call_routine(0xBAD0, 0xBAD1);
}

void abi_SinkWhileCounting051B(void)
{
    SinkWhileCounting051B(cpu.x);
}

/*
 * @routine 051C:2D1 DripByWaveCounter051B
 * @run 70:BAD1
 * WHAT IT DOES: When the wave counter's low six bits match one of the eight values at $BB11, starts a kind $9E object in a free slot
 * from $30 with the speeds of that value's row, sound $15 on the first screen, and moves it one step.
 * HOW: A search of the table; X kept in zClimbStepBefore and the row in zBurstCount.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DripByWaveCounter051B(uint8_t slot)
{
    (void)slot;
    cpu.y = 0;
    for (;;) {
        lda(RD8(ZP->wave_counter));
        and_a(0x3F);
        cmp(cpu.a, rd8((uint16_t)(0xBB11 + cpu.y)));
        if (EQ) {
            break;
        }
        iny();
        cmp(cpu.y, 0x08);
        if (CS) {
            return;
        }
    }
    WR8(ZP->climb_step_before, cpu.x);
    WR8(ZP->burst_count, cpu.y);
    JSR(0xBAE8, k_free_from30());
    if (CC) {
        return;
    }
    lda(RD8(OBJ->x_hi[cpu.x]));
    if (EQ) {
        lda(0x15);
        JSR(0xBAF4, k_sound_first_screen());
    }
    lda(0x9E);
    JSR(0xBAF9, k_start_at_place());
    lda(RD8(ZP->burst_count));
    asl_a();
    tax();
    lda(rd8((uint16_t)(0xBB19 + cpu.x)));
    WR8(OBJ->vx_hi[cpu.y], cpu.a);
    lda(rd8((uint16_t)(0xBB1A + cpu.x)));
    WR8(OBJ->vy_hi[cpu.y], cpu.a);
    sxy();
    JSR(0xBB0D, k_move());
    sxy();
    ldx(RD8(ZP->climb_step_before));
}

void abi_DripByWaveCounter051B(void)
{
    DripByWaveCounter051B(cpu.x);
}

/*
 * @routine 051C:329 ByState051BBB29
 * @run 70:BB29
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $BB3B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState051BBB29(uint8_t slot)
{
    jump_by_state(slot, 0xBB3B);
}

void abi_ByState051BBB29(void)
{
    ByState051BBB29(cpu.x);
}

/*
 * @routine 051C:341 LoadWithBoxAndCount051BBB41
 * @run 70:BB41
 * WHAT IT DOES: Loads the record; a count of $40 and the hit box offset -4, $20.
 * HOW: LoadObjectRecord and stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LoadWithBoxAndCount051BBB41(uint8_t slot)
{
    (void)slot;
    JSR(0xBB44, k_load_record());
    lda(0x40);
    WR8(OBJ->work_d[cpu.x], cpu.a);
    lda(0xFC);
    WR8(OBJ->hitbox_dx[cpu.x], cpu.a);
    lda(0x20);
    WR8(OBJ->hitbox_dy[cpu.x], cpu.a);
}

void abi_LoadWithBoxAndCount051BBB41(void)
{
    LoadWithBoxAndCount051BBB41(cpu.x);
}

/*
 * @routine 051C:354 FollowTwoBelowAndStep051B
 * @run 70:BB54
 * WHAT IT DOES: Takes the place of the object two slots down and steps the list at $BB67.
 * HOW: CopyObjectPlaceBack and StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FollowTwoBelowAndStep051B(uint8_t slot)
{
    (void)slot;
    txa();
    tay();
    dey();
    dey();
    JSR(0xBB5B, k_copy_place_back());
    list_to_temp10(0xBB67);
    JSR(0xBB66, k_step_frames());
}

void abi_FollowTwoBelowAndStep051B(void)
{
    FollowTwoBelowAndStep051B(cpu.x);
}

/*
 * @routine 051C:378 RiseAndExpire051B
 * @run 70:BB78
 * WHAT IT DOES: No hit box; moves by $BB91, draws with attribute bit 4 and clears the object when its count runs out.
 * HOW: A call and a countdown of OBJ_WORK_D.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void RiseAndExpire051B(uint8_t slot)
{
    WR8(OBJ->hitbox_on[slot], 0);
    call_routine(0xBB7E, 0xBB91);
    slot = cpu.x;
    lda(RD8(OBJ->sprite_attr[slot]));
    ora_a(0x10);
    WR8(OBJ->sprite_attr[slot], cpu.a);
    abi_nz(DEC8(OBJ->work_d[slot]));
    if (EQ) {
        k_clear();
    }
}

void abi_RiseAndExpire051B(void)
{
    RiseAndExpire051B(cpu.x);
}

/*
 * @routine 051C:391 WaterLevelByDistance051B
 * @run 70:BB91
 * WHAT IT DOES: Sets the water level from the table at $BBB1, by the player's distance and the count in OBJ_WORK_J.
 * HOW: A word table indexed by distance / 4 in steps of 16, plus the count's low bits doubled.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WaterLevelByDistance051B(uint8_t slot)
{
    lda(RD8(OBJ->work_j[slot]));
    and_a(0x07);
    asl_a();
    WR8(ZP->temp00, cpu.a);
    JSR(0xBB9C, k_distance_x());
    lsr_a();
    lsr_a();
    and_a(0xF0);
    clc();
    adc_a(RD8(ZP->temp00));
    tay();
    lda(rd8((uint16_t)(0xBBB1 + cpu.y)));
    WR8(LEVELW->y.lo, cpu.a);
    lda(rd8((uint16_t)(0xBBB2 + cpu.y)));
    WR8(LEVELW->y.hi, cpu.a);
}

void abi_WaterLevelByDistance051B(void)
{
    WaterLevelByDistance051B(cpu.x);
}

/*
 * @routine 051C:3F1 ByState051BBBF1
 * @run 70:BBF1
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $BC03.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState051BBBF1(uint8_t slot)
{
    jump_by_state(slot, 0xBC03);
}

void abi_ByState051BBBF1(void)
{
    ByState051BBBF1(cpu.x);
}

/*
 * @routine 051C:409 LaunchByStartArgAngle051B
 * @run 70:BC09
 * WHAT IT DOES: Loads the record, moves one step by speed high bytes $F4, $DA, then sets the velocity from the angle the table at
 * $BC2B holds for OBJ_START_ARG, magnitude $0200.
 * HOW: ApplyVelocity and SetVelocityFromAngle.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LaunchByStartArgAngle051B(uint8_t slot)
{
    (void)slot;
    JSR(0xBC0C, k_load_record());
    lda(0xF4);
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    lda(0xDA);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    JSR(0xBC19, k_move());
    lda(0x00);
    WR8(ZP->temp00, cpu.a);
    lda(0x02);
    WR8(ZP->temp01, cpu.a);
    ldy(RD8(OBJ->start_arg[cpu.x]));
    lda(rd8((uint16_t)(0xBC2B + cpu.y)));
    JSR(0xBC2A, k_velocity_from_angle());
}

void abi_LaunchByStartArgAngle051B(void)
{
    LaunchByStartArgAngle051B(cpu.x);
}

/*
 * @routine 051C:42E FlyUntilTerrain051B
 * @run 70:BC2E
 * WHAT IT DOES: Steps the list at $BC6C and moves; on terrain stops the sideways move, rises at -1 and goes to the next state as
 * sprite set 3.
 * HOW: ProbeTerrain request $2B with SpriteFlipMask 8.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlyUntilTerrain051B(uint8_t slot)
{
    list_to_temp10(0xBC6C);
    JSR(0xBC39, k_step_frames());
    JSR(0xBC3C, k_move());
    lda(0x08);
    WR8(ZP->sprite_flip_mask, cpu.a);
    lda(0x2B);
    JSR(0xBC45, k_probe());
    and_a(0x03);
    if (EQ) {
        return;
    }
    slot = cpu.x;
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    lda(0xFF);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->vy_lo[slot], 0);
    lda(0x03);
    WR8(OBJ->sprite_set[slot], cpu.a);
    WR8(OBJ->frame[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_FlyUntilTerrain051B(void)
{
    FlyUntilTerrain051B(cpu.x);
}

/*
 * @routine 051C:475 StepThenClear051BBC75
 * @run 70:BC75
 * WHAT IT DOES: Steps the list at $BC86 and clears the object when it ends.
 * HOW: StepObjectFrames, ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StepThenClear051BBC75(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xBC86);
    JSR(0xBC80, k_step_frames());
    if (EQ) {
        k_clear();
    }
}

void abi_StepThenClear051BBC75(void)
{
    StepThenClear051BBC75(cpu.x);
}

/*
 * @routine 051C:495 SpawnWhileSlot16Absent051B
 * @run 70:BC95
 * WHAT IT DOES: State 0 loads the record. Then, unless the object in slot $10 is kind $24, counts the 16-bit wait in
 * OBJ_PIECE_NEXT:PIECE_PLACE down and at zero starts a kind $46 object in a free slot from $30, waiting $C0 again.
 * HOW: A 16-bit decrement and FindFreeSlotFrom30.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SpawnWhileSlot16Absent051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        k_load_record();
        return;
    }
    lda(RD8(OBJ->kind[0x10]));
    cmp(cpu.a, 0x24);
    if (EQ) {
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
    JSR(0xBCC1, k_free_from30());
    if (CC) {
        return;
    }
    lda(0x46);
    JSR(0xBCC8, k_start_at_place());
    WR8(OBJ->work_d[cpu.x], 0);
    lda(0xC0);
    WR8(OBJ->work_e[cpu.x], cpu.a);
}

void abi_SpawnWhileSlot16Absent051B(void)
{
    SpawnWhileSlot16Absent051B(cpu.x);
}

/*
 * @routine 051C:4D9 StepThenByState051BBCD9
 * @run 70:BCD9
 * WHAT IT DOES: Steps the list at $BCFE ($BCF2) and goes to the state's handler.
 * HOW: A table of handlers at $BCEE.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StepThenByState051BBCD9(uint8_t slot)
{
    (void)slot;
    call_routine(0xBCDC, 0xBCF2);
    jump_by_state(cpu.x, 0xBCEE);
}

void abi_StepThenByState051BBCD9(void)
{
    StepThenByState051BBCD9(cpu.x);
}

/*
 * @routine 051C:4F2 StepListBCFE051B
 * @run 70:BCF2
 * WHAT IT DOES: Steps the list at $BCFE.
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StepListBCFE051B(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xBCFE);
    JSR(0xBCFD, k_step_frames());
}

void abi_StepListBCFE051B(void)
{
    StepListBCFE051B(cpu.x);
}

/*
 * @routine 051C:507 ScrollShortOfRightEnd051B
 * @run 70:BD07
 * WHAT IT DOES: Carry set when the camera is two screens or more short of the right end, or one short with its low byte under ;
 * clear otherwise.
 * HOW: ScrollXMaxHi less ScrollXHiScreen, and ScrollXLo.
 * WAITS: no.
 * Entry: none. Exit: C the answer, A the difference or ScrollXLo.
 */
void ScrollShortOfRightEnd051B(uint8_t slot)
{
    (void)slot;
    lda(RD8(SCROLLLIM->x_max.hi));
    sec();
    sbc_a(RD8(SCROLLE->x_hi_screen[0]));
    cmp(cpu.a, 0x02);
    if (CS) {
        return;
    }
    cmp(cpu.a, 0x00);
    if (NE) {
        lda(RD8(SCROLL->x_lo[0]));
        cmp(cpu.a, 0xE0);
        if (CC) {
            sec();
            return;
        }
    }
    clc();
}

void abi_ScrollShortOfRightEnd051B(void)
{
    ScrollShortOfRightEnd051B(cpu.x);
}

/*
 * @routine 051C:521 EnterFromEdgeBySide051B
 * @run 70:BD21
 * WHAT IT DOES: Loads the record and places the object at X $FFE0 with X speed $0140 and OBJ_FACING_LEFT 1, or at X $0120 with
 * speed $FEC0: the former when the player's X low is $C0 or more, or between $40 and $C0 with the camera near its
 * right end or the player facing left. Y is the player's less $10.
 * HOW: The player's X low and facing; ScrollShortOfRightEnd051B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void EnterFromEdgeBySide051B(uint8_t slot)
{
    bool from_left = false;

    JSR(0xBD24, k_load_record());
    slot = cpu.x;
    lda(RD8(OBJ->x_lo[0]));
    cmp(cpu.a, 0x40);
    if (CS) {
        cmp(cpu.a, 0xC0);
        if (CS) {
            from_left = true;
        } else {
            call_routine(0xBD31, 0xBD07);
            if (CC) {
                from_left = true;
            } else {
                lda(RD8(OBJ->facing_left[0]));
                from_left = NE;
            }
        }
    }
    if (from_left) {
        lda(0x01);
        WR8(OBJ->facing_left[slot], cpu.a);
        lda(0xFF);
        WR8(OBJ->x_hi[slot], cpu.a);
        lda(0xE0);
        WR8(OBJ->x_lo[slot], cpu.a);
        lda(0x01);
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(0x40);
        WR8(OBJ->vx_lo[slot], cpu.a);
    } else {
        lda(0x01);
        WR8(OBJ->x_hi[slot], cpu.a);
        lda(0x20);
        WR8(OBJ->x_lo[slot], cpu.a);
        lda(0xFE);
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(0xC0);
        WR8(OBJ->vx_lo[slot], cpu.a);
    }
    lda(RD8(OBJ->y_lo[0]));
    sec();
    sbc_a(0x10);
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(RD8(OBJ->y_hi[0]));
    sbc_a(0x00);
    WR8(OBJ->y_hi[slot], cpu.a);
}

void abi_EnterFromEdgeBySide051B(void)
{
    EnterFromEdgeBySide051B(cpu.x);
}

/*
 * @routine 051C:579 BobOnCosine051B
 * @run 70:BD79
 * WHAT IT DOES: Y speed from the cosine of OBJ_WORK_D, magnitude 1.0; moves, and advances the angle by 2.
 * HOW: ScaleByCosine and ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BobOnCosine051B(uint8_t slot)
{
    lda(0x00);
    WR8(ZP->temp00, cpu.a);
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    JSR(0xBD87, k_cosine());
    lda(RD8(ZP->plane_rows_left));
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    lda(RD8(ZP->sprite_flip_mask));
    WR8(OBJ->vy_lo[cpu.x], cpu.a);
    JSR(0xBD94, k_move());
    lda(RD8(OBJ->work_d[cpu.x]));
    clc();
    adc_a(0x02);
    WR8(OBJ->work_d[cpu.x], cpu.a);
}

void abi_BobOnCosine051B(void)
{
    BobOnCosine051B(cpu.x);
}

/*
 * @routine 051C:59E CountHitThenByState051BBD9E
 * @run 70:BD9E
 * WHAT IT DOES: Counts OBJ_WORK_D up, checks for a hit ($BDCE) and goes to the state's handler.
 * HOW: A table of handlers at $BDB6.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountHitThenByState051BBD9E(uint8_t slot)
{
    abi_nz(INC8(OBJ->work_d[slot]));
    call_routine(0xBDA4, 0xBDCE);
    jump_by_state(cpu.x, 0xBDB6);
}

void abi_CountHitThenByState051BBD9E(void)
{
    CountHitThenByState051BBD9E(cpu.x);
}

/*
 * @routine 051C:5CE State11WhenHit051BBDCE
 * @run 70:BDCE
 * WHAT IT DOES: Under state $0B, a hit (OBJ_HIT_STATUS bit 7) goes to state $0B, substate 0.
 * HOW: Two tests.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void State11WhenHit051BBDCE(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x0B);
    if (CS) {
        return;
    }
    lda(RD8(OBJ->hit_status[slot]));
    and_a(0x80);
    if (EQ) {
        return;
    }
    lda(0x0B);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_State11WhenHit051BBDCE(void)
{
    State11WhenHit051BBDCE(cpu.x);
}
