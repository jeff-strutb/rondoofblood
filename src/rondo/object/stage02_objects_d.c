/*
 * Stage 02's objects, runs C302-C829.
 */
#include "stage_obj_ops.h"
#include "stage02_objects_d.h"
#include "calls_b.h"

/*
 * @routine 051D:302 FallingRubble051B
 * @run 71:C302
 * WHAT IT DOES: State 0 loads the record and turns the speed by the player's side. State 1 waits out its count (then becomes kind
 * $9B), steps its list on odd counts, falls and lands (state on, attribute $10, Y down by it) or rises away at frame
 * $62 up. Later states count out, starting a kind $50 object every eighth count.
 * HOW: Branches by state.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FallingRubble051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xC30A, k_load_record());
        JSR(0xC30D, k_side());
        and_a(0x01);
        if (NE) {
            JSR(0xC314, k_negate_x());
        }
        return;
    }
    dec_a();
    if (NE) {
        abi_nz(DEC8(OBJ->work_d[slot]));
        if (EQ) {
            k_clear();
            return;
        }
        lda(RD8(OBJ->work_d[slot]));
        and_a(0x07);
        if (NE) {
            return;
        }
        JSR(0xC37E, k_free_from30());
        if (CC) {
            return;
        }
        lda(0x50);
        JSR(0xC385, k_start_at_place());
        lda(RD8(OBJ->facing_left[cpu.x]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        return;
    }
    abi_nz(DEC8(OBJ->work_d[slot]));
    if (EQ) {
        lda(0x9B);
        WR8(OBJ->kind[slot], cpu.a);
        WR8(OBJ->state[slot], 0);
        return;
    }
    lda(RD8(OBJ->work_d[slot]));
    lsr_a();
    if (CC) {
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xC38C + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xC38D + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    JSR(0xC349, k_step_frames());
    if (EQ) {
        k_clear();
        return;
    }
    lda(RD8(OBJ->frame[cpu.x]));
    cmp(cpu.a, 0x62);
    if (CS) {
        lda(0xFF);
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        WR8(OBJ->vy_lo[cpu.x], 0);
        k_move_y();
        return;
    }
    JSR(0xC355, k_gravity_unlimited());
    lda(0x04);
    JSR(0xC35A, k_stop_on_landing());
    if (CC) {
        return;
    }
    slot = cpu.x;
    abi_nz(INC8(OBJ->state[slot]));
    lda(0x10);
    WR8(OBJ->sprite_attr[slot], cpu.a);
    clc();
    adc_a(RD8(OBJ->y_lo[slot]));
    WR8(OBJ->y_lo[slot], cpu.a);
}

void abi_FallingRubble051B(void)
{
    FallingRubble051B(cpu.x);
}

/*
 * @routine 051D:3C2 RisingPuff051B
 * @run 71:C3C2
 * WHAT IT DOES: State 0 loads the record with sound $13 and a rise of -3; after that moves up and steps the list at $C3F4,
 * clearing the object at its end.
 * HOW: Branches by state.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void RisingPuff051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xC3CA, k_load_record());
        lda(0x13);
        JSR(0xC3CF, k_sound());
        slot = cpu.x;
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        lda(0xFD);
        WR8(OBJ->vy_hi[slot], cpu.a);
        lda(0x00);
        WR8(OBJ->vy_lo[slot], cpu.a);
        return;
    }
    JSR(0xC3E3, k_move_y());
    list_to_temp10(0xC3F4);
    JSR(0xC3EE, k_step_frames());
    if (EQ) {
        k_clear();
    }
}

void abi_RisingPuff051B(void)
{
    RisingPuff051B(cpu.x);
}

/*
 * @routine 051D:40D LoadPictureByStartArg051B
 * @run 71:C40D
 * WHAT IT DOES: State 0 loads the record, takes the frame from OBJ_START_ARG and sprite set 6.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LoadPictureByStartArg051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (NE) {
        return;
    }
    JSR(0xC415, k_load_record());
    lda(RD8(OBJ->start_arg[cpu.x]));
    WR8(OBJ->frame[cpu.x], cpu.a);
    lda(0x06);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
}

void abi_LoadPictureByStartArg051B(void)
{
    LoadPictureByStartArg051B(cpu.x);
}

/*
 * @routine 051D:421 FadeOutWhenDoorHit051B
 * @run 71:C421
 * WHAT IT DOES: State 0 loads the record. With the door ready and not held, a hit count of 1 or 2 sets zField8C to $80 plus
 * OBJ_START_ARG and sub-step 3, and fades the CD out.
 * HOW: cd_fade $0C by a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FadeOutWhenDoorHit051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        k_load_record();
        return;
    }
    lda(rd8(StageDoorHeld));
    eor_a(0x01);
    and_a(RD8(PVARS->door_ready));
    if (EQ) {
        return;
    }
    lda(RD8(OBJ->hit_taken[slot]));
    if (EQ) {
        return;
    }
    cmp(cpu.a, 0x03);
    if (CS) {
        return;
    }
    lda(0x80);
    ora_a(RD8(OBJ->start_arg[slot]));
    WR8(ZP->field8c, cpu.a);
    lda(0x03);
    WR8(ZP->sub_step, cpu.a);
    lda(0x0C);
    cd_fade_exit_shim(vec_cd_fade(cpu.a));
}

void abi_FadeOutWhenDoorHit051B(void)
{
    FadeOutWhenDoorHit051B(cpu.x);
}

/*
 * @routine 051D:44D EndAfterScripts051B
 * @run 71:C44D
 * WHAT IT DOES: State 0 loads the record. State 1 waits for script slot 5's script to be $80, then starts the fade. From then on
 * keeps the wave counter at 0 and the player's piece count 4, pose 2 and swoop count 0, and once script slot 5 is
 * empty sets zField8C $80 and sub-step 3.
 * HOW: Branches by state.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void EndAfterScripts051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xC455, k_load_record());
        return;
    }
    dec_a();
    if (EQ) {
        lda(RD8(SLOTS->script[5]));
        cmp(cpu.a, 0x80);
        if (NE) {
            return;
        }
        lda(0x01);
        WR8(ZP->fade_active, cpu.a);
        abi_nz(INC8(OBJ->state[slot]));
    }
    WR8(ZP->wave_counter, 0);
    lda(0x04);
    WR8(OBJ->work_e[0], cpu.a);
    lda(0x02);
    WR8(PVARS2->start_pose, cpu.a);
    WR8(OBJ->work_f[0], 0);
    lda(RD8(SLOTS->script[5]));
    if (NE) {
        return;
    }
    lda(0x80);
    WR8(ZP->field8c, cpu.a);
    lda(0x03);
    WR8(ZP->sub_step, cpu.a);
}

void abi_EndAfterScripts051B(void)
{
    EndAfterScripts051B(cpu.x);
}

/*
 * @routine 051D:484 StepListByStartArg051B
 * @run 71:C484
 * WHAT IT DOES: State 0 loads the record with sprite set 6; then steps the list the table at $C4A1 names for OBJ_START_ARG.
 * HOW: A tail jump to StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StepListByStartArg051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xC48C, k_load_record());
        lda(0x06);
        WR8(OBJ->sprite_set[cpu.x], cpu.a);
    }
    ldy(RD8(OBJ->start_arg[cpu.x]));
    lda(rd8((uint16_t)(0xC4A1 + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xC4A2 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    k_step_frames();
}

void abi_StepListByStartArg051B(void)
{
    StepListByStartArg051B(cpu.x);
}

/*
 * @routine 051D:4C6 FlickerColoursThenDrawWater051B
 * @run 71:C4C6
 * WHAT IT DOES: Starts two scripts in slots 8 and 9 by the wave counter, then in state 0 loads the record and keeps the camera's
 * place ($C4E8), else draws the water ($C4F5).
 * HOW: StartScriptSlotKind4 twice with a number from the table at $C4B6.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FlickerColoursThenDrawWater051B(uint8_t slot)
{
    (void)slot;
    phx();
    lda(RD8(ZP->wave_counter));
    lsr_a();
    and_a(0x0F);
    tax();
    lda(rd8((uint16_t)(0xC4B6 + cpu.x)));
    ora_a(0x80);
    pha();
    ldx(0x08);
    JSR(0xC4D8, k_script4());
    pla();
    inc_a();
    ldx(0x09);
    JSR(0xC4DF, k_script4());
    plx();
    lda(RD8(OBJ->state[cpu.x]));
    if (NE) {
        run_routine(0xC4F5);
        return;
    }
    JSR(0xC4E8, k_load_record());
    run_routine(0xC4E8);
}

void abi_FlickerColoursThenDrawWater051B(void)
{
    FlickerColoursThenDrawWater051B(cpu.x);
}

/*
 * @routine 051D:4F5 DrawWaterTiles051B
 * @run 71:C4F5
 * WHAT IT DOES: Picks a wave place from the camera's movement and draws the water's tiles into VRAM from the bank $14 above the
 * RAM base: eight and two of the moving rows ($C56B), then five merged rows ($C58F).
 * HOW: MPR4 changed around the three loops; zTemp08-09 the source.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DrawWaterTiles051B(uint8_t slot)
{
    (void)slot;
    lda(RD8(SCROLLE->step_x_lo[6]));
    eor_a(RD8(SCROLLE->step_x_hi[6]));
    if (NE) {
        JSR(0xC500, k_random());
        and_a(0x03);
        clc();
        adc_a(RD8(OBJ->work_d[cpu.x]));
    } else {
        lda(RD8(OBJ->work_d[cpu.x]));
    }
    lsr_a();
    and_a(0x0F);
    eor_a(0x0F);
    WR8(ZP->temp00, cpu.a);
    call_routine(0xC515, 0xC4E8);
    cpu.a = tma(0x10);
    pha();
    lda(0x14);
    clc();
    adc_a(RD8(ZP->ram_base_bank));
    tam(0x10, cpu.a);
    cpu.y = 0;
    do {
        tya();
        asl_a();
        asl_a();
        asl_a();
        asl_a();
        asl_a();
        WR8(ZP->temp08, cpu.a);
        lda(RD8(ZP->temp00));
        adc_a(0x80);
        WR8(ZP->temp09, cpu.a);
        call_routine(0xC531, 0xC56B);
        iny();
        cmp(cpu.y, 0x08);
    } while (NE);
    do {
        tya();
        and_a(0x07);
        asl_a();
        asl_a();
        asl_a();
        asl_a();
        asl_a();
        WR8(ZP->temp08, cpu.a);
        lda(RD8(ZP->temp00));
        adc_a(0x80);
        WR8(ZP->temp09, cpu.a);
        call_routine(0xC549, 0xC56B);
        iny();
        cmp(cpu.y, 0x0A);
    } while (NE);
    cpu.y = 0;
    do {
        lda(RD8(ZP->temp00));
        and_a(0x03);
        asl_a();
        asl_a();
        asl_a();
        asl_a();
        asl_a();
        WR8(ZP->temp08, cpu.a);
        cpu.a = 0;
        adc_a(0x90);
        WR8(ZP->temp09, cpu.a);
        call_routine(0xC562, 0xC58F);
        iny();
        cmp(cpu.y, 0x05);
    } while (NE);
    pla();
    tam(0x10, cpu.a);
}

void abi_DrawWaterTiles051B(void)
{
    DrawWaterTiles051B(cpu.x);
}

/*
 * @routine 051D:56B CopyEightWordsToVram051B
 * @run 71:C56B
 * WHAT IT DOES: Copies the eight words at (zTemp08) to VRAM $1800 plus 16 times Y.
 * HOW: Through the VDC data ports.
 * WAITS: no.
 * Entry: Y the row. Exit: Y kept.
 */
void CopyEightWordsToVram051B(uint8_t slot)
{
    (void)slot;
    JSR(0xC56E, k_select_vram_write());
    tya();
    asl_a();
    asl_a();
    asl_a();
    asl_a();
    WR8(VDC->data_lo, cpu.a);
    st2(0x18);
    JSR(0xC57B, k_select_vram_data());
    phy();
    cpu.y = 0;
    do {
        lda(RD8_PTR_Y(ZP->temp08, cpu.y));
        WR8(VDC->data_lo, cpu.a);
        iny();
        lda(RD8_PTR_Y(ZP->temp08, cpu.y));
        WR8(VDC->data_hi, cpu.a);
        iny();
        cmp(cpu.y, 0x10);
    } while (NE);
    ply();
}

void abi_CopyEightWordsToVram051B(void)
{
    CopyEightWordsToVram051B(cpu.x);
}

/*
 * @routine 051D:58F MergeEightWordsToVram051B
 * @run 71:C58F
 * WHAT IT DOES: Writes the eight words at (zTemp08) to VRAM $18A0 plus 16 times Y, reading the VRAM word first each time.
 * HOW: The read address set as the write address.
 * WAITS: no.
 * Entry: Y the row. Exit: Y kept.
 */
void MergeEightWordsToVram051B(uint8_t slot)
{
    (void)slot;
    JSR(0xC592, k_select_vram_write());
    tya();
    asl_a();
    asl_a();
    asl_a();
    asl_a();
    adc_a(0xA0);
    WR8(VDC->data_lo, cpu.a);
    st2(0x18);
    JSR(0xC5A1, k_select_vram_read());
    tya();
    asl_a();
    asl_a();
    asl_a();
    asl_a();
    adc_a(0xA0);
    WR8(VDC->data_lo, cpu.a);
    st2(0x18);
    JSR(0xC5B0, k_select_vram_data());
    phy();
    cpu.y = 0;
    do {
        lda(RD8(VDC->data_lo));
        lda(RD8_PTR_Y(ZP->temp08, cpu.y));
        WR8(VDC->data_lo, cpu.a);
        iny();
        lda(RD8(VDC->data_hi));
        WR8(VDC->data_hi, cpu.a);
        iny();
        cmp(cpu.y, 0x10);
    } while (NE);
    ply();
}

void abi_MergeEightWordsToVram051B(void)
{
    MergeEightWordsToVram051B(cpu.x);
}

/*
 * @routine 051D:5C8 LoadAndRecordFlipped051B
 * @run 71:C5C8
 * WHAT IT DOES: Loads the record, sets the player's sprite flip byte to $C0 and records the event.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LoadAndRecordFlipped051B(uint8_t slot)
{
    (void)slot;
    JSR(0xC5CB, k_load_record());
    lda(0xC0);
    WR8(OBJ->sprite_flip[0], cpu.a);
    k_record_event();
}

void abi_LoadAndRecordFlipped051B(void)
{
    LoadAndRecordFlipped051B(cpu.x);
}

/*
 * @routine 051D:5D3 CloseScrollOnce051B
 * @run 71:C5D3
 * WHAT IT DOES: Once zField8F has a bit of OBJ_START_ARG: limits the camera (X minimum 1 for bit 3, else maximum 3), sets
 * zField8E bit 7 and records the event.
 * HOW: Done once, kept in OBJ_PIECE_NEXT.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CloseScrollOnce051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xC5DB, k_load_record());
        slot = cpu.x;
    }
    lda(RD8(OBJ->work_d[slot]));
    if (NE) {
        return;
    }
    lda(RD8(ZP->field8f));
    and_a(RD8(OBJ->start_arg[slot]));
    if (EQ) {
        return;
    }
    cmp(cpu.a, 0x08);
    if (EQ) {
        lda(0x01);
        WR8(SCROLLLIM->x_min_hi, cpu.a);
    } else {
        lda(0x03);
        WR8(SCROLLLIM->x_max.hi, cpu.a);
    }
    abi_nz(INC8(OBJ->work_d[slot]));
    RMW8(ZP->field8e, was, (uint8_t)(was | 0x80));
    JSR(0xC5FF, k_record_event());
}

void abi_CloseScrollOnce051B(void)
{
    CloseScrollOnce051B(cpu.x);
}

/*
 * @routine 051D:600 MarkOtherWhenFlag051B
 * @run 71:C600
 * WHAT IT DOES: Once zField8F bit 3 is set, puts 8 into the low digit of the OBJ_START_ARG of the object OBJ_START_ARG names.
 * HOW: Done once, kept in OBJ_PIECE_NEXT.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void MarkOtherWhenFlag051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xC608, k_load_record());
        slot = cpu.x;
    }
    lda(RD8(OBJ->work_d[slot]));
    if (NE) {
        return;
    }
    lda(RD8(ZP->field8f));
    and_a(0x08);
    if (EQ) {
        return;
    }
    ldy(RD8(OBJ->start_arg[slot]));
    lda(RD8(OBJ->start_arg[cpu.y]));
    and_a(0xF0);
    ora_a(0x08);
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    abi_nz(INC8(OBJ->work_d[slot]));
}

void abi_MarkOtherWhenFlag051B(void)
{
    MarkOtherWhenFlag051B(cpu.x);
}

/*
 * @routine 051D:624 StartScriptWhenOtherAdvances051B
 * @run 71:C624
 * WHAT IT DOES: Once the object OBJ_START_ARG names is past state 1: starts in slot 8 the script the table at $9C1A holds for
 * twice zField82 + 1 plus the character, and records the event.
 * HOW: StartScriptSlotKind1.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StartScriptWhenOtherAdvances051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        k_load_record();
        return;
    }
    ldy(RD8(OBJ->start_arg[slot]));
    lda(RD8(OBJ->state[cpu.y]));
    cmp(cpu.a, 0x02);
    if (CC) {
        return;
    }
    phx();
    lda(RD8(ZP->field82));
    inc_a();
    asl_a();
    adc_a(RD8(ZP->character));
    tax();
    lda(rd8((uint16_t)(0x9C1A + cpu.x)));
    ldx(0x08);
    JSR(0xC646, k_script1());
    plx();
    k_record_event();
}

void abi_StartScriptWhenOtherAdvances051B(void)
{
    StartScriptWhenOtherAdvances051B(cpu.x);
}

/*
 * @routine 051D:66E RaftOnWater051B
 * @run 71:C66E
 * WHAT IT DOES: A floating platform. State 0 loads the record as sprite set 6 frame 1 and keeps its map X. After: steps the list
 * once started; at four in OBJ_KNOCK_FORWARD breaks up ($C7CF); else swings ($C6DE), moves sideways unless holding
 * the player while the screen scrolls, checks for the player ($C736) and sinks on a new touch ($C781).
 * HOW: The calls in turn; OBJ_WORK_K marks a touch that began this frame.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void RaftOnWater051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xC676, k_load_record());
        slot = cpu.x;
        lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
        lda(0x01);
        WR8(OBJ->frame[slot], cpu.a);
        lda(RD8(SCROLL->x_lo[0]));
        clc();
        adc_a(RD8(OBJ->x_lo[slot]));
        WR8(OBJ->work_g[slot], cpu.a);
        lda(RD8(SCROLLE->x_hi_screen[0]));
        adc_a(RD8(OBJ->x_hi[slot]));
        WR8(OBJ->work_f[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->anim_step[slot]));
    if (NE) {
        lda(0xC6);
        WR8(ZP->temp11, cpu.a);
        lda(0x4B);
        WR8(ZP->temp10, cpu.a);
        JSR(0xC6A4, k_step_frames());
        slot = cpu.x;
    }
    lda(RD8(OBJ->work_m[slot]));
    cmp(cpu.a, 0x04);
    if (CS) {
        run_routine(0xC7CF);
        return;
    }
    call_routine(0xC6B0, 0xC6DE);
    slot = cpu.x;
    lda(RD8(OBJ->loop_flags[slot]));
    and_a(0x01);
    if (NE) {
        lda(RD8(SCROLLE->step_x_lo[0]));
        ora_a(RD8(SCROLLE->step_x_hi[0]));
    }
    if (EQ) {
        JSR(0xC6C2, k_move_x());
        slot = cpu.x;
    }
    WR8(OBJ->work_k[slot], 0);
    lda(RD8(OBJ->work_i[slot]));
    WR8(OBJ->work_l[slot], cpu.a);
    call_routine(0xC6CE, 0xC736);
    slot = cpu.x;
    lda(RD8(OBJ->work_l[slot]));
    eor_a(RD8(OBJ->work_i[slot]));
    and_a(RD8(OBJ->work_i[slot]));
    WR8(OBJ->work_k[slot], cpu.a);
    call_routine(0xC6DD, 0xC781);
}

void abi_RaftOnWater051B(void)
{
    RaftOnWater051B(cpu.x);
}

/*
 * @routine 051D:6DE SwingAndDrift051B
 * @run 71:C6DE
 * WHAT IT DOES: X speed from the cosine of a 16-bit angle (OBJ_PIECE_PLACE:PIECE_NEXT) times $B0; every full turn corrects X by
 * how far it has drifted from the kept map X.
 * HOW: ScaleByCosine; the angle steps by $0180.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SwingAndDrift051B(uint8_t slot)
{
    lda(0xB0);
    WR8(ZP->temp00, cpu.a);
    lda(0x00);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    JSR(0xC6EC, k_cosine());
    slot = cpu.x;
    lda(RD8(ZP->plane_rows_left));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(RD8(ZP->sprite_flip_mask));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    clc();
    adc_a(0x80);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    adc_a(0x01);
    WR8(OBJ->work_e[slot], cpu.a);
    if (CC) {
        return;
    }
    WR8(OBJ->work_d[slot], 0);
    WR8(OBJ->work_e[slot], 0);
    WR8(OBJ->x_frac[slot], 0);
    lda(RD8(SCROLL->x_lo[0]));
    clc();
    adc_a(RD8(OBJ->x_lo[slot]));
    sec();
    sbc_a(RD8(OBJ->work_g[slot]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(SCROLLE->x_hi_screen[0]));
    adc_a(RD8(OBJ->x_hi[slot]));
    sbc_a(RD8(OBJ->work_f[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(ZP->temp00));
    eor_a(0xFF);
    inc_a();
    WR8(OBJ->vx_hi[slot], cpu.a);
    WR8(OBJ->vx_lo[slot], 0);
}

void abi_SwingAndDrift051B(void)
{
    SwingAndDrift051B(cpu.x);
}

/*
 * @routine 051D:736 CarryTouchingPlayer051B
 * @run 71:C736
 * WHAT IT DOES: With the player touching (box $20 by $10): marks it in OBJ_RAIN_COUNT (and OBJ_WORK_L on a column hit) and, the
 * player standing still, pushes them by the object's X speed (zPlayerPush bit 4) and sets loop flag bit 0; else
 * clears that bit.
 * HOW: ContactWithPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CarryTouchingPlayer051B(uint8_t slot)
{
    WR8(OBJ->work_i[slot], 0);
    lda(0x20);
    WR8(ZP->temp08, cpu.a);
    lda(0x10);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xC744, 0xAA35);
    slot = cpu.x;
    if (CS) {
        lda(RD8(PVARS->column_hit[0]));
        ora_a(RD8(PVARS->column_hit[1]));
        ora_a(RD8(PVARS->column_hit[2]));
        if (NE) {
            lda(0x01);
            WR8(OBJ->work_l[slot], cpu.a);
        }
        lda(0x01);
        WR8(OBJ->work_i[slot], cpu.a);
        lda(RD8(OBJ->vx_lo[0]));
        ora_a(RD8(OBJ->vx_hi[0]));
        if (EQ) {
            RMW8(ZP->player_push, was, (uint8_t)(was | 0x10));
            lda(RD8(OBJ->vx_lo[slot]));
            WR8(ZP->push_x.lo, cpu.a);
            lda(RD8(OBJ->vx_hi[slot]));
            WR8(ZP->push_x.hi, cpu.a);
            lda(RD8(OBJ->loop_flags[slot]));
            ora_a(0x01);
            WR8(OBJ->loop_flags[slot], cpu.a);
            return;
        }
    }
    lda(RD8(OBJ->loop_flags[slot]));
    and_a(0xFE);
    WR8(OBJ->loop_flags[slot], cpu.a);
}

void abi_CarryTouchingPlayer051B(void)
{
    CarryTouchingPlayer051B(cpu.x);
}

/*
 * @routine 051D:781 SinkAndThrowOnTouch051B
 * @run 71:C781
 * WHAT IT DOES: On a new touch, starts a bob (OBJ_WORK_J) and, a free slot found, starts a piece ($C7B7), counting up to four in
 * OBJ_KNOCK_FORWARD; while bobbing, takes Y from the table at $C7E6.
 * HOW: A 15-frame bob.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void SinkAndThrowOnTouch051B(uint8_t slot)
{
    lda(RD8(OBJ->work_j[slot]));
    if (EQ) {
        lda(RD8(OBJ->work_k[slot]));
        if (EQ) {
            return;
        }
        abi_nz(INC8(OBJ->work_j[slot]));
        lda(0x01);
        WR8(OBJ->anim_step[slot], cpu.a);
        abi_nz(INC8(OBJ->work_m[slot]));
        JSR(0xC799, k_free_from30());
        if (CS) {
            run_routine(0xC7B7);
            return;
        }
        abi_nz(DEC8(OBJ->work_m[cpu.x]));
        return;
    }
    ldy(RD8(OBJ->work_j[slot]));
    cmp(cpu.y, 0x10);
    if (EQ) {
        WR8(OBJ->work_j[slot], 0);
        return;
    }
    lda(rd8((uint16_t)(0xC7E6 + cpu.y)));
    WR8(OBJ->y_lo[slot], cpu.a);
    abi_nz(INC8(OBJ->work_j[slot]));
}

void abi_SinkAndThrowOnTouch051B(void)
{
    SinkAndThrowOnTouch051B(cpu.x);
}

/*
 * @routine 051D:7B7 StartPiece4CBeside051B
 * @run 71:C7B7
 * WHAT IT DOES: Kind $4C into $C7B9.
 * HOW: One load, then on.
 * WAITS: no.
 * Entry: X object, Y a free slot. Exit: as $C7B9.
 */
void StartPiece4CBeside051B(uint8_t slot)
{
    (void)slot;
    lda(0x4C);
    run_routine(0xC7B9);
}

void abi_StartPiece4CBeside051B(void)
{
    StartPiece4CBeside051B(cpu.x);
}

/*
 * @routine 051D:7B9 SetKindAndPlaceOfY051B
 * @run 71:C7B9
 * WHAT IT DOES: Gives slot Y the kind in A and this object's Y low and X.
 * HOW: Four stores.
 * WAITS: no.
 * Entry: A kind, X object, Y the other slot. Exit: A the X high byte.
 */
void SetKindAndPlaceOfY051B(uint8_t slot)
{
    WR8(OBJ->kind[cpu.y], cpu.a);
    lda(RD8(OBJ->y_lo[slot]));
    WR8(OBJ->y_lo[cpu.y], cpu.a);
    lda(RD8(OBJ->x_lo[slot]));
    WR8(OBJ->x_lo[cpu.y], cpu.a);
    lda(RD8(OBJ->x_hi[slot]));
    WR8(OBJ->x_hi[cpu.y], cpu.a);
}

void abi_SetKindAndPlaceOfY051B(void)
{
    SetKindAndPlaceOfY051B(cpu.x);
}

/*
 * @routine 051D:7CF BreakRaftIntoSix051B
 * @run 71:C7CF
 * WHAT IT DOES: Clears slots $10-$15 and starts a kind $4D piece in each at this place, sound $19, and clears this object.
 * HOW: ClearObjectInY and SetKindAndPlaceOfY051B per slot.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BreakRaftIntoSix051B(uint8_t slot)
{
    (void)slot;
    ldy(0x10);
    do {
        JSR(0xC7D4, k_clear_in_y());
        lda(0x4D);
        call_routine(0xC7D8, 0xC7B9);
        iny();
        cmp(cpu.y, 0x16);
    } while (NE);
    lda(0x19);
    JSR(0xC7E2, k_sound());
    JSR(0xC7E5, k_clear());
}

void abi_BreakRaftIntoSix051B(void)
{
    BreakRaftIntoSix051B(cpu.x);
}

/*
 * @routine 051D:7F6 DebrisFall051B
 * @run 71:C7F6
 * WHAT IT DOES: State 0 loads the record as sprite set 6 frame $37; after that falls by $28 a frame and is cleared on Y screen 1.
 * HOW: A 16-bit add to the Y speed, then ApplyVelocityY and ApplyVelocityX.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DebrisFall051B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xC7FE, k_load_record());
        lda(0x06);
        WR8(OBJ->sprite_set[cpu.x], cpu.a);
        lda(0x37);
        WR8(OBJ->frame[cpu.x], cpu.a);
        return;
    }
    lda(RD8(OBJ->vy_lo[slot]));
    clc();
    adc_a(0x28);
    WR8(OBJ->vy_lo[slot], cpu.a);
    cpu.a = 0;
    adc_a(RD8(OBJ->vy_hi[slot]));
    WR8(OBJ->vy_hi[slot], cpu.a);
    JSR(0xC81C, k_move_y());
    JSR(0xC81F, k_move_x());
    lda(RD8(OBJ->y_hi[cpu.x]));
    cmp(cpu.a, 0x01);
    if (EQ) {
        k_clear();
    }
}

void abi_DebrisFall051B(void)
{
    DebrisFall051B(cpu.x);
}
