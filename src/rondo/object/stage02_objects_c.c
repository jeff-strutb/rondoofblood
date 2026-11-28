/*
 * Stage 02's objects, runs BDE5-C226.
 */
#include "stage_obj_ops.h"
#include "stage02_objects_c.h"
#include "calls_b.h"

/*
 * @routine 051C:5E5 BurstIntoTwoPieces051B
 * @run 70:BDE5
 * WHAT IT DOES: Starts two kind $4F objects in free slots from $30 with this one's facing and each piece's argument, count and
 * speeds from the tables at $BE2A, then clears this one.
 * HOW: A loop of two; the piece number in zBurstCount.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BurstIntoTwoPieces051B(uint8_t slot)
{
    (void)slot;
    cpu.y = 0;
    do {
        WR8(ZP->burst_count, cpu.y);
        JSR(0xBDEB, k_free_from30());
        if (CC) {
            break;
        }
        lda(0x4F);
        JSR(0xBDF2, k_start_at_place());
        lda(RD8(OBJ->facing_left[cpu.x]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        phx();
        ldx(RD8(ZP->burst_count));
        lda(rd8((uint16_t)(0xBE2A + cpu.x)));
        WR8(OBJ->start_arg[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xBE2C + cpu.x)));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xBE2E + cpu.x)));
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xBE30 + cpu.x)));
        WR8(OBJ->vx_lo[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xBE32 + cpu.x)));
        WR8(OBJ->vy_hi[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xBE34 + cpu.x)));
        WR8(OBJ->vy_lo[cpu.y], cpu.a);
        plx();
        ldy(RD8(ZP->burst_count));
        iny();
        cmp(cpu.y, 0x02);
    } while (CC);
    k_clear();
}

void abi_BurstIntoTwoPieces051B(void)
{
    BurstIntoTwoPieces051B(cpu.x);
}

/*
 * @routine 051C:636 LoadThenByStartArgSign051B
 * @run 70:BE36
 * WHAT IT DOES: Loads the record and goes to substate 1; with bit 7 of OBJ_START_ARG, also drops the hit box and loop flag bit 6
 * and goes to state 4.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LoadThenByStartArgSign051B(uint8_t slot)
{
    JSR(0xBE39, k_load_record());
    slot = cpu.x;
    lda(0x01);
    WR8(OBJ->substate[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    if (PL) {
        return;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    lda(RD8(OBJ->loop_flags[slot]));
    and_a(0xBF);
    WR8(OBJ->loop_flags[slot], cpu.a);
    lda(0x04);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_LoadThenByStartArgSign051B(void)
{
    LoadThenByStartArgSign051B(cpu.x);
}

/*
 * @routine 051C:658 SlowAndLandWhileList14_051B
 * @run 70:BE58
 * WHAT IT DOES: Slows the X speed, steps list $14 ($C200) until it ends, then goes to state 2, else moves and lands.
 * HOW: SlowByFacing051B, StepListByNumber051B, MoveAndLand with height 4.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SlowAndLandWhileList14_051B(uint8_t slot)
{
    (void)slot;
    call_routine(0xBE5B, 0xBEE8);
    ldy(0x14);
    call_routine(0xBE60, 0xC200);
    if (EQ) {
        run_routine(0xC15C);
        return;
    }
    lda(0x04);
    JSR(0xBE6A, k_move_and_land());
}

void abi_SlowAndLandWhileList14_051B(void)
{
    SlowAndLandWhileList14_051B(cpu.x);
}

/*
 * @routine 051C:66B FaceOffThePlayer051B
 * @run 70:BE6B
 * WHAT IT DOES: Facing as the player faces, goes to $BF92. Else substate 0 steps list $12 and, within $40, goes to substate 1;
 * substate 1 slows, steps list 4 until it ends (state 2) unless the player faces away in state 3, which goes to
 * state $0A facing the other way. Moves and lands while the list runs.
 * HOW: Calls and compares.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FaceOffThePlayer051B(uint8_t slot)
{
    lda(RD8(OBJ->facing_left[0]));
    eor_a(RD8(OBJ->facing_left[slot]));
    if (EQ) {
        run_routine(0xBF92);
        return;
    }
    lda(RD8(OBJ->substate[slot]));
    if (EQ) {
        ldy(0x12);
        call_routine(0xBE80, 0xC200);
        JSR(0xBE83, k_distance_x());
        cmp(cpu.a, 0x40);
        if (CC) {
            WR8(OBJ->anim_step[cpu.x], 0);
            WR8(OBJ->anim_timer[cpu.x], 0);
            abi_nz(INC8(OBJ->substate[cpu.x]));
        }
    } else {
        call_routine(0xBE95, 0xBEE8);
        JSR(0xBE98, k_side());
        and_a(0x01);
        cmp(cpu.a, RD8(OBJ->facing_left[cpu.x]));
        if (NE) {
            lda(RD8(OBJ->state[0]));
            cmp(cpu.a, 0x03);
            if (EQ) {
                lda(0x0B);
                WR8(OBJ->piece_place[cpu.x], cpu.a);
                call_routine(0xBEBE, 0xC0A3);
                slot = cpu.x;
                lda(0x0A);
                WR8(OBJ->state[slot], cpu.a);
                lda(RD8(OBJ->facing_left[slot]));
                eor_a(0x01);
                WR8(OBJ->facing_left[slot], cpu.a);
                WR8(OBJ->substate[slot], 0);
                WR8(OBJ->anim_step[slot], 0);
                WR8(OBJ->anim_timer[slot], 0);
                return;
            }
        }
        ldy(0x04);
        call_routine(0xBEAB, 0xC200);
        if (EQ) {
            run_routine(0xC15C);
            return;
        }
    }
    lda(0x04);
    JSR(0xBEB5, k_move_and_land());
}

void abi_FaceOffThePlayer051B(void)
{
    FaceOffThePlayer051B(cpu.x);
}

/*
 * @routine 051C:6D5 LandSlowList8_051B
 * @run 70:BED5
 * WHAT IT DOES: Moves and lands, slows, steps list 8 and goes to state 2 when it ends.
 * HOW: MoveAndLand, SlowByFacing051B, StepListByNumber051B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LandSlowList8_051B(uint8_t slot)
{
    (void)slot;
    lda(0x04);
    JSR(0xBEDA, k_move_and_land());
    call_routine(0xBEDD, 0xBEE8);
    ldy(0x08);
    call_routine(0xBEE2, 0xC200);
    if (EQ) {
        call_routine(0xBEE7, 0xC15C);
    }
}

void abi_LandSlowList8_051B(void)
{
    LandSlowList8_051B(cpu.x);
}

/*
 * @routine 051C:6E8 SlowByFacing051B
 * @run 70:BEE8
 * WHAT IT DOES: Takes the value for the facing in the table at $BF03 from the X speed.
 * HOW: A 16-bit subtraction.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SlowByFacing051B(uint8_t slot)
{
    lda(RD8(OBJ->facing_left[slot]));
    and_a(0x01);
    asl_a();
    tay();
    lda(RD8(OBJ->vx_lo[slot]));
    sec();
    sbc_a(rd8((uint16_t)(0xBF03 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    sbc_a(rd8((uint16_t)(0xBF04 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
}

void abi_SlowByFacing051B(void)
{
    SlowByFacing051B(cpu.x);
}

/*
 * @routine 051C:707 List10ThenLand051B
 * @run 70:BF07
 * WHAT IT DOES: Substate 0 steps list $10 and lands ($BF1D); substate 1 is $BF2E.
 * HOW: StepListByNumber051B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void List10ThenLand051B(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (NE) {
        run_routine(0xBF2E);
        return;
    }
    ldy(0x10);
    call_routine(0xBF11, 0xC200);
    run_routine(0xBF1D);
}

void abi_List10ThenLand051B(void)
{
    List10ThenLand051B(cpu.x);
}

/*
 * @routine 051C:713 List0AThenLand051B
 * @run 70:BF13
 * WHAT IT DOES: Substate 0 steps list $0A and lands ($BF1D); substate 1 is $BF2E.
 * HOW: StepListByNumber051B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void List0AThenLand051B(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (NE) {
        run_routine(0xBF2E);
        return;
    }
    ldy(0x0A);
    call_routine(0xBF1D, 0xC200);
    run_routine(0xBF1D);
}

void abi_List0AThenLand051B(void)
{
    List0AThenLand051B(cpu.x);
}

/*
 * @routine 051C:71D NextSubstateOnLanding051B
 * @run 70:BF1D
 * WHAT IT DOES: Stops on landing; landed, the next substate with the animation from its start.
 * HOW: StopOnLanding with height 4.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void NextSubstateOnLanding051B(uint8_t slot)
{
    lda(0x04);
    JSR(0xBF22, k_stop_on_landing());
    if (CC) {
        return;
    }
    slot = cpu.x;
    abi_nz(INC8(OBJ->substate[slot]));
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
}

void abi_NextSubstateOnLanding051B(void)
{
    NextSubstateOnLanding051B(cpu.x);
}

/*
 * @routine 051C:72E List0EThenState2_051B
 * @run 70:BF2E
 * WHAT IT DOES: Steps list $0E and goes to state 2 when it ends.
 * HOW: StepListByNumber051B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void List0EThenState2_051B(uint8_t slot)
{
    (void)slot;
    ldy(0x0E);
    call_routine(0xBF33, 0xC200);
    if (EQ) {
        run_routine(0xC15C);
    }
}

void abi_List0EThenState2_051B(void)
{
    List0EThenState2_051B(cpu.x);
}

/*
 * @routine 051C:738 StartState8_051B
 * @run 70:BF38
 * WHAT IT DOES: Velocity by $C0A3 with OBJ_PIECE_PLACE 6, then state 8 from its start.
 * HOW: SpeedByPlayerSide051B and stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StartState8_051B(uint8_t slot)
{
    lda(0x06);
    WR8(OBJ->piece_place[slot], cpu.a);
    call_routine(0xBF40, 0xC0A3);
    slot = cpu.x;
    lda(0x08);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
}

void abi_StartState8_051B(void)
{
    StartState8_051B(cpu.x);
}

/*
 * @routine 051C:74F DecideByDistance051B
 * @run 70:BF4F
 * WHAT IT DOES: Within $40 backs off ($BF92). Further, unless slot 3 holds a kind under $58 (then $BF81), the pad's bit 6 clear
 * goes to state 8 ($BF38), set to state 7 ($BF65).
 * HOW: DistanceXToPlayer, the kind in slot 3 and BiosPadState.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DecideByDistance051B(uint8_t slot)
{
    (void)slot;
    JSR(0xBF52, k_distance_x());
    cmp(cpu.a, 0x40);
    if (CC) {
        run_routine(0xBF92);
        return;
    }
    ldy(RD8(OBJ->kind[3]));
    if (EQ) {
        run_routine(0xBF81);
        return;
    }
    cmp(cpu.a, 0x58);
    if (CS) {
        run_routine(0xBF81);
        return;
    }
    lda(RD8(PADS->now));
    asl_a();
    if (PL) {
        run_routine(0xBF38);
        return;
    }
    run_routine(0xBF65);
}

void abi_DecideByDistance051B(void)
{
    DecideByDistance051B(cpu.x);
}

/*
 * @routine 051C:765 StartState7_051B
 * @run 70:BF65
 * WHAT IT DOES: Velocity by $C0A3 with OBJ_PIECE_PLACE 4, then state 7 rising at -4.
 * HOW: SpeedByPlayerSide051B and stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StartState7_051B(uint8_t slot)
{
    lda(0x04);
    WR8(OBJ->piece_place[slot], cpu.a);
    call_routine(0xBF6D, 0xC0A3);
    slot = cpu.x;
    lda(0x07);
    WR8(OBJ->state[slot], cpu.a);
    lda(0xFC);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
}

void abi_StartState7_051B(void)
{
    StartState7_051B(cpu.x);
}

/*
 * @routine 051C:781 WaitWhileCharged051B
 * @run 70:BF81
 * WHAT IT DOES: Substate 0: while the player moves toward it, counts OBJ_SWOOP_SPEED_LO out and backs off ($BF92) at zero;
 * otherwise reloads the count ($BFAE). Substate 1 counts down ($BFB7).
 * HOW: PlayerHeadingToObject051B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WaitWhileCharged051B(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (NE) {
        run_routine(0xBFB7);
        return;
    }
    call_routine(0xBF89, 0xC08D);
    if (CC) {
        run_routine(0xBFAE);
        return;
    }
    if (NE) {
        run_routine(0xBFAE);
        return;
    }
    abi_nz(DEC8(OBJ->swoop_speed_lo[cpu.x]));
    if (NE) {
        return;
    }
    run_routine(0xBF92);
}

void abi_WaitWhileCharged051B(void)
{
    WaitWhileCharged051B(cpu.x);
}

/*
 * @routine 051C:792 StartState6_051B
 * @run 70:BF92
 * WHAT IT DOES: Velocity by $C0A3 with OBJ_PIECE_PLACE 2, then state 6 rising at -6.
 * HOW: SpeedByPlayerSide051B and stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StartState6_051B(uint8_t slot)
{
    lda(0x02);
    WR8(OBJ->piece_place[slot], cpu.a);
    call_routine(0xBF9A, 0xC0A3);
    slot = cpu.x;
    lda(0x06);
    WR8(OBJ->state[slot], cpu.a);
    lda(0xFA);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
}

void abi_StartState6_051B(void)
{
    StartState6_051B(cpu.x);
}

/*
 * @routine 051C:7AE ReloadWaitThenNext051B
 * @run 70:BFAE
 * WHAT IT DOES: A wait of $50 in OBJ_SWOOP_SPEED_LO and the next substate.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ReloadWaitThenNext051B(uint8_t slot)
{
    lda(0x50);
    WR8(OBJ->swoop_speed_lo[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ReloadWaitThenNext051B(void)
{
    ReloadWaitThenNext051B(cpu.x);
}

/*
 * @routine 051C:7B7 CountThenState2_051B
 * @run 70:BFB7
 * WHAT IT DOES: Counts OBJ_SWOOP_SPEED_LO down and goes to state 2 at zero.
 * HOW: A decrement.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountThenState2_051B(uint8_t slot)
{
    abi_nz(DEC8(OBJ->swoop_speed_lo[slot]));
    if (EQ) {
        run_routine(0xC15C);
    }
}

void abi_CountThenState2_051B(void)
{
    CountThenState2_051B(cpu.x);
}

/*
 * @routine 051C:7BF List6ThenState2_051B
 * @run 70:BFBF
 * WHAT IT DOES: Steps list 6 and goes to state 2 when it ends.
 * HOW: StepListByNumber051B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void List6ThenState2_051B(uint8_t slot)
{
    (void)slot;
    ldy(0x06);
    call_routine(0xBFC4, 0xC200);
    if (EQ) {
        call_routine(0xBFC9, 0xC15C);
    }
}

void abi_List6ThenState2_051B(void)
{
    List6ThenState2_051B(cpu.x);
}

/*
 * @routine 051C:7CA BySubstate051BBFCA
 * @run 70:BFCA
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $BFDC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate051BBFCA(uint8_t slot)
{
    jump_by_substate(slot, 0xBFDC);
}

void abi_BySubstate051BBFCA(void)
{
    BySubstate051BBFCA(cpu.x);
}

/*
 * @routine 051C:7E2 List0TwiceThenTurn051B
 * @run 70:BFE2
 * WHAT IT DOES: Steps list 0; the second time it ends, counts OBJ_PIECE_PLACE up and sets the speed anew. Moves on ($C012), or
 * while the list runs looks at the player ($C01E) when the step's carry is clear.
 * HOW: StepListByNumber051B and the substate as the count.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void List0TwiceThenTurn051B(uint8_t slot)
{
    cpu.y = 0;
    call_routine(0xBFE6, 0xC200);
    if (NE) {
        if (CC) {
            run_routine(0xC01E);
            return;
        }
        run_routine(0xC012);
        return;
    }
    slot = cpu.x;
    abi_nz(INC8(OBJ->substate[slot]));
    lda(RD8(OBJ->substate[slot]));
    cmp(cpu.a, 0x02);
    if (NE) {
        run_routine(0xC012);
        return;
    }
    abi_nz(INC8(OBJ->piece_place[slot]));
    call_routine(0xBFFC, 0xC0A3);
    run_routine(0xC012);
}

void abi_List0TwiceThenTurn051B(void)
{
    List0TwiceThenTurn051B(cpu.x);
}

/*
 * @routine 051C:7FE List2ThenTurnBack051B
 * @run 70:BFFE
 * WHAT IT DOES: Steps list 2; when it ends, substate 0 with OBJ_PIECE_PLACE 0 and the speed anew. Moves on ($C012), or while the
 * list runs looks at the player ($C01E) when the step's carry is clear.
 * HOW: StepListByNumber051B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void List2ThenTurnBack051B(uint8_t slot)
{
    ldy(0x02);
    call_routine(0xC003, 0xC200);
    if (NE) {
        if (CC) {
            run_routine(0xC01E);
            return;
        }
        run_routine(0xC012);
        return;
    }
    slot = cpu.x;
    WR8(OBJ->substate[slot], 0);
    WR8(OBJ->piece_place[slot], 0);
    call_routine(0xC012, 0xC0A3);
    run_routine(0xC012);
}

void abi_List2ThenTurnBack051B(void)
{
    List2ThenTurnBack051B(cpu.x);
}

/*
 * @routine 051D:012 MoveLandThenLookOrJump051B
 * @run 71:C012
 * WHAT IT DOES: Moves and lands; off the ground (bit 0 clear) goes to state 7 ($BF65), else looks at the player ($C01E).
 * HOW: MoveAndLand with height 4.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void MoveLandThenLookOrJump051B(uint8_t slot)
{
    (void)slot;
    lda(0x04);
    JSR(0xC017, k_move_and_land());
    and_a(0x01);
    if (NE) {
        run_routine(0xC01E);
        return;
    }
    run_routine(0xBF65);
}

void abi_MoveLandThenLookOrJump051B(void)
{
    MoveLandThenLookOrJump051B(cpu.x);
}

/*
 * @routine 051D:01E ChooseAttackByPlayer051B
 * @run 71:C01E
 * WHAT IT DOES: Within $28 goes to state 3. Under state 3 with the player facing the other way, to state 9. Otherwise, while the
 * player moves away with the list stopped, to state 5.
 * HOW: DistanceXToPlayer, facing, PlayerHeadingToObject051B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ChooseAttackByPlayer051B(uint8_t slot)
{
    JSR(0xC021, k_distance_x());
    slot = cpu.x;
    cmp(cpu.a, 0x28);
    if (CC) {
        WR8(OBJ->piece_place[slot], 0);
        WR8(OBJ->vx_hi[slot], 0);
        WR8(OBJ->vx_lo[slot], 0);
        lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        return;
    }
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x03);
    if (CC) {
        lda(RD8(OBJ->facing_left[0]));
        eor_a(RD8(OBJ->facing_left[slot]));
        if (NE) {
            lda(0x08);
            WR8(OBJ->piece_place[slot], cpu.a);
            call_routine(0xC07E, 0xC0A3);
            slot = cpu.x;
            lda(0x09);
            WR8(OBJ->state[slot], cpu.a);
            WR8(OBJ->anim_step[slot], 0);
            WR8(OBJ->anim_timer[slot], 0);
            WR8(OBJ->substate[slot], 0);
            return;
        }
    }
    call_routine(0xC037, 0xC08D);
    if (CC) {
        return;
    }
    if (NE) {
        return;
    }
    slot = cpu.x;
    WR8(OBJ->piece_place[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    lda(0x40);
    WR8(OBJ->swoop_speed_lo[slot], cpu.a);
    lda(0x32);
    WR8(OBJ->frame[slot], cpu.a);
    call_routine(0xC06A, 0xC171);
    slot = cpu.x;
    lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
}

void abi_ChooseAttackByPlayer051B(void)
{
    ChooseAttackByPlayer051B(cpu.x);
}

/*
 * @routine 051D:08D PlayerHeadingToObject051B
 * @run 71:C08D
 * WHAT IT DOES: Carry clear when the player is not moving; else carry set and A bit 0 of the sign of its X speed against the
 * object's facing.
 * HOW: The player's X speed bytes; two ROLs bring the sign to bit 0.
 * WAITS: no.
 * Entry: X object, C as passed. Exit: C the answer; A, N and Z by the arm.
 */
void PlayerHeadingToObject051B(uint8_t slot)
{
    lda(RD8(OBJ->vx_lo[0]));
    ora_a(RD8(OBJ->vx_hi[0]));
    if (EQ) {
        clc();
        return;
    }
    lda(RD8(OBJ->vx_hi[0]));
    rol_a();
    rol_a();
    eor_a(RD8(OBJ->facing_left[slot]));
    and_a(0x01);
    sec();
}

void abi_PlayerHeadingToObject051B(void)
{
    PlayerHeadingToObject051B(cpu.x);
}

/*
 * @routine 051D:0A3 SpeedByPlayerSide051B
 * @run 71:C0A3
 * WHAT IT DOES: Faces the player and takes the X speed the table at $C0BD holds for the facing and OBJ_PIECE_PLACE.
 * HOW: SideOfPlayer and a word table.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SpeedByPlayerSide051B(uint8_t slot)
{
    (void)slot;
    JSR(0xC0A6, k_side());
    and_a(0x01);
    WR8(OBJ->facing_left[cpu.x], cpu.a);
    eor_a(RD8(OBJ->piece_place[cpu.x]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xC0BE + cpu.y)));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    lda(rd8((uint16_t)(0xC0BD + cpu.y)));
    WR8(OBJ->vx_lo[cpu.x], cpu.a);
}

void abi_SpeedByPlayerSide051B(void)
{
    SpeedByPlayerSide051B(cpu.x);
}

/*
 * @routine 051D:0D5 BySubstate051BC0D5
 * @run 71:C0D5
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $C0E7.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate051BC0D5(uint8_t slot)
{
    jump_by_substate(slot, 0xC0E7);
}

void abi_BySubstate051BC0D5(void)
{
    BySubstate051BC0D5(cpu.x);
}

/*
 * @routine 051D:0ED BreakAndLeapWhenCameraReaches051B
 * @run 71:C0ED
 * WHAT IT DOES: Once its X reaches the column in OBJ_START_ARG's low digit: draws the tile script the high digit names, clears the
 * next slot, starts four pieces ($B6ED), records the event, sound $81, and leaps by the player's side.
 * HOW: As BreakWhenCameraReaches051B, then a speed from the table at $C14C and -4 upward.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BreakAndLeapWhenCameraReaches051B(uint8_t slot)
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
    JSR(0xC112, k_draw_tiles());
    txa();
    tay();
    iny();
    JSR(0xC118, k_clear_in_y());
    call_routine(0xC11B, 0xB6ED);
    JSR(0xC11E, k_record_event());
    lda(0x81);
    JSR(0xC123, k_sound());
    lda(0x02);
    WR8(OBJ->hitbox_on[cpu.x], cpu.a);
    lda(RD8(OBJ->loop_flags[cpu.x]));
    ora_a(0x40);
    WR8(OBJ->loop_flags[cpu.x], cpu.a);
    JSR(0xC133, k_side());
    and_a(0x01);
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xC14C + cpu.y)));
    WR8(OBJ->vx_lo[cpu.x], cpu.a);
    lda(rd8((uint16_t)(0xC14D + cpu.y)));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    lda(0xFC);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    abi_nz(INC8(OBJ->substate[cpu.x]));
}

void abi_BreakAndLeapWhenCameraReaches051B(void)
{
    BreakAndLeapWhenCameraReaches051B(cpu.x);
}

/*
 * @routine 051D:150 ListCStateTwoOnLanding051B
 * @run 71:C150
 * WHAT IT DOES: Steps list $0C and, on landing, goes to state 2 ($C15C).
 * HOW: StepListByNumber051B, StopOnLanding with height 4.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ListCStateTwoOnLanding051B(uint8_t slot)
{
    (void)slot;
    ldy(0x0C);
    call_routine(0xC155, 0xC200);
    lda(0x04);
    JSR(0xC15A, k_stop_on_landing());
    if (CC) {
        return;
    }
    run_routine(0xC15C);
}

void abi_ListCStateTwoOnLanding051B(void)
{
    ListCStateTwoOnLanding051B(cpu.x);
}

/*
 * @routine 051D:15C StartState2_051B
 * @run 71:C15C
 * WHAT IT DOES: Velocity by $C0A3 with OBJ_PIECE_PLACE 0, then state 2 from its start.
 * HOW: SpeedByPlayerSide051B and stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StartState2_051B(uint8_t slot)
{
    WR8(OBJ->piece_place[slot], 0);
    call_routine(0xC162, 0xC0A3);
    slot = cpu.x;
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
}

void abi_StartState2_051B(void)
{
    StartState2_051B(cpu.x);
}

/*
 * @routine 051D:171 SetHitBoxByFrameTable051B
 * @run 71:C171
 * WHAT IT DOES: The hit box and OBJ_WORK_C and B from the tables, by the frame from $29.
 * HOW: A row index table, then six columns.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SetHitBoxByFrameTable051B(uint8_t slot)
{
    lda(RD8(OBJ->frame[slot]));
    sec();
    sbc_a(0x29);
    tay();
    lda(rd8((uint16_t)(0xC1C5 + cpu.y)));
    tay();
    lda(rd8((uint16_t)(0xC1A1 + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(rd8((uint16_t)(0xC1A7 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    lda(rd8((uint16_t)(0xC1AD + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(rd8((uint16_t)(0xC1B3 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
    lda(rd8((uint16_t)(0xC1B9 + cpu.y)));
    WR8(OBJ->work_c[slot], cpu.a);
    lda(rd8((uint16_t)(0xC1BF + cpu.y)));
    WR8(OBJ->work_b[slot], cpu.a);
}

void abi_SetHitBoxByFrameTable051B(void)
{
    SetHitBoxByFrameTable051B(cpu.x);
}

/*
 * @routine 051D:200 StepListByNumber051B
 * @run 71:C200
 * WHAT IT DOES: Steps the list the table at $C213 names for Y, then sets the hit box by the frame ($C171), keeping the step's flags.
 * HOW: PHP and PLP around the call.
 * WAITS: no.
 * Entry: X object, Y the list number doubled. Exit: the step's flags; A, X, Y as the calls left them.
 */
void StepListByNumber051B(uint8_t slot)
{
    (void)slot;
    lda(rd8((uint16_t)(0xC213 + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xC214 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    JSR(0xC20D, k_step_frames());
    push8((uint8_t)((cpu.p | P_B) & (uint8_t)~P_T));
    call_routine(0xC211, 0xC171);
    cpu.p = pull8();
}

void abi_StepListByNumber051B(void)
{
    StepListByNumber051B(cpu.x);
}
