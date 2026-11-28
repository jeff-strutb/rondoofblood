/*
 * Stage 04's objects, runs B4A4-BE45.
 */
#include "stage_obj_ops.h"
#include "stage04_objects_b.h"
#include "calls_b.h"

/*
 * @routine 07DB:4A4 BySubstate07DAB4A4
 * @run 70:B4A4
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $B4B6.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate07DAB4A4(uint8_t slot)
{
    jump_by_substate(slot, 0xB4B6);
}

void abi_BySubstate07DAB4A4(void)
{
    BySubstate07DAB4A4(cpu.x);
}

/*
 * @routine 07DB:4BE SpringAway07DB
 * @run 70:B4BE
 * WHAT IT DOES: Keeps the facing, picks a count from the distance to the player and a speed ($B4E7), turns both speeds back,
 * halves them ($B553), waits $0C and goes to the next substate.
 * HOW: PlayerDistanceSum, NegateVelocityX and Y.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SpringAway07DB(uint8_t slot)
{
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->work_k[slot], cpu.a);
    JSR(0xB4C7, k_distance_sum());
    and_a(0x0F);
    WR8(OBJ->work_i[cpu.x], cpu.a);
    call_routine(0xB4CF, 0xB4E7);
    JSR(0xB4D2, k_negate_x());
    JSR(0xB4D5, k_negate_y());
    call_routine(0xB4D8, 0xB553);
    slot = cpu.x;
    lda(0x0C);
    WR8(OBJ->work_d[slot], cpu.a);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SpringAway07DB(void)
{
    SpringAway07DB(cpu.x);
}

/*
 * @routine 07DB:553 HalveVelocity07DB
 * @run 70:B553
 * WHAT IT DOES: Halves both speeds, keeping their signs.
 * HOW: ASL for the sign into carry, then ROR of the high and low bytes.
 * WAITS: no.
 * Entry: X object. Exit: C the last bit out, N and Z from VY low.
 */
void HalveVelocity07DB(uint8_t slot)
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

void abi_HalveVelocity07DB(void)
{
    HalveVelocity07DB(cpu.x);
}

/*
 * @routine 07DB:592 ChaseAndRise07DB
 * @run 70:B592
 * WHAT IT DOES: Moves, accelerates toward the player (clamped to 2), and rises faster. While moving down, steps the list and
 * shows frame $4A when moving toward the player; once moving up, throws upward and sounds, and at the height in
 * OBJ_WORK_J goes to the next substate.
 * HOW: Calls and 16-bit adds.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ChaseAndRise07DB(uint8_t slot)
{
    JSR(0xB595, k_move());
    JSR(0xB598, k_side());
    slot = cpu.x;
    and_a(0x01);
    WR8(ZP->temp00, cpu.a);
    asl_a();
    tay();
    lda(RD8(OBJ->vx_lo[slot]));
    clc();
    adc_a(rd8((uint16_t)(0xB5F9 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    adc_a(rd8((uint16_t)(0xB5FA + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    call_routine(0xB5B4, 0xB5FD);
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
        call_routine(0xB5CC, 0xB3ED);
        slot = cpu.x;
        lda(RD8(OBJ->vx_hi[slot]));
        rol_a();
        rol_a();
        and_a(0x01);
        eor_a(RD8(ZP->temp00));
        if (EQ) {
            return;
        }
        lda(0x4A);
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    call_routine(0xB5E0, 0xB622);
    ldy(0x04);
    call_routine(0xB5E5, 0xB3ED);
    call_routine(0xB5E8, 0xB632);
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

void abi_ChaseAndRise07DB(void)
{
    ChaseAndRise07DB(cpu.x);
}

/*
 * @routine 07DB:63F HoverNearTop07DB
 * @run 70:B63F
 * WHAT IT DOES: Steps the list with its sound and moves. On the first screen: high up, goes to state 5; otherwise every other
 * count halves the speeds, and once they have died away (0 or -1) goes to state 5.
 * HOW: HalveVelocity07DB.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void HoverNearTop07DB(uint8_t slot)
{
    ldy(0x04);
    call_routine(0xB644, 0xB3ED);
    call_routine(0xB647, 0xB632);
    JSR(0xB64A, k_move());
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
        call_routine(0xB660, 0xB553);
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

void abi_HoverNearTop07DB(void)
{
    HoverNearTop07DB(cpu.x);
}

/*
 * @routine 07DB:6FC CountThenByState07DB6FC
 * @run 70:B6FC
 * WHAT IT DOES: Counts down to state 4 ($B71B) and goes to the state's handler.
 * HOW: A table of handlers at $B711.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountThenByState07DB6FC(uint8_t slot)
{
    (void)slot;
    call_routine(0xB6FF, 0xB71B);
    jump_by_state(cpu.x, 0xB711);
}

void abi_CountThenByState07DB6FC(void)
{
    CountThenByState07DB6FC(cpu.x);
}

/*
 * @routine 07DB:78E FlagThenByState07DB78E
 * @run 70:B78E
 * WHAT IT DOES: Enters state 6 if flagged ($B7B1) and goes to the state's handler.
 * HOW: A table of handlers at $B7A3.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlagThenByState07DB78E(uint8_t slot)
{
    (void)slot;
    call_routine(0xB791, 0xB7B1);
    jump_by_state(cpu.x, 0xB7A3);
}

void abi_FlagThenByState07DB78E(void)
{
    FlagThenByState07DB78E(cpu.x);
}

/*
 * @routine 07DB:7C7 LandUnlessSlot54_07DB
 * @run 70:B7C7
 * WHAT IT DOES: Unless the object in slot $54 shows frame 4, goes on when it lands.
 * HOW: StopOnLanding with height 4.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LandUnlessSlot54_07DB(uint8_t slot)
{
    (void)slot;
    lda(RD8(OBJ->frame[0x54]));
    cmp(cpu.a, 0x04);
    if (EQ) {
        return;
    }
    lda(0x04);
    JSR(0xB7D3, k_stop_on_landing());
    if (CC) {
        return;
    }
    abi_nz(INC8(OBJ->state[cpu.x]));
}

void abi_LandUnlessSlot54_07DB(void)
{
    LandUnlessSlot54_07DB(cpu.x);
}

/*
 * @routine 07DB:7D9 WalkNearOrAttack07DB
 * @run 70:B7D9
 * WHAT IT DOES: Walks ($B7F1), keeps the facing in OBJ_PIECE_PLACE and, within $4C, goes to state 3; then checks the ground
 * ($B878).
 * HOW: DistanceXToPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WalkNearOrAttack07DB(uint8_t slot)
{
    call_routine(0xB7DC, 0xB7F1);
    slot = cpu.x;
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->work_e[slot], cpu.a);
    JSR(0xB7E5, k_distance_x());
    cmp(cpu.a, 0x4C);
    if (CC) {
        lda(0x03);
        WR8(OBJ->state[cpu.x], cpu.a);
    }
    run_routine(0xB878);
}

void abi_WalkNearOrAttack07DB(void)
{
    WalkNearOrAttack07DB(cpu.x);
}

/*
 * @routine 07DC:078 CheckGroundAhead07DB
 * @run 70:B878
 * WHAT IT DOES: Without solid ground ahead and below, goes to state 5; otherwise counts OBJ_WORK_D down to state 4.
 * HOW: ProbeTerrain request 9 with SpriteFlipMask 4; the three TileHit answers.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CheckGroundAhead07DB(uint8_t slot)
{
    lda(0x04);
    WR8(ZP->sprite_flip_mask, cpu.a);
    lda(0x09);
    JSR(0xB881, k_probe());
    slot = cpu.x;
    lda(rd8(TileHit));
    and_a(rd8((uint16_t)(TileHit + 1)));
    and_a(rd8((uint16_t)(TileHit + 2)));
    and_a(0x01);
    if (EQ) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->substate[slot], 0);
        lda(0x05);
        WR8(OBJ->state[slot], cpu.a);
        return;
    }
    run_routine(0xB89D);
}

void abi_CheckGroundAhead07DB(void)
{
    CheckGroundAhead07DB(cpu.x);
}

/*
 * @routine 07DC:0AE WalkFarOrRetreat07DB
 * @run 70:B8AE
 * WHAT IT DOES: Walks, keeps the facing reversed in OBJ_PIECE_PLACE and, from $6C away, goes to state 2; then checks the ground.
 * HOW: DistanceXToPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WalkFarOrRetreat07DB(uint8_t slot)
{
    call_routine(0xB8B1, 0xB7F1);
    slot = cpu.x;
    lda(RD8(OBJ->facing_left[slot]));
    eor_a(0x01);
    WR8(OBJ->work_e[slot], cpu.a);
    JSR(0xB8BC, k_distance_x());
    cmp(cpu.a, 0x6C);
    if (CS) {
        lda(0x02);
        WR8(OBJ->state[cpu.x], cpu.a);
    }
    run_routine(0xB878);
}

void abi_WalkFarOrRetreat07DB(void)
{
    WalkFarOrRetreat07DB(cpu.x);
}

/*
 * @routine 07DC:10B BySubstate07DAB90B
 * @run 70:B90B
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $B91D.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate07DAB90B(uint8_t slot)
{
    jump_by_substate(slot, 0xB91D);
}

void abi_BySubstate07DAB90B(void)
{
    BySubstate07DAB90B(cpu.x);
}

/*
 * @routine 07DC:123 StartHopAtListEnd07DB
 * @run 70:B923
 * WHAT IT DOES: Steps the list ($B817); at its end jumps the way OBJ_PIECE_PLACE says, rising at -3.
 * HOW: A table of speeds by direction.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StartHopAtListEnd07DB(uint8_t slot)
{
    ldy(0x04);
    call_routine(0xB928, 0xB817);
    if (NE) {
        return;
    }
    slot = cpu.x;
    ldy(RD8(OBJ->work_e[slot]));
    lda(rd8((uint16_t)(0xB948 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0xFD);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_StartHopAtListEnd07DB(void)
{
    StartHopAtListEnd07DB(cpu.x);
}

/*
 * @routine 07DC:14A NextSubstateOnLanding07DB
 * @run 70:B94A
 * WHAT IT DOES: Goes to the next substate on landing.
 * HOW: StopOnLanding with height 4.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void NextSubstateOnLanding07DB(uint8_t slot)
{
    (void)slot;
    lda(0x04);
    JSR(0xB94F, k_stop_on_landing());
    if (CC) {
        return;
    }
    abi_nz(INC8(OBJ->substate[cpu.x]));
}

void abi_NextSubstateOnLanding07DB(void)
{
    NextSubstateOnLanding07DB(cpu.x);
}

/*
 * @routine 07DC:155 State3AtListEnd07DB
 * @run 70:B955
 * WHAT IT DOES: Steps the list ($B817); at its end goes to state 3.
 * HOW: StepFramesFacingPlayerAlways with list 6.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void State3AtListEnd07DB(uint8_t slot)
{
    ldy(0x06);
    call_routine(0xB95A, 0xB817);
    if (NE) {
        return;
    }
    slot = cpu.x;
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->substate[slot], 0);
}

void abi_State3AtListEnd07DB(void)
{
    State3AtListEnd07DB(cpu.x);
}

/*
 * @routine 07DC:291 ThrowFromHandAndMove07DB
 * @run 70:BA91
 * WHAT IT DOES: Counts, picks a speed at random toward the player at state starts ($BAE3), handles a hit ($BAFA), sets the hit
 * box ($BB32) and goes to the state's handler ($BB44). While the slot below holds kind $27 in a state under 2,
 * places it at the hand with the speed the frame picks, mirrored by facing, and moves it.
 * HOW: CopyObjectPlace to the slot below.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ThrowFromHandAndMove07DB(uint8_t slot)
{
    abi_nz(INC8(OBJ->work_g[slot]));
    call_routine(0xBA97, 0xBAE3);
    call_routine(0xBA9A, 0xBAFA);
    call_routine(0xBA9D, 0xBB32);
    call_routine(0xBAA0, 0xBB44);
    lda(rd8((uint16_t)(0x274F + cpu.x)));
    cmp(cpu.a, 0x27);
    if (NE) {
        return;
    }
    lda(rd8((uint16_t)(0x27A7 + cpu.x)));
    cmp(cpu.a, 0x02);
    if (CS) {
        return;
    }
    txa();
    tay();
    dey();
    JSR(0xBAB4, k_copy_place());
    WR8(ZP->climb_step_before, cpu.x);
    lda(RD8(OBJ->frame[cpu.x]));
    sec();
    sbc_a(0x01);
    asl_a();
    tax();
    lda(rd8((uint16_t)(0xBFCC + cpu.x)));
    WR8(OBJ->vx_hi[cpu.y], cpu.a);
    lda(rd8((uint16_t)(0xBFCD + cpu.x)));
    WR8(OBJ->vy_hi[cpu.y], cpu.a);
    ldx(RD8(ZP->climb_step_before));
    lda(RD8(OBJ->facing_left[cpu.x]));
    wr8((uint16_t)(0x2ABF + cpu.x), cpu.a);
    if (NE) {
        lda(RD8(OBJ->vx_hi[cpu.y]));
        eor_a(0xFF);
        inc_a();
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
    }
    sxy();
    JSR(0xBAE1, k_move());
    sxy();
}

void abi_ThrowFromHandAndMove07DB(void)
{
    ThrowFromHandAndMove07DB(cpu.x);
}

/*
 * @routine 07DC:2E3 FacePlayerInSomeStates07DB
 * @run 70:BAE3
 * WHAT IT DOES: Faces the player in the states the table at $BAF4 marks.
 * HOW: SideOfPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FacePlayerInSomeStates07DB(uint8_t slot)
{
    ldy(RD8(OBJ->state[slot]));
    lda(rd8((uint16_t)(0xBAF4 + cpu.y)));
    if (EQ) {
        return;
    }
    JSR(0xBAEE, k_side());
    and_a(0x01);
    WR8(OBJ->facing_left[cpu.x], cpu.a);
}

void abi_FacePlayerInSomeStates07DB(void)
{
    FacePlayerInSomeStates07DB(cpu.x);
}

/*
 * @routine 07DC:2FA DropThrownAndRecoil07DB
 * @run 70:BAFA
 * WHAT IT DOES: A hit outside state 5 drops the object below and sends this one up and back for $40 frames in state 5.
 * HOW: ClearObjectInY and stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DropThrownAndRecoil07DB(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x05);
    if (EQ) {
        return;
    }
    lda(RD8(OBJ->hit_status[slot]));
    and_a(0x80);
    if (EQ) {
        return;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    txa();
    tay();
    dey();
    JSR(0xBB11, k_clear_in_y());
    slot = cpu.x;
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    lda(0xFE);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_DropThrownAndRecoil07DB(void)
{
    DropThrownAndRecoil07DB(cpu.x);
}

/*
 * @routine 07DC:332 SetHitBoxByFrame12_07DB
 * @run 70:BB32
 * WHAT IT DOES: Half width 8, or $13 at frame $12.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SetHitBoxByFrame12_07DB(uint8_t slot)
{
    lda(0x08);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x12);
    if (NE) {
        return;
    }
    lda(0x13);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
}

void abi_SetHitBoxByFrame12_07DB(void)
{
    SetHitBoxByFrame12_07DB(cpu.x);
}

/*
 * @routine 07DC:344 ByState07DABB44
 * @run 70:BB44
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $BB56.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState07DABB44(uint8_t slot)
{
    jump_by_state(slot, 0xBB56);
}

void abi_ByState07DABB44(void)
{
    ByState07DABB44(cpu.x);
}

/*
 * @routine 07DC:362 LoadThrowerWithObject07DB
 * @run 70:BB62
 * WHAT IT DOES: Loads the record facing the player with a wait of $40 and starts the kind $27 object it holds in the slot below;
 * number 2 draws behind (attribute $87).
 * HOW: ClearObjectInY and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LoadThrowerWithObject07DB(uint8_t slot)
{
    JSR(0xBB65, k_load_record());
    JSR(0xBB68, k_side());
    slot = cpu.x;
    and_a(0x01);
    WR8(OBJ->facing_left[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->work_d[slot], cpu.a);
    txa();
    tay();
    dey();
    JSR(0xBB78, k_clear_in_y());
    lda(0x27);
    JSR(0xBB7D, k_start_at_place());
    lda(RD8(OBJ->start_arg[cpu.x]));
    cmp(cpu.a, 0x02);
    if (NE) {
        return;
    }
    lda(0x87);
    WR8(OBJ->sprite_attr[cpu.x], cpu.a);
}

void abi_LoadThrowerWithObject07DB(void)
{
    LoadThrowerWithObject07DB(cpu.x);
}

/*
 * @routine 07DC:38A NextStateOnLanding07DC38A
 * @run 70:BB8A
 * WHAT IT DOES: Goes to the next state on landing, with height 6.
 * HOW: StopOnLanding.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void NextStateOnLanding07DC38A(uint8_t slot)
{
    (void)slot;
    lda(0x06);
    JSR(0xBB8F, k_stop_on_landing());
    if (CC) {
        return;
    }
    abi_nz(INC8(OBJ->state[cpu.x]));
}

void abi_NextStateOnLanding07DC38A(void)
{
    NextStateOnLanding07DC38A(cpu.x);
}

/*
 * @routine 07DC:395 WalkUntilNear07DB
 * @run 70:BB95
 * WHAT IT DOES: Walks ($BBB0) the way it faces; unless an attack starts ($BBE5), goes to state 3 within $4E.
 * HOW: DistanceXToPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WalkUntilNear07DB(uint8_t slot)
{
    call_routine(0xBB98, 0xBBB0);
    slot = cpu.x;
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->work_f[slot], cpu.a);
    call_routine(0xBBA1, 0xBBE5);
    if (CS) {
        return;
    }
    JSR(0xBBA6, k_distance_x());
    cmp(cpu.a, 0x4E);
    if (CS) {
        return;
    }
    lda(0x03);
    WR8(OBJ->state[cpu.x], cpu.a);
}

void abi_WalkUntilNear07DB(void)
{
    WalkUntilNear07DB(cpu.x);
}

/*
 * @routine 07DC:3B0 StepAndWalk07DB
 * @run 70:BBB0
 * WHAT IT DOES: Steps the list at $BBD8 and walks the way OBJ_WORK_F says, bouncing off walls.
 * HOW: StepObjectFrames, then BounceOffWallsHigh07DA with SpriteFlipMask 6.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StepAndWalk07DB(uint8_t slot)
{
    lda(0xBB);
    WR8(ZP->temp11, cpu.a);
    lda(0xD8);
    WR8(ZP->temp10, cpu.a);
    JSR(0xBBBB, k_step_frames());
    slot = cpu.x;
    lda(RD8(OBJ->work_f[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xBBD5 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xBBD4 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(0x06);
    WR8(ZP->sprite_flip_mask, cpu.a);
    call_routine(0xBBD3, 0xAA35);
}

void abi_StepAndWalk07DB(void)
{
    StepAndWalk07DB(cpu.x);
}

/*
 * @routine 07DC:3E5 AttackWhenWaitOver07DB
 * @run 70:BBE5
 * WHAT IT DOES: While OBJ_WORK_D counts, carry clear. At zero picks the next wait from the table by number and OBJ_RAIN_COUNT,
 * starts state 4 and returns carry set.
 * HOW: Eight waits per number.
 * WAITS: no.
 * Entry: X object. Exit: C whether an attack starts.
 */
void AttackWhenWaitOver07DB(uint8_t slot)
{
    lda(RD8(OBJ->work_d[slot]));
    if (NE) {
        abi_nz(DEC8(OBJ->work_d[slot]));
        clc();
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    asl_a();
    asl_a();
    asl_a();
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_i[slot]));
    and_a(0x07);
    clc();
    adc_a(RD8(ZP->temp00));
    tay();
    lda(rd8((uint16_t)(0xBC16 + cpu.y)));
    WR8(OBJ->work_d[slot], cpu.a);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x04);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
    sec();
}

void abi_AttackWhenWaitOver07DB(void)
{
    AttackWhenWaitOver07DB(cpu.x);
}

/*
 * @routine 07DC:42E WalkAwayUntilFar07DB
 * @run 70:BC2E
 * WHAT IT DOES: Walks away from the way it faces; unless an attack starts, goes to state 2 from $64 away.
 * HOW: DistanceXToPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WalkAwayUntilFar07DB(uint8_t slot)
{
    call_routine(0xBC31, 0xBBB0);
    slot = cpu.x;
    lda(RD8(OBJ->facing_left[slot]));
    eor_a(0x01);
    WR8(OBJ->work_f[slot], cpu.a);
    call_routine(0xBC3C, 0xBBE5);
    if (CS) {
        return;
    }
    JSR(0xBC41, k_distance_x());
    cmp(cpu.a, 0x64);
    if (CC) {
        return;
    }
    lda(0x02);
    WR8(OBJ->state[cpu.x], cpu.a);
}

void abi_WalkAwayUntilFar07DB(void)
{
    WalkAwayUntilFar07DB(cpu.x);
}

/*
 * @routine 07DC:44B BySubstate07DABC4B
 * @run 70:BC4B
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $BC5D.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate07DABC4B(uint8_t slot)
{
    jump_by_substate(slot, 0xBC5D);
}

void abi_BySubstate07DABC4B(void)
{
    BySubstate07DABC4B(cpu.x);
}

/*
 * @routine 07DC:467 ChooseNextMove07DB
 * @run 70:BC67
 * WHAT IT DOES: Off the first screen, state 3. With the player within $18 in height, takes the substate from the table by number
 * and count; further, every fourth count goes to substate 3, else state 3.
 * HOW: DistanceYToPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ChooseNextMove07DB(uint8_t slot)
{
    lda(RD8(OBJ->x_hi[slot]));
    if (EQ) {
        JSR(0xBC6F, k_distance_y());
        slot = cpu.x;
        cmp(cpu.a, 0x18);
        if (CC) {
            lda(RD8(OBJ->start_arg[slot]));
            asl_a();
            asl_a();
            asl_a();
            asl_a();
            WR8(ZP->temp00, cpu.a);
            lda(RD8(OBJ->work_i[slot]));
            and_a(0x0F);
            clc();
            adc_a(RD8(ZP->temp00));
            tay();
            lda(rd8((uint16_t)(0xBCA5 + cpu.y)));
            WR8(OBJ->substate[slot], cpu.a);
            abi_nz(INC8(OBJ->work_i[slot]));
            return;
        }
        abi_nz(INC8(OBJ->work_i[slot]));
        lda(RD8(OBJ->work_i[slot]));
        and_a(0x03);
        if (EQ) {
            lda(0x03);
            WR8(OBJ->substate[slot], cpu.a);
            return;
        }
    }
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_ChooseNextMove07DB(void)
{
    ChooseNextMove07DB(cpu.x);
}

/*
 * @routine 07DC:4D5 ThrowAlongList07DB4D5
 * @run 70:BCD5
 * WHAT IT DOES: Steps the list at $BCF7: at its end starts state 3 ($BCE8); on a new frame throws ($BD04).
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ThrowAlongList07DB4D5(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xBCF7);
    JSR(0xBCE0, k_step_frames());
    if (EQ) {
        run_routine(0xBCE8);
        return;
    }
    if (CC) {
        return;
    }
    run_routine(0xBD04);
}

void abi_ThrowAlongList07DB4D5(void)
{
    ThrowAlongList07DB4D5(cpu.x);
}

/*
 * @routine 07DC:4E8 RestartInState3_07DB
 * @run 70:BCE8
 * WHAT IT DOES: State 3 from its start.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void RestartInState3_07DB(uint8_t slot)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_RestartInState3_07DB(void)
{
    RestartInState3_07DB(cpu.x);
}

/*
 * @routine 07DC:504 ThrowAtFrameFive07DB
 * @run 70:BD04
 * WHAT IT DOES: At animation step 5, sound 6 and a kind $26 object with this facing, its OBJ_PIECE_PLACE the substate less one.
 * HOW: FindFreeSlotFrom30 and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ThrowAtFrameFive07DB(uint8_t slot)
{
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x05);
    if (NE) {
        return;
    }
    lda(0x06);
    JSR(0xBD10, k_sound());
    JSR(0xBD13, k_free_from30());
    if (CC) {
        return;
    }
    lda(0x26);
    JSR(0xBD1A, k_start_at_place());
    lda(RD8(OBJ->facing_left[cpu.x]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    lda(RD8(OBJ->substate[cpu.x]));
    dec_a();
    WR8(OBJ->work_e[cpu.y], cpu.a);
}

void abi_ThrowAtFrameFive07DB(void)
{
    ThrowAtFrameFive07DB(cpu.x);
}

/*
 * @routine 07DC:528 ThrowAlongList07DB528
 * @run 70:BD28
 * WHAT IT DOES: As ThrowAlongList07DB4D5 with the list at $BD3B.
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ThrowAlongList07DB528(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xBD3B);
    JSR(0xBD33, k_step_frames());
    if (EQ) {
        run_routine(0xBCE8);
        return;
    }
    if (CC) {
        return;
    }
    run_routine(0xBD04);
}

void abi_ThrowAlongList07DB528(void)
{
    ThrowAlongList07DB528(cpu.x);
}

/*
 * @routine 07DC:546 ThrowAlongList07DB546
 * @run 70:BD46
 * WHAT IT DOES: As ThrowAlongList07DB4D5 with the list at $BD59.
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ThrowAlongList07DB546(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xBD59);
    JSR(0xBD51, k_step_frames());
    if (EQ) {
        run_routine(0xBCE8);
        return;
    }
    if (CC) {
        return;
    }
    run_routine(0xBD04);
}

void abi_ThrowAlongList07DB546(void)
{
    ThrowAlongList07DB546(cpu.x);
}

/*
 * @routine 07DC:568 DashAlongList07DB
 * @run 70:BD68
 * WHAT IT DOES: Steps the list at $BD98 (state 3 at its end), moving the way it faces and, from step 2, bouncing off walls.
 * HOW: StepObjectFrames, BounceOffWallsHigh07DA with SpriteFlipMask 6.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DashAlongList07DB(uint8_t slot)
{
    list_to_temp10(0xBD98);
    JSR(0xBD73, k_step_frames());
    if (EQ) {
        run_routine(0xBCE8);
        return;
    }
    slot = cpu.x;
    lda(RD8(OBJ->facing_left[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xBDB2 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xBDB1 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x02);
    if (CC) {
        return;
    }
    lda(0x06);
    WR8(ZP->sprite_flip_mask, cpu.a);
    call_routine(0xBD97, 0xAA35);
}

void abi_DashAlongList07DB(void)
{
    DashAlongList07DB(cpu.x);
}

/*
 * @routine 07DC:5B5 BurningDebris07DB
 * @run 70:BDB5
 * WHAT IT DOES: Steps the list at $BE03, counts out and clears; drips ($BE0C); on landing at frame $1B bounces by the table,
 * smaller each time, six times.
 * HOW: StepObjectFrames, StopOnLanding with height 6.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BurningDebris07DB(uint8_t slot)
{
    lda(0xBE);
    WR8(ZP->temp11, cpu.a);
    lda(0x03);
    WR8(ZP->temp10, cpu.a);
    JSR(0xBDC0, k_step_frames());
    abi_nz(DEC8(OBJ->work_d[cpu.x]));
    if (EQ) {
        k_clear();
        return;
    }
    call_routine(0xBDCB, 0xBE0C);
    lda(0x06);
    JSR(0xBDD0, k_stop_on_landing());
    if (CC) {
        return;
    }
    slot = cpu.x;
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x1B);
    if (NE) {
        return;
    }
    lda(RD8(OBJ->work_h[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xBDF6 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xBDF5 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->work_h[slot]));
    cmp(cpu.a, 0x06);
    if (EQ) {
        return;
    }
    abi_nz(INC8(OBJ->work_h[slot]));
}

void abi_BurningDebris07DB(void)
{
    BurningDebris07DB(cpu.x);
}

/*
 * @routine 07DC:60C SparkByCount07DB
 * @run 70:BE0C
 * WHAT IT DOES: When OBJ_WORK_D matches one of eight values at $BE46, starts a kind $9C spark with that row's speed, sound $15,
 * and moves it once.
 * HOW: As DripByWaveCounter051B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SparkByCount07DB(uint8_t slot)
{
    (void)slot;
    cpu.y = 0;
    for (;;) {
        lda(RD8(OBJ->work_d[cpu.x]));
        cmp(cpu.a, rd8((uint16_t)(0xBE46 + cpu.y)));
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
    JSR(0xBE22, k_free_from30());
    if (CC) {
        return;
    }
    lda(0x9C);
    JSR(0xBE29, k_start_at_place());
    lda(0x15);
    JSR(0xBE2E, k_sound_first_screen());
    lda(RD8(ZP->burst_count));
    asl_a();
    tax();
    lda(rd8((uint16_t)(0xBE4E + cpu.x)));
    WR8(OBJ->vx_hi[cpu.y], cpu.a);
    lda(rd8((uint16_t)(0xBE4F + cpu.x)));
    WR8(OBJ->vy_hi[cpu.y], cpu.a);
    sxy();
    JSR(0xBE42, k_move());
    sxy();
    ldx(RD8(ZP->climb_step_before));
}

void abi_SparkByCount07DB(void)
{
    SparkByCount07DB(cpu.x);
}
