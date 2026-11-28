/*
 * The text object of the cutscene overlays of scenes $58-$59 (sectors $267B-$267E) and $5A-$5B ($2703-$2706): an object of
 * kind 1 that writes a line of the card's 16 by 16 font (or of the overlay's own patterns) into VRAM a glyph a frame, then
 * shows it as a sprite that rises and is cleared. Both overlays hold the same instructions $EE bytes apart, so each routine
 * is one body taking its run address, with a wrapper at each place. Also the $5A overlay's quake and jitter scroll objects
 * (kinds 15 and 14) and the jump at $8003 of the overlay at sector $267F.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "ops6280.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"
#include "richter.h"
#include "stage_ops_c.h"
#include "scene_text_c.h"

/* What a card call left, applied to the registers and flags (the card's card_exit_apply, card.h, which the game's
 * machine model cannot include beside its own). */
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

/* SceneTextObjectAt: the body of the places below, at: the place's run address. */
enum {
    SceneTextObjectStates = 0x12,   /* the five steps' addresses */
};

void SceneTextObjectAt(uint16_t at, uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)((uint16_t)(at + SceneTextObjectStates) + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)((uint16_t)(at + SceneTextObjectStates + 1) + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

/*
 * @routine 267B:7AB Scene58TextObject
 * @run 6B:87AB
 * WHAT IT DOES: An object handler (kind 1): runs the step of the object's state from the table after it. (Scene
 * $58's overlay.)
 * HOW: OBJ_STATE doubled indexes five words; a jump through zTemp00.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene58TextObject(uint8_t slot)
{
    SceneTextObjectAt(0x87AB, slot);
}

void abi_Scene58TextObject(void)
{
    Scene58TextObject(cpu.x);
}

/* SceneTextStartAt: the body of the places below, at: the place's run address. */
enum {
    SceneTextStartLines = 0x21B,   /* the address of each line, a word each */
    SceneTextStartVramRows = 0x6AB,   /* the VRAM address of each line place, a word each */
};

void SceneTextStartAt(uint16_t at)
{
    op_lda(RD8(OBJ->start_arg[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    jsr_push((uint16_t)(at + 0x8));
    SelectVramWriteAddress();
    cpu.a = STAGE_C_VDC_MAWR;
    jsr_pop();
    op_lda(rd8((uint16_t)((uint16_t)(at + SceneTextStartVramRows) + cpu.y)));
    WR8(VDC->data_lo, cpu.a);
    op_lda(rd8((uint16_t)((uint16_t)(at + SceneTextStartVramRows + 1) + cpu.y)));
    WR8(VDC->data_hi, cpu.a);
    jsr_push((uint16_t)(at + 0x17));
    SelectVramData();
    cpu.a = STAGE_C_VDC_VRAM;
    abi_nz(STAGE_C_VDC_VRAM);
    jsr_pop();
    op_lda(0xA0);
    WR8(OBJ->vy_lo[cpu.x], cpu.a);
    op_lda(0xFF);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    op_phx();
    op_lda(0x04);
    WR8(ZP->temp00, cpu.a);
    op_clx();
at_plus_027:
    op_st1(0x00);
    op_st2(0x00);
    op_dex();
    if (!flag_z()) {
        goto at_plus_027;
    }
    RMW8(ZP->temp00, v, f_dec(v));
    if (!flag_z()) {
        goto at_plus_027;
    }
    op_plx();
    op_lda(RD8(OBJ->work_d[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)((uint16_t)(at + SceneTextStartLines) + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)((uint16_t)(at + SceneTextStartLines + 1) + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    op_ldy(0x01);
    op_lda(RD8_PTR_Y(ZP->temp10, cpu.y));
    WR8(OBJ->sprite_attr[cpu.x], cpu.a);
    op_lda(0x12);
    WR8(OBJ->work_h[cpu.x], cpu.a);
    RMW8(OBJ->state[cpu.x], v, f_inc(v));
}

/*
 * @routine 267B:7C7 Scene58TextStart
 * @run 6B:87C7
 * WHAT IT DOES: The text object's state 0: points the VDC at the line's VRAM place by OBJ_START_ARG, clears $400
 * words there, sets a rising Y speed of -$60, takes the line OBJ_WORK_D names, its sprite attribute from the line's
 * second byte, a wait of $12 in OBJ_WORK_H, and steps the state on. (Scene $58's overlay.)
 * HOW: SelectVramWriteAddress, a table word into the address port, SelectVramData, a loop of $400 ST1/ST2 zeros;
 * table reads.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene58TextStart(void)
{
    SceneTextStartAt(0x87C7);
}

void abi_Scene58TextStart(void)
{
    Scene58TextStart();
}

/* SceneTextSkipSpaceAt: the body of the places below, at: the place's run address. */
void SceneTextSkipSpaceAt(uint16_t at, uint8_t slot)
{
    (void)at;   /* nothing in it moves with its place */
    RMW8(OBJ->work_g[slot], v, f_inc(v));
}

/*
 * @routine 267C:019 Scene58TextSkipSpace
 * @run 6B:8819
 * WHAT IT DOES: Steps the pattern line's code on (OBJ_WORK_G), passing a space. (Scene $58's overlay.)
 * HOW: INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene58TextSkipSpace(uint8_t slot)
{
    SceneTextSkipSpaceAt(0x8819, slot);
}

void abi_Scene58TextSkipSpace(void)
{
    Scene58TextSkipSpace(cpu.x);
}

/* SceneTextPatternsAt: the body of the places below, at: the place's run address. */
void SceneTextPatternsAt(uint16_t at, uint8_t slot)
{
    WR8(ZP->temp08, 0);
    WR8(ZP->bios_a.lo, 0);
    jsr_push((uint16_t)(at + 0x7));
    SceneTextGlyphVramAddressAt((uint16_t)(at + 0x163), slot);
    jsr_pop();
    op_ldy(RD8(OBJ->work_g[slot]));
    op_iny();
    op_iny();
    op_lda(RD8_PTR_Y(ZP->temp10, cpu.y));
    op_cmp(0x20);
    if (flag_z()) {
        SceneTextSkipSpaceAt((uint16_t)(at - 0x4), slot);
        return;
    }
    WR8(ZP->temp00, 0);
    cpu.a = f_lsr(cpu.a);
    RMW8(ZP->temp00, v, f_ror(v));
    op_ora(0xB0);
    WR8(ZP->temp01, cpu.a);
    op_cly();
    do {
        op_lda(RD8_PTR_Y(ZP->temp00, cpu.y));
        WR8(VDC->data_lo, cpu.a);
        op_iny();
        op_lda(RD8_PTR_Y(ZP->temp00, cpu.y));
        WR8(VDC->data_hi, cpu.a);
        op_iny();
        op_cpy(0x80);
    } while (!flag_z());
    RMW8(OBJ->work_g[slot], v, f_inc(v));
    op_ldy(RD8(OBJ->work_g[slot]));
    op_iny();
    op_iny();
    op_lda(RD8_PTR_Y(ZP->temp10, cpu.y));
    if (flag_n()) {
        SceneTextLineDoneAt((uint16_t)(at + 0x87), slot);
        return;
    }
}

/*
 * @routine 267C:01D Scene58TextPatterns
 * @run 6B:881D
 * WHAT IT DOES: A pattern line: for each code of the line copies $40 words of the overlay's pattern bank (from
 * $B000 by the code) to the glyph's VRAM place, skipping code $20; at the line's end the object is placed and its
 * state stepped on. (Scene $58's overlay.)
 * HOW: The VRAM address set; the code halved into the pattern's address; a loop of $80 bytes to the data port.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene58TextPatterns(uint8_t slot)
{
    SceneTextPatternsAt(0x881D, slot);
}

void abi_Scene58TextPatterns(void)
{
    Scene58TextPatterns(cpu.x);
}

/* SceneTextWriteAt: the body of the places below, at: the place's run address. */
enum {
    SceneTextWriteLines = 0x189,   /* the address of each line, a word each */
};

void SceneTextWriteAt(uint16_t at)
{
    RMW8(OBJ->work_h[cpu.x], v, f_dec(v));
    op_lda(RD8(OBJ->work_d[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)((uint16_t)(at + SceneTextWriteLines) + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)((uint16_t)(at + SceneTextWriteLines + 1) + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    op_lda(RD8(OBJ->sprite_attr[cpu.x]));
    if (flag_n()) {
        SceneTextPatternsAt((uint16_t)(at - 0x3C), cpu.x);
        return;
    }
    op_lda(RD8(OBJ->work_g[cpu.x]));
    cpu.a = f_inc(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(RD8_PTR_Y(ZP->temp10, cpu.y));
    WR8(ZP->bios_a.hi, cpu.a);
    op_cmp(0xE0);
    if (flag_z()) {
        SceneTextBlankGlyphAt((uint16_t)(at + 0x5E));
        return;
    }
    op_iny();
    op_lda(RD8_PTR_Y(ZP->temp10, cpu.y));
    WR8(ZP->bios_a.lo, cpu.a);
    op_lda(0x40);
    WR8(ZP->bios_b.lo, cpu.a);
    op_lda(0x3B);
    WR8(ZP->bios_b.hi, cpu.a);
    op_lda(0x00);
    WR8(ZP->bios_d.hi, cpu.a);
    op_phx();
    jsr_push((uint16_t)(at + 0x3A));
    card_exit_to_cpu(vec_ex_getfnt());
    jsr_pop();
    op_plx();
    /* falls into the routine after it */
    SceneTextNextGlyphAt((uint16_t)(at + 0x3B));
}

/*
 * @routine 267C:059 Scene58TextWrite
 * @run 6B:8859
 * WHAT IT DOES: The text object's state 1: counts OBJ_WORK_H down and takes the line again; a line marked by bit 7
 * of the sprite attribute is drawn from the overlay's patterns instead; otherwise the next code of the line
 * (OBJ_WORK_G) is looked up in the card's font into the buffer at $3B40, or a code $E0.. is the blank glyph, and
 * drawn. (Scene $58's overlay.)
 * HOW: The code's two bytes into zBiosAh and zBiosAl, zBiosB $3B40 and zBiosDh 0, ex_getfnt between PHX and PLX,
 * then the glyph drawn.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene58TextWrite(void)
{
    SceneTextWriteAt(0x8859);
}

void abi_Scene58TextWrite(void)
{
    Scene58TextWrite();
}

/* SceneTextNextGlyphAt: the body of the places below, at: the place's run address. */
void SceneTextNextGlyphAt(uint16_t at)
{
    jsr_push((uint16_t)(at + 0x3));
    SceneTextDrawGlyphAt((uint16_t)(at + 0x52));
    jsr_pop();
    RMW8(OBJ->work_g[cpu.x], v, f_inc(v));
    op_lda(RD8(OBJ->work_g[cpu.x]));
    cpu.a = f_inc(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(RD8_PTR_Y(ZP->temp10, cpu.y));
    if (!flag_z()) {
        SceneTextReturnAt((uint16_t)(at + 0x22));
        return;
    }
    /* falls into the routine after it */
    SceneTextLineDoneAt((uint16_t)(at + 0x10), cpu.x);
}

/*
 * @routine 267C:094 Scene58TextNextGlyph
 * @run 6B:8894
 * WHAT IT DOES: Draws the glyph in the buffer at $3B40, steps OBJ_WORK_G on, and ends the line when the next code
 * is zero. (Scene $58's overlay.)
 * HOW: The draw, INC, a read through zTemp10.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene58TextNextGlyph(void)
{
    SceneTextNextGlyphAt(0x8894);
}

void abi_Scene58TextNextGlyph(void)
{
    Scene58TextNextGlyph();
}

/* SceneTextLineDoneAt: the body of the places below, at: the place's run address. */
void SceneTextLineDoneAt(uint16_t at, uint8_t slot)
{
    op_lda(RD8_PTR(ZP->temp10));
    WR8(OBJ->x_lo[slot], cpu.a);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->work_g[slot], 0);
    op_cpx(0x2E);
    if (flag_z()) {
    }
    RMW8(OBJ->state[slot], v, f_inc(v));
    /* falls into the routine after it */
    SceneTextReturnAt((uint16_t)(at + 0x12));
}

/*
 * @routine 267C:0A4 Scene58TextLineDone
 * @run 6B:88A4
 * WHAT IT DOES: At the end of a line: the line's first byte becomes the object's X, its Y high byte and OBJ_WORK_G
 * are cleared and the state steps on. (Scene $58's overlay.)
 * HOW: Stores through zTemp10; a compare of X whose branch goes on either way.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene58TextLineDone(uint8_t slot)
{
    SceneTextLineDoneAt(0x88A4, slot);
}

void abi_Scene58TextLineDone(void)
{
    Scene58TextLineDone(cpu.x);
}

/* SceneTextReturnAt: the body of the places below, at: the place's run address. */
void SceneTextReturnAt(uint16_t at)
{
    (void)at;   /* nothing in it moves with its place */
}

/*
 * @routine 267C:0B6 Scene58TextReturn
 * @run 6B:88B6
 * WHAT IT DOES: Returns: the end the text steps branch to. (Scene $58's overlay.)
 * HOW: RTS.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene58TextReturn(void)
{
    SceneTextReturnAt(0x88B6);
}

void abi_Scene58TextReturn(void)
{
    Scene58TextReturn();
}

/* SceneTextBlankGlyphAt: the body of the places below, at: the place's run address. */
enum {
    SceneTextBlankGlyphBlank = 0xF,   /* the blank glyph's 32 bytes */
};

void SceneTextBlankGlyphAt(uint16_t at)
{
    op_cly();
    do {
        op_lda(rd8((uint16_t)((uint16_t)(at + SceneTextBlankGlyphBlank) + cpu.y)));
        WR8(STAGEWORK->mask_buffer[cpu.y], cpu.a);
        op_iny();
        op_cpy(0x20);
    } while (!flag_z());
    SceneTextNextGlyphAt((uint16_t)(at - 0x23));
}

/*
 * @routine 267C:0B7 Scene58TextBlankGlyph
 * @run 6B:88B7
 * WHAT IT DOES: For a code $E0..: copies the blank glyph's 32 bytes into the buffer at $3B40 and draws it. (Scene
 * $58's overlay.)
 * HOW: A copy loop, then the draw.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene58TextBlankGlyph(void)
{
    SceneTextBlankGlyphAt(0x88B7);
}

void abi_Scene58TextBlankGlyph(void)
{
    Scene58TextBlankGlyph();
}

/* SceneTextDrawGlyphAt: the body of the places below, at: the place's run address. */
void SceneTextDrawGlyphAt(uint16_t at)
{
    WR8(ZP->temp08, 0);
    WR8(ZP->bios_a.lo, 0);
    jsr_push((uint16_t)(at + 0x7));
    SceneTextGlyphVramAddressAt((uint16_t)(at + 0x9A), cpu.x);
    jsr_pop();
    op_phx();
    op_clx();
    do {
        op_lda(RD8(VDC->data_lo));
        op_ora(RD8(STAGEWORK->mask_buffer[cpu.x + 1]));
        WR8(VDC->data_lo, cpu.a);
        op_lda(RD8(VDC->data_hi));
        op_ora(RD8(STAGEWORK->mask_buffer[cpu.x]));
        WR8(VDC->data_hi, cpu.a);
        op_inx();
        op_inx();
        op_cpx(0x20);
    } while (!flag_z());
    op_plx();
    RMW8(ZP->bios_a.lo, v, f_inc(v));
    op_lda(0x10);
    WR8(ZP->temp08, cpu.a);
    jsr_push((uint16_t)(at + 0x2B));
    SceneTextGlyphVramAddressAt((uint16_t)(at + 0x9A), cpu.x);
    jsr_pop();
    op_phx();
    op_clx();
    do {
        op_lda(RD8(VDC->data_lo));
        op_ora(RD8(STAGEWORK->mask_buffer[cpu.x + 1]));
        WR8(VDC->data_lo, cpu.a);
        op_lda(RD8(VDC->data_hi));
        op_ora(RD8(STAGEWORK->mask_buffer[cpu.x]));
        WR8(VDC->data_hi, cpu.a);
        op_inx();
        op_inx();
        op_cpx(0x1E);
    } while (!flag_z());
    op_plx();
    WR8(ZP->bios_a.lo, 0);
    jsr_push((uint16_t)(at + 0x4B));
    SceneTextShiftGlyphAt((uint16_t)(at + 0x8A));
    jsr_pop();
    jsr_push((uint16_t)(at + 0x4E));
    SceneTextGlyphVramAddressAt((uint16_t)(at + 0x9A), cpu.x);
    jsr_pop();
    op_phx();
    op_clx();
    do {
        op_lda(RD8(VDC->data_lo));
        op_ora(RD8(STAGEWORK->mask_buffer[cpu.x + 1]));
        WR8(VDC->data_lo, cpu.a);
        op_lda(RD8(VDC->data_hi));
        op_ora(RD8(STAGEWORK->mask_buffer[cpu.x]));
        WR8(VDC->data_hi, cpu.a);
        op_inx();
        op_inx();
        op_cpx(0x20);
    } while (!flag_z());
    op_plx();
    RMW8(ZP->bios_a.lo, v, f_inc(v));
    jsr_push((uint16_t)(at + 0x6E));
    SceneTextGlyphVramAddressAt((uint16_t)(at + 0x9A), cpu.x);
    jsr_pop();
    op_phx();
    op_clx();
    do {
        op_lda(RD8(VDC->data_lo));
        op_ora(RD8(STAGEWORK->mask_buffer[cpu.x + 1]));
        WR8(VDC->data_lo, cpu.a);
        op_lda(RD8(VDC->data_hi));
        op_ora(RD8(STAGEWORK->mask_buffer[cpu.x]));
        WR8(VDC->data_hi, cpu.a);
        op_inx();
        op_inx();
        op_cpx(0x1E);
    } while (!flag_z());
    op_plx();
}

/*
 * @routine 267C:0E6 Scene58TextDrawGlyph
 * @run 6B:88E6
 * WHAT IT DOES: Draws the 16 by 16 glyph in the buffer at $3B40 into VRAM: its four 8-line quarters ORed into the
 * tiles at the glyph's place, the right half shifted a bit to the right. (Scene $58's overlay.)
 * HOW: Four times the VRAM address set and $10 or $0F word pairs read, ORed and written back; the buffer shifted
 * once.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene58TextDrawGlyph(void)
{
    SceneTextDrawGlyphAt(0x88E6);
}

void abi_Scene58TextDrawGlyph(void)
{
    Scene58TextDrawGlyph();
}

/* SceneTextShiftGlyphAt: the body of the places below, at: the place's run address. */
void SceneTextShiftGlyphAt(uint16_t at)
{
    (void)at;   /* nothing in it moves with its place */
    op_phx();
    op_clx();
    do {
        RMW8(STAGEWORK->mask_buffer[cpu.x], v, f_lsr(v));
        RMW8(STAGEWORK->mask_buffer[cpu.x + 1], v, f_ror(v));
        op_inx();
        op_inx();
        op_cpx(0x20);
    } while (!flag_z());
    op_plx();
}

/*
 * @routine 267C:170 Scene58TextShiftGlyph
 * @run 6B:8970
 * WHAT IT DOES: Shifts the 32-byte glyph buffer at $3B40 right by one bit, a word at a time. (Scene $58's overlay.)
 * HOW: A loop of LSR and ROR, X kept.
 * WAITS: no.
 * Entry: X as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene58TextShiftGlyph(void)
{
    SceneTextShiftGlyphAt(0x8970);
}

void abi_Scene58TextShiftGlyph(void)
{
    Scene58TextShiftGlyph();
}

/* SceneTextGlyphVramAddressAt: the body of the places below, at: the place's run address. */
enum {
    SceneTextGlyphVramAddressVramRows = 0x4F2,   /* the VRAM address of each line place, a word each */
};

void SceneTextGlyphVramAddressAt(uint16_t at, uint8_t slot)
{
    op_lda(RD8(OBJ->work_g[slot]));
    WR8(ZP->temp00, 0);
    cpu.a = f_lsr(cpu.a);
    RMW8(ZP->temp00, v, f_ror(v));
    cpu.a = f_lsr(cpu.a);
    RMW8(ZP->temp00, v, f_ror(v));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(ZP->temp00));
    op_ora(RD8(ZP->temp08));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->start_arg[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    jsr_push((uint16_t)(at + 0x1B));
    SelectVramWriteAddress();
    cpu.a = STAGE_C_VDC_MAWR;
    jsr_pop();
    op_lda(rd8((uint16_t)((uint16_t)(at + SceneTextGlyphVramAddressVramRows) + cpu.y)));
    op_clc();
    op_adc(RD8(ZP->bios_a.lo));
    op_clc();
    op_adc(RD8(ZP->temp00));
    WR8(ZP->temp02, cpu.a);
    WR8(VDC->data_lo, cpu.a);
    op_lda(rd8((uint16_t)((uint16_t)(at + SceneTextGlyphVramAddressVramRows + 1) + cpu.y)));
    op_adc(RD8(ZP->temp01));
    WR8(ZP->temp03, cpu.a);
    WR8(VDC->data_hi, cpu.a);
    jsr_push((uint16_t)(at + 0x36));
    SelectVramReadAddress();
    cpu.a = STAGE_C_VDC_MARR;
    abi_nz(STAGE_C_VDC_MARR);
    jsr_pop();
    op_lda(RD8(ZP->temp02));
    WR8(VDC->data_lo, cpu.a);
    op_lda(RD8(ZP->temp03));
    WR8(VDC->data_hi, cpu.a);
    SelectVramData();
    cpu.a = STAGE_C_VDC_VRAM;
    abi_nz(STAGE_C_VDC_VRAM);
}

/*
 * @routine 267C:180 Scene58TextGlyphVramAddress
 * @run 6B:8980
 * WHAT IT DOES: Points the VDC at the glyph's VRAM place for both reading and writing: the line's place (by
 * OBJ_START_ARG) plus the glyph's column (OBJ_WORK_G times $40), plus zBiosAl and zTemp08. (Scene $58's overlay.)
 * HOW: Shifts and adds into zTemp02-03; SelectVramWriteAddress and SelectVramReadAddress with the address, then
 * SelectVramData as the tail.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene58TextGlyphVramAddress(uint8_t slot)
{
    SceneTextGlyphVramAddressAt(0x8980, slot);
}

void abi_Scene58TextGlyphVramAddress(void)
{
    Scene58TextGlyphVramAddress(cpu.x);
}

/* SceneTextRiseAt: the body of the places below, at: the place's run address. */
void SceneTextRiseAt(uint16_t at)
{
    jsr_push((uint16_t)(at + 0x3));
    axis_move_exit_shim(ApplyVelocityY(cpu.x));
    jsr_pop();
}

/*
 * @routine 267C:1C3 Scene58TextRise
 * @run 6B:89C3
 * WHAT IT DOES: The text object's state 3: moves by its Y speed. (Scene $58's overlay.)
 * HOW: ApplyVelocityY.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene58TextRise(void)
{
    SceneTextRiseAt(0x89C3);
}

void abi_Scene58TextRise(void)
{
    Scene58TextRise();
}

/* SceneTextShowAt: the body of the places below, at: the place's run address. */
enum {
    SceneTextShowFrames = 0x4B7,   /* the sprite frame of each line place */
    SceneTextShowSpriteSets = 0x4BD,   /* the sprite set of each line place */
};

void SceneTextShowAt(uint16_t at, uint8_t slot)
{
    RMW8(OBJ->work_h[slot], v, f_dec(v));
    if (flag_z()) {
        op_ldy(RD8(OBJ->start_arg[slot]));
        op_lda(rd8((uint16_t)((uint16_t)(at + SceneTextShowFrames) + cpu.y)));
        WR8(OBJ->frame[slot], cpu.a);
        op_lda(rd8((uint16_t)((uint16_t)(at + SceneTextShowSpriteSets) + cpu.y)));
        WR8(OBJ->sprite_set[slot], cpu.a);
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

/*
 * @routine 267C:1C7 Scene58TextShow
 * @run 6B:89C7
 * WHAT IT DOES: The text object's state 2: counts OBJ_WORK_H down; at zero takes its frame and sprite set by
 * OBJ_START_ARG and steps the state on. (Scene $58's overlay.)
 * HOW: DEC, two table reads.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene58TextShow(uint8_t slot)
{
    SceneTextShowAt(0x89C7, slot);
}

void abi_Scene58TextShow(void)
{
    Scene58TextShow(cpu.x);
}

/* SceneTextEndAt: the body of the places below, at: the place's run address. */
void SceneTextEndAt(uint16_t at)
{
    (void)at;   /* nothing in it moves with its place */
    ClearObject(cpu.x);
}

/*
 * @routine 267C:1DF Scene58TextEnd
 * @run 6B:89DF
 * WHAT IT DOES: The text object's state 4: clears the object. (Scene $58's overlay.)
 * HOW: A tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene58TextEnd(void)
{
    SceneTextEndAt(0x89DF);
}

void abi_Scene58TextEnd(void)
{
    Scene58TextEnd();
}

/*
 * @routine 267F:003 SceneEffectsVecStep
 * @run 6C:8003
 * WHAT IT DOES: The second entry of the jump table at the start of the overlay at sector $267F: goes to the effect
 * step at $829F.
 * HOW: A JMP to the routine loaded there.
 * WAITS: no.
 * Entry: X as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SceneEffectsVecStep(void)
{
    run_routine(0x829F);
}

void abi_SceneEffectsVecStep(void)
{
    SceneEffectsVecStep();
}

/*
 * @routine 2703:248 Scene5AQuakeScroll
 * @run 6B:8248
 * WHAT IT DOES: An object handler (kind 15) of the $5A overlay: in state 0 sets a count of $A1 in OBJ_WORK_D and
 * steps the state on; in state 1 counts it down, shaking the scroll's Y ($3E20) by a random step plus half of $A0
 * less the count, and steps on at zero; after that the shake rides on $50.
 * HOW: A test of the state; NextRandom's low four bits index the shake steps.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    Scene5AQuakeScrollSteps = 0x8294,   /* the shake step for each of 16 random values */
};

void Scene5AQuakeScroll(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (!flag_z()) {
        goto at_8256;
    }
    op_lda(0xA1);
    WR8(OBJ->work_d[slot], cpu.a);
at_8252:
    RMW8(OBJ->state[slot], v, f_inc(v));
    return;
at_8256:
    cpu.a = f_dec(cpu.a);
    if (flag_z()) {
        RMW8(OBJ->work_d[slot], v, f_dec(v));
        if (flag_z()) {
            goto at_8252;
        }
        op_lda(0xA0);
        op_sec();
        op_sbc(RD8(OBJ->work_d[slot]));
        cpu.a = f_lsr(cpu.a);
        WR8(ZP->temp00, cpu.a);
        jsr_push(0x826A);
        random_exit_shim(NextRandom());
        jsr_pop();
        op_and(0x0F);
        op_tay();
        op_lda(rd8((uint16_t)(Scene5AQuakeScrollSteps + cpu.y)));
        op_clc();
        op_adc(RD8(ZP->temp00));
        WR8(SCROLL->y_lo[0x00], cpu.a);
        return;
    }
    jsr_push(0x827A);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x0F);
    op_tay();
    op_lda(rd8((uint16_t)(Scene5AQuakeScrollSteps + cpu.y)));
    op_clc();
    op_adc(0x50);
    WR8(SCROLL->y_lo[0x00], cpu.a);
}

void abi_Scene5AQuakeScroll(void)
{
    Scene5AQuakeScroll(cpu.x);
}

/*
 * @routine 2703:287 Scene5AJitterScroll
 * @run 6B:8287
 * WHAT IT DOES: An object handler (kind 14) of the $5A overlay: sets the scroll's Y ($3E20) to a random shake step.
 * HOW: NextRandom's low four bits index the shake steps.
 * WAITS: no.
 * Entry: A as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    Scene5AJitterScrollSteps = 0x8294,   /* the shake step for each of 16 random values */
};

void Scene5AJitterScroll(void)
{
    jsr_push(0x828A);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x0F);
    op_tay();
    op_lda(rd8((uint16_t)(Scene5AJitterScrollSteps + cpu.y)));
    WR8(SCROLL->y_lo[0x00], cpu.a);
}

void abi_Scene5AJitterScroll(void)
{
    Scene5AJitterScroll();
}

/*
 * @routine 2703:6BD Scene5ATextObject
 * @run 6B:86BD
 * WHAT IT DOES: An object handler (kind 1): runs the step of the object's state from the table after it. (Scene
 * $5A's overlay.)
 * HOW: OBJ_STATE doubled indexes five words; a jump through zTemp00.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene5ATextObject(uint8_t slot)
{
    SceneTextObjectAt(0x86BD, slot);
}

void abi_Scene5ATextObject(void)
{
    Scene5ATextObject(cpu.x);
}

/*
 * @routine 2703:6D9 Scene5ATextStart
 * @run 6B:86D9
 * WHAT IT DOES: The text object's state 0: points the VDC at the line's VRAM place by OBJ_START_ARG, clears $400
 * words there, sets a rising Y speed of -$60, takes the line OBJ_WORK_D names, its sprite attribute from the line's
 * second byte, a wait of $12 in OBJ_WORK_H, and steps the state on. (Scene $5A's overlay.)
 * HOW: SelectVramWriteAddress, a table word into the address port, SelectVramData, a loop of $400 ST1/ST2 zeros;
 * table reads.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene5ATextStart(void)
{
    SceneTextStartAt(0x86D9);
}

void abi_Scene5ATextStart(void)
{
    Scene5ATextStart();
}

/*
 * @routine 2703:72B Scene5ATextSkipSpace
 * @run 6B:872B
 * WHAT IT DOES: Steps the pattern line's code on (OBJ_WORK_G), passing a space. (Scene $5A's overlay.)
 * HOW: INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene5ATextSkipSpace(uint8_t slot)
{
    SceneTextSkipSpaceAt(0x872B, slot);
}

void abi_Scene5ATextSkipSpace(void)
{
    Scene5ATextSkipSpace(cpu.x);
}

/*
 * @routine 2703:72F Scene5ATextPatterns
 * @run 6B:872F
 * WHAT IT DOES: A pattern line: for each code of the line copies $40 words of the overlay's pattern bank (from
 * $B000 by the code) to the glyph's VRAM place, skipping code $20; at the line's end the object is placed and its
 * state stepped on. (Scene $5A's overlay.)
 * HOW: The VRAM address set; the code halved into the pattern's address; a loop of $80 bytes to the data port.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene5ATextPatterns(uint8_t slot)
{
    SceneTextPatternsAt(0x872F, slot);
}

void abi_Scene5ATextPatterns(void)
{
    Scene5ATextPatterns(cpu.x);
}

/*
 * @routine 2703:76B Scene5ATextWrite
 * @run 6B:876B
 * WHAT IT DOES: The text object's state 1: counts OBJ_WORK_H down and takes the line again; a line marked by bit 7
 * of the sprite attribute is drawn from the overlay's patterns instead; otherwise the next code of the line
 * (OBJ_WORK_G) is looked up in the card's font into the buffer at $3B40, or a code $E0.. is the blank glyph, and
 * drawn. (Scene $5A's overlay.)
 * HOW: The code's two bytes into zBiosAh and zBiosAl, zBiosB $3B40 and zBiosDh 0, ex_getfnt between PHX and PLX,
 * then the glyph drawn.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene5ATextWrite(void)
{
    SceneTextWriteAt(0x876B);
}

void abi_Scene5ATextWrite(void)
{
    Scene5ATextWrite();
}

/*
 * @routine 2703:7A6 Scene5ATextNextGlyph
 * @run 6B:87A6
 * WHAT IT DOES: Draws the glyph in the buffer at $3B40, steps OBJ_WORK_G on, and ends the line when the next code
 * is zero. (Scene $5A's overlay.)
 * HOW: The draw, INC, a read through zTemp10.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene5ATextNextGlyph(void)
{
    SceneTextNextGlyphAt(0x87A6);
}

void abi_Scene5ATextNextGlyph(void)
{
    Scene5ATextNextGlyph();
}

/*
 * @routine 2703:7B6 Scene5ATextLineDone
 * @run 6B:87B6
 * WHAT IT DOES: At the end of a line: the line's first byte becomes the object's X, its Y high byte and OBJ_WORK_G
 * are cleared and the state steps on. (Scene $5A's overlay.)
 * HOW: Stores through zTemp10; a compare of X whose branch goes on either way.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene5ATextLineDone(uint8_t slot)
{
    SceneTextLineDoneAt(0x87B6, slot);
}

void abi_Scene5ATextLineDone(void)
{
    Scene5ATextLineDone(cpu.x);
}

/*
 * @routine 2703:7C8 Scene5ATextReturn
 * @run 6B:87C8
 * WHAT IT DOES: Returns: the end the text steps branch to. (Scene $5A's overlay.)
 * HOW: RTS.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene5ATextReturn(void)
{
    SceneTextReturnAt(0x87C8);
}

void abi_Scene5ATextReturn(void)
{
    Scene5ATextReturn();
}

/*
 * @routine 2703:7C9 Scene5ATextBlankGlyph
 * @run 6B:87C9
 * WHAT IT DOES: For a code $E0..: copies the blank glyph's 32 bytes into the buffer at $3B40 and draws it. (Scene
 * $5A's overlay.)
 * HOW: A copy loop, then the draw.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene5ATextBlankGlyph(void)
{
    SceneTextBlankGlyphAt(0x87C9);
}

void abi_Scene5ATextBlankGlyph(void)
{
    Scene5ATextBlankGlyph();
}

/*
 * @routine 2703:7F8 Scene5ATextDrawGlyph
 * @run 6B:87F8
 * WHAT IT DOES: Draws the 16 by 16 glyph in the buffer at $3B40 into VRAM: its four 8-line quarters ORed into the
 * tiles at the glyph's place, the right half shifted a bit to the right. (Scene $5A's overlay.)
 * HOW: Four times the VRAM address set and $10 or $0F word pairs read, ORed and written back; the buffer shifted
 * once.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene5ATextDrawGlyph(void)
{
    SceneTextDrawGlyphAt(0x87F8);
}

void abi_Scene5ATextDrawGlyph(void)
{
    Scene5ATextDrawGlyph();
}

/*
 * @routine 2704:082 Scene5ATextShiftGlyph
 * @run 6B:8882
 * WHAT IT DOES: Shifts the 32-byte glyph buffer at $3B40 right by one bit, a word at a time. (Scene $5A's overlay.)
 * HOW: A loop of LSR and ROR, X kept.
 * WAITS: no.
 * Entry: X as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene5ATextShiftGlyph(void)
{
    SceneTextShiftGlyphAt(0x8882);
}

void abi_Scene5ATextShiftGlyph(void)
{
    Scene5ATextShiftGlyph();
}

/*
 * @routine 2704:092 Scene5ATextGlyphVramAddress
 * @run 6B:8892
 * WHAT IT DOES: Points the VDC at the glyph's VRAM place for both reading and writing: the line's place (by
 * OBJ_START_ARG) plus the glyph's column (OBJ_WORK_G times $40), plus zBiosAl and zTemp08. (Scene $5A's overlay.)
 * HOW: Shifts and adds into zTemp02-03; SelectVramWriteAddress and SelectVramReadAddress with the address, then
 * SelectVramData as the tail.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene5ATextGlyphVramAddress(uint8_t slot)
{
    SceneTextGlyphVramAddressAt(0x8892, slot);
}

void abi_Scene5ATextGlyphVramAddress(void)
{
    Scene5ATextGlyphVramAddress(cpu.x);
}

/*
 * @routine 2704:0D5 Scene5ATextRise
 * @run 6B:88D5
 * WHAT IT DOES: The text object's state 3: moves by its Y speed. (Scene $5A's overlay.)
 * HOW: ApplyVelocityY.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene5ATextRise(void)
{
    SceneTextRiseAt(0x88D5);
}

void abi_Scene5ATextRise(void)
{
    Scene5ATextRise();
}

/*
 * @routine 2704:0D9 Scene5ATextShow
 * @run 6B:88D9
 * WHAT IT DOES: The text object's state 2: counts OBJ_WORK_H down; at zero takes its frame and sprite set by
 * OBJ_START_ARG and steps the state on. (Scene $5A's overlay.)
 * HOW: DEC, two table reads.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene5ATextShow(uint8_t slot)
{
    SceneTextShowAt(0x88D9, slot);
}

void abi_Scene5ATextShow(void)
{
    Scene5ATextShow(cpu.x);
}

/*
 * @routine 2704:0F1 Scene5ATextEnd
 * @run 6B:88F1
 * WHAT IT DOES: The text object's state 4: clears the object. (Scene $5A's overlay.)
 * HOW: A tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void Scene5ATextEnd(void)
{
    SceneTextEndAt(0x88F1);
}

void abi_Scene5ATextEnd(void)
{
    Scene5ATextEnd();
}
