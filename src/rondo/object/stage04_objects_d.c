/*
 * Stage 04's objects, runs C25F-C543.
 */
#include "stage_obj_ops.h"
#include "stage04_objects_d.h"
#include "calls_b.h"

/*
 * @routine 07DD:25F SwingAroundNextAndSound07DD
 * @run 71:C25F
 * WHAT IT DOES: Steps the list at $C21A and circles the object in the next slot, its height halved; each full turn sounds $4A and
 * goes back a state.
 * HOW: CopyObjectPlaceBack, ApplyVelocityY $23, SetVelocityFromAngle magnitude $1000.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SwingAroundNextAndSound07DD(uint8_t slot)
{
    lda(0xC2);
    WR8(ZP->temp11, cpu.a);
    lda(0x1A);
    WR8(ZP->temp10, cpu.a);
    JSR(0xC26A, k_step_frames());
    txa();
    tay();
    iny();
    JSR(0xC270, k_copy_place_back());
    lda(0x23);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    JSR(0xC278, k_move_y());
    lda(0x10);
    WR8(ZP->temp01, cpu.a);
    lda(0x00);
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_d[cpu.x]));
    JSR(0xC286, k_velocity_from_angle());
    slot = cpu.x;
    lda(RD8(OBJ->vy_hi[slot]));
    asl_a();
    ROR_MEM(OBJ->vy_hi[slot]);
    ROR_MEM(OBJ->vy_lo[slot]);
    JSR(0xC293, k_move());
    slot = cpu.x;
    lda(RD8(OBJ->work_d[slot]));
    clc();
    adc_a(0x08);
    WR8(OBJ->work_d[slot], cpu.a);
    if (CC) {
        return;
    }
    lda(0x4A);
    JSR(0xC2A3, k_sound());
    abi_nz(DEC8(OBJ->state[cpu.x]));
}

void abi_SwingAroundNextAndSound07DD(void)
{
    SwingAroundNextAndSound07DD(cpu.x);
}

/*
 * @routine 07DD:2AB CountHitsThenByState07DD
 * @run 71:C2AB
 * WHAT IT DOES: Counts, takes the hurt time ($C2D0), drops without the arm ($C2E2) and goes to the state's handler.
 * HOW: A table of handlers at $C2C6.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountHitsThenByState07DD(uint8_t slot)
{
    abi_nz(INC8(OBJ->work_d[slot]));
    call_routine(0xC2B1, 0xC2D0);
    call_routine(0xC2B4, 0xC2E2);
    jump_by_state(cpu.x, 0xC2C6);
}

void abi_CountHitsThenByState07DD(void)
{
    CountHitsThenByState07DD(cpu.x);
}

/*
 * @routine 07DD:2D0 HurtTimeByHit07DD
 * @run 71:C2D0
 * WHAT IT DOES: Sets the hurt time the table gives for a hit's type and $18 frames of invulnerability.
 * HOW: OBJ_HIT_TAKEN indexes the table.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void HurtTimeByHit07DD(uint8_t slot)
{
    lda(RD8(OBJ->hit_taken[slot]));
    if (EQ) {
        return;
    }
    tay();
    lda(rd8((uint16_t)(0xA27D + cpu.y)));
    WR8(OBJ->hurt_timer[slot], cpu.a);
    lda(0x18);
    WR8(OBJ->invuln_timer[slot], cpu.a);
}

void abi_HurtTimeByHit07DD(void)
{
    HurtTimeByHit07DD(cpu.x);
}

/*
 * @routine 07DD:2E2 DropWithoutBase07DD
 * @run 71:C2E2
 * WHAT IT DOES: Under state 2, once the slot below no longer holds kind $AB, goes to state 2 with pace bits 3-6 cleared, the
 * animation restarted, no hit box and a wait of $30.
 * HOW: The kind of slot X-1.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DropWithoutBase07DD(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x02);
    if (CS) {
        return;
    }
    lda(rd8((uint16_t)(0x274F + cpu.x)));
    cmp(cpu.a, 0xAB);
    if (EQ) {
        return;
    }
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
    lda(RD8(OBJ->pace[slot]));
    and_a(0x87);
    WR8(OBJ->pace[slot], cpu.a);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
    lda(0x30);
    WR8(OBJ->work_e[slot], cpu.a);
}

void abi_DropWithoutBase07DD(void)
{
    DropWithoutBase07DD(cpu.x);
}

/*
 * @routine 07DD:30C TailByBodyState07DD
 * @run 71:C30C
 * WHAT IT DOES: Steps the list at $C366; frame $59 unless the body two slots below is alive. While the slot below is in state 1,
 * bobs and drifts by screen side; else sits at Y $8E.
 * HOW: Reads of slots X-2 and X-1.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void TailByBodyState07DD(uint8_t slot)
{
    lda(0xC3);
    WR8(ZP->temp11, cpu.a);
    lda(0x66);
    WR8(ZP->temp10, cpu.a);
    JSR(0xC317, k_step_frames());
    slot = cpu.x;
    {
        bool alive = false;

        lda(rd8((uint16_t)(0x274E + cpu.x)));
        cmp(cpu.a, 0xAA);
        if (EQ) {
            lda(rd8((uint16_t)(0x27A6 + cpu.x)));
            cmp(cpu.a, 0x02);
            alive = CC;
        }
        if (!alive) {
            lda(0x59);
            WR8(OBJ->frame[slot], cpu.a);
        }
    }
    lda(rd8((uint16_t)(0x27A7 + cpu.x)));
    cmp(cpu.a, 0x01);
    if (NE) {
        lda(0x8E);
        WR8(OBJ->y_lo[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->work_d[slot]));
    lsr_a();
    lsr_a();
    and_a(0x01);
    tay();
    lda(RD8(OBJ->y_lo[slot]));
    clc();
    adc_a(rd8((uint16_t)(0xC360 + cpu.y)));
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(RD8(OBJ->x_hi[slot]));
    and_a(0x01);
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xC363 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC362 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    JSR(0xC359, k_move_x());
}

void abi_TailByBodyState07DD(void)
{
    TailByBodyState07DD(cpu.x);
}

/*
 * @routine 07DD:36B NextStateAfterWait30_07DD
 * @run 71:C36B
 * WHAT IT DOES: Counts OBJ_PIECE_PLACE out, reloads $30 and goes on.
 * HOW: A countdown.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void NextStateAfterWait30_07DD(uint8_t slot)
{
    abi_nz(DEC8(OBJ->work_e[slot]));
    if (NE) {
        return;
    }
    lda(0x30);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_NextStateAfterWait30_07DD(void)
{
    NextStateAfterWait30_07DD(cpu.x);
}

/*
 * @routine 07DD:379 BurningWaterAtCounts07DD
 * @run 71:C379
 * @stop C3B4
 * WHAT IT DOES: Steps the list at $C3B4; at its end clears the water level and the object. Meanwhile flickers the water level,
 * counts down, and at eight counts throws embers ($C3D1).
 * HOW: A search of the counts.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BurningWaterAtCounts07DD(uint8_t slot)
{
    (void)slot;
    lda(0xC3);
    WR8(ZP->temp11, cpu.a);
    lda(0xB4);
    WR8(ZP->temp10, cpu.a);
    JSR(0xC384, k_step_frames());
    if (EQ) {
        WR8(LEVELW->y.lo, 0);
        WR8(LEVELW->y.hi, 0);
        k_clear();
        return;
    }
    lda(RD8(OBJ->work_e[cpu.x]));
    and_a(0x07);
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xC3C1 + cpu.y)));
    WR8(LEVELW->y.lo, cpu.a);
    lda(rd8((uint16_t)(0xC3C2 + cpu.y)));
    WR8(LEVELW->y.hi, cpu.a);
    abi_nz(DEC8(OBJ->work_e[cpu.x]));
    cpu.y = 0;
    for (;;) {
        lda(RD8(OBJ->work_e[cpu.x]));
        cmp(cpu.a, rd8((uint16_t)(0xC3F5 + cpu.y)));
        if (EQ) {
            run_routine(0xC3D1);
            return;
        }
        iny();
        cmp(cpu.y, 0x08);
        if (CS) {
            return;
        }
    }
}

void abi_BurningWaterAtCounts07DD(void)
{
    BurningWaterAtCounts07DD(cpu.x);
}

/*
 * @routine 07DD:3D1 ThrowEmberOfRow07DD
 * @run 71:C3D1
 * WHAT IT DOES: Starts a kind $9E ember with the speed pair Y picks and moves it.
 * HOW: The row doubled kept in zBurstCount.
 * WAITS: no.
 * Entry: X object, Y the row. Exit: as the last instruction left them.
 */
void ThrowEmberOfRow07DD(uint8_t slot)
{
    (void)slot;
    tya();
    asl_a();
    WR8(ZP->burst_count, cpu.a);
    JSR(0xC3D8, k_free_from30());
    if (CC) {
        return;
    }
    lda(0x9E);
    JSR(0xC3DF, k_start_at_place());
    phx();
    ldx(RD8(ZP->burst_count));
    lda(rd8((uint16_t)(0xC3FD + cpu.x)));
    WR8(OBJ->vx_hi[cpu.y], cpu.a);
    lda(rd8((uint16_t)(0xC3FE + cpu.x)));
    WR8(OBJ->vy_hi[cpu.y], cpu.a);
    plx();
    sxy();
    JSR(0xC3F3, k_move());
    sxy();
}

void abi_ThrowEmberOfRow07DD(void)
{
    ThrowEmberOfRow07DD(cpu.x);
}

/*
 * @routine 07DD:40D SpawnWhenSlotsFree07DD
 * @run 71:C40D
 * WHAT IT DOES: Counts OBJ_WORK_D out, then, a slot of $30-$34 free, starts a kind $29 object there with this argument and picks
 * the next wait from the table.
 * HOW: FindEmptyOf30to34_07DD and NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SpawnWhenSlotsFree07DD(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        k_load_record();
        return;
    }
    lda(RD8(OBJ->work_d[slot]));
    if (NE) {
        abi_nz(DEC8(OBJ->work_d[slot]));
        return;
    }
    call_routine(0xC421, 0xC44D);
    if (CC) {
        return;
    }
    JSR(0xC426, k_clear_in_y());
    lda(0x29);
    JSR(0xC42B, k_start_at_place());
    slot = cpu.x;
    lda(RD8(OBJ->start_arg[slot]));
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    asl_a();
    asl_a();
    WR8(ZP->temp00, cpu.a);
    JSR(0xC438, k_random());
    and_a(0x03);
    clc();
    adc_a(RD8(ZP->temp00));
    tay();
    lda(rd8((uint16_t)(0xC445 + cpu.y)));
    WR8(OBJ->work_d[cpu.x], cpu.a);
}

void abi_SpawnWhenSlotsFree07DD(void)
{
    SpawnWhenSlotsFree07DD(cpu.x);
}

/*
 * @routine 07DD:44D FindEmptyOf30to34_07DD
 * @run 71:C44D
 * WHAT IT DOES: Y the first of slots $30-$34 holding no object, carry set; carry clear when none is.
 * HOW: A walk of the kinds.
 * WAITS: no.
 * Entry: none. Exit: C found, Y the slot or $35.
 */
void FindEmptyOf30to34_07DD(uint8_t slot)
{
    (void)slot;
    ldy(0x30);
    do {
        lda(RD8(OBJ->kind[cpu.y]));
        if (EQ) {
            sec();
            return;
        }
        iny();
        cmp(cpu.y, 0x35);
    } while (CC);
    clc();
}

void abi_FindEmptyOf30to34_07DD(void)
{
    FindEmptyOf30to34_07DD(cpu.x);
}

/*
 * @routine 07DD:45D ByState07DAC45D
 * @run 71:C45D
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $C46F.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState07DAC45D(uint8_t slot)
{
    jump_by_state(slot, 0xC46F);
}

void abi_ByState07DAC45D(void)
{
    ByState07DAC45D(cpu.x);
}

/*
 * @routine 07DD:477 EnterFromRandomSide07DD
 * @run 71:C477
 * WHAT IT DOES: Loads the record; number 0 faces right, others at random, but half the time not at all while one of slots $14,
 * $1C or $24 holds kind $35 (then cleared). Takes the X for number and facing.
 * HOW: NextRandom and the kinds of three slots.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void EnterFromRandomSide07DD(uint8_t slot)
{
    JSR(0xC47A, k_load_record());
    slot = cpu.x;
    cpu.a = 0;
    lda(RD8(OBJ->start_arg[slot]));
    if (NE) {
        JSR(0xC483, k_random());
        and_a(0x01);
        if (NE) {
            lda(RD8(OBJ->kind[0x14]));
            cmp(cpu.a, 0x35);
            if (EQ) {
                k_clear();
                return;
            }
            lda(RD8(OBJ->kind[0x1C]));
            cmp(cpu.a, 0x35);
            if (EQ) {
                k_clear();
                return;
            }
            lda(RD8(OBJ->kind[0x24]));
            cmp(cpu.a, 0x35);
            if (EQ) {
                k_clear();
                return;
            }
        }
        JSR(0xC4A8, k_random());
        and_a(0x01);
    }
    slot = cpu.x;
    WR8(OBJ->facing_left[slot], cpu.a);
    asl_a();
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    asl_a();
    asl_a();
    clc();
    adc_a(RD8(ZP->temp00));
    tay();
    lda(rd8((uint16_t)(0xC4C7 + cpu.y)));
    WR8(OBJ->x_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC4C6 + cpu.y)));
    WR8(OBJ->x_lo[slot], cpu.a);
}

void abi_EnterFromRandomSide07DD(void)
{
    EnterFromRandomSide07DD(cpu.x);
}

/*
 * @routine 07DD:4CE WalkOnLanding07DD
 * @run 71:C4CE
 * WHAT IT DOES: On landing, takes the walking speed for the facing and goes on.
 * HOW: StopOnLanding with height $0C.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WalkOnLanding07DD(uint8_t slot)
{
    lda(0x0C);
    JSR(0xC4D3, k_stop_on_landing());
    if (CC) {
        return;
    }
    slot = cpu.x;
    lda(RD8(OBJ->facing_left[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xC4EB + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC4EA + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_WalkOnLanding07DD(void)
{
    WalkOnLanding07DD(cpu.x);
}

/*
 * @routine 07DD:4EE WalkUntilNearSlot4D07DD
 * @run 71:C4EE
 * WHAT IT DOES: Within $60 of the object in slot $4D becomes kind $9C with sound $8E. Else steps the list for its argument,
 * moves and lands: a wall sends it to state 3, off the ground to state 1.
 * HOW: RightOfObject4DWithin, MoveAndLand with height $0C.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WalkUntilNearSlot4D07DD(uint8_t slot)
{
    lda(0x60);
    call_routine(0xC4F3, 0xD349);
    slot = cpu.x;
    if (CS) {
        lda(0x8E);
        JSR(0xC4FA, k_sound());
        lda(0x9C);
        WR8(OBJ->kind[cpu.x], cpu.a);
        WR8(OBJ->state[cpu.x], 0);
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xC544 + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xC545 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    JSR(0xC515, k_step_frames());
    lda(0x0C);
    JSR(0xC51A, k_move_and_land());
    slot = cpu.x;
    if (CC) {
        and_a(0x01);
        if (NE) {
            return;
        }
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->vx_hi[slot], 0);
        WR8(OBJ->vx_lo[slot], 0);
        lda(0x01);
        WR8(OBJ->state[slot], cpu.a);
        return;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->substate[slot], 0);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_WalkUntilNearSlot4D07DD(void)
{
    WalkUntilNearSlot4D07DD(cpu.x);
}
