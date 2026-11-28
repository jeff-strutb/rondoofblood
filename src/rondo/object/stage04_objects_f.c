/*
 * Stage 04's objects, runs C829-CC78.
 */
#include "stage_obj_ops.h"
#include "stage04_objects_f.h"
#include "calls_b.h"

/*
 * @routine 07DE:029 AdvanceWhenRiderHit07DE
 * @run 71:C829
 * WHAT IT DOES: On the first screen follows its rider ($C83D); once the object OBJ_START_ARG names is hit, goes on.
 * HOW: A read of that object's hit type.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void AdvanceWhenRiderHit07DE(uint8_t slot)
{
    lda(RD8(OBJ->x_hi[slot]));
    if (NE) {
        return;
    }
    call_routine(0xC831, 0xC83D);
    ldy(RD8(OBJ->start_arg[cpu.x]));
    lda(RD8(OBJ->hit_taken[cpu.y]));
    if (EQ) {
        return;
    }
    abi_nz(INC8(OBJ->state[cpu.x]));
}

void abi_AdvanceWhenRiderHit07DE(void)
{
    AdvanceWhenRiderHit07DE(cpu.x);
}

/*
 * @routine 07DE:03D FollowCarrier07DE
 * @run 71:C83D
 * WHAT IT DOES: With an argument under $80, on the first screen: takes the place of the object it names and moves down 4.
 * HOW: CopyObjectPlaceBack and a tail jump to ApplyVelocityY.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FollowCarrier07DE(uint8_t slot)
{
    lda(RD8(OBJ->start_arg[slot]));
    if (MI) {
        return;
    }
    lda(RD8(OBJ->x_hi[slot]));
    ora_a(RD8(OBJ->y_hi[slot]));
    if (NE) {
        return;
    }
    ldy(RD8(OBJ->start_arg[slot]));
    JSR(0xC850, k_copy_place_back());
    lda(0x04);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    k_move_y();
}

void abi_FollowCarrier07DE(void)
{
    FollowCarrier07DE(cpu.x);
}

/*
 * @routine 07DE:059 RaiseSixSegments07DE
 * @run 71:C859
 * WHAT IT DOES: Follows its rider (argument $FF bobs first) and steps the list at $C8BC. At its end: next state with a wait of $F0,
 * a hit box and 3 health, and six kind $35 segments two slots on, with their numbers, waits $03 up by 8 and parent.
 * HOW: A loop of ClearObjectInY and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void RaiseSixSegments07DE(uint8_t slot)
{
    call_routine(0xC85C, 0xC83D);
    slot = cpu.x;
    lda(RD8(OBJ->start_arg[slot]));
    cmp(cpu.a, 0xFF);
    if (EQ) {
        call_routine(0xC866, 0xC7FF);
    }
    list_to_temp10(0xC8BC);
    JSR(0xC871, k_step_frames());
    if (NE) {
        return;
    }
    slot = cpu.x;
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    abi_nz(INC8(OBJ->state[slot]));
    lda(0xF0);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    lda(0x03);
    WR8(OBJ->health[slot], cpu.a);
    txa();
    tay();
    iny();
    iny();
    lda(0x06);
    WR8(ZP->temp00, cpu.a);
    lda(0x03);
    WR8(ZP->temp01, cpu.a);
    WR8(ZP->temp02, 0);
    do {
        JSR(0xC89C, k_clear_in_y());
        lda(0x35);
        JSR(0xC8A1, k_start_at_place());
        txa();
        WR8(OBJ->work_l[cpu.y], cpu.a);
        lda(RD8(ZP->temp01));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        clc();
        adc_a(0x08);
        WR8(ZP->temp01, cpu.a);
        lda(RD8(ZP->temp02));
        WR8(OBJ->work_m[cpu.y], cpu.a);
        abi_nz(INC8(ZP->temp02));
        iny();
        abi_nz(DEC8(ZP->temp00));
    } while (NE);
}

void abi_RaiseSixSegments07DE(void)
{
    RaiseSixSegments07DE(cpu.x);
}

/*
 * @routine 07DE:0C9 HomingOnPlayer07DE
 * @run 71:C8C9
 * WHAT IT DOES: Steps the list at $C922, turns its angle toward the player 32 dots up by one, and moves at a speed that pulses on
 * the cosine of OBJ_FALL_LO ($0140 plus).
 * HOW: AngleToPlayerOffset, TurnAngleToward, ScaleByCosine, SetVelocityFromAngle.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void HomingOnPlayer07DE(uint8_t slot)
{
    list_to_temp10(0xC922);
    JSR(0xC8D4, k_step_frames());
    WR8(ZP->temp08, 0);
    WR8(ZP->temp09, 0);
    lda(0xE0);
    WR8(ZP->temp0a, cpu.a);
    lda(0xFF);
    WR8(ZP->temp0b, cpu.a);
    JSR(0xC8E3, k_angle_to_player_offset());
    WR8(ZP->temp02, cpu.a);
    lda(0x01);
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_d[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    JSR(0xC8F1, k_turn_angle());
    WR8(OBJ->work_d[cpu.x], cpu.a);
    lda(0xC0);
    WR8(ZP->temp00, cpu.a);
    lda(0x00);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_h[cpu.x]));
    JSR(0xC902, k_cosine());
    slot = cpu.x;
    lda(RD8(OBJ->work_h[slot]));
    clc();
    adc_a(0x02);
    WR8(OBJ->work_h[slot], cpu.a);
    lda(RD8(ZP->sprite_flip_mask));
    clc();
    adc_a(0x40);
    WR8(ZP->temp00, cpu.a);
    lda(RD8(ZP->plane_rows_left));
    adc_a(0x01);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    JSR(0xC91E, k_velocity_from_angle());
    JSR(0xC921, k_move());
}

void abi_HomingOnPlayer07DE(void)
{
    HomingOnPlayer07DE(cpu.x);
}

/*
 * @routine 07DE:12B CountThenClear07DE
 * @run 71:C92B
 * WHAT IT DOES: Counts OBJ_PIECE_PLACE out and clears the object.
 * HOW: A countdown.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountThenClear07DE(uint8_t slot)
{
    abi_nz(DEC8(OBJ->work_e[slot]));
    if (EQ) {
        k_clear();
    }
}

void abi_CountThenClear07DE(void)
{
    CountThenClear07DE(cpu.x);
}

/*
 * @routine 07DE:134 ByState07DEC934
 * @run 71:C934
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $C946.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState07DEC934(uint8_t slot)
{
    jump_by_state(slot, 0xC946);
}

void abi_ByState07DEC934(void)
{
    ByState07DEC934(cpu.x);
}

/*
 * @routine 07DE:14C SegmentWaitForBody07DE
 * @run 71:C94C
 * WHAT IT DOES: Follows its body; once the body is in state 3, counts OBJ_PIECE_NEXT out, takes its frame and a wait of $40.
 * HOW: The body's slot in OBJ_WORK_L.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SegmentWaitForBody07DE(uint8_t slot)
{
    ldy(RD8(OBJ->work_l[slot]));
    JSR(0xC952, k_copy_place_back());
    slot = cpu.x;
    lda(RD8(OBJ->state[cpu.y]));
    cmp(cpu.a, 0x03);
    if (CC) {
        return;
    }
    abi_nz(DEC8(OBJ->work_d[slot]));
    if (NE) {
        return;
    }
    ldy(RD8(OBJ->work_m[slot]));
    lda(rd8((uint16_t)(0xC970 + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_SegmentWaitForBody07DE(void)
{
    SegmentWaitForBody07DE(cpu.x);
}

/*
 * @routine 07DE:176 SegmentSwing07DE
 * @run 71:C976
 * WHAT IT DOES: Takes the body's attribute and its place along the chain ($C9FE); its attack power out, becomes kind $9B with
 * sound $10. Else swings: the radius grows to six steps, and the angle swings by a pendulum of two accelerations.
 * HOW: ScaleByCosine; a 16-bit angle and speed.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SegmentSwing07DE(uint8_t slot)
{
    ldy(RD8(OBJ->work_l[slot]));
    lda(RD8(OBJ->sprite_attr[cpu.y]));
    WR8(OBJ->sprite_attr[slot], cpu.a);
    call_routine(0xC982, 0xC9FE);
    slot = cpu.x;
    lda(RD8(OBJ->work_n[slot]));
    if (NE) {
        abi_nz(DEC8(OBJ->work_n[slot]));
        if (EQ) {
            lda(0x10);
            JSR(0xC991, k_sound_first_screen());
            lda(0x9B);
            WR8(OBJ->kind[cpu.x], cpu.a);
            WR8(OBJ->state[cpu.x], 0);
            return;
        }
    }
    lda(RD8(OBJ->work_j[slot]));
    cmp(cpu.a, 0x06);
    if (CC) {
        lda(RD8(OBJ->work_k[slot]));
        clc();
        adc_a(0x08);
        WR8(OBJ->work_k[slot], cpu.a);
        if (CS) {
            abi_nz(INC8(OBJ->work_j[slot]));
        }
    }
    lda(0x04);
    WR8(ZP->temp01, cpu.a);
    lda(0x00);
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_g[slot]));
    JSR(0xC9BD, k_cosine());
    slot = cpu.x;
    lda(RD8(OBJ->work_f[slot]));
    clc();
    adc_a(RD8(ZP->sprite_flip_mask));
    WR8(OBJ->work_f[slot], cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    adc_a(RD8(ZP->plane_rows_left));
    WR8(OBJ->work_e[slot], cpu.a);
    lda(RD8(OBJ->work_i[slot]));
    and_a(0x03);
    asl_a();
    tay();
    lda(RD8(OBJ->work_h[slot]));
    clc();
    adc_a(rd8((uint16_t)(0xC9F6 + cpu.y)));
    WR8(OBJ->work_h[slot], cpu.a);
    lda(RD8(OBJ->work_g[slot]));
    adc_a(rd8((uint16_t)(0xC9F7 + cpu.y)));
    WR8(OBJ->work_g[slot], cpu.a);
    if (CC) {
        return;
    }
    lda(0x40);
    WR8(OBJ->work_e[slot], cpu.a);
    WR8(OBJ->work_f[slot], 0);
    abi_nz(INC8(OBJ->work_i[slot]));
}

void abi_SegmentSwing07DE(void)
{
    SegmentSwing07DE(cpu.x);
}

/*
 * @routine 07DE:1FE PlaceAlongChain07DE
 * @run 71:C9FE
 * WHAT IT DOES: Places itself at radius OBJ_WORK_J-K and angle OBJ_PIECE_PLACE from the segment before: the first from two slots
 * before and 4 down, others from the one before.
 * HOW: CopyObjectPlaceBack, SetVelocityFromAngle, ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void PlaceAlongChain07DE(uint8_t slot)
{
    txa();
    tay();
    dey();
    lda(RD8(OBJ->work_m[slot]));
    if (NE) {
        JSR(0xCA09, k_copy_place_back());
    } else {
        dey();
        JSR(0xCA0F, k_copy_place_back());
        slot = cpu.x;
        lda(RD8(OBJ->y_lo[slot]));
        clc();
        adc_a(0x04);
        WR8(OBJ->y_lo[slot], cpu.a);
        if (CS) {
            abi_nz(INC8(OBJ->y_hi[slot]));
        }
    }
    slot = cpu.x;
    lda(RD8(OBJ->work_k[slot]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_j[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    JSR(0xCA2D, k_velocity_from_angle());
    JSR(0xCA30, k_move());
}

void abi_PlaceAlongChain07DE(void)
{
    PlaceAlongChain07DE(cpu.x);
}

/*
 * @routine 07DE:231 ByState07DECA31
 * @run 71:CA31
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $CA43.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState07DECA31(uint8_t slot)
{
    jump_by_state(slot, 0xCA43);
}

void abi_ByState07DECA31(void)
{
    ByState07DECA31(cpu.x);
}

/*
 * @routine 07DE:247 FaceFrameTowardPlayer07DE
 * @run 71:CA47
 * WHAT IT DOES: Takes the attribute and place of the slot below and the frame for the angle to the player.
 * HOW: AngleToPlayer in eighths from frame $85.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FaceFrameTowardPlayer07DE(uint8_t slot)
{
    (void)slot;
    lda(rd8((uint16_t)(0x2B17 + cpu.x)));
    WR8(OBJ->sprite_attr[cpu.x], cpu.a);
    txa();
    tay();
    dey();
    sxy();
    JSR(0xCA54, k_copy_place());
    sxy();
    JSR(0xCA58, k_angle_to_player());
    clc();
    adc_a(0x10);
    lsr_a();
    lsr_a();
    lsr_a();
    lsr_a();
    lsr_a();
    clc();
    adc_a(0x85);
    WR8(OBJ->frame[cpu.x], cpu.a);
}

void abi_FaceFrameTowardPlayer07DE(void)
{
    FaceFrameTowardPlayer07DE(cpu.x);
}

/*
 * @routine 07DE:267 EmitFourThenWait07DE
 * @run 71:CA67
 * WHAT IT DOES: State 0: loads the record, sets raster effect $19 with argument $C8 and emits four times ($CA8C). Later counts
 * OBJ_PIECE_NEXT out between emissions.
 * HOW: A loop with Y on the stack.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void EmitFourThenWait07DE(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xCA6F, k_load_record());
        lda(0x19);
        WR8(ZP->raster_effect, cpu.a);
        lda(0xC8);
        WR8(ZP->raster_arg, cpu.a);
        cpu.y = 0;
        do {
            phy();
            call_routine(0xCA7C, 0xCA8C);
            ply();
            iny();
            cmp(cpu.y, 0x04);
        } while (CC);
        return;
    }
    lda(RD8(OBJ->work_d[slot]));
    if (EQ) {
        run_routine(0xCA8C);
        return;
    }
    abi_nz(DEC8(OBJ->work_d[slot]));
}

void abi_EmitFourThenWait07DE(void)
{
    EmitFourThenWait07DE(cpu.x);
}

/*
 * @routine 07DE:28C EmitOneOfEight07DE
 * @run 71:CA8C
 * WHAT IT DOES: Counts OBJ_PIECE_PLACE up and, a slot of $20-$27 free, starts the kind the table gives there with the wait
 * OBJ_START_ARG.
 * HOW: FindEmptyOf20to27_07DE.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void EmitOneOfEight07DE(uint8_t slot)
{
    abi_nz(INC8(OBJ->work_e[slot]));
    call_routine(0xCA92, 0xCAB1);
    if (CC) {
        return;
    }
    phy();
    lda(RD8(OBJ->work_e[cpu.x]));
    and_a(0x07);
    tay();
    lda(rd8((uint16_t)(0xCAA9 + cpu.y)));
    ply();
    JSR(0xCAA2, k_start_at_place());
    lda(RD8(OBJ->start_arg[cpu.x]));
    WR8(OBJ->work_d[cpu.x], cpu.a);
}

void abi_EmitOneOfEight07DE(void)
{
    EmitOneOfEight07DE(cpu.x);
}

/*
 * @routine 07DE:2B1 FindEmptyOf20to27_07DE
 * @run 71:CAB1
 * WHAT IT DOES: Clears and gives in Y the first empty slot of $20-$27, carry set; carry clear when none is.
 * HOW: A walk of the kinds, then ClearObjectInY.
 * WAITS: no.
 * Entry: none. Exit: C found, Y the slot.
 */
void FindEmptyOf20to27_07DE(uint8_t slot)
{
    (void)slot;
    ldy(0x20);
    for (;;) {
        lda(RD8(OBJ->kind[cpu.y]));
        if (EQ) {
            break;
        }
        iny();
        cmp(cpu.y, 0x28);
        if (CS) {
            clc();
            return;
        }
    }
    JSR(0xCAC2, k_clear_in_y());
    sec();
}

void abi_FindEmptyOf20to27_07DE(void)
{
    FindEmptyOf20to27_07DE(cpu.x);
}

/*
 * @routine 07DE:2C4 AscendHitThenByState07DE
 * @run 71:CAC4
 * WHAT IT DOES: Counts the rise down ($CAF0), takes a hit ($CB13) and goes to the state's handler.
 * HOW: A table of handlers at $CADC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void AscendHitThenByState07DE(uint8_t slot)
{
    (void)slot;
    call_routine(0xCAC7, 0xCAF0);
    call_routine(0xCACA, 0xCB13);
    jump_by_state(cpu.x, 0xCADC);
}

void abi_AscendHitThenByState07DE(void)
{
    AscendHitThenByState07DE(cpu.x);
}

/*
 * @routine 07DE:2F0 CountRiseTime07DE
 * @run 71:CAF0
 * WHAT IT DOES: In state 5 counts the 16-bit OBJ_SWOOP_SPEED out and goes to state 8.
 * HOW: A 16-bit decrement.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CountRiseTime07DE(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x05);
    if (NE) {
        return;
    }
    lda(RD8(OBJ->work_g[slot]));
    sec();
    sbc_a(0x01);
    WR8(OBJ->work_g[slot], cpu.a);
    lda(RD8(OBJ->work_f[slot]));
    sbc_a(0x00);
    WR8(OBJ->work_f[slot], cpu.a);
    ora_a(RD8(OBJ->work_g[slot]));
    if (NE) {
        return;
    }
    lda(0x08);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_CountRiseTime07DE(void)
{
    CountRiseTime07DE(cpu.x);
}

/*
 * @routine 07DE:313 DieWhenHit07DE
 * @run 71:CB13
 * WHAT IT DOES: A hit under state 9 turns it into sprite set 3 rising slowly, without a hit box, in state 9.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DieWhenHit07DE(uint8_t slot)
{
    lda(RD8(OBJ->hit_status[slot]));
    and_a(0x80);
    if (EQ) {
        return;
    }
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x09);
    if (CS) {
        return;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x03);
    WR8(OBJ->sprite_set[slot], cpu.a);
    WR8(OBJ->frame[slot], 0);
    lda(0xFE);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0xFE);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(0x09);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_DieWhenHit07DE(void)
{
    DieWhenHit07DE(cpu.x);
}

/*
 * @routine 07DE:342 SetUpGhost07DE
 * @run 71:CB42
 * WHAT IT DOES: Loads the record with attribute $8F, a random place ($CB7A), random bob phase and life, facing the player, rising
 * at -0.75.
 * HOW: NextRandom and a table.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SetUpGhost07DE(uint8_t slot)
{
    (void)slot;
    JSR(0xCB45, k_load_record());
    lda(0x8F);
    WR8(OBJ->sprite_attr[cpu.x], cpu.a);
    call_routine(0xCB4D, 0xCB7A);
    JSR(0xCB50, k_random());
    WR8(OBJ->work_j[cpu.x], cpu.a);
    JSR(0xCB56, k_random());
    and_a(0x03);
    tay();
    lda(rd8((uint16_t)(0xCB72 + cpu.y)));
    WR8(OBJ->work_g[cpu.x], cpu.a);
    JSR(0xCB62, k_side());
    and_a(0x01);
    WR8(OBJ->facing_left[cpu.x], cpu.a);
    lda(0xFF);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    lda(0x40);
    WR8(OBJ->vy_lo[cpu.x], cpu.a);
}

void abi_SetUpGhost07DE(void)
{
    SetUpGhost07DE(cpu.x);
}

/*
 * @routine 07DE:37A RandomPlaceByScroll07DE
 * @run 71:CB7A
 * WHAT IT DOES: Puts the object at a random X and Y from the tables, shifted to the screen edge the camera moves toward.
 * HOW: Two NextRandom and ShiftByScrollDirection07DE.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void RandomPlaceByScroll07DE(uint8_t slot)
{
    WR8(OBJ->x_hi[slot], 0);
    WR8(OBJ->y_hi[slot], 0);
    JSR(0xCB83, k_random());
    and_a(0x07);
    tay();
    lda(rd8((uint16_t)(0xCB9C + cpu.y)));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    call_routine(0xCB8F, 0xCBA8);
    JSR(0xCB92, k_random());
    and_a(0x03);
    tay();
    lda(rd8((uint16_t)(0xCBA4 + cpu.y)));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
}

void abi_RandomPlaceByScroll07DE(void)
{
    RandomPlaceByScroll07DE(cpu.x);
}

/*
 * @routine 07DE:3A8 ShiftByScrollDirection07DE
 * @run 71:CBA8
 * WHAT IT DOES: When the camera scrolls sideways, moves X by the table's step for its direction.
 * HOW: The sign of ScrollSpeedX.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ShiftByScrollDirection07DE(uint8_t slot)
{
    lda(RD8(SCROLLE->speed_x[0]));
    if (EQ) {
        return;
    }
    rol_a();
    rol_a();
    and_a(0x01);
    asl_a();
    tay();
    lda(RD8(OBJ->x_lo[slot]));
    clc();
    adc_a(rd8((uint16_t)(0xCBC7 + cpu.y)));
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(RD8(OBJ->x_hi[slot]));
    adc_a(rd8((uint16_t)(0xCBC8 + cpu.y)));
    WR8(OBJ->x_hi[slot], cpu.a);
}

void abi_ShiftByScrollDirection07DE(void)
{
    ShiftByScrollDirection07DE(cpu.x);
}

/*
 * @routine 07DE:3CB SwayAndRise07DE
 * @run 71:CBCB
 * WHAT IT DOES: Sways sideways on the cosine of OBJ_WORK_J and rises; above Y $E0 on the first screen goes on.
 * HOW: ScaleByCosine, ApplyVelocityX and ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SwayAndRise07DE(uint8_t slot)
{
    lda(0xA0);
    WR8(ZP->temp00, cpu.a);
    lda(0x00);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_j[slot]));
    JSR(0xCBD9, k_cosine());
    lda(RD8(ZP->plane_rows_left));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    lda(RD8(ZP->sprite_flip_mask));
    WR8(OBJ->vx_lo[cpu.x], cpu.a);
    JSR(0xCBE6, k_move_x());
    abi_nz(INC8(OBJ->work_j[cpu.x]));
    JSR(0xCBEC, k_move());
    lda(RD8(OBJ->y_hi[cpu.x]));
    if (NE) {
        return;
    }
    lda(RD8(OBJ->y_lo[cpu.x]));
    cmp(cpu.a, 0xE0);
    if (CS) {
        return;
    }
    abi_nz(INC8(OBJ->state[cpu.x]));
}

void abi_SwayAndRise07DE(void)
{
    SwayAndRise07DE(cpu.x);
}

/*
 * @routine 07DE:3FC GhostDrift07DE
 * @run 71:CBFC
 * WHAT IT DOES: Steps the list at $CC89, wandering on a sine and cosine and counting its life. While the life lasts, once $38 or
 * more from the player, leaps with a random speed, sound $8F, and goes on.
 * HOW: ScaleBySine and ScaleByCosine.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void GhostDrift07DE(uint8_t slot)
{
    list_to_temp10(0xCC89);
    JSR(0xCC07, k_step_frames());
    lda(0x80);
    WR8(ZP->temp00, cpu.a);
    lda(0x00);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_i[cpu.x]));
    JSR(0xCC15, k_sine());
    lda(RD8(ZP->plane_rows_left));
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    lda(RD8(ZP->sprite_flip_mask));
    WR8(OBJ->vy_lo[cpu.x], cpu.a);
    JSR(0xCC22, k_move_y());
    lda(0xA0);
    WR8(ZP->temp00, cpu.a);
    lda(0x00);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_j[cpu.x]));
    JSR(0xCC30, k_cosine());
    lda(RD8(ZP->plane_rows_left));
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    lda(RD8(ZP->sprite_flip_mask));
    WR8(OBJ->vx_lo[cpu.x], cpu.a);
    JSR(0xCC3D, k_move_x());
    abi_nz(INC8(OBJ->work_i[cpu.x]));
    abi_nz(INC8(OBJ->work_j[cpu.x]));
    abi_nz(DEC8(OBJ->work_g[cpu.x]));
    if (EQ) {
        return;
    }
    JSR(0xCC4B, k_distance_x());
    cmp(cpu.a, 0x38);
    if (CC) {
        return;
    }
    slot = cpu.x;
    lda(0xF6);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->work_h[slot], cpu.a);
    JSR(0xCC61, k_random());
    and_a(0x0E);
    tay();
    lda(rd8((uint16_t)(0xCC7A + cpu.y)));
    WR8(OBJ->work_f[cpu.x], cpu.a);
    lda(rd8((uint16_t)(0xCC79 + cpu.y)));
    WR8(OBJ->work_g[cpu.x], cpu.a);
    lda(0x8F);
    JSR(0xCC75, k_sound_first_screen());
    abi_nz(INC8(OBJ->state[cpu.x]));
}

void abi_GhostDrift07DE(void)
{
    GhostDrift07DE(cpu.x);
}
