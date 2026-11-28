/*
 * Stage 02's objects, runs CF54-D442: the segmented creature and its shots.
 */
#include "stage_obj_ops.h"
#include "stage02_objects_f.h"
#include "calls_b.h"

/*
 * @routine 051E:754 SetUpSegmentChain051B
 * @run 71:CF54
 * WHAT IT DOES: The eight segments in slots $10-$17. First checks for the hit that starts them ($CF11); in state 0, slot $10
 * copies itself as kind $35 into slots $11-$17, each loads its record, takes its hit box (slot $10), flip, facing,
 * piece number and frame; slot $17 gets $10 health. Later states run $CFE6.
 * HOW: ClearObject and a copy of the place per slot.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SetUpSegmentChain051B(uint8_t slot)
{
    call_routine(0xCF56, 0xCF11);
    slot = cpu.x;
    lda(RD8(OBJ->state[slot]));
    if (NE) {
        run_routine(0xCFE6);
        return;
    }
    cmp(cpu.x, 0x10);
    if (EQ) {
        lda(0x03);
        WR8(ZP->raster_stage, cpu.a);
        phx();
        lda(RD8(OBJ->x_lo[cpu.x]));
        WR8(ZP->temp00, cpu.a);
        lda(RD8(OBJ->x_hi[cpu.x]));
        WR8(ZP->temp01, cpu.a);
        lda(RD8(OBJ->y_lo[cpu.x]));
        WR8(ZP->temp02, cpu.a);
        lda(RD8(OBJ->y_hi[cpu.x]));
        WR8(ZP->temp03, cpu.a);
        do {
            inx();
            JSR(0xCF7F, k_clear());
            lda(0x35);
            WR8(OBJ->kind[cpu.x], cpu.a);
            lda(RD8(ZP->temp00));
            WR8(OBJ->x_lo[cpu.x], cpu.a);
            lda(RD8(ZP->temp01));
            WR8(OBJ->x_hi[cpu.x], cpu.a);
            lda(RD8(ZP->temp02));
            WR8(OBJ->y_lo[cpu.x], cpu.a);
            lda(RD8(ZP->temp03));
            WR8(OBJ->y_hi[cpu.x], cpu.a);
            cmp(cpu.x, 0x17);
        } while (NE);
        plx();
    }
    JSR(0xCFA0, k_load_record());
    cmp(cpu.x, 0x10);
    if (EQ) {
        lda(0x10);
        WR8(OBJ->hitbox_half_w[cpu.x], cpu.a);
        lda(0x16);
        WR8(OBJ->hitbox_half_h[cpu.x], cpu.a);
        lda(0xEC);
        WR8(OBJ->hitbox_dy[cpu.x], cpu.a);
        lda(0x10);
        WR8(OBJ->hitbox_dy[cpu.x], cpu.a);
    }
    lda(0x80);
    WR8(OBJ->sprite_flip[cpu.x], cpu.a);
    call_routine(0xCFC0, 0xCF39);
    slot = cpu.x;
    txa();
    sec();
    sbc_a(0x10);
    tay();
    lda(rd8((uint16_t)(0xCE67 + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
    lda(0x00);
    WR8(OBJ->work_g[slot], cpu.a);
    lda(0x01);
    WR8(OBJ->work_f[slot], cpu.a);
    lda(rd8((uint16_t)(0xCE6F + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
    cmp(cpu.x, 0x17);
    if (EQ) {
        lda(0x10);
        WR8(OBJ->health[slot], cpu.a);
    }
}

void abi_SetUpSegmentChain051B(void)
{
    SetUpSegmentChain051B(cpu.x);
}

/*
 * @routine 051E:7E6 SegmentChainFollow051B
 * @run 71:CFE6
 * WHAT IT DOES: State 1: the last segment runs the head's moves ($D0A8). Every segment moves, then places the next segment
 * at the cosine and sine of its angle from it, less the camera's movement, and swings its angle between $A0 and $F0
 * either way. Later states go to $D1BF.
 * HOW: SetVelocityFromAngle into the next slot, a 16-bit swing.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SegmentChainFollow051B(uint8_t slot)
{
    dec_a();
    if (NE) {
        run_routine(0xD1BF);
        return;
    }
    cmp(cpu.x, 0x17);
    if (EQ) {
        call_routine(0xCFF3, 0xD0A8);
    }
    call_routine(0xCFF6, 0xCEC1);
    JSR(0xCFF9, k_move_x());
    JSR(0xCFFC, k_move_y());
    slot = cpu.x;
    lda(0x00);
    WR8(ZP->temp00, cpu.a);
    lda(0x08);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    inx();
    JSR(0xD00B, k_velocity_from_angle());
    dex();
    slot = cpu.x;
    {
        unsigned next = slot + 1u;

        WR8(ZP->temp00, 0);
        WR8(ZP->temp01, 0);
        lda(RD8(SCROLLE->speed_x[0]));
        if (MI) {
            abi_nz(DEC8(ZP->temp00));
        }
        lda(RD8(SCROLLE->speed_y[0]));
        if (MI) {
            abi_nz(DEC8(ZP->temp01));
        }
        lda(RD8(OBJ->x_frac[slot]));
        WR8(OBJ->x_frac[next], cpu.a);
        lda(RD8(OBJ->x_lo[slot]));
        clc();
        adc_a(RD8(SCROLLE->speed_x[0]));
        WR8(OBJ->x_lo[next], cpu.a);
        lda(RD8(OBJ->x_hi[slot]));
        adc_a(RD8(ZP->temp00));
        WR8(OBJ->x_hi[next], cpu.a);
        lda(RD8(OBJ->y_frac[slot]));
        WR8(OBJ->y_frac[next], cpu.a);
        lda(RD8(OBJ->y_lo[slot]));
        clc();
        adc_a(RD8(SCROLLE->speed_y[0]));
        WR8(OBJ->y_lo[next], cpu.a);
        lda(RD8(OBJ->y_hi[slot]));
        adc_a(RD8(ZP->temp01));
        WR8(OBJ->y_hi[next], cpu.a);
    }
    lda(RD8(OBJ->work_f[slot]));
    if (PL) {
        lda(RD8(OBJ->work_g[slot]));
        clc();
        adc_a(RD8(OBJ->work_d[slot]));
        WR8(OBJ->work_d[slot], cpu.a);
        lda(RD8(OBJ->work_f[slot]));
        clc();
        adc_a(RD8(OBJ->work_e[slot]));
        WR8(OBJ->work_e[slot], cpu.a);
        cmp(cpu.a, 0xF0);
        if (CC) {
            return;
        }
        lda(0xF0);
    } else {
        lda(RD8(OBJ->work_g[slot]));
        clc();
        adc_a(RD8(OBJ->work_d[slot]));
        WR8(OBJ->work_d[slot], cpu.a);
        lda(RD8(OBJ->work_f[slot]));
        clc();
        adc_a(RD8(OBJ->work_e[slot]));
        WR8(OBJ->work_e[slot], cpu.a);
        cmp(cpu.a, 0xA0);
        if (CS) {
            return;
        }
        lda(0xA0);
    }
    WR8(OBJ->work_e[slot], cpu.a);
    WR8(OBJ->work_d[slot], 0);
    lda(RD8(OBJ->work_f[slot]));
    eor_a(0xFF);
    WR8(OBJ->work_f[slot], cpu.a);
    lda(RD8(OBJ->work_g[slot]));
    eor_a(0xFF);
    inc_a();
    WR8(OBJ->work_g[slot], cpu.a);
    if (NE) {
        return;
    }
    abi_nz(INC8(OBJ->work_f[slot]));
}

void abi_SegmentChainFollow051B(void)
{
    SegmentChainFollow051B(cpu.x);
}

/*
 * @routine 051F:0A8 HeadAttacks051B
 * @run 71:D0A8
 * WHAT IT DOES: The head. Substate 0: its box ($D195) and list; when the list ends throws ($D12A); under 8 health moves on with
 * list OBJ_FALL_LO 1. Substate 1: steps that list and turns the swings about ($D105). Substate 2: its wider box
 * ($D1AA) and list, throwing again ($D158) when the animation timer is 7.
 * HOW: Branches by substate.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void HeadAttacks051B(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (EQ) {
        call_routine(0xD0C3, 0xD195);
        call_routine(0xD0C5, 0xD0F2);
        if (EQ) {
            call_routine(0xD0CA, 0xD12A);
        }
        slot = cpu.x;
        lda(RD8(OBJ->health[slot]));
        cmp(cpu.a, 0x08);
        if (CS) {
            return;
        }
        abi_nz(INC8(OBJ->work_h[slot]));
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    dec_a();
    if (EQ) {
        call_routine(0xD0E1, 0xD0F2);
        if (NE) {
            return;
        }
        slot = cpu.x;
        abi_nz(INC8(OBJ->work_h[slot]));
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        call_routine(0xD0EE, 0xD105);
        abi_nz(INC8(OBJ->substate[cpu.x]));
        return;
    }
    call_routine(0xD0B3, 0xD1AA);
    call_routine(0xD0B5, 0xD0F2);
    slot = cpu.x;
    lda(RD8(OBJ->anim_timer[slot]));
    cmp(cpu.a, 0x07);
    if (EQ) {
        call_routine(0xD0BF, 0xD158);
    }
}

void abi_HeadAttacks051B(void)
{
    HeadAttacks051B(cpu.x);
}

/*
 * @routine 051F:0F2 StepHeadList051B
 * @run 71:D0F2
 * WHAT IT DOES: Steps the list OBJ_FALL_LO picks from the table at $CEBB.
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StepHeadList051B(uint8_t slot)
{
    lda(RD8(OBJ->work_h[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xCEBB + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xCEBC + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    JSR(0xD104, k_step_frames());
}

void abi_StepHeadList051B(void)
{
    StepHeadList051B(cpu.x);
}

/*
 * @routine 051F:105 SpeedSwings051B
 * @run 71:D105
 * WHAT IT DOES: Gives slots $10-$17 a new swing speed: $01C0 for those swinging forward, $FE40 for those swinging back.
 * HOW: A loop over the eight.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SpeedSwings051B(uint8_t slot)
{
    (void)slot;
    phx();
    ldx(0x10);
    do {
        lda(RD8(OBJ->work_f[cpu.x]));
        if (PL) {
            lda(0xC0);
            WR8(OBJ->work_g[cpu.x], cpu.a);
            lda(0x01);
            WR8(OBJ->work_f[cpu.x], cpu.a);
        } else {
            lda(0x40);
            WR8(OBJ->work_g[cpu.x], cpu.a);
            lda(0xFE);
            WR8(OBJ->work_f[cpu.x], cpu.a);
        }
        inx();
        cmp(cpu.x, 0x18);
    } while (NE);
    plx();
}

void abi_SpeedSwings051B(void)
{
    SpeedSwings051B(cpu.x);
}

/*
 * @routine 051F:12A ThrowFromMouth051B
 * @run 71:D12A
 * WHAT IT DOES: Starts a kind $3A object $28 ahead and 6 above, in a free slot from $30 on screen.
 * HOW: FindFreeSlotFrom30, CopyFacingToY051B, PlaceAheadInY051B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ThrowFromMouth051B(uint8_t slot)
{
    JSR(0xD12D, k_free_from30());
    if (CC) {
        return;
    }
    call_routine(0xD132, 0xCEDF);
    lda(0x28);
    WR8(ZP->temp00, cpu.a);
    lda(0x00);
    WR8(ZP->temp01, cpu.a);
    call_routine(0xD13D, 0xCEE6);
    if (NE) {
        return;
    }
    slot = cpu.x;
    lda(RD8(OBJ->y_lo[slot]));
    clc();
    adc_a(0xFA);
    WR8(OBJ->y_lo[cpu.y], cpu.a);
    lda(RD8(OBJ->y_hi[slot]));
    adc_a(0xFF);
    WR8(OBJ->y_hi[cpu.y], cpu.a);
    if (NE) {
        return;
    }
    lda(0x3A);
    WR8(OBJ->kind[cpu.y], cpu.a);
}

void abi_ThrowFromMouth051B(void)
{
    ThrowFromMouth051B(cpu.x);
}

/*
 * @routine 051F:158 ThrowFromSlotPool051B
 * @run 71:D158
 * WHAT IT DOES: Takes the first empty slot of $19-$28, clears it and starts a kind $3B object $0C ahead, on screen.
 * HOW: As ThrowFromMouth051B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ThrowFromSlotPool051B(uint8_t slot)
{
    ldy(0x19);
    for (;;) {
        lda(RD8(OBJ->kind[cpu.y]));
        if (EQ) {
            break;
        }
        iny();
        cmp(cpu.y, 0x29);
        if (EQ) {
            return;
        }
    }
    phx();
    sxy();
    JSR(0xD16A, k_clear());
    sxy();
    plx();
    call_routine(0xD16F, 0xCEDF);
    lda(0x0C);
    WR8(ZP->temp00, cpu.a);
    lda(0x00);
    WR8(ZP->temp01, cpu.a);
    call_routine(0xD17A, 0xCEE6);
    if (NE) {
        return;
    }
    slot = cpu.x;
    lda(RD8(OBJ->y_lo[slot]));
    clc();
    adc_a(0x00);
    WR8(OBJ->y_lo[cpu.y], cpu.a);
    lda(RD8(OBJ->y_hi[slot]));
    adc_a(0x00);
    WR8(OBJ->y_hi[cpu.y], cpu.a);
    if (NE) {
        return;
    }
    lda(0x3B);
    WR8(OBJ->kind[cpu.y], cpu.a);
}

void abi_ThrowFromSlotPool051B(void)
{
    ThrowFromSlotPool051B(cpu.x);
}

/*
 * @routine 051F:195 SetHeadHitBox051B
 * @run 71:D195
 * WHAT IT DOES: The head's hit box: half size $14 by $0C, offset $14, 0.
 * HOW: Four stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SetHeadHitBox051B(uint8_t slot)
{
    lda(0x14);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(0x0C);
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
    lda(0x14);
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(0x00);
    WR8(OBJ->hitbox_dy[slot], cpu.a);
}

void abi_SetHeadHitBox051B(void)
{
    SetHeadHitBox051B(cpu.x);
}

/*
 * @routine 051F:1AA SetHeadHitBoxOpen051B
 * @run 71:D1AA
 * WHAT IT DOES: The head's open-mouth hit box: half size $14 by $14, offset $14, 0.
 * HOW: Four stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SetHeadHitBoxOpen051B(uint8_t slot)
{
    lda(0x14);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(0x14);
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
    lda(0x14);
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(0x00);
    WR8(OBJ->hitbox_dy[slot], cpu.a);
}

void abi_SetHeadHitBoxOpen051B(void)
{
    SetHeadHitBoxOpen051B(cpu.x);
}

/*
 * @routine 051F:1BF SegmentDies051B
 * @run 71:D1BF
 * WHAT IT DOES: Substate 0: no hit box or loop flag bit 0, a wait by segment, attribute bit 4. Then, the wait over, slot $10
 * throws the pieces ($D239); another (on screen with its sound) is cleared but for its place and becomes the
 * segment's debris kind.
 * HOW: Tables by segment; the place kept on the stack across ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SegmentDies051B(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (EQ) {
        WR8(OBJ->hitbox_on[slot], 0);
        lda(RD8(OBJ->loop_flags[slot]));
        and_a(0xFE);
        WR8(OBJ->loop_flags[slot], cpu.a);
        txa();
        sec();
        sbc_a(0x10);
        tay();
        lda(rd8((uint16_t)(0xD2A6 + cpu.y)));
        WR8(OBJ->anim_timer[slot], cpu.a);
        lda(0x10);
        ora_a(RD8(OBJ->sprite_attr[slot]));
        WR8(OBJ->sprite_attr[slot], cpu.a);
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    lda(RD8(OBJ->anim_timer[slot]));
    if (NE) {
        abi_nz(DEC8(OBJ->anim_timer[slot]));
        if (NE) {
            return;
        }
    }
    cmp(cpu.x, 0x10);
    if (EQ) {
        run_routine(0xD239);
        return;
    }
    lda(RD8(OBJ->x_hi[slot]));
    if (EQ) {
        txa();
        sec();
        sbc_a(0x10);
        tay();
        lda(rd8((uint16_t)(0xD2D6 + cpu.y)));
        JSR(0xD205, k_sound());
        slot = cpu.x;
    }
    lda(RD8(OBJ->x_lo[slot]));
    pha();
    lda(RD8(OBJ->x_hi[slot]));
    pha();
    lda(RD8(OBJ->y_lo[slot]));
    pha();
    lda(RD8(OBJ->y_hi[slot]));
    pha();
    JSR(0xD218, k_clear());
    slot = cpu.x;
    pla();
    WR8(OBJ->y_hi[slot], cpu.a);
    pla();
    WR8(OBJ->y_lo[slot], cpu.a);
    pla();
    WR8(OBJ->x_hi[slot], cpu.a);
    pla();
    WR8(OBJ->x_lo[slot], cpu.a);
    txa();
    sec();
    sbc_a(0x10);
    tay();
    lda(rd8((uint16_t)(0xD2AE + cpu.y)));
    WR8(OBJ->kind[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->sprite_flip[slot], cpu.a);
}

void abi_SegmentDies051B(void)
{
    SegmentDies051B(cpu.x);
}

/*
 * @routine 051F:239 ThrowRemains051B
 * @run 71:D239
 * WHAT IT DOES: Every fourth frame starts a kind $9E piece at one of eight places around it, sound $15; after eight, clears itself.
 * HOW: A table of offset words indexed by OBJ_WORK_K.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ThrowRemains051B(uint8_t slot)
{
    lda(RD8(OBJ->work_k[slot]));
    cmp(cpu.a, 0x08);
    if (EQ) {
        k_clear();
        return;
    }
    {
        uint8_t counter = RD8(ZP->wave_counter);

        abi_flag(P_Z, (counter & 0x03) == 0);
        abi_flag(P_N, (counter & 0x80) != 0);
        abi_flag(P_V, (counter & 0x40) != 0);
    }
    if (NE) {
        return;
    }
    JSR(0xD24B, k_free_from30());
    if (CC) {
        return;
    }
    slot = cpu.x;
    phx();
    lda(0x9E);
    WR8(OBJ->kind[cpu.y], cpu.a);
    lda(0x80);
    WR8(OBJ->sprite_flip[cpu.y], cpu.a);
    lda(RD8(OBJ->work_k[slot]));
    asl_a();
    asl_a();
    tax();
    lda(rd8((uint16_t)(0xD2B6 + cpu.x)));
    WR8(OBJ->x_lo[cpu.y], cpu.a);
    lda(rd8((uint16_t)(0xD2B7 + cpu.x)));
    WR8(OBJ->x_hi[cpu.y], cpu.a);
    lda(rd8((uint16_t)(0xD2B8 + cpu.x)));
    WR8(OBJ->y_lo[cpu.y], cpu.a);
    lda(rd8((uint16_t)(0xD2B9 + cpu.x)));
    WR8(OBJ->y_hi[cpu.y], cpu.a);
    plx();
    lda(RD8(OBJ->x_lo[slot]));
    clc();
    adc_a(RD8(OBJ->x_lo[cpu.y]));
    WR8(OBJ->x_lo[cpu.y], cpu.a);
    lda(RD8(OBJ->x_hi[slot]));
    adc_a(RD8(OBJ->x_hi[cpu.y]));
    WR8(OBJ->x_hi[cpu.y], cpu.a);
    lda(RD8(OBJ->y_lo[slot]));
    clc();
    adc_a(RD8(OBJ->y_lo[cpu.y]));
    WR8(OBJ->y_lo[cpu.y], cpu.a);
    lda(RD8(OBJ->y_hi[slot]));
    adc_a(RD8(OBJ->y_hi[cpu.y]));
    WR8(OBJ->y_hi[cpu.y], cpu.a);
    lda(0x15);
    JSR(0xD2A2, k_sound());
    abi_nz(INC8(OBJ->work_k[cpu.x]));
}

void abi_ThrowRemains051B(void)
{
    ThrowRemains051B(cpu.x);
}

/*
 * @routine 051F:2DE BouncingShot051B
 * @run 71:D2DE
 * WHAT IT DOES: State 0 loads it flying at 4 the facing way. State 1, at terrain ($D367), turns it to rise away slowly; else it
 * flies falling by 9 and is cleared off the first screen. Later it falls by $18 ($D34E).
 * HOW: Branches by state.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BouncingShot051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xD2E6, k_load_record());
        slot = cpu.x;
        lda(0x80);
        WR8(OBJ->sprite_flip[slot], cpu.a);
        lda(0x19);
        WR8(OBJ->frame[slot], cpu.a);
        lda(0x00);
        WR8(OBJ->vx_lo[slot], cpu.a);
        lda(0x04);
        WR8(OBJ->vx_hi[slot], cpu.a);
        call_routine(0xD2FD, 0xCEC1);
        lda(0x00);
        WR8(OBJ->vy_lo[cpu.x], cpu.a);
        lda(0x00);
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        return;
    }
    dec_a();
    if (NE) {
        run_routine(0xD34E);
        return;
    }
    call_routine(0xD30E, 0xD367);
    slot = cpu.x;
    if (CS) {
        lda(0x00);
        WR8(OBJ->vx_lo[slot], cpu.a);
        lda(0xFF);
        WR8(OBJ->vx_hi[slot], cpu.a);
        call_routine(0xD31D, 0xCEC1);
        slot = cpu.x;
        lda(0x00);
        WR8(OBJ->vy_lo[slot], cpu.a);
        lda(0xFF);
        WR8(OBJ->vy_hi[slot], cpu.a);
        abi_nz(INC8(OBJ->state[slot]));
        return;
    }
    JSR(0xD32E, k_move_x());
    JSR(0xD331, k_move_y());
    slot = cpu.x;
    lda(RD8(OBJ->vy_lo[slot]));
    clc();
    adc_a(0x09);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    adc_a(0x00);
    WR8(OBJ->vy_hi[slot], cpu.a);
    run_routine(0xD342);
}

void abi_BouncingShot051B(void)
{
    BouncingShot051B(cpu.x);
}

/*
 * @routine 051F:342 ClearOffFirstScreen051B
 * @run 71:D342
 * WHAT IT DOES: Clears the object once X or Y is off the first screen.
 * HOW: The high bytes together.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ClearOffFirstScreen051B(uint8_t slot)
{
    lda(RD8(OBJ->y_hi[slot]));
    ora_a(RD8(OBJ->x_hi[slot]));
    if (NE) {
        k_clear();
    }
}

void abi_ClearOffFirstScreen051B(void)
{
    ClearOffFirstScreen051B(cpu.x);
}

/*
 * @routine 051F:34E FallingShot051B
 * @run 71:D34E
 * WHAT IT DOES: Moves, falls by $18 and is cleared off the first screen.
 * HOW: Into ClearOffFirstScreen051B.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FallingShot051B(uint8_t slot)
{
    JSR(0xD351, k_move_x());
    JSR(0xD354, k_move_y());
    slot = cpu.x;
    lda(RD8(OBJ->vy_lo[slot]));
    clc();
    adc_a(0x18);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    adc_a(0x00);
    WR8(OBJ->vy_hi[slot], cpu.a);
    run_routine(0xD342);
}

void abi_FallingShot051B(void)
{
    FallingShot051B(cpu.x);
}

/*
 * @routine 051F:367 TerrainAtPlace051B
 * @run 71:D367
 * WHAT IT DOES: Carry set when the terrain at the object's place has bit 0 or 1.
 * HOW: TerrainTileRead at X zTemp00-01, Y zTemp02-03.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void TerrainAtPlace051B(uint8_t slot)
{
    lda(RD8(OBJ->x_lo[slot]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->x_hi[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->y_lo[slot]));
    WR8(ZP->temp02, cpu.a);
    lda(RD8(OBJ->y_hi[slot]));
    WR8(ZP->temp03, cpu.a);
    call_routine(0xD37E, 0x967F);
    and_a(0x03);
    if (EQ) {
        clc();
        return;
    }
    sec();
}

void abi_TerrainAtPlace051B(void)
{
    TerrainAtPlace051B(cpu.x);
}

/*
 * @routine 051F:38E WobblingShot051B
 * @run 71:D38E
 * WHAT IT DOES: A shot at 1 the facing way, drifting up or down by slot and wobbling on a sine; cleared at terrain or off screen.
 * HOW: Tables by slot's low bits; ScaleBySine of a stepping angle.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WobblingShot051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xD396, k_load_record());
        slot = cpu.x;
        lda(0x80);
        WR8(OBJ->sprite_flip[slot], cpu.a);
        lda(0x00);
        WR8(OBJ->vx_lo[slot], cpu.a);
        lda(0x01);
        WR8(OBJ->vx_hi[slot], cpu.a);
        call_routine(0xD3A8, 0xCEC1);
        slot = cpu.x;
        txa();
        and_a(0x03);
        asl_a();
        tay();
        lda(rd8((uint16_t)(0xD386 + cpu.y)));
        WR8(OBJ->work_d[slot], cpu.a);
        lda(rd8((uint16_t)(0xD387 + cpu.y)));
        WR8(OBJ->work_e[slot], cpu.a);
        JSR(0xD3BC, k_random());
        WR8(OBJ->work_g[cpu.x], cpu.a);
        return;
    }
    list_to_temp10(0xCE9F);
    JSR(0xD3CB, k_step_frames());
    JSR(0xD3CE, k_move_x());
    slot = cpu.x;
    WR8(ZP->temp00, 0);
    lda(RD8(OBJ->work_e[slot]));
    if (MI) {
        abi_nz(DEC8(ZP->temp00));
    }
    lda(RD8(OBJ->y_frac[slot]));
    clc();
    adc_a(RD8(OBJ->work_d[slot]));
    WR8(OBJ->y_frac[slot], cpu.a);
    lda(RD8(OBJ->y_lo[slot]));
    adc_a(RD8(OBJ->work_e[slot]));
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(RD8(ZP->temp00));
    adc_a(RD8(OBJ->y_hi[slot]));
    WR8(OBJ->y_hi[slot], cpu.a);
    JSR(0xD3F5, k_move_y());
    slot = cpu.x;
    lda(0x30);
    WR8(ZP->temp00, cpu.a);
    lda(0x00);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_g[slot]));
    JSR(0xD403, k_sine());
    slot = cpu.x;
    lda(RD8(ZP->plane_rows_left));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(RD8(ZP->sprite_flip_mask));
    WR8(OBJ->vy_lo[slot], cpu.a);
    JSR(0xD410, k_move_y());
    slot = cpu.x;
    lda(RD8(OBJ->work_f[slot]));
    clc();
    adc_a(0x80);
    WR8(OBJ->work_f[slot], cpu.a);
    lda(RD8(OBJ->work_g[slot]));
    clc();
    adc_a(0x02);
    WR8(OBJ->work_g[slot], cpu.a);
    lda(RD8(OBJ->x_lo[slot]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->x_hi[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->y_lo[slot]));
    WR8(ZP->temp02, cpu.a);
    lda(RD8(OBJ->y_hi[slot]));
    WR8(ZP->temp03, cpu.a);
    call_routine(0xD439, 0x967F);
    and_a(0x03);
    if (EQ) {
        run_routine(0xD342);
        return;
    }
    k_clear();
}

void abi_WobblingShot051B(void)
{
    WobblingShot051B(cpu.x);
}
