/*
 * Stage 05's objects, runs D083-D81B.
 */
#include "stage_obj_ops.h"
#include "stage05_objects_d.h"
#include "calls_b.h"

/*
 * @routine 093F:083 SetUpRidersAround093F
 * @run 71:D083
 * WHAT IT DOES: Loads the record, starts list channel 2 with list $84, a wait of $50, and a kind $39 object below and a kind $41 above.
 * HOW: ClearObjectInY and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SetUpRidersAround093F(uint8_t slot)
{
    (void)slot;
    JSR(0xD086, k_load_record());
    phx();
    lda(0x84);
    ldx(0x05);
    JSR(0xD08E, k_list2());
    plx();
    lda(0x50);
    WR8(OBJ->work_e[cpu.x], cpu.a);
    txa();
    tay();
    dey();
    JSR(0xD09A, k_clear_in_y());
    lda(0x39);
    JSR(0xD09F, k_start_at_place());
    iny();
    iny();
    JSR(0xD0A4, k_clear_in_y());
    lda(0x41);
    JSR(0xD0A9, k_start_at_place());
}

void abi_SetUpRidersAround093F(void)
{
    SetUpRidersAround093F(cpu.x);
}

/*
 * @routine 093F:0AA NextOnFirstScreen093F
 * @run 71:D0AA
 * WHAT IT DOES: Goes on once on the first screen.
 * HOW: X high.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void NextOnFirstScreen093F(uint8_t slot)
{
    lda(RD8(OBJ->x_hi[slot]));
    if (NE) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_NextOnFirstScreen093F(void)
{
    NextOnFirstScreen093F(cpu.x);
}

/*
 * @routine 093F:0B3 WalkLeftUntilMap720_093F
 * @run 71:D0B3
 * WHAT IT DOES: Walks ($D0DD) leftward; at map X below $0720 goes to state 3. Counts the pause down ($D144).
 * HOW: The map X from the camera.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WalkLeftUntilMap720_093F(uint8_t slot)
{
    call_routine(0xD0B6, 0xD0DD);
    slot = cpu.x;
    WR8(OBJ->work_f[slot], 0);
    lda(RD8(SCROLL->x_lo[0]));
    clc();
    adc_a(RD8(OBJ->x_lo[slot]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(SCROLLE->x_hi_screen[0]));
    adc_a(RD8(OBJ->x_hi[slot]));
    cmp(cpu.a, 0x07);
    bool turn = CC;
    if (!turn && EQ) {
        lda(RD8(ZP->temp00));
        cmp(cpu.a, 0x20);
        turn = CC;
    }
    if (turn) {
        lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
    }
    call_routine(0xD0DC, 0xD144);
}

void abi_WalkLeftUntilMap720_093F(void)
{
    WalkLeftUntilMap720_093F(cpu.x);
}

/*
 * @routine 093F:0DD StepAndPickSpeed093F
 * @run 71:D0DD
 * WHAT IT DOES: Steps the list at $D137; on a new frame takes the speed for OBJ_SWOOP_SPEED_LO and moves turning at walls ($D0F3).
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StepAndPickSpeed093F(uint8_t slot)
{
    list_to_temp10(0xD137);
    JSR(0xD0E8, k_step_frames());
    if (CC) {
        return;
    }
    slot = cpu.x;
    ldy(RD8(OBJ->work_f[slot]));
    lda(rd8((uint16_t)(0xD135 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    run_routine(0xD0F3);
}

void abi_StepAndPickSpeed093F(void)
{
    StepAndPickSpeed093F(cpu.x);
}

/*
 * @routine 093F:0F3 MoveTurningAtWalls093F
 * @run 71:D0F3
 * WHAT IT DOES: Moves sideways; when the terrain ahead is solid at probes 9 and 2, turns back and moves.
 * HOW: ApplyVelocityX and two ProbeTerrain with SpriteFlipMask 8.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void MoveTurningAtWalls093F(uint8_t slot)
{
    JSR(0xD0F6, k_move_x());
    lda(0x08);
    WR8(ZP->sprite_flip_mask, cpu.a);
    lda(0x09);
    JSR(0xD0FF, k_probe());
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
        lda(0x08);
        WR8(ZP->sprite_flip_mask, cpu.a);
        lda(0x02);
        JSR(0xD119, k_probe());
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
            return;
        }
    }
    JSR(0xD132, k_negate_x());
    k_move_x();
}

void abi_MoveTurningAtWalls093F(void)
{
    MoveTurningAtWalls093F(cpu.x);
}

/*
 * @routine 093F:144 CountPauseThenState4_093F
 * @run 71:D144
 * WHAT IT DOES: Counts OBJ_PIECE_PLACE out and goes to state 4.
 * HOW: A countdown.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountPauseThenState4_093F(uint8_t slot)
{
    lda(RD8(OBJ->work_e[slot]));
    if (NE) {
        abi_nz(DEC8(OBJ->work_e[slot]));
        return;
    }
    lda(0x04);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_CountPauseThenState4_093F(void)
{
    CountPauseThenState4_093F(cpu.x);
}

/*
 * @routine 093F:156 WalkRightUntilMap8A0_093F
 * @run 71:D156
 * WHAT IT DOES: Walks rightward; from map X $07A0 goes to state 2. Counts the pause down.
 * HOW: The map X from the camera.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WalkRightUntilMap8A0_093F(uint8_t slot)
{
    call_routine(0xD159, 0xD0DD);
    slot = cpu.x;
    lda(0x01);
    WR8(OBJ->work_f[slot], cpu.a);
    lda(RD8(SCROLL->x_lo[0]));
    clc();
    adc_a(RD8(OBJ->x_lo[slot]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(SCROLLE->x_hi_screen[0]));
    adc_a(RD8(OBJ->x_hi[slot]));
    cmp(cpu.a, 0x08);
    bool turn = CS;
    if (!turn) {
        cmp(cpu.a, 0x07);
        if (EQ) {
            lda(RD8(ZP->temp00));
            cmp(cpu.a, 0xA0);
            turn = CS;
        }
    }
    if (turn) {
        lda(0x02);
        WR8(OBJ->state[slot], cpu.a);
    }
    call_routine(0xD183, 0xD144);
}

void abi_WalkRightUntilMap8A0_093F(void)
{
    WalkRightUntilMap8A0_093F(cpu.x);
}

/*
 * @routine 093F:184 BySubstate093FD184
 * @run 71:D184
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $D196.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate093FD184(uint8_t slot)
{
    jump_by_substate(slot, 0xD196);
}

void abi_BySubstate093FD184(void)
{
    BySubstate093FD184(cpu.x);
}

/*
 * @routine 093F:1A2 PickNextAttack093F
 * @run 71:D1A2
 * WHAT IT DOES: Restarts the animation, a random repeat count and the next substate from the table by count.
 * HOW: NextRandom and a table.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void PickNextAttack093F(uint8_t slot)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    JSR(0xD1AB, k_random());
    slot = cpu.x;
    and_a(0x03);
    WR8(OBJ->work_k[slot], cpu.a);
    lda(RD8(OBJ->work_h[slot]));
    and_a(0x07);
    tay();
    lda(rd8((uint16_t)(0xD1C0 + cpu.y)));
    WR8(OBJ->substate[slot], cpu.a);
    abi_nz(INC8(OBJ->work_h[slot]));
}

void abi_PickNextAttack093F(void)
{
    PickNextAttack093F(cpu.x);
}

/*
 * @routine 093F:1C8 SlashForward093F
 * @run 71:D1C8
 * WHAT IT DOES: Steps the list at $D200, at step 8 sounding $95 and $1A, moving forward turning at walls; at its end a pause of $40 and the next substate.
 * HOW: MoveTurningAtWalls093F.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SlashForward093F(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xD200);
    JSR(0xD1D3, k_step_frames());
    if (EQ) {
        lda(0x40);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        abi_nz(INC8(OBJ->substate[cpu.x]));
        return;
    }
    if (CC) {
        return;
    }
    lda(RD8(OBJ->anim_step[cpu.x]));
    cmp(cpu.a, 0x08);
    if (EQ) {
        lda(0x95);
        JSR(0xD1E6, k_sound());
        lda(0x1A);
        JSR(0xD1EB, k_sound());
    }
    ldy(RD8(OBJ->facing_left[cpu.x]));
    lda(rd8((uint16_t)(0xD219 + cpu.y)));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    run_routine(0xD0F3);
}

void abi_SlashForward093F(void)
{
    SlashForward093F(cpu.x);
}

/*
 * @routine 093F:21B BackStepWaiting093F
 * @run 71:D21B
 * WHAT IT DOES: Steps the list at $D23D, backing on new frames; the pause over, ends the attack ($D325).
 * HOW: MoveTurningAtWalls093F.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BackStepWaiting093F(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xD23D);
    JSR(0xD226, k_step_frames());
    if (CS) {
        ldy(RD8(OBJ->facing_left[cpu.x]));
        lda(rd8((uint16_t)(0xD24A + cpu.y)));
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        call_routine(0xD234, 0xD0F3);
    }
    abi_nz(DEC8(OBJ->work_d[cpu.x]));
    if (EQ) {
        run_routine(0xD325);
    }
}

void abi_BackStepWaiting093F(void)
{
    BackStepWaiting093F(cpu.x);
}

/*
 * @routine 093F:24C ThrustForward093F
 * @run 71:D24C
 * WHAT IT DOES: As SlashForward093F with the list at $D285, sounding $95 and 7 at step 9; at its end substate 2.
 * HOW: MoveTurningAtWalls093F.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ThrustForward093F(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xD285);
    JSR(0xD257, k_step_frames());
    if (EQ) {
        lda(0x40);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        lda(0x02);
        WR8(OBJ->substate[cpu.x], cpu.a);
        return;
    }
    if (CC) {
        return;
    }
    lda(RD8(OBJ->anim_step[cpu.x]));
    cmp(cpu.a, 0x09);
    if (EQ) {
        lda(0x95);
        JSR(0xD267, k_sound());
        lda(0x07);
        JSR(0xD26C, k_sound());
    }
    ldy(RD8(OBJ->facing_left[cpu.x]));
    lda(rd8((uint16_t)(0xD283 + cpu.y)));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    run_routine(0xD0F3);
}

void abi_ThrustForward093F(void)
{
    ThrustForward093F(cpu.x);
}

/*
 * @routine 093F:2C8 ThrowRepeatedly093F
 * @run 71:D2C8
 * WHAT IT DOES: Steps the list at $D30A, throwing a kind $3B object with sounds $1B and $96 at frame $24; at its end repeats from step 6 while OBJ_WORK_K lasts, then ends the attack.
 * HOW: FindFreeSlotFrom30.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ThrowRepeatedly093F(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xD30A);
    JSR(0xD2D3, k_step_frames());
    if (EQ) {
        lda(RD8(OBJ->work_k[cpu.x]));
        if (EQ) {
            run_routine(0xD325);
            return;
        }
        abi_nz(DEC8(OBJ->work_k[cpu.x]));
        lda(0x06);
        WR8(OBJ->anim_step[cpu.x], cpu.a);
        return;
    }
    if (CC) {
        return;
    }
    lda(RD8(OBJ->frame[cpu.x]));
    cmp(cpu.a, 0x24);
    if (NE) {
        return;
    }
    lda(0x1B);
    JSR(0xD2E3, k_sound());
    lda(0x96);
    JSR(0xD2E8, k_sound());
    JSR(0xD2EB, k_free_from30());
    if (CC) {
        return;
    }
    lda(0x3B);
    JSR(0xD2F2, k_start_at_place());
    lda(RD8(OBJ->facing_left[cpu.x]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
}

void abi_ThrowRepeatedly093F(void)
{
    ThrowRepeatedly093F(cpu.x);
}

/*
 * @routine 093F:325 EndAttack093F
 * @run 71:D325
 * WHAT IT DOES: State 3 from its start with the next pause from the table.
 * HOW: A count of attacks in OBJ_SWOOP_SPEED_HI.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void EndAttack093F(uint8_t slot)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    abi_nz(INC8(OBJ->work_g[slot]));
    lda(RD8(OBJ->work_g[slot]));
    and_a(0x03);
    tay();
    lda(rd8((uint16_t)(0xD340 + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
}

void abi_EndAttack093F(void)
{
    EndAttack093F(cpu.x);
}

/*
 * @routine 093F:344 BySubstate093FD344
 * @run 71:D344
 * WHAT IT DOES: Goes to the substate's handler.
 * HOW: A table of handlers at $D356.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BySubstate093FD344(uint8_t slot)
{
    jump_by_substate(slot, 0xD356);
}

void abi_BySubstate093FD344(void)
{
    BySubstate093FD344(cpu.x);
}

/*
 * @routine 093F:35C RoarThenWait093F
 * @run 71:D35C
 * WHAT IT DOES: Sound $97, a count of $40 and the next substate.
 * HOW: SoundCommand.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void RoarThenWait093F(uint8_t slot)
{
    (void)slot;
    lda(0x97);
    JSR(0xD361, k_sound());
    lda(0x40);
    WR8(OBJ->work_d[cpu.x], cpu.a);
    abi_nz(INC8(OBJ->substate[cpu.x]));
}

void abi_RoarThenWait093F(void)
{
    RoarThenWait093F(cpu.x);
}

/*
 * @routine 093F:36A RainDown093F
 * @run 71:D36A
 * WHAT IT DOES: Steps the list at $D386, counting and raining ($D391); at its end the next substate with a count of $80.
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void RainDown093F(uint8_t slot)
{
    (void)slot;
    list_to_temp10(0xD386);
    JSR(0xD375, k_step_frames());
    if (EQ) {
        abi_nz(INC8(OBJ->substate[cpu.x]));
        lda(0x80);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        return;
    }
    abi_nz(DEC8(OBJ->work_d[cpu.x]));
    run_routine(0xD391);
}

void abi_RainDown093F(void)
{
    RainDown093F(cpu.x);
}

/*
 * @routine 093F:391 RainAtCounts093F
 * @run 71:D391
 * WHAT IT DOES: At one of eight counts starts a kind $9E drop with sound $18 and the row's speeds, and moves it.
 * HOW: X kept in zClimbStepBefore, the row in zBurstCount.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void RainAtCounts093F(uint8_t slot)
{
    (void)slot;
    cpu.y = 0;
    for (;;) {
        lda(RD8(OBJ->work_d[cpu.x]));
        cmp(cpu.a, rd8((uint16_t)(0xD3CB + cpu.y)));
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
    JSR(0xD3A7, k_free_from30());
    if (CC) {
        return;
    }
    lda(0x9E);
    JSR(0xD3AE, k_start_at_place());
    lda(0x18);
    JSR(0xD3B3, k_sound());
    lda(RD8(ZP->burst_count));
    asl_a();
    tax();
    lda(rd8((uint16_t)(0xD3D3 + cpu.x)));
    WR8(OBJ->vx_hi[cpu.y], cpu.a);
    lda(rd8((uint16_t)(0xD3D4 + cpu.x)));
    WR8(OBJ->vy_hi[cpu.y], cpu.a);
    sxy();
    JSR(0xD3C7, k_move());
    sxy();
    ldx(RD8(ZP->climb_step_before));
}

void abi_RainAtCounts093F(void)
{
    RainAtCounts093F(cpu.x);
}

/*
 * @routine 093F:3E3 DissolveThenBurst093F
 * @run 71:D3E3
 * WHAT IT DOES: Every other count of OBJ_WORK_D dissolves a strip of tiles by OBJ_WORK_L; at the end becomes kind $93 rising.
 * HOW: DissolveStrips093F.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DissolveThenBurst093F(uint8_t slot)
{
    abi_nz(DEC8(OBJ->work_d[slot]));
    if (NE) {
        lda(RD8(OBJ->work_d[slot]));
        and_a(0x01);
        if (NE) {
            return;
        }
        lda(RD8(OBJ->work_l[slot]));
        clc();
        adc_a(0x13);
        and_a(0x3F);
        WR8(OBJ->work_l[slot], cpu.a);
        call_routine(0xD3FD, 0xD41A);
        return;
    }
    lda(0x93);
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    lda(0xFB);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->vy_lo[slot], 0);
    lda(0x16);
    WR8(OBJ->invuln_timer[slot], cpu.a);
}

void abi_DissolveThenBurst093F(void)
{
    DissolveThenBurst093F(cpu.x);
}

/*
 * @routine 093F:41A DissolveStrips093F
 * @run 71:D41A
 * WHAT IT DOES: Makes the mask ($D429) and applies it to ten tile strips ($D454).
 * HOW: X the strip.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DissolveStrips093F(uint8_t slot)
{
    (void)slot;
    phx();
    call_routine(0xD41E, 0xD429);
    cpu.x = 0;
    do {
        call_routine(0xD422, 0xD454);
        inx();
        cmp(cpu.x, 0x0A);
    } while (CC);
    plx();
}

void abi_DissolveStrips093F(void)
{
    DissolveStrips093F(cpu.x);
}

/*
 * @routine 093F:429 MakeDissolveMask093F
 * @run 71:D429
 * WHAT IT DOES: From A: zTemp00 the line (high nibble), zTemp01-02 all bits but the low nibble's.
 * HOW: A shift loop and inversion.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void MakeDissolveMask093F(uint8_t slot)
{
    (void)slot;
    pha();
    lsr_a();
    lsr_a();
    lsr_a();
    lsr_a();
    WR8(ZP->temp00, cpu.a);
    pla();
    and_a(0x0F);
    WR8(ZP->temp03, cpu.a);
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    WR8(ZP->temp02, 0);
    for (;;) {
        lda(RD8(ZP->temp03));
        if (EQ) {
            break;
        }
        abi_flag(P_C, asl_mem(zTemp01));
        abi_flag(P_C, rol_mem(zTemp02, CS));
        abi_nz(DEC8(ZP->temp03));
    }
    lda(RD8(ZP->temp01));
    eor_a(0xFF);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(ZP->temp02));
    eor_a(0xFF);
    WR8(ZP->temp02, cpu.a);
}

void abi_MakeDissolveMask093F(void)
{
    MakeDissolveMask093F(cpu.x);
}

/*
 * @routine 093F:454 DissolveStripX093F
 * @run 71:D454
 * WHAT IT DOES: Dissolves the strip at the VRAM address the table gives for X.
 * HOW: DissolveWords093F.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DissolveStripX093F(uint8_t slot)
{
    (void)slot;
    txa();
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xD465 + cpu.y)));
    WR8(ZP->bios_c.lo, cpu.a);
    lda(rd8((uint16_t)(0xD466 + cpu.y)));
    WR8(ZP->bios_c.hi, cpu.a);
    call_routine(0xD464, 0xD479);
}

void abi_DissolveStripX093F(void)
{
    DissolveStripX093F(cpu.x);
}

/*
 * @routine 093F:479 DissolveWords093F
 * @run 71:D479
 * WHAT IT DOES: Over $40 words from the address in zBiosCl-Ch, masks the words whose index low bits are zTemp00.
 * HOW: Reads and writes each word through the VDC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DissolveWords093F(uint8_t slot)
{
    (void)slot;
    phy();
    cpu.y = 0;
    do {
        tya();
        and_a(0x03);
        cmp(cpu.a, RD8(ZP->temp00));
        if (EQ) {
            JSR(0xD485, k_select_vram_write());
            lda(RD8(ZP->bios_c.lo));
            WR8(VDC->data_lo, cpu.a);
            lda(RD8(ZP->bios_c.hi));
            WR8(VDC->data_hi, cpu.a);
            JSR(0xD492, k_select_vram_read());
            lda(RD8(ZP->bios_c.lo));
            WR8(VDC->data_lo, cpu.a);
            lda(RD8(ZP->bios_c.hi));
            WR8(VDC->data_hi, cpu.a);
            JSR(0xD49F, k_select_vram_data());
            lda(RD8(VDC->data_lo));
            and_a(RD8(ZP->temp01));
            WR8(VDC->data_lo, cpu.a);
            lda(RD8(VDC->data_hi));
            and_a(RD8(ZP->temp02));
            WR8(VDC->data_hi, cpu.a);
        }
        lda(RD8(ZP->bios_c.lo));
        clc();
        adc_a(0x01);
        WR8(ZP->bios_c.lo, cpu.a);
        lda(RD8(ZP->bios_c.hi));
        adc_a(0x00);
        WR8(ZP->bios_c.hi, cpu.a);
        iny();
        cmp(cpu.y, 0x40);
    } while (CC);
    ply();
}

void abi_DissolveWords093F(void)
{
    DissolveWords093F(cpu.x);
}

/*
 * @routine 093F:4C3 ThrownDagger093F
 * @run 71:D4C3
 * @stop D4DD
 * WHAT IT DOES: State 0 loads it with the facing's speed and $1B upward, and moves.
 * HOW: ApplyVelocity; later states at $D4DF.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ThrownDagger093F(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (NE) {
        run_routine(0xD4DF);
        return;
    }
    JSR(0xD4CB, k_load_record());
    slot = cpu.x;
    ldy(RD8(OBJ->facing_left[slot]));
    lda(rd8((uint16_t)(0xD4DD + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0x1B);
    WR8(OBJ->vy_hi[slot], cpu.a);
    JSR(0xD4DC, k_move());
}

void abi_ThrownDagger093F(void)
{
    ThrownDagger093F(cpu.x);
}

/*
 * @routine 093F:526 ByState093FD526
 * @run 71:D526
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $D538.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState093FD526(uint8_t slot)
{
    jump_by_state(slot, 0xD538);
}

void abi_ByState093FD526(void)
{
    ByState093FD526(cpu.x);
}

/*
 * @routine 093F:53E HopFromFacing093F
 * @run 71:D53E
 * @stop D565
 * WHAT IT DOES: Substate 0 hops by the facing without loop flag bit 1; substate 1 falls ($D569).
 * HOW: A speed table.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void HopFromFacing093F(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (NE) {
        run_routine(0xD569);
        return;
    }
    lda(RD8(OBJ->facing_left[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xD566 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xD565 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(0xFE);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(RD8(OBJ->loop_flags[slot]));
    and_a(0xFD);
    WR8(OBJ->loop_flags[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_HopFromFacing093F(void)
{
    HopFromFacing093F(cpu.x);
}

/*
 * @routine 093F:569 FallStep093F
 * @run 71:D569
 * WHAT IT DOES: Falls without the speed limit and moves.
 * HOW: ApplyGravityUnlimited and a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FallStep093F(uint8_t slot)
{
    (void)slot;
    JSR(0xD56C, k_gravity_unlimited());
    k_move();
}

void abi_FallStep093F(void)
{
    FallStep093F(cpu.x);
}

/*
 * @routine 093F:5B9 ShieldOnHolder093F
 * @run 71:D5B9
 * WHAT IT DOES: State 0 loads it. Then, while the slot below holds kind $38, takes its place, facing, hit box setting and the frame's hit box; else cleared.
 * HOW: Reads of slot X-1.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ShieldOnHolder093F(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        k_load_record();
        return;
    }
    lda(rd8((uint16_t)(0x274F + cpu.x)));
    cmp(cpu.a, 0x38);
    if (NE) {
        k_clear();
        return;
    }
    txa();
    tay();
    dey();
    JSR(0xD5D1, k_copy_place_back());
    slot = cpu.x;
    lda(rd8((uint16_t)(0x2A67 + cpu.x)));
    sec();
    sbc_a(0x01);
    cmp(cpu.a, 0x22);
    if (CS) {
        lda(0x22);
    }
    asl_a();
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xD691 + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(rd8((uint16_t)(0xD692 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    lda(rd8((uint16_t)(0xD693 + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(rd8((uint16_t)(0xD694 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
    lda(rd8((uint16_t)(0x2ABF + cpu.x)));
    WR8(OBJ->facing_left[slot], cpu.a);
    lda(rd8((uint16_t)(0x324F + cpu.x)));
    WR8(OBJ->hitbox_on[slot], cpu.a);
}

void abi_ShieldOnHolder093F(void)
{
    ShieldOnHolder093F(cpu.x);
}

/*
 * @routine 093F:745 ByState093FD745
 * @run 71:D745
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $D757.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState093FD745(uint8_t slot)
{
    jump_by_state(slot, 0xD757);
}

void abi_ByState093FD745(void)
{
    ByState093FD745(cpu.x);
}

/*
 * @routine 093F:75F LoadThenWaitByArgument093F
 * @run 71:D75F
 * WHAT IT DOES: Loads the record and sets the wait ($D766).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LoadThenWaitByArgument093F(uint8_t slot)
{
    (void)slot;
    JSR(0xD762, k_load_record());
    call_routine(0xD765, 0xD766);
}

void abi_LoadThenWaitByArgument093F(void)
{
    LoadThenWaitByArgument093F(cpu.x);
}

/*
 * @routine 093F:766 WaitByArgument093F
 * @run 71:D766
 * WHAT IT DOES: The wait OBJ_START_ARG picks.
 * HOW: A table.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WaitByArgument093F(uint8_t slot)
{
    ldy(RD8(OBJ->start_arg[slot]));
    lda(rd8((uint16_t)(0xD770 + cpu.y)));
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_WaitByArgument093F(void)
{
    WaitByArgument093F(cpu.x);
}

/*
 * @routine 093F:778 WalkLeftOnLanding093F
 * @run 71:D778
 * WHAT IT DOES: On landing (height $0A) goes on walking left at -2.
 * HOW: StopOnLanding.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WalkLeftOnLanding093F(uint8_t slot)
{
    lda(0x0A);
    JSR(0xD77D, k_stop_on_landing());
    if (CC) {
        return;
    }
    slot = cpu.x;
    abi_nz(INC8(OBJ->state[slot]));
    lda(0xFE);
    WR8(OBJ->vx_hi[slot], cpu.a);
}

void abi_WalkLeftOnLanding093F(void)
{
    WalkLeftOnLanding093F(cpu.x);
}

/*
 * @routine 093F:788 WalkAndPause093F
 * @run 71:D788
 * @stop D7D3
 * WHAT IT DOES: Steps the list OBJ_START_ARG picks and walks with the ground, turning at walls ($D810), off the ground to state 1; the wait over, state 3.
 * HOW: Frames $86-$87 do not move.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WalkAndPause093F(uint8_t slot)
{
    lda(RD8(OBJ->start_arg[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xD7D3 + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xD7D4 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    JSR(0xD79A, k_step_frames());
    if (CS) {
        lda(RD8(OBJ->frame[cpu.x]));
        cmp(cpu.a, 0x86);
        if (NE) {
            cmp(cpu.a, 0x87);
            if (NE) {
                lda(0x0A);
                JSR(0xD7AC, k_move_and_land());
                if (CS) {
                    run_routine(0xD810);
                    return;
                }
                and_a(0x01);
                if (EQ) {
                    WR8(OBJ->vx_hi[cpu.x], 0);
                    lda(0x01);
                    WR8(OBJ->state[cpu.x], cpu.a);
                    return;
                }
            }
        }
    }
    slot = cpu.x;
    lda(RD8(OBJ->work_d[slot]));
    cmp(cpu.a, 0xFF);
    if (EQ) {
        return;
    }
    abi_nz(DEC8(OBJ->work_d[slot]));
    if (NE) {
        return;
    }
    call_routine(0xD7CA, 0xD766);
    slot = cpu.x;
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_WalkAndPause093F(void)
{
    WalkAndPause093F(cpu.x);
}

/*
 * @routine 093F:810 TurnBack093F
 * @run 71:D810
 * WHAT IT DOES: Turns the speed and facing about.
 * HOW: NegateVelocityX.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void TurnBack093F(uint8_t slot)
{
    (void)slot;
    JSR(0xD813, k_negate_x());
    lda(RD8(OBJ->facing_left[cpu.x]));
    eor_a(0x01);
    WR8(OBJ->facing_left[cpu.x], cpu.a);
}

void abi_TurnBack093F(void)
{
    TurnBack093F(cpu.x);
}

/*
 * @routine 093F:4DF FlyDagger093F
 * @run 71:D4DF
 * WHAT IT DOES: Steps the list at $D511 and takes the hit box the frame names; cleared at the list's end.
 * HOW: A box table by frame from $2B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlyDagger093F(uint8_t slot)
{
    list_to_temp10(0xD511);
    JSR(0xD4EA, k_step_frames());
    if (EQ) {
        k_clear();
        return;
    }
    slot = cpu.x;
    lda(RD8(OBJ->frame[slot]));
    sec();
    sbc_a(0x2B);
    asl_a();
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xD71D + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(rd8((uint16_t)(0xD71E + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    lda(rd8((uint16_t)(0xD71F + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(rd8((uint16_t)(0xD720 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_FlyDagger093F(void)
{
    FlyDagger093F(cpu.x);
}
