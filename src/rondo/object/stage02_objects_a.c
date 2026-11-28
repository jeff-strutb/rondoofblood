/*
 * Stage 02's objects, runs B000-BAFF.
 */
#include "stage_obj_ops.h"
#include "stage02_objects_a.h"
#include "calls_b.h"

/*
 * @routine 051B:195 BounceSpeeds051B
 * @run 70:B195
 * @stop B1C3
 * WHAT IT DOES: In state 0 loads the record, sets sprite set 4 and picks one of eight speed pairs at random; else goes on at $B203.
 * HOW: A table row of four bytes by NextRandom's low three bits.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BounceSpeeds051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (NE) {
        run_routine(0xB203);
        return;
    }
    JSR(0xB19D, k_load_record());
    lda(0x04);
    WR8(OBJ->sprite_set[slot], cpu.a);
    JSR(0xB1A5, k_random());
    and_a(0x07);
    asl_a();
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xB1C4 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xB1C3 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xB1C6 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xB1C5 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
}

void abi_BounceSpeeds051B(void)
{
    BounceSpeeds051B(cpu.x);
}

/*
 * @routine 051B:203 StepListThenFallOrLeave051B
 * @run 70:B203
 * WHAT IT DOES: Steps the list at $B21E; on the first screen below its top (Y high 0 or negative) falls under gravity, else the slot is cleared.
 * HOW: StepObjectFrames, then ApplyGravityUnlimited and ApplyVelocity, or ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StepListThenFallOrLeave051B(uint8_t slot)
{
    list_to_temp10(0xB21E);
    JSR(0xB20E, k_step_frames());
    lda(RD8(OBJ->y_hi[slot]));
    if (NE && PL) {
        k_clear();
        return;
    }
    JSR(0xB21B, k_gravity_unlimited());
    k_move();
}

void abi_StepListThenFallOrLeave051B(void)
{
    StepListThenFallOrLeave051B(cpu.x);
}

/*
 * @routine 051B:2E7 CountCheckThenByState051B
 * @run 70:B2E7
 * WHAT IT DOES: Counts OBJ_WORK_F up, runs the hit check at $B30D and goes to the state's handler.
 * HOW: A table of handlers at $B2FF.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountCheckThenByState051B(uint8_t slot)
{
    abi_nz(INC8(OBJ->swoop_speed_lo[slot]));
    call_routine(0xB2ED, 0xB30D);
    jump_by_state(cpu.x, 0xB2FF);
}

void abi_CountCheckThenByState051B(void)
{
    CountCheckThenByState051B(cpu.x);
}

/*
 * @routine 051B:475 TurnListThenState2_051B
 * @run 70:B475
 * WHAT IT DOES: Runs $B4B0 for the hit box, steps the list picked by the swoop speed and facing, and when it ends sets the box and state 2.
 * HOW: A table of list addresses at $B498.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void TurnListThenState2_051B(uint8_t slot)
{
    call_routine(0xB478, 0xB4B0);
    slot = cpu.x;
    lda(RD8(OBJ->swoop_speed_hi[slot]));
    eor_a(RD8(OBJ->facing_left[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xB498 + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xB499 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    JSR(0xB48D, k_step_frames());
    if (NE) {
        return;
    }
    call_routine(0xB492, 0xB4BF);
    lda(0x02);
    WR8(OBJ->state[cpu.x], cpu.a);
}

void abi_TurnListThenState2_051B(void)
{
    TurnListThenState2_051B(cpu.x);
}

/*
 * @routine 051B:4B0 SetHitBoxByFrame051B
 * @run 70:B4B0
 * WHAT IT DOES: Sets the hit box by the frame: $0A and $0B stretched ($B4CD), $0D crouched ($B4E2), else the standing box ($B4BF).
 * HOW: Compares, then goes to the arm.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SetHitBoxByFrame051B(uint8_t slot)
{
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x0A);
    if (EQ) {
        run_routine(0xB4CD);
        return;
    }
    cmp(cpu.a, 0x0B);
    if (EQ) {
        run_routine(0xB4CD);
        return;
    }
    cmp(cpu.a, 0x0D);
    if (EQ) {
        run_routine(0xB4E2);
        return;
    }
    run_routine(0xB4BF);
}

void abi_SetHitBoxByFrame051B(void)
{
    SetHitBoxByFrame051B(cpu.x);
}

/*
 * @routine 051B:4CD SetHitBoxStretched051B
 * @run 70:B4CD
 * WHAT IT DOES: The stretched hit box: offset -8, 4; half size $14 by $14.
 * HOW: Four stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SetHitBoxStretched051B(uint8_t slot)
{
    lda(0xF8);
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(0x04);
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    lda(0x14);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(0x14);
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_SetHitBoxStretched051B(void)
{
    SetHitBoxStretched051B(cpu.x);
}

/*
 * @routine 051B:4E2 SetHitBoxCrouched051B
 * @run 70:B4E2
 * WHAT IT DOES: The crouched hit box: offset 9, 2; half size $0C by $17.
 * HOW: Four stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SetHitBoxCrouched051B(uint8_t slot)
{
    lda(0x09);
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(0x02);
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    lda(0x0C);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(0x17);
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_SetHitBoxCrouched051B(void)
{
    SetHitBoxCrouched051B(cpu.x);
}

/*
 * @routine 051B:4F7 LandThenBySubstate051B
 * @run 70:B4F7
 * WHAT IT DOES: Asks StopOnLanding with height 4, then goes to the substate's handler.
 * HOW: A table of handlers at $B50E.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LandThenBySubstate051B(uint8_t slot)
{
    (void)slot;
    lda(0x04);
    JSR(0xB4FC, k_stop_on_landing());
    jump_by_substate(cpu.x, 0xB50E);
}

void abi_LandThenBySubstate051B(void)
{
    LandThenBySubstate051B(cpu.x);
}

/*
 * @routine 051B:52F ThrowOnAnimation051B
 * @run 70:B52F
 * WHAT IT DOES: Steps the list at $B57A and clears the object when it ends; at frame $1D or later starts a thrown object in a free slot
 * from $30, its kind and speeds by OBJ_FALL_LO, which counts the throws.
 * HOW: FindFreeSlotFrom30, StartObjectAtPlace, speeds from a table, one step of movement for the new object.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ThrowOnAnimation051B(uint8_t slot)
{
    list_to_temp10(0xB57A);
    JSR(0xB53A, k_step_frames());
    if (EQ) {
        k_clear();
        return;
    }
    if (CC) {
        return;
    }
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x1D);
    if (CC) {
        return;
    }
    JSR(0xB54B, k_free_from30());
    if (CC) {
        return;
    }
    lda(0x15);
    JSR(0xB552, k_sound());
    phy();
    lda(RD8(OBJ->fall_lo[cpu.x]));
    tay();
    lda(rd8((uint16_t)(0xB59D + cpu.y)));
    ply();
    JSR(0xB55E, k_start_at_place());
    phx();
    lda(RD8(OBJ->fall_lo[cpu.x]));
    asl_a();
    tax();
    lda(rd8((uint16_t)(0xB5A5 + cpu.x)));
    WR8(OBJ->vx_hi[cpu.y], cpu.a);
    lda(rd8((uint16_t)(0xB5A6 + cpu.x)));
    WR8(OBJ->vy_hi[cpu.y], cpu.a);
    plx();
    sxy();
    JSR(0xB575, k_move());
    sxy();
    abi_nz(INC8(OBJ->fall_lo[cpu.x]));
}

void abi_ThrowOnAnimation051B(void)
{
    ThrowOnAnimation051B(cpu.x);
}

/*
 * @routine 051B:678 BySubstate051B678
 * @run 70:B678
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $B68A.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate051B678(uint8_t slot)
{
    jump_by_substate(slot, 0xB68A);
}

void abi_BySubstate051B678(void)
{
    BySubstate051B678(cpu.x);
}

/*
 * @routine 051B:6A1 BreakWhenCameraReaches051B
 * @run 70:B6A1
 * WHAT IT DOES: On the first screen, once its X reaches the column in OBJ_START_ARG's low digit: draws the tile script the high digit
 * names, clears the next slot, starts four pieces ($B6ED), records the event, sound $81, and goes on as the broken thing.
 * HOW: A compare of X high and X low / 16, then the calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BreakWhenCameraReaches051B(uint8_t slot)
{
    lda(RD8(OBJ->x_hi[slot]));
    if (NE) {
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    and_a(0x0F);
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->x_lo[slot]));
    lsr_a();
    lsr_a();
    lsr_a();
    lsr_a();
    cmp(cpu.a, RD8(ZP->temp00));
    if (CS) {
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    and_a(0x70);
    lsr_a();
    lsr_a();
    lsr_a();
    lsr_a();
    inc_a();
    cpu.y = 0;
    JSR(0xB6C6, k_draw_tiles());
    txa();
    tay();
    iny();
    JSR(0xB6CC, k_clear_in_y());
    call_routine(0xB6CF, 0xB6ED);
    JSR(0xB6D2, k_record_event());
    lda(0x81);
    JSR(0xB6D7, k_sound());
    lda(0x02);
    WR8(OBJ->hitbox_on[cpu.x], cpu.a);
    lda(RD8(OBJ->loop_flags[cpu.x]));
    ora_a(0x40);
    WR8(OBJ->loop_flags[cpu.x], cpu.a);
    lda(0x0E);
    WR8(OBJ->frame[cpu.x], cpu.a);
    abi_nz(INC8(OBJ->substate[cpu.x]));
}

void abi_BreakWhenCameraReaches051B(void)
{
    BreakWhenCameraReaches051B(cpu.x);
}

/*
 * @routine 051B:730 HitThenByState051B730
 * @run 70:B730
 * WHAT IT DOES: Under state 4, a hit (OBJ_HIT_STATUS bit 7) restarts the animation, sets OBJ_PIECE_PLACE $20 and state 4; then goes
 * to the state's handler.
 * HOW: A table of handlers at $B760.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void HitThenByState051B730(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x04);
    if (CC) {
        lda(RD8(OBJ->hit_status[slot]));
        and_a(0x80);
        if (NE) {
            WR8(OBJ->anim_step[slot], 0);
            WR8(OBJ->anim_timer[slot], 0);
            lda(0x20);
            WR8(OBJ->piece_place[slot], cpu.a);
            lda(0x04);
            WR8(OBJ->state[slot], cpu.a);
        }
    }
    jump_by_state(slot, 0xB760);
}

void abi_HitThenByState051B730(void)
{
    HitThenByState051B730(cpu.x);
}

/*
 * @routine 051B:7C4 StepListB819
 * @run 70:B7C4
 * WHAT IT DOES: Steps the list at $B819 and goes on at $B7CF.
 * HOW: StepObjectFrames, then into LandWithSpeedsB7CF.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as LandWithSpeedsB7CF.
 */
void StepListB819(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xB819);
    JSR(0xB7CF, k_step_frames());
    run_routine(0xB7CF);
}

void abi_StepListB819(void)
{
    StepListB819(cpu.x);
}

/*
 * @routine 051B:7CF LandWithSpeedsB7CF
 * @run 70:B7CF
 * WHAT IT DOES: Asks StopOnLanding with height 6. Against a wall sets OBJ_HIT_STATUS bit 7. On landing takes speeds from the row of
 * four at $B83A that OBJ_WORK_D picks and, while it is under 5, counts it up, with sound $2C the first time.
 * HOW: StopOnLanding with its flags kept by PHP and PLP across the wall test.
 * WAITS: no.
 * Entry: X object, Y and C as the frame step left them. Exit: as the last instruction left them.
 */
void LandWithSpeedsB7CF(uint8_t slot)
{
    lda(0x06);
    JSR(0xB7D4, k_stop_on_landing());
    push8((uint8_t)((cpu.p | P_B) & (uint8_t)~P_T));
    and_a(0x02);
    if (NE) {
        cpu.p = pull8();
        lda(RD8(OBJ->hit_status[slot]));
        ora_a(0x80);
        WR8(OBJ->hit_status[slot], cpu.a);
        return;
    }
    cpu.p = pull8();
    if (CC) {
        return;
    }
    lda(RD8(OBJ->work_d[slot]));
    asl_a();
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xB83B + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xB83A + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xB83D + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xB83C + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    cmp(cpu.a, 0x05);
    if (CS) {
        return;
    }
    lda(RD8(OBJ->work_d[slot]));
    if (EQ) {
        lda(0x2C);
        JSR(0xB815, k_sound());
    }
    abi_nz(INC8(OBJ->work_d[cpu.x]));
}

void abi_LandWithSpeedsB7CF(void)
{
    LandWithSpeedsB7CF(cpu.x);
}

/*
 * @routine 051C:052 StepThenCountOutOrLand051B
 * @run 70:B852
 * WHAT IT DOES: Steps the list at $B86C; counts OBJ_PIECE_PLACE down and clears the object at zero, else goes on at $B7CF.
 * HOW: No hit box meanwhile.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StepThenCountOutOrLand051B(uint8_t slot)
{
    WR8(OBJ->hitbox_on[slot], 0);
    list_to_temp10(0xB86C);
    JSR(0xB860, k_step_frames());
    abi_nz(DEC8(OBJ->piece_place[slot]));
    if (EQ) {
        k_clear();
        return;
    }
    run_routine(0xB7CF);
}

void abi_StepThenCountOutOrLand051B(void)
{
    StepThenCountOutOrLand051B(cpu.x);
}

/*
 * @routine 051C:071 CountBoxHitByState051B871
 * @run 70:B871
 * WHAT IT DOES: Counts OBJ_WORK_J up, sets the hit box ($B894), checks for a hit ($B8E7) and goes to the state's handler.
 * HOW: A table of handlers at $B88C.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountBoxHitByState051B871(uint8_t slot)
{
    abi_nz(INC8(OBJ->work_j[slot]));
    call_routine(0xB877, 0xB894);
    call_routine(0xB87A, 0xB8E7);
    jump_by_state(cpu.x, 0xB88C);
}

void abi_CountBoxHitByState051B871(void)
{
    CountBoxHitByState051B871(cpu.x);
}

/*
 * @routine 051C:094 SetHitBoxByFrame051B894
 * @run 70:B894
 * WHAT IT DOES: The hit box by the frame: none at frame 0, a long one at 5 and a longer one at 6, else the standing one.
 * HOW: Compares and stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SetHitBoxByFrame051B894(uint8_t slot)
{
    lda(RD8(OBJ->frame[slot]));
    if (EQ) {
        WR8(OBJ->hitbox_half_w[slot], 0);
        WR8(OBJ->hitbox_half_h[slot], 0);
        return;
    }
    cmp(cpu.a, 0x05);
    if (EQ) {
        lda(0xD4);
        WR8(OBJ->hitbox_dx[slot], cpu.a);
        lda(0xF8);
        WR8(OBJ->hitbox_dy[slot], cpu.a);
        lda(0x3C);
        WR8(OBJ->hitbox_half_w[slot], cpu.a);
        lda(0x10);
        WR8(OBJ->hitbox_half_h[slot], cpu.a);
        return;
    }
    cmp(cpu.a, 0x06);
    if (EQ) {
        lda(0xC8);
        WR8(OBJ->hitbox_dx[slot], cpu.a);
        lda(0xF8);
        WR8(OBJ->hitbox_dy[slot], cpu.a);
        lda(0x48);
        WR8(OBJ->hitbox_half_w[slot], cpu.a);
        lda(0x10);
        WR8(OBJ->hitbox_half_h[slot], cpu.a);
        return;
    }
    lda(0xF4);
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(0xF0);
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    lda(0x1C);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(0x28);
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_SetHitBoxByFrame051B894(void)
{
    SetHitBoxByFrame051B894(cpu.x);
}

/*
 * @routine 051C:0E7 State3WhenHit051B
 * @run 70:B8E7
 * WHAT IT DOES: Under state 3, a hit (OBJ_HIT_STATUS bit 7) goes to state 3, substate 0.
 * HOW: Two tests.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void State3WhenHit051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x03);
    if (CS) {
        return;
    }
    lda(RD8(OBJ->hit_status[slot]));
    and_a(0x80);
    if (EQ) {
        return;
    }
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_State3WhenHit051B(void)
{
    State3WhenHit051B(cpu.x);
}

/*
 * @routine 051C:0FE LoadThenPartTwoSlotsOn051B
 * @run 70:B8FE
 * WHAT IT DOES: Loads the record, clears the slot two on and starts a kind $25 object there at this one's place.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LoadThenPartTwoSlotsOn051B(uint8_t slot)
{
    (void)slot;
    JSR(0xB901, k_load_record());
    txa();
    tay();
    iny();
    iny();
    JSR(0xB908, k_clear_in_y());
    lda(0x25);
    JSR(0xB90D, k_start_at_place());
}

void abi_LoadThenPartTwoSlotsOn051B(void)
{
    LoadThenPartTwoSlotsOn051B(cpu.x);
}

/*
 * @routine 051C:10E SwayThenState2_051B
 * @run 70:B90E
 * WHAT IT DOES: Every $40 counts of OBJ_WORK_J goes to state 2. Meanwhile steps the list at $B950 and, on frames that step, moves
 * sideways by the speed OBJ_FALL_LO picks, turning every $20 moves.
 * HOW: StepObjectFrames, ApplyVelocityX, a count in OBJ_SWOOP_SPEED_HI.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SwayThenState2_051B(uint8_t slot)
{
    lda(RD8(OBJ->work_j[slot]));
    and_a(0x3F);
    if (EQ) {
        lda(0x02);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
        return;
    }
    list_to_temp10(0xB950);
    JSR(0xB929, k_step_frames());
    if (CC) {
        return;
    }
    ldy(RD8(OBJ->fall_lo[slot]));
    lda(rd8((uint16_t)(0xB961 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    WR8(OBJ->vx_lo[slot], 0);
    JSR(0xB93A, k_move_x());
    abi_nz(INC8(OBJ->swoop_speed_hi[slot]));
    lda(RD8(OBJ->swoop_speed_hi[slot]));
    cmp(cpu.a, 0x20);
    if (CC) {
        return;
    }
    lda(RD8(OBJ->fall_lo[slot]));
    eor_a(0x01);
    WR8(OBJ->fall_lo[slot], cpu.a);
    WR8(OBJ->swoop_speed_hi[slot], 0);
}

void abi_SwayThenState2_051B(void)
{
    SwayThenState2_051B(cpu.x);
}

/*
 * @routine 051C:163 BySubstate051B963
 * @run 70:B963
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $B975.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate051B963(uint8_t slot)
{
    jump_by_substate(slot, 0xB975);
}

void abi_BySubstate051B963(void)
{
    BySubstate051B963(cpu.x);
}

/*
 * @routine 051C:17F NextAttackFromList051B
 * @run 70:B97F
 * WHAT IT DOES: Takes the next substate from the eight at $B99D, counted by OBJ_SWOOP_SPEED_LO; substate 0 means state 1.
 * HOW: A table read; the animation restarts for another substate.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void NextAttackFromList051B(uint8_t slot)
{
    lda(RD8(OBJ->swoop_speed_lo[slot]));
    and_a(0x07);
    tay();
    abi_nz(INC8(OBJ->swoop_speed_lo[slot]));
    lda(rd8((uint16_t)(0xB99D + cpu.y)));
    WR8(OBJ->substate[slot], cpu.a);
    if (EQ) {
        lda(0x01);
        WR8(OBJ->state[slot], cpu.a);
        return;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
}

void abi_NextAttackFromList051B(void)
{
    NextAttackFromList051B(cpu.x);
}

/*
 * @routine 051C:1A5 SubstateThreeIfNear051B
 * @run 70:B9A5
 * WHAT IT DOES: Half the time, with the player within $70, goes to substate 3; else to the next substate.
 * HOW: NextRandom bit 0, DistanceXToPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SubstateThreeIfNear051B(uint8_t slot)
{
    (void)slot;
    JSR(0xB9A8, k_random());
    and_a(0x01);
    if (NE) {
        JSR(0xB9AF, k_distance_x());
        cmp(cpu.a, 0x70);
        if (CC) {
            lda(0x03);
            WR8(OBJ->substate[cpu.x], cpu.a);
            return;
        }
    }
    abi_nz(INC8(OBJ->substate[cpu.x]));
}

void abi_SubstateThreeIfNear051B(void)
{
    SubstateThreeIfNear051B(cpu.x);
}

/*
 * @routine 051C:1BD AnimateThenDropThree051B
 * @run 70:B9BD
 * WHAT IT DOES: Steps the list at $BA06. At its end goes to state 1. At step $0F sounds $25 and starts up to three kind $43 objects
 * in free slots from $51, numbered 0 to 2 in OBJ_START_ARG.
 * HOW: A walk over slots $51-$56 for empty ones.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void AnimateThenDropThree051B(uint8_t slot)
{
    list_to_temp10(0xBA06);
    JSR(0xB9C8, k_step_frames());
    if (EQ) {
        WR8(OBJ->piece_place[slot], 0);
        lda(0x01);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
        return;
    }
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x0F);
    if (NE) {
        return;
    }
    lda(0x25);
    JSR(0xB9D6, k_sound());
    WR8(ZP->temp00, 0);
    ldy(0x51);
    do {
        lda(RD8(OBJ->kind[cpu.y]));
        if (EQ) {
            JSR(0xB9E2, k_clear_in_y());
            lda(0x43);
            JSR(0xB9E7, k_start_at_place());
            lda(RD8(ZP->temp00));
            WR8(OBJ->start_arg[cpu.y], cpu.a);
            abi_nz(INC8(ZP->temp00));
            lda(RD8(ZP->temp00));
            cmp(cpu.a, 0x03);
            if (EQ) {
                return;
            }
        }
        iny();
        cmp(cpu.y, 0x57);
    } while (CC);
}

void abi_AnimateThenDropThree051B(void)
{
    AnimateThenDropThree051B(cpu.x);
}
