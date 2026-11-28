/*
 * Stage 05's objects, runs A8BC-B546.
 */
#include "stage_obj_ops.h"
#include "stage05_objects_a.h"
#include "calls_b.h"

/* What a card call left, applied to the registers and flags (card_exit_apply, which the game's machine model
 * cannot include beside its own). */
static void card_exit_to_cpu(CardExit e)
{
    if (e.set_a) {
        cpu.a = e.a;
    }
    if (e.set_x) {
        cpu.x = e.x;
    }
    if (e.set_y) {
        cpu.y = e.y;
    }
    if (e.set_nz) {
        abi_flag(P_N, (e.nz & 0x80) != 0);
        abi_flag(P_Z, e.nz == 0);
    }
    if (e.set_n) {
        abi_flag(P_N, e.n);
    }
    if (e.set_z) {
        abi_flag(P_Z, e.z);
    }
    if (e.set_c) {
        abi_flag(P_C, e.c);
    }
    if (e.set_v) {
        abi_flag(P_V, e.v);
    }
}

/*
 * @routine 093A:0BC BreakPieceByCount093A
 * @run 70:A8BC
 * WHAT IT DOES: Sound $19 and debris; counts a hit. Under the piece record's hits takes its speeds by count and moves; at them
 * sets the piece's zField8F bit and becomes the record's kind, at rest.
 * HOW: As BreakPieceByHits051B with the records at $B3A8.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BreakPieceByCount093A(uint8_t slot)
{
    lda(0x19);
    JSR(0xA8C1, k_sound());
    lda(0x01);
    call_routine(0xA8C6, 0xA90B);
    slot = cpu.x;
    ldy(RD8(OBJ->work_e[slot]));
    abi_nz(INC8(OBJ->work_d[slot]));
    lda(RD8(OBJ->work_d[slot]));
    cmp(cpu.a, rd8((uint16_t)(0xB3A7 + cpu.y)));
    if (CC) {
        lda(RD8(OBJ->work_d[slot]));
        dec_a();
        asl_a();
        clc();
        adc_a(RD8(OBJ->work_e[slot]));
        tay();
        lda(rd8((uint16_t)(0xB3AD + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xB3AE + cpu.y)));
        WR8(OBJ->vy_hi[slot], cpu.a);
        JSR(0xA8ED, k_move());
        return;
    }
    ldy(RD8(OBJ->start_arg[slot]));
    call_routine(0xA8F4, 0xA855);
    ora_a(RD8(ZP->field8f));
    WR8(ZP->field8f, cpu.a);
    slot = cpu.x;
    ldy(RD8(OBJ->work_e[slot]));
    lda(rd8((uint16_t)(0xB3AC + cpu.y)));
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
}

void abi_BreakPieceByCount093A(void)
{
    BreakPieceByCount093A(cpu.x);
}

/*
 * @routine 093A:689 PictureByArgument093A
 * @run 70:AE89
 * WHAT IT DOES: State 0 loads the record with the frame OBJ_START_ARG, sprite set 6.
 * HOW: Stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void PictureByArgument093A(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (NE) {
        return;
    }
    JSR(0xAE91, k_load_record());
    lda(RD8(OBJ->start_arg[cpu.x]));
    WR8(OBJ->frame[cpu.x], cpu.a);
    lda(0x06);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
}

void abi_PictureByArgument093A(void)
{
    PictureByArgument093A(cpu.x);
}

/*
 * @routine 093A:69D StartScriptAndVanish093A
 * @run 70:AE9D
 * WHAT IT DOES: Starts script $88 in the slot OBJ_START_ARG names, records the event and clears the object.
 * HOW: StartScriptSlotKind1.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void StartScriptAndVanish093A(uint8_t slot)
{
    (void)slot;
    JSR(0xAEA0, k_load_record());
    phx();
    lda(RD8(OBJ->start_arg[cpu.x]));
    tax();
    lda(0x88);
    JSR(0xAEAA, k_script1());
    plx();
    JSR(0xAEAE, k_record_event());
    k_clear();
}

void abi_StartScriptAndVanish093A(void)
{
    StartScriptAndVanish093A(cpu.x);
}

/*
 * @routine 093A:6B1 ContactPicture093A
 * @run 70:AEB1
 * WHAT IT DOES: State 0 loads the record as sprite set 6 frame $25; then checks the player against a box $14 by $40.
 * HOW: ContactWithPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ContactPicture093A(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xAEB9, k_load_record());
        slot = cpu.x;
        lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
        lda(0x25);
        WR8(OBJ->frame[slot], cpu.a);
    }
    lda(0x14);
    WR8(ZP->temp08, cpu.a);
    lda(0x40);
    WR8(ZP->temp09, cpu.a);
    run_routine(0xAA35);
}

void abi_ContactPicture093A(void)
{
    ContactPicture093A(cpu.x);
}

/*
 * @routine 093A:6CE BreakableWall093A
 * @run 70:AECE
 * WHAT IT DOES: State 0 loads the record and draws it ($AED6); later states at $AEE0.
 * HOW: A branch by state.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BreakableWall093A(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (NE) {
        run_routine(0xAEE0);
        return;
    }
    JSR(0xAED6, k_load_record());
    run_routine(0xAED6);
}

void abi_BreakableWall093A(void)
{
    BreakableWall093A(cpu.x);
}

/*
 * @routine 093A:6D6 DrawWallWhole093A
 * @run 70:AED6
 * WHAT IT DOES: The whole wall's tile script, $0F.
 * HOW: Into DrawWallScript093A.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DrawWallWhole093A(uint8_t slot)
{
    (void)slot;
    lda(0x0F);
    run_routine(0xAED8);
}

void abi_DrawWallWhole093A(void)
{
    DrawWallWhole093A(cpu.x);
}

/*
 * @routine 093A:6D8 DrawWallScript093A
 * @run 70:AED8
 * WHAT IT DOES: Draws the tile script A plus OBJ_START_ARG.
 * HOW: A tail jump to DrawTileScript.
 * WAITS: no.
 * Entry: A the script's base, X object. Exit: as DrawTileScript.
 */
void DrawWallScript093A(uint8_t slot)
{
    clc();
    adc_a(RD8(OBJ->start_arg[slot]));
    cpu.y = 0;
    k_draw_tiles();
}

void abi_DrawWallScript093A(void)
{
    DrawWallScript093A(cpu.x);
}

/*
 * @routine 093A:6E0 BreakableWallBroken093A
 * @run 70:AEE0
 * WHAT IT DOES: State 1: while the tiles beneath are not the wall's, redraws it; a hit of 1 or 2 shows the broken frame and waits
 * $20. State 2 counts and draws script $16 plus the argument. Later states fall ($B1E4).
 * HOW: A read of VRAM at the wall's column.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BreakableWallBroken093A(uint8_t slot)
{
    dec_a();
    if (NE) {
        dec_a();
        if (EQ) {
            abi_nz(DEC8(OBJ->work_d[slot]));
            if (NE) {
                return;
            }
            lda(0x16);
            call_routine(0xAF38, 0xAED8);
            abi_nz(INC8(OBJ->state[cpu.x]));
        }
        run_routine(0xB1E4);
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    asl_a();
    WR8(ZP->temp00, cpu.a);
    JSR(0xAEEC, k_select_vram_read());
    lda(0x31);
    clc();
    adc_a(RD8(ZP->temp00));
    WR8(VDC->data_lo, cpu.a);
    lda(0x03);
    WR8(VDC->data_hi, cpu.a);
    JSR(0xAEFC, k_select_vram_data());
    for (int i = 0; i < 2; i++) {
        lda(RD8(VDC->data_lo));
        ldy(RD8(VDC->data_hi));
        cmp(cpu.a, 0x4D);
        if (EQ) {
            run_routine(0xAED6);
            return;
        }
    }
    slot = cpu.x;
    lda(RD8(OBJ->hit_taken[slot]));
    if (EQ) {
        return;
    }
    cmp(cpu.a, 0x03);
    if (CS) {
        return;
    }
    ldy(RD8(OBJ->start_arg[slot]));
    lda(rd8((uint16_t)(0xAF3E + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x20);
    WR8(OBJ->work_d[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_BreakableWallBroken093A(void)
{
    BreakableWallBroken093A(cpu.x);
}

/*
 * @routine 093A:745 BannerSpelledOut093A
 * @run 70:AF45
 * WHAT IT DOES: State 0 loads the record, starts script $86 in slot 5, clears the text area ($B001) and takes the text's address
 * by argument. State 1 writes one character every eighth frame until the text's zero, sounding $3B except for
 * Shift-JIS $8140. Then counts $40 and clears itself.
 * HOW: Shift-JIS pairs read through zTemp00.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void BannerSpelledOut093A(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xAF4D, k_load_record());
        phx();
        lda(0x86);
        ldx(0x05);
        JSR(0xAF55, k_script1());
        plx();
        call_routine(0xAF59, 0xB001);
        slot = cpu.x;
        lda(0xD0);
        WR8(OBJ->x_lo[slot], cpu.a);
        lda(0x16);
        WR8(OBJ->y_lo[slot], cpu.a);
        lda(RD8(OBJ->start_arg[slot]));
        asl_a();
        tay();
        lda(rd8((uint16_t)(0xAFC3 + cpu.y)));
        WR8(OBJ->work_e[slot], cpu.a);
        lda(rd8((uint16_t)(0xAFC4 + cpu.y)));
        WR8(OBJ->work_f[slot], cpu.a);
        return;
    }
    dec_a();
    if (EQ) {
        lda(RD8(ZP->wave_counter));
        and_a(0x07);
        if (NE) {
            return;
        }
        lda(RD8(OBJ->work_e[slot]));
        WR8(ZP->temp00, cpu.a);
        lda(RD8(OBJ->work_f[slot]));
        WR8(ZP->temp01, cpu.a);
        lda(RD8_PTR(ZP->temp00));
        if (EQ) {
            lda(0x40);
            WR8(OBJ->work_g[slot], cpu.a);
            abi_nz(INC8(OBJ->state[slot]));
            return;
        }
        {
            bool silent = false;

            cmp(cpu.a, 0x81);
            if (EQ) {
                ldy(0x01);
                lda(RD8_PTR_Y(ZP->temp00, cpu.y));
                cmp(cpu.a, 0x40);
                silent = EQ;
            }
            if (!silent) {
                lda(0x3B);
                JSR(0xAF9D, k_sound());
            }
        }
        call_routine(0xAFA0, 0xB022);
        slot = cpu.x;
        abi_nz(INC8(OBJ->work_d[slot]));
        lda(RD8(OBJ->work_e[slot]));
        clc();
        adc_a(0x02);
        WR8(OBJ->work_e[slot], cpu.a);
        if (CS) {
            abi_nz(INC8(OBJ->work_f[slot]));
        }
        return;
    }
    abi_nz(DEC8(OBJ->work_g[slot]));
    if (EQ) {
        k_clear();
    }
}

void abi_BannerSpelledOut093A(void)
{
    BannerSpelledOut093A(cpu.x);
}

/*
 * @routine 093B:001 ClearBannerArea093A
 * @run 70:B001
 * WHAT IT DOES: Clears $1000 VRAM words from $0C00.
 * HOW: ST1 and ST2 of zero, 16 times 64.
 * WAITS: no.
 * Entry: X kept. Exit: A as SelectVramData, Y 0.
 */
void ClearBannerArea093A(uint8_t slot)
{
    (void)slot;
    JSR(0xB004, k_select_vram_write());
    lda(0x00);
    WR8(VDC->data_lo, cpu.a);
    lda(0x0C);
    WR8(VDC->data_hi, cpu.a);
    JSR(0xB011, k_select_vram_data());
    phx();
    ldy(0x10);
    do {
        ldx(0x40);
        do {
            st1(0x00);
            st2(0x00);
            dex();
        } while (NE);
        dey();
    } while (NE);
    plx();
}

void abi_ClearBannerArea093A(void)
{
    ClearBannerArea093A(cpu.x);
}

/*
 * @routine 093B:022 WriteBannerGlyph093A
 * @run 70:B022
 * WHAT IT DOES: Reads the card's glyph for the Shift-JIS pair at (zTemp00), shifts its lines three pixels, and writes it at the
 * VRAM place for OBJ_PIECE_NEXT, its shadow plane half a line on.
 * HOW: ex_getfnt into StageMaskBuffer's page, then the VDC.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WriteBannerGlyph093A(uint8_t slot)
{
    (void)slot;
    cpu.y = 0;
    lda(RD8_PTR_Y(ZP->temp00, cpu.y));
    WR8(ZP->bios_a.hi, cpu.a);
    iny();
    lda(RD8_PTR_Y(ZP->temp00, cpu.y));
    WR8(ZP->bios_a.lo, cpu.a);
    lda(0x42);
    WR8(ZP->bios_b.lo, cpu.a);
    lda(0x3B);
    WR8(ZP->bios_b.hi, cpu.a);
    lda(0x01);
    WR8(ZP->bios_d.hi, cpu.a);
    phx();
    JSR(0xB03C, card_exit_to_cpu(vec_ex_getfnt()));
    plx();
    JSR(0xB040, k_select_vram_write());
    lda(RD8(OBJ->work_d[cpu.x]));
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xB0AF + cpu.y)));
    WR8(VDC->data_lo, cpu.a);
    lda(rd8((uint16_t)(0xB0B0 + cpu.y)));
    WR8(VDC->data_hi, cpu.a);
    JSR(0xB054, k_select_vram_data());
    wr8(StageMaskBuffer, 0);
    wr8((uint16_t)(StageMaskBuffer + 1), 0);
    phx();
    cpu.x = 0;
    do {
        for (int i = 0; i < 3; i++) {
            abi_flag(P_C, lsr_mem((uint16_t)(StageMaskBuffer + cpu.x)));
            abi_flag(P_C, ror_mem((uint16_t)(StageMaskBuffer + 1 + cpu.x), CS));
        }
        inx();
        inx();
        cmp(cpu.x, 0x20);
    } while (NE);
    cpu.x = 0;
    do {
        lda(rd8((uint16_t)(StageMaskBuffer + 1 + cpu.x)));
        WR8(VDC->data_lo, cpu.a);
        lda(rd8((uint16_t)(StageMaskBuffer + cpu.x)));
        WR8(VDC->data_hi, cpu.a);
        inx();
        inx();
        cmp(cpu.x, 0x20);
    } while (NE);
    st1(0x00);
    st2(0x00);
    inx();
    inx();
    do {
        lda(rd8((uint16_t)(0x3B1E + cpu.x)));
        lsr_a();
        pha();
        lda(rd8((uint16_t)(0x3B1F + cpu.x)));
        ror_a();
        WR8(VDC->data_lo, cpu.a);
        pla();
        WR8(VDC->data_hi, cpu.a);
        inx();
        inx();
        cmp(cpu.x, 0x40);
    } while (NE);
    st1(0x00);
    st2(0x00);
    do {
        st1(0x00);
        st2(0x00);
        inx();
        inx();
        cmp(cpu.x, 0x80);
    } while (NE);
    plx();
}

void abi_WriteBannerGlyph093A(void)
{
    WriteBannerGlyph093A(cpu.x);
}

/*
 * @routine 093B:0CF ExitWhenDoorHit093A
 * @run 70:B0CF
 * WHAT IT DOES: State 0 loads the record. With the door ready and not held, a hit of 1 or 2 sets zField8C to $80 plus
 * OBJ_START_ARG and sub-step 3.
 * HOW: As FadeOutWhenDoorHit051B without the fade.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ExitWhenDoorHit093A(uint8_t slot)
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
}

void abi_ExitWhenDoorHit093A(void)
{
    ExitWhenDoorHit093A(cpu.x);
}

/*
 * @routine 093B:104 CrumblingColumn093A
 * @run 70:B104
 * WHAT IT DOES: State 0 loads the record and draws it whole ($B10C); later states at $B119.
 * HOW: A branch by state.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CrumblingColumn093A(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (NE) {
        run_routine(0xB119);
        return;
    }
    JSR(0xB10C, k_load_record());
    run_routine(0xB10C);
}

void abi_CrumblingColumn093A(void)
{
    CrumblingColumn093A(cpu.x);
}

/*
 * @routine 093B:10C DrawColumnWhole093A
 * @run 70:B10C
 * WHAT IT DOES: Four pieces left: draws tile script 4 plus OBJ_START_ARG.
 * HOW: A tail jump to DrawTileScript.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void DrawColumnWhole093A(uint8_t slot)
{
    lda(0x04);
    WR8(OBJ->work_d[slot], cpu.a);
    clc();
    adc_a(RD8(OBJ->start_arg[slot]));
    cpu.y = 0;
    k_draw_tiles();
}

void abi_DrawColumnWhole093A(void)
{
    DrawColumnWhole093A(cpu.x);
}

/*
 * @routine 093B:119 CrumbleColumnByTile093A
 * @run 70:B119
 * WHAT IT DOES: State 1: redraws it whole while the tile beneath is $15 or more; below X $E0 goes on with a wait of $10. State 2
 * crumbles one piece per wait with sound $91, then drops five kind $37 stones and records the event. State 3 draws
 * the stump and clears itself.
 * HOW: A read of VRAM under the column.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CrumbleColumnByTile093A(uint8_t slot)
{
    dec_a();
    if (EQ) {
        lda(RD8(OBJ->start_arg[slot]));
        lsr_a();
        and_a(0x06);
        tay();
        JSR(0xB126, k_select_vram_read());
        lda(rd8((uint16_t)(0xB19F + cpu.y)));
        WR8(VDC->data_lo, cpu.a);
        lda(rd8((uint16_t)(0xB1A0 + cpu.y)));
        WR8(VDC->data_hi, cpu.a);
        JSR(0xB135, k_select_vram_data());
        lda(RD8(VDC->data_lo));
        ldy(RD8(VDC->data_hi));
        cmp(cpu.a, 0x15);
        if (CS) {
            run_routine(0xB10C);
            return;
        }
        slot = cpu.x;
        lda(RD8(OBJ->x_hi[slot]));
        if (NE) {
            return;
        }
        lda(RD8(OBJ->x_lo[slot]));
        cmp(cpu.a, 0xE0);
        if (CS) {
            return;
        }
        abi_nz(INC8(OBJ->state[slot]));
        lda(0x10);
        WR8(OBJ->work_e[slot], cpu.a);
        return;
    }
    dec_a();
    if (NE) {
        lda(RD8(OBJ->start_arg[slot]));
        cpu.y = 0;
        JSR(0xB19C, k_draw_tiles());
        k_clear();
        return;
    }
    abi_nz(DEC8(OBJ->work_e[slot]));
    if (NE) {
        return;
    }
    abi_nz(DEC8(OBJ->work_d[slot]));
    lda(RD8(OBJ->work_d[slot]));
    if (NE) {
        clc();
        adc_a(RD8(OBJ->start_arg[slot]));
        cpu.y = 0;
        JSR(0xB168, k_draw_tiles());
        slot = cpu.x;
    }
    lda(0x91);
    JSR(0xB16D, k_sound());
    slot = cpu.x;
    lda(RD8(OBJ->work_d[slot]));
    if (NE) {
        lda(0x10);
        WR8(OBJ->work_e[slot], cpu.a);
        return;
    }
    lda(0x05);
    WR8(ZP->temp00, cpu.a);
    do {
        JSR(0xB179, k_free_from30());
        if (CC) {
            break;
        }
        lda(0x37);
        JSR(0xB180, k_start_at_place());
        lda(RD8(ZP->temp00));
        WR8(OBJ->start_arg[cpu.y], cpu.a);
        abi_nz(DEC8(ZP->temp00));
    } while (PL);
    abi_nz(INC8(OBJ->state[cpu.x]));
    k_record_event();
}

void abi_CrumbleColumnByTile093A(void)
{
    CrumbleColumnByTile093A(cpu.x);
}

/*
 * @routine 093B:1A5 FallingStone093A
 * @run 70:B1A5
 * WHAT IT DOES: State 0 loads the record as sprite set 6, its frame and place offsets and Y speed by argument; later falls.
 * HOW: Tables by argument.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FallingStone093A(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (NE) {
        run_routine(0xB1E4);
        return;
    }
    JSR(0xB1AD, k_load_record());
    slot = cpu.x;
    lda(0x06);
    WR8(OBJ->sprite_set[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    tay();
    clc();
    adc_a(0x14);
    WR8(OBJ->frame[slot], cpu.a);
    lda(rd8((uint16_t)(0xB208 + cpu.y)));
    clc();
    adc_a(RD8(OBJ->x_lo[slot]));
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(0xFF);
    adc_a(RD8(OBJ->x_hi[slot]));
    WR8(OBJ->x_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xB20E + cpu.y)));
    clc();
    adc_a(RD8(OBJ->y_lo[slot]));
    WR8(OBJ->y_lo[slot], cpu.a);
    if (CS) {
        abi_nz(INC8(OBJ->y_hi[slot]));
    }
    lda(rd8((uint16_t)(0xB214 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
}

void abi_FallingStone093A(void)
{
    FallingStone093A(cpu.x);
}

/*
 * @routine 093B:1E4 FallStone093A
 * @run 70:B1E4
 * WHAT IT DOES: Checks the player (box $0C by $1D), falls a little faster, and is cleared past the first screen.
 * HOW: ContactWithPlayer, ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FallStone093A(uint8_t slot)
{
    lda(0x0C);
    WR8(ZP->temp08, cpu.a);
    lda(0x1D);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xB1EF, 0xAA35);
    slot = cpu.x;
    lda(RD8(OBJ->vy_lo[slot]));
    clc();
    adc_a(0x14);
    WR8(OBJ->vy_lo[slot], cpu.a);
    if (CS) {
        abi_nz(INC8(OBJ->vy_hi[slot]));
    }
    JSR(0xB200, k_move());
    lda(RD8(OBJ->y_hi[cpu.x]));
    if (NE) {
        k_clear();
    }
}

void abi_FallStone093A(void)
{
    FallStone093A(cpu.x);
}

/*
 * @routine 093B:21A WavingBackdrop093A
 * @run 70:B21A
 * WHAT IT DOES: State 0 with argument 0 starts list channel 3 with list 0; loads the record. Then draws the wave of rows by the
 * second plane's scroll ($B24C).
 * HOW: The scroll of plane 1 doubled, plus 8, as the phase.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void WavingBackdrop093A(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        lda(RD8(OBJ->start_arg[slot]));
        if (EQ) {
            phx();
            cpu.a = 0;
            ldx(0x05);
            JSR(0xB22B, k_list3());
            plx();
        }
        JSR(0xB22F, k_load_record());
    }
    lda(RD8(SCROLL->x_frac[1]));
    sec();
    sbc_a(RD8(SCROLLE->step_x_lo[1]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(SCROLL->x_lo[1]));
    sbc_a(RD8(SCROLLE->step_x_hi[1]));
    WR8(ZP->temp01, cpu.a);
    abi_flag(P_C, asl_mem(zTemp00));
    lda(RD8(ZP->temp01));
    adc_a(0x08);
    and_a(0x1F);
    call_routine(0xB24B, 0xB24C);
}

void abi_WavingBackdrop093A(void)
{
    WavingBackdrop093A(cpu.x);
}

/*
 * @routine 093B:24C CopyWaveColumns093A
 * @run 70:B24C
 * WHAT IT DOES: Copies twelve eight-word runs between VRAM addresses: from the pairs at the table for the phase's low bits, to
 * the table of destinations turned by the phase's high bits.
 * HOW: The VDC's read and write addresses per run.
 * WAITS: no.
 * Entry: A the phase. Exit: as the last instruction left them.
 */
void CopyWaveColumns093A(uint8_t slot)
{
    (void)slot;
    pha();
    and_a(0x07);
    asl_a();
    tay();
    lda(rd8((uint16_t)(0xB2D5 + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    lda(rd8((uint16_t)(0xB2D6 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    WR8(ZP->temp02, 0);
    pla();
    lsr_a();
    lsr_a();
    lsr_a();
    WR8(ZP->temp10, cpu.a);
    do {
        JSR(0xB266, k_clear_vdc_high());
        JSR(0xB269, k_select_vram_read());
        lda(RD8_PTR(ZP->temp00));
        WR8(VDC->data_lo, cpu.a);
        ldy(0x01);
        lda(RD8_PTR_Y(ZP->temp00, cpu.y));
        WR8(VDC->data_hi, cpu.a);
        JSR(0xB278, k_select_vram_write());
        lda(RD8(ZP->temp02));
        and_a(0xFC);
        WR8(ZP->temp03, cpu.a);
        lda(RD8(ZP->temp02));
        clc();
        adc_a(RD8(ZP->temp10));
        and_a(0x03);
        ora_a(RD8(ZP->temp03));
        asl_a();
        tay();
        lda(rd8((uint16_t)(0xB2BD + cpu.y)));
        WR8(VDC->data_lo, cpu.a);
        lda(rd8((uint16_t)(0xB2BE + cpu.y)));
        WR8(VDC->data_hi, cpu.a);
        JSR(0xB298, k_select_vram_data());
        ldy(0x08);
        do {
            lda(RD8(VDC->data_lo));
            WR8(VDC->data_lo, cpu.a);
            lda(RD8(VDC->data_hi));
            WR8(VDC->data_hi, cpu.a);
            dey();
        } while (NE);
        lda(0x02);
        clc();
        adc_a(RD8(ZP->temp00));
        WR8(ZP->temp00, cpu.a);
        if (CS) {
            abi_nz(INC8(ZP->temp01));
        }
        abi_nz(INC8(ZP->temp02));
        lda(RD8(ZP->temp02));
        cmp(cpu.a, 0x0C);
    } while (NE);
}

void abi_CopyWaveColumns093A(void)
{
    CopyWaveColumns093A(cpu.x);
}

/*
 * @routine 093B:3D7 ByState093AB3D7
 * @run 70:B3D7
 * WHAT IT DOES: Goes to the state's handler.
 * HOW: A table of handlers at $B3E9.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ByState093AB3D7(uint8_t slot)
{
    jump_by_state(slot, 0xB3E9);
}

void abi_ByState093AB3D7(void)
{
    ByState093AB3D7(cpu.x);
}

/*
 * @routine 093B:3EF ThrowWhenStruck093A
 * @run 70:B3EF
 * WHAT IT DOES: Holds the object it carries ($B423); five hits throw a kind $8B object, record the event, sound $91 and wait $40.
 * HOW: FindFreeSlotFrom30 and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void ThrowWhenStruck093A(uint8_t slot)
{
    call_routine(0xB3F2, 0xB423);
    slot = cpu.x;
    lda(RD8(OBJ->start_arg[slot]));
    if (MI) {
        return;
    }
    lda(RD8(OBJ->hit_taken[slot]));
    cmp(cpu.a, 0x05);
    if (CC) {
        return;
    }
    JSR(0xB401, k_free_from30());
    if (CC) {
        return;
    }
    lda(0x8B);
    JSR(0xB408, k_start_at_place());
    lda(0xFB);
    WR8(OBJ->vy_hi[cpu.y], cpu.a);
    lda(0x16);
    WR8(OBJ->invuln_timer[cpu.y], cpu.a);
    JSR(0xB415, k_record_event());
    lda(0x91);
    JSR(0xB41A, k_sound());
    lda(0x40);
    WR8(OBJ->work_d[cpu.x], cpu.a);
    abi_nz(INC8(OBJ->state[cpu.x]));
}

void abi_ThrowWhenStruck093A(void)
{
    ThrowWhenStruck093A(cpu.x);
}

/*
 * @routine 093B:423 CarryChild093A
 * @run 70:B423
 * WHAT IT DOES: While the object OBJ_START_ARG names is kind $2E under state 3: places it here, clears it off the first screen,
 * else lifts it at -$1A00.
 * HOW: CopyObjectPlace into that slot.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void CarryChild093A(uint8_t slot)
{
    lda(RD8(OBJ->start_arg[slot]));
    and_a(0x7F);
    tay();
    lda(RD8(OBJ->kind[cpu.y]));
    cmp(cpu.a, 0x2E);
    if (NE) {
        return;
    }
    lda(RD8(OBJ->state[cpu.y]));
    cmp(cpu.a, 0x03);
    if (CS) {
        return;
    }
    JSR(0xB43A, k_copy_place());
    lda(RD8(OBJ->y_hi[cpu.y]));
    if (NE) {
        k_clear_in_y();
        return;
    }
    lda(0xE6);
    WR8(OBJ->vy_hi[cpu.y], cpu.a);
    sxy();
    JSR(0xB44B, k_move_y());
    sxy();
}

void abi_CarryChild093A(void)
{
    CarryChild093A(cpu.x);
}

/*
 * @routine 093B:44D FallCarryingThenBurst093A
 * @run 70:B44D
 * WHAT IT DOES: Holds the child, falls, and counts out; at eight counts on the way sparks ($CE43).
 * HOW: A search of the counts.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void FallCarryingThenBurst093A(uint8_t slot)
{
    (void)slot;
    call_routine(0xB450, 0xB423);
    JSR(0xB453, k_gravity_unlimited());
    JSR(0xB456, k_move_y());
    abi_nz(DEC8(OBJ->work_d[cpu.x]));
    if (EQ) {
        k_clear();
        return;
    }
    cpu.y = 0;
    do {
        lda(RD8(OBJ->work_d[cpu.x]));
        cmp(cpu.a, rd8((uint16_t)(0xB470 + cpu.y)));
        if (EQ) {
            run_routine(0xCE43);
            return;
        }
        iny();
        cmp(cpu.y, 0x08);
    } while (CC);
}

void abi_FallCarryingThenBurst093A(void)
{
    FallCarryingThenBurst093A(cpu.x);
}

/*
 * @routine 093B:478 PlatformOnWater093A
 * @run 70:B478
 * WHAT IT DOES: State 0 loads the record as sprite set 6 frame $1B and keeps its Y. Then floats toward Y $A0 ($DA9A) with the
 * player's weight (box $18 by $1D), dipping once as they step on.
 * HOW: FloatToLine093F, ContactWithPlayer.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void PlatformOnWater093A(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (EQ) {
        JSR(0xB480, k_load_record());
        slot = cpu.x;
        lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
        lda(0x1B);
        WR8(OBJ->frame[slot], cpu.a);
        lda(RD8(OBJ->y_lo[slot]));
        WR8(OBJ->work_e[slot], cpu.a);
        return;
    }
    lda(0xA0);
    call_routine(0xB496, 0xDA9A);
    lda(0x18);
    WR8(ZP->temp08, cpu.a);
    lda(0x1D);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xB4A1, 0xAA35);
    slot = cpu.x;
    if (CC) {
        WR8(OBJ->work_d[slot], 0);
        return;
    }
    lda(RD8(OBJ->work_d[slot]));
    if (EQ) {
        lda(RD8(OBJ->y_lo[slot]));
        cmp(cpu.a, 0xA0);
        if (EQ) {
            lda(0x01);
            WR8(OBJ->vy_hi[slot], cpu.a);
            lda(0xC0);
            WR8(OBJ->vy_lo[slot], cpu.a);
        }
    }
    lda(0x01);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_PlatformOnWater093A(void)
{
    PlatformOnWater093A(cpu.x);
}

/*
 * @routine 093B:525 LoomingPicture093A
 * @run 70:B525
 * WHAT IT DOES: State as PictureByArgument093A, drawn with attribute $87 and loop flag bit 1; at X $01xx past $17F wraps to the
 * left.
 * HOW: A call and stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction, call or jump left them.
 */
void LoomingPicture093A(uint8_t slot)
{
    call_routine(0xB528, 0xAE89);
    slot = cpu.x;
    lda(0x87);
    WR8(OBJ->sprite_attr[slot], cpu.a);
    lda(RD8(OBJ->loop_flags[slot]));
    ora_a(0x02);
    WR8(OBJ->loop_flags[slot], cpu.a);
    lda(RD8(OBJ->x_hi[slot]));
    cmp(cpu.a, 0x01);
    if (NE) {
        return;
    }
    lda(RD8(OBJ->x_lo[slot]));
    if (PL) {
        return;
    }
    lda(0xFF);
    WR8(OBJ->x_hi[slot], cpu.a);
}

void abi_LoomingPicture093A(void)
{
    LoomingPicture093A(cpu.x);
}

/*
 * @routine 093E:643 SparkOfRow093E
 * @run 71:CE43
 * WHAT IT DOES: Starts a kind $9C spark in a free slot with sound $18 and the row's X speed, and moves it.
 * HOW: As SparkByCount07DB for row Y.
 * WAITS: no.
 * Entry: X object, Y the row. Exit: as the last instruction left them.
 */
void SparkOfRow093E(uint8_t slot)
{
    (void)slot;
    WR8(ZP->climb_step_before, cpu.x);
    WR8(ZP->burst_count, cpu.y);
    JSR(0xCE4A, k_free_from30());
    if (CC) {
        return;
    }
    lda(0x9C);
    JSR(0xCE51, k_start_at_place());
    lda(0x18);
    JSR(0xCE56, k_sound());
    lda(RD8(ZP->burst_count));
    asl_a();
    tax();
    lda(rd8((uint16_t)(0xCE70 + cpu.x)));
    WR8(OBJ->vx_hi[cpu.y], cpu.a);
    sxy();
    JSR(0xCE64, k_move());
    sxy();
    ldx(RD8(ZP->climb_step_before));
}

void abi_SparkOfRow093E(void)
{
    SparkOfRow093E(cpu.x);
}

/*
 * @routine 0940:29A FloatToLine0940
 * @run 71:DA9A
 * WHAT IT DOES: Moves down the Y speed; at or past the line in A stops there, else eases the speed upward by $40.
 * HOW: ApplyVelocityY and a compare.
 * WAITS: no.
 * Entry: A the line, X object. Exit: as the last instruction left them.
 */
void FloatToLine0940(uint8_t slot)
{
    WR8(ZP->temp00, cpu.a);
    JSR(0xDA9F, k_move_y());
    slot = cpu.x;
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, RD8(ZP->temp00));
    if (NE && CS) {
        lda(RD8(OBJ->vy_lo[slot]));
        sec();
        sbc_a(0x40);
        WR8(OBJ->vy_lo[slot], cpu.a);
        lda(RD8(OBJ->vy_hi[slot]));
        sbc_a(0x00);
        WR8(OBJ->vy_hi[slot], cpu.a);
        return;
    }
    lda(RD8(ZP->temp00));
    WR8(OBJ->y_lo[slot], cpu.a);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
}

void abi_FloatToLine0940(void)
{
    FloatToLine0940(cpu.x);
}
