/*
 * Stage 02's objects, runs C82A-CF53.
 */
#include "stage_obj_ops.h"
#include "stage02_objects_e.h"
#include "calls_b.h"

/*
 * @routine 051E:02A LoadPieceBySlot051B
 * @run 71:C82A
 * WHAT IT DOES: State 0 loads the record as sprite set 6, with the frame and speeds the tables give for this slot (from $10);
 * later it falls ($C809).
 * HOW: Tables by slot less $10.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LoadPieceBySlot051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (NE) {
        run_routine(0xC809);
        return;
    }
    JSR(0xC832, k_load_record());
    slot = cpu.x;
    lda(0x06);
    WR8(OBJ->sprite_set[slot], cpu.a);
    txa();
    sec();
    sbc_a(0x10);
    tay();
    lda(rd8((uint16_t)(0xC85E + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
    tya();
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xC864 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xC865 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC870 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xC871 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
}

void abi_LoadPieceBySlot051B(void)
{
    LoadPieceBySlot051B(cpu.x);
}

/*
 * @routine 051E:07C LoadPictureFrame0C_051B
 * @run 71:C87C
 * WHAT IT DOES: State 0 loads the record as sprite set 6 frame $0C; later does nothing.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LoadPictureFrame0C_051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (NE) {
        return;
    }
    JSR(0xC884, k_load_record());
    lda(0x06);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    lda(0x0C);
    WR8(OBJ->frame[cpu.x], cpu.a);
}

void abi_LoadPictureFrame0C_051B(void)
{
    LoadPictureFrame0C_051B(cpu.x);
}

/*
 * @routine 051E:101 EmitBubbles051B
 * @run 71:C901
 * WHAT IT DOES: State 0 loads the record and picks the first wait ($C909); later counts and emits ($C915).
 * HOW: A branch by state.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void EmitBubbles051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (NE) {
        run_routine(0xC915);
        return;
    }
    JSR(0xC909, k_load_record());
    run_routine(0xC909);
}

void abi_EmitBubbles051B(void)
{
    EmitBubbles051B(cpu.x);
}

/*
 * @routine 051E:17E BubbleRiseAndSway051B
 * @run 71:C97E
 * WHAT IT DOES: A bubble. State 0 loads it as sprite set 6 with a rise, drift and swing from tables and a life of $80-$FF. Then
 * steps its list, rises, drifts left by its drift word and swings on the cosine; on the life's end it flickers
 * (attribute bit 4) for $10 frames and goes. Off the first screen it goes; it also moves against the camera's
 * horizontal speed times 64.
 * HOW: Tables by start argument and count; two cosine speeds.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BubbleRiseAndSway051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xC986, k_load_record());
        slot = cpu.x;
        lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
        ldy(RD8(OBJ->start_arg[slot]));
        lda(RD8(OBJ->work_e[slot]));
        asl_a();
        asl_a();
        clc();
        adc_a(rd8((uint16_t)(0xC97C + cpu.y)));
        tay();
        lda(rd8((uint16_t)(0xCAE4 + cpu.y)));
        WR8(OBJ->vy_lo[slot], cpu.a);
        lda(rd8((uint16_t)(0xCAE5 + cpu.y)));
        WR8(OBJ->vy_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xCAE6 + cpu.y)));
        WR8(OBJ->work_f[slot], cpu.a);
        lda(rd8((uint16_t)(0xCAE7 + cpu.y)));
        WR8(OBJ->work_g[slot], cpu.a);
        lda(RD8(OBJ->work_e[slot]));
        asl_a();
        tay();
        lda(rd8((uint16_t)(0xCB04 + cpu.y)));
        WR8(OBJ->work_j[slot], cpu.a);
        lda(rd8((uint16_t)(0xCB05 + cpu.y)));
        WR8(OBJ->work_k[slot], cpu.a);
        JSR(0xC9C4, k_random());
        and_a(0x7F);
        clc();
        adc_a(0x80);
        WR8(OBJ->work_d[cpu.x], cpu.a);
        return;
    }
    abi_nz(DEC8(OBJ->work_d[slot]));
    if (EQ) {
        uint8_t attr = RD8(OBJ->sprite_attr[slot]);

        abi_flag(P_Z, (attr & 0x10) == 0);
        abi_flag(P_N, (attr & 0x80) != 0);
        abi_flag(P_V, (attr & 0x40) != 0);
        if (NE) {
            k_clear();
            return;
        }
        lda(0x10);
        WR8(OBJ->work_d[slot], cpu.a);
        lda(0x10);
        ora_a(RD8(OBJ->sprite_attr[slot]));
        WR8(OBJ->sprite_attr[slot], cpu.a);
    }
    list_to_temp10(0xCAD9);
    JSR(0xC9F3, k_step_frames());
    JSR(0xC9F6, k_move_y());
    slot = cpu.x;
    lda(RD8(OBJ->x_frac[slot]));
    clc();
    adc_a(RD8(OBJ->work_f[slot]));
    WR8(OBJ->x_frac[slot], cpu.a);
    lda(RD8(OBJ->x_lo[slot]));
    adc_a(RD8(OBJ->work_g[slot]));
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(0xFF);
    adc_a(RD8(OBJ->x_hi[slot]));
    WR8(OBJ->x_hi[slot], cpu.a);
    JSR(0xCA14, k_move_x());
    slot = cpu.x;
    lda(RD8(OBJ->work_j[slot]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_k[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_i[slot]));
    JSR(0xCA24, k_cosine());
    slot = cpu.x;
    lda(RD8(ZP->plane_rows_left));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(RD8(ZP->sprite_flip_mask));
    WR8(OBJ->vx_lo[slot], cpu.a);
    JSR(0xCA31, k_move_x());
    slot = cpu.x;
    lda(RD8(OBJ->work_h[slot]));
    clc();
    adc_a(0x00);
    WR8(OBJ->work_h[slot], cpu.a);
    lda(RD8(OBJ->work_i[slot]));
    adc_a(0x02);
    WR8(OBJ->work_i[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    if (EQ) {
        lda(RD8(OBJ->y_hi[slot]));
        and_a(0x80);
        ora_a(RD8(OBJ->x_hi[slot]));
        and_a(0x80);
        if (NE) {
            k_clear();
            return;
        }
    } else {
        lda(RD8(OBJ->y_hi[slot]));
        and_a(0x01);
        ora_a(RD8(OBJ->x_hi[slot]));
        and_a(0x80);
        if (NE) {
            k_clear();
            return;
        }
    }
    WR8(ZP->temp00, 0);
    WR8(ZP->temp02, 0);
    lda(RD8(SCROLLE->speed_x[0]));
    WR8(ZP->temp01, cpu.a);
    if (EQ) {
        return;
    }
    if (MI) {
        abi_nz(DEC8(ZP->temp02));
    }
    for (int i = 0; i < 2; i++) {
        lda(RD8(ZP->temp02));
        lsr_a();
        abi_flag(P_C, ROR8(ZP->temp01, CS));
        abi_flag(P_C, ROR8(ZP->temp00, CS));
    }
    lda(RD8(OBJ->x_frac[slot]));
    sec();
    sbc_a(RD8(ZP->temp00));
    WR8(OBJ->x_frac[slot], cpu.a);
    lda(RD8(OBJ->x_lo[slot]));
    sbc_a(RD8(ZP->temp01));
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(RD8(OBJ->x_hi[slot]));
    sbc_a(RD8(ZP->temp02));
    WR8(OBJ->x_hi[slot], cpu.a);
}

void abi_BubbleRiseAndSway051B(void)
{
    BubbleRiseAndSway051B(cpu.x);
}

/*
 * @routine 051E:30C PieceObjectInit051B
 * @run 71:CB0C
 * WHAT IT DOES: State 0: unless zField8F has the bit OBJ_START_ARG picks (then the object is cleared), loads the record and draws
 * the tile script its 16-byte piece record names, with the record's hit box. Later: breaks by hits ($CB4F).
 * HOW: As the stage's other piece objects.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void PieceObjectInit051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (NE) {
        run_routine(0xCB4F);
        return;
    }
    ldy(RD8(OBJ->start_arg[slot]));
    call_routine(0xCB17, 0xCB44);
    and_a(RD8(ZP->field8f));
    if (NE) {
        k_clear();
        return;
    }
    JSR(0xCB21, k_load_record());
    slot = cpu.x;
    lda(RD8(OBJ->start_arg[slot]));
    asl_a();
    asl_a();
    asl_a();
    asl_a();
    WR8(OBJ->work_e[slot], cpu.a);
    tay();
    lda(rd8((uint16_t)(PieceRecords051A + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(rd8((uint16_t)(PieceRecords051A + 1 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
    lda(rd8((uint16_t)(PieceRecords051A + 2 + cpu.y)));
    clc();
    adc_a(RD8(OBJ->work_d[slot]));
    cpu.y = 0;
    JSR(0xCB43, k_draw_tiles());
}

void abi_PieceObjectInit051B(void)
{
    PieceObjectInit051B(cpu.x);
}

/*
 * @routine 051E:344 BitOfY051B
 * @run 71:CB44
 * WHAT IT DOES: A with bit Y set.
 * HOW: A shift loop.
 * WAITS: no.
 * Entry: Y the bit. Exit: A the mask, Y 0, C and N, Z from the last compare.
 */
void BitOfY051B(uint8_t slot)
{
    (void)slot;
    lda(0x01);
    for (;;) {
        cmp(cpu.y, 0x00);
        if (EQ) {
            return;
        }
        asl_a();
        dey();
    }
}

void abi_BitOfY051B(void)
{
    BitOfY051B(cpu.x);
}

/*
 * @routine 051E:34F BreakPieceByHits051B
 * @run 71:CB4F
 * WHAT IT DOES: Redraws the piece's current tile script; a hit that the piece's table takes draws the next one, with sound $7F
 * and debris, and moves by the piece's speeds. After the record's number of hits it sets its zField8F bit and becomes
 * the record's kind, at rest.
 * HOW: Piece records of 16 bytes by OBJ_PIECE_PLACE, a hit-type table at $B0DE.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BreakPieceByHits051B(uint8_t slot)
{
    lda(RD8(OBJ->x_hi[slot]));
    ora_a(RD8(OBJ->y_hi[slot]));
    if (NE) {
        ldy(RD8(OBJ->work_e[slot]));
        lda(rd8((uint16_t)(0xB081 + cpu.y)));
        clc();
        adc_a(RD8(OBJ->work_d[slot]));
        cpu.y = 0;
        JSR(0xCB65, k_draw_tiles());
        slot = cpu.x;
    }
    lda(RD8(OBJ->hit_taken[slot]));
    WR8(ZP->temp00, cpu.a);
    ldy(RD8(OBJ->work_e[slot]));
    lda(rd8((uint16_t)(0xB082 + cpu.y)));
    asl_a();
    asl_a();
    asl_a();
    asl_a();
    clc();
    adc_a(RD8(ZP->temp00));
    tay();
    lda(rd8((uint16_t)(0xB0DE + cpu.y)));
    if (EQ) {
        return;
    }
    lda(0x18);
    WR8(OBJ->invuln_timer[slot], cpu.a);
    ldy(RD8(OBJ->work_e[slot]));
    lda(rd8((uint16_t)(0xB081 + cpu.y)));
    clc();
    adc_a(RD8(OBJ->work_d[slot]));
    inc_a();
    cpu.y = 0;
    JSR(0xCB91, k_draw_tiles());
    lda(0x7F);
    JSR(0xCB96, k_sound());
    lda(0x01);
    call_routine(0xCB9B, 0xA90B);
    slot = cpu.x;
    ldy(RD8(OBJ->work_e[slot]));
    abi_nz(INC8(OBJ->work_d[slot]));
    lda(RD8(OBJ->work_d[slot]));
    cmp(cpu.a, rd8((uint16_t)(0xB07E + cpu.y)));
    if (CC) {
        lda(RD8(OBJ->work_d[slot]));
        dec_a();
        asl_a();
        clc();
        adc_a(RD8(OBJ->work_e[slot]));
        tay();
        lda(rd8((uint16_t)(0xB084 + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xB085 + cpu.y)));
        WR8(OBJ->vy_hi[slot], cpu.a);
        JSR(0xCBC2, k_move());
        return;
    }
    ldy(RD8(OBJ->start_arg[slot]));
    call_routine(0xCBC9, 0xCB44);
    ora_a(RD8(ZP->field8f));
    WR8(ZP->field8f, cpu.a);
    ldy(RD8(OBJ->work_e[slot]));
    lda(rd8((uint16_t)(0xB083 + cpu.y)));
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
}

void abi_BreakPieceByHits051B(void)
{
    BreakPieceByHits051B(cpu.x);
}

/*
 * @routine 051E:6C1 NegateSpeedIfFacingLeft051B
 * @run 71:CEC1
 * WHAT IT DOES: Facing left, negates the X speed.
 * HOW: A 16-bit negation.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void NegateSpeedIfFacingLeft051B(uint8_t slot)
{
    lda(RD8(OBJ->facing_left[slot]));
    if (EQ) {
        return;
    }
    lda(RD8(OBJ->vx_hi[slot]));
    eor_a(0xFF);
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->vx_lo[slot]));
    eor_a(0xFF);
    clc();
    adc_a(0x01);
    WR8(OBJ->vx_lo[slot], cpu.a);
    cpu.a = 0;
    adc_a(RD8(ZP->temp00));
    WR8(OBJ->vx_hi[slot], cpu.a);
}

void abi_NegateSpeedIfFacingLeft051B(void)
{
    NegateSpeedIfFacingLeft051B(cpu.x);
}

/*
 * @routine 051E:6DF CopyFacingToY051B
 * @run 71:CEDF
 * WHAT IT DOES: Gives slot Y this object's facing.
 * HOW: One byte.
 * WAITS: no.
 * Entry: X object, Y the other. Exit: A the facing.
 */
void CopyFacingToY051B(uint8_t slot)
{
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
}

void abi_CopyFacingToY051B(void)
{
    CopyFacingToY051B(cpu.x);
}

/*
 * @routine 051E:6E6 PlaceAheadInY051B
 * @run 71:CEE6
 * WHAT IT DOES: Gives slot Y this object's X plus zTemp00-01, taken the other way when facing left.
 * HOW: The offset negated in place by facing.
 * WAITS: no.
 * Entry: X object, Y the other, zTemp00-01 the offset. Exit: A the X high byte.
 */
void PlaceAheadInY051B(uint8_t slot)
{
    lda(RD8(OBJ->facing_left[slot]));
    if (NE) {
        lda(RD8(ZP->temp01));
        eor_a(0xFF);
        WR8(ZP->temp04, cpu.a);
        lda(RD8(ZP->temp00));
        eor_a(0xFF);
        clc();
        adc_a(0x01);
        WR8(ZP->temp00, cpu.a);
        cpu.a = 0;
        adc_a(RD8(ZP->temp04));
        WR8(ZP->temp01, cpu.a);
    }
    lda(RD8(OBJ->x_lo[slot]));
    clc();
    adc_a(RD8(ZP->temp00));
    WR8(OBJ->x_lo[cpu.y], cpu.a);
    lda(RD8(OBJ->x_hi[slot]));
    adc_a(RD8(ZP->temp01));
    WR8(OBJ->x_hi[cpu.y], cpu.a);
}

void abi_PlaceAheadInY051B(void)
{
    PlaceAheadInY051B(cpu.x);
}

/*
 * @routine 051E:711 StartEightState2WhenHit051B
 * @run 71:CF11
 * WHAT IT DOES: Under state 2, a hit records the event of slot $10, puts slots $10-$17 in state 2 and lets the camera scroll to
 * screen 3.
 * HOW: A loop over the eight.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StartEightState2WhenHit051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x02);
    if (CS) {
        return;
    }
    lda(RD8(OBJ->hit_status[slot]));
    and_a(0x80);
    if (EQ) {
        return;
    }
    phx();
    ldx(0x10);
    JSR(0xCF25, k_record_event());
    do {
        lda(0x02);
        WR8(OBJ->state[cpu.x], cpu.a);
        WR8(OBJ->substate[cpu.x], 0);
        inx();
        cmp(cpu.x, 0x18);
    } while (NE);
    plx();
    lda(0x03);
    WR8(SCROLLLIM->x_max.hi, cpu.a);
}

void abi_StartEightState2WhenHit051B(void)
{
    StartEightState2WhenHit051B(cpu.x);
}

/*
 * @routine 051E:739 FacePlayerByPlace051B
 * @run 71:CF39
 * WHAT IT DOES: Faces left when the object is right of the player: off the first screen to the right, or on it at the player's X
 * low or more.
 * HOW: X high, then X low against the player's.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FacePlayerByPlace051B(uint8_t slot)
{
    lda(RD8(OBJ->x_hi[slot]));
    if (NE) {
        if (MI) {
            WR8(OBJ->facing_left[slot], 0);
            return;
        }
    } else {
        lda(RD8(OBJ->x_lo[slot]));
        cmp(cpu.a, RD8(OBJ->x_lo[0]));
        if (CC) {
            WR8(OBJ->facing_left[slot], 0);
            return;
        }
    }
    lda(0x01);
    WR8(OBJ->facing_left[slot], cpu.a);
}

void abi_FacePlayerByPlace051B(void)
{
    FacePlayerByPlace051B(cpu.x);
}
