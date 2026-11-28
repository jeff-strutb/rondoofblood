/*
 * Routines of single stage overlays that call nothing: an object's hit box by its frame, its place moved with its
 * scroll band, its velocity summed, and its stop when slot 16 is marked.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "leaf_b.h"

static void lda(uint8_t value) { cpu.a = value; abi_nz(value); }

static void cmp(uint8_t reg, uint8_t value)
{
    abi_flag(P_C, reg >= value);
    abi_nz((uint8_t)(reg - value));
}

static void adc(uint8_t value)
{
    Sum sum = add_bytes(cpu.a, value, (cpu.p & P_C) != 0);

    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
}

static void sbc(uint8_t value)
{
    Sum difference = sub_bytes(cpu.a, value, (cpu.p & P_C) != 0);

    lda(difference.value);
    abi_flag(P_C, difference.carry);
    abi_flag(P_V, difference.overflow);
}

/*
 * @routine 101B:7ED HitBoxByFrameRow101B
 * @run 70:B7ED
 * WHAT IT DOES: Gives the object the hit box of its animation frame (Stage 0A's overlay): frames from $25 on name a
 * row by the table at $B841, and the row's bytes become the box and OBJ_WORK_C and OBJ_WORK_B.
 * HOW: The frame less $25, the row, then six table reads and stores.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte, N and Z from it, Y the row, C and V from the subtract, X kept.
 */
void HitBoxByFrameRow101B(uint8_t slot)
{
    uint8_t row;

    lda(RD8(OBJ->frame[slot]));
    abi_flag(P_C, true);
    sbc(0x25);
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xB841 + cpu.y)));
    row = cpu.a;
    cpu.y = row;
    lda(rd8((uint16_t)(0xB81D + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(rd8((uint16_t)(0xB823 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    lda(rd8((uint16_t)(0xB829 + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(rd8((uint16_t)(0xB82F + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
    lda(rd8((uint16_t)(0xB835 + cpu.y)));
    WR8(OBJ->work_c[slot], cpu.a);
    lda(rd8((uint16_t)(0xB83B + cpu.y)));
    WR8(OBJ->work_b[slot], cpu.a);
}

void abi_HitBoxByFrameRow101B(void)
{
    HitBoxByFrameRow101B(cpu.x);
}

/*
 * @routine 161C:31A HitBoxByFrameRange161C
 * @run 70:BB1A
 * WHAT IT DOES: Gives the object a low, forward hit box while its frame is 7 to 13, else a tall one (Stage 0E's overlay).
 * HOW: Two compares of the frame, four stores.
 * WAITS: no.
 * Entry: X object. Exit: A the half height, N 0, Z 0; C set only for frames from 14; X, Y and V kept.
 */
void HitBoxByFrameRange161C(uint8_t slot)
{
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x07);
    if (cpu.a >= 0x07) {
        cmp(cpu.a, 0x0E);
        if (cpu.a < 0x0E) {
            lda(0xF1);
            WR8(OBJ->hitbox_dx[slot], cpu.a);
            lda(0x0D);
            WR8(OBJ->hitbox_dy[slot], cpu.a);
            lda(0x10);
            WR8(OBJ->hitbox_half_w[slot], cpu.a);
            lda(0x0B);
            WR8(OBJ->hitbox_half_h[slot], cpu.a);
            return;
        }
    }
    WR8(OBJ->hitbox_dx[slot], 0);
    WR8(OBJ->hitbox_dy[slot], 0);
    lda(0x0C);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(0x16);
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_HitBoxByFrameRange161C(void)
{
    HitBoxByFrameRange161C(cpu.x);
}

/*
 * @routine 117B:299 MoveXWithScrollBand117B
 * @run 70:B299
 * WHAT IT DOES: Takes from the object's X what the scroll is ahead of its band's (Stage 0B's overlay); the Y
 * differences are left in zTemp02-03 and the Y fraction's also at ScrollBandYFracStep by band.
 * HOW: The band (OBJ_SCROLL_BAND, bits 0 to 6); four subtracts; the X difference, sign extended into zTemp10, off the
 * 24-bit position.
 * WAITS: no.
 * Entry: X object. Exit: A the new X high byte, N, Z, C and V from its subtract, Y the band, X kept.
 */
void MoveXWithScrollBand117B(uint8_t slot)
{
    lda(RD8(OBJ->scroll_band[slot]));
    lda((uint8_t)(cpu.a & 0x7F));
    cpu.y = cpu.a;
    lda(RD8(SCROLL->x_frac[cpu.y]));
    abi_flag(P_C, true);
    sbc(rd8((uint16_t)(ScrollBandFirst + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(SCROLL->x_lo[cpu.y]));
    sbc(rd8((uint16_t)(ScrollBgxLo + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(SCROLL->y_frac[cpu.y]));
    abi_flag(P_C, true);
    sbc(rd8((uint16_t)(ScrollBandFourth + cpu.y)));
    WR8(ZP->temp02, cpu.a);
    wr8((uint16_t)(ScrollBandYFracStep + cpu.y), cpu.a);
    lda(RD8(SCROLL->y_lo[cpu.y]));
    sbc(rd8((uint16_t)(ScrollBgyLo + cpu.y)));
    WR8(ZP->temp03, cpu.a);
    WR8(ZP->temp10, 0);
    lda(RD8(ZP->temp01));
    if ((cpu.a & 0x80) != 0) {
        abi_nz(DEC8(ZP->temp10));
    }
    lda(RD8(OBJ->x_frac[slot]));
    abi_flag(P_C, true);
    sbc(RD8(ZP->temp00));
    WR8(OBJ->x_frac[slot], cpu.a);
    lda(RD8(OBJ->x_lo[slot]));
    sbc(RD8(ZP->temp01));
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(RD8(OBJ->x_hi[slot]));
    sbc(RD8(ZP->temp10));
    WR8(OBJ->x_hi[slot], cpu.a);
}

void abi_MoveXWithScrollBand117B(void)
{
    MoveXWithScrollBand117B(cpu.x);
}

/*
 * @routine 0EBC:6B7 SumXVelocity0EBC
 * @run 70:BEB7
 * WHAT IT DOES: Adds the object's X velocity into a 24-bit sum of its own (Stage 09's overlay), keeping the sum's
 * middle byte between -$12 and $18.
 * HOW: The velocity sign extended through zTemp00 and added; two compares clamp.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the middle byte or its bound, N and Z from the last load or compare, C from the
 * compare, V from the high byte's add; X and Y kept.
 */
void SumXVelocity0EBC(uint8_t slot)
{
    WR8(ZP->temp00, 0);
    lda(RD8(OBJ->vx_hi[slot]));
    if ((cpu.a & 0x80) != 0) {
        abi_nz(DEC8(ZP->temp00));
    }
    lda(RD8(OBJ->vx_lo[slot]));
    abi_flag(P_C, false);
    adc(RD8(OBJ->vx_sum_frac[slot]));
    WR8(OBJ->vx_sum_frac[slot], cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    adc(RD8(OBJ->vx_sum[slot]));
    WR8(OBJ->vx_sum[slot], cpu.a);
    lda(RD8(ZP->temp00));
    adc(RD8(OBJ->vx_sum_hi[slot]));
    WR8(OBJ->vx_sum_hi[slot], cpu.a);
    lda(RD8(OBJ->vx_sum[slot]));
    if ((cpu.a & 0x80) == 0) {
        cmp(cpu.a, 0x18);
        if (cpu.a >= 0x18) {
            lda(0x18);
            WR8(OBJ->vx_sum[slot], cpu.a);
        }
        return;
    }
    cmp(cpu.a, 0xEE);
    if (cpu.a < 0xEE) {
        lda(0xEE);
        WR8(OBJ->vx_sum[slot], cpu.a);
    }
}

void abi_SumXVelocity0EBC(void)
{
    SumXVelocity0EBC(cpu.x);
}

/*
 * @routine 0E0C:0F1 StopWhenSlot16Marked0E0C
 * @run 70:B8F1
 * WHAT IT DOES: While in states 1 to 6, stops the object once bit 7 of slot $10's OBJ_HIT_STATUS is set (Secret stage
 * 08's overlay): Y velocity, hit box and animation cleared, attribute bit 4, frame $1E, OBJ_WORK_E $60, state 7.
 * HOW: Two tests of the state, one of slot $10's byte, the stores.
 * WAITS: no.
 * Entry: X object. Exit: A the state, zero, or 7, N and Z from it; C from the state's compare; X, Y and V kept.
 */
void StopWhenSlot16Marked0E0C(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        return;
    }
    cmp(cpu.a, 0x07);
    if (cpu.a >= 0x07) {
        return;
    }
    lda(RD8(OBJ->hit_status[0x10]));
    lda((uint8_t)(cpu.a & 0x80));
    if (cpu.a == 0) {
        return;
    }
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(RD8(OBJ->sprite_attr[slot]));
    lda((uint8_t)(cpu.a | 0x10));
    WR8(OBJ->sprite_attr[slot], cpu.a);
    lda(0x1E);
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x60);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(0x07);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_StopWhenSlot16Marked0E0C(void)
{
    StopWhenSlot16Marked0E0C(cpu.x);
}
