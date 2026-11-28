/*
 * Small routines of stages $09 to $10 and the secret stages that call nothing: counts and state changes, screen
 * tests, speeds from tables.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "leaf_b3.h"

static void lda(uint8_t value) { cpu.a = value; abi_nz(value); }
static void ldy(uint8_t value) { cpu.y = value; abi_nz(value); }
static void carry(bool set) { abi_flag(P_C, set); }

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

static void asl(void)
{
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
}

static void rol(void)
{
    bool out = (cpu.a & 0x80) != 0;

    lda((uint8_t)((cpu.a << 1) | ((cpu.p & P_C) != 0 ? 1 : 0)));
    abi_flag(P_C, out);
}

/* The animation restarted. */
static void restart_animation(uint8_t slot)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
}

static void to_state(uint8_t slot, uint8_t state)
{
    lda(state);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

/* A count in OBJ_WORK_K: while it runs, C clear; at its end it is set again, state 4 and C set. */
static void countdown_to_state4(uint8_t slot, uint8_t again)
{
    lda(RD8(OBJ->work_k[slot]));
    if (cpu.a == 0) {
        WR8(OBJ->work_p[slot], 0);
        lda(again);
        WR8(OBJ->work_k[slot], cpu.a);
        to_state(slot, 4);
        carry(true);
        return;
    }
    abi_nz(DEC8(OBJ->work_k[slot]));
    carry(false);
}

/*
 * @routine 117B:178 FarOffScreenX117B
 * @run 70:B178
 * WHAT IT DOES: C set when the object is more than 32 dots off the screen along X (Stage 0B's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: C the answer; A the byte last read.
 */
void FarOffScreenX117B(uint8_t slot)
{
    lda(RD8(OBJ->x_hi[slot]));
    if (cpu.a == 0) {
        carry(false);
        return;
    }
    if ((cpu.a & 0x80) != 0) {
        cmp(cpu.a, 0xFF);
        if (cpu.a == 0xFF) {
            lda(RD8(OBJ->x_lo[slot]));
            cmp(cpu.a, 0xE0);
            if (cpu.a >= 0xE0) {
                carry(false);
                return;
            }
        }
    } else {
        cmp(cpu.a, 0x01);
        if (cpu.a == 0x01) {
            lda(RD8(OBJ->x_lo[slot]));
            cmp(cpu.a, 0x20);
            if (cpu.a < 0x20) {
                carry(false);
                return;
            }
        }
    }
    carry(true);
}

void abi_FarOffScreenX117B(void)
{
    FarOffScreenX117B(cpu.x);
}

/*
 * @routine 0EBE:1EF LevelXBefore150_0EBE
 * @run 71:C9EF
 * WHAT IT DOES: C set while the object's level X is under $150 (Stage 09's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: C the answer; A the byte last read; zTemp00 the low byte.
 */
void LevelXBefore150_0EBE(uint8_t slot)
{
    lda(rd8(ScrollXLo));
    carry(false);
    adc(RD8(OBJ->x_lo[slot]));
    WR8(ZP->temp00, cpu.a);
    lda(rd8(ScrollXHiScreen));
    adc(RD8(OBJ->x_hi[slot]));
    if ((cpu.a & 0x80) == 0) {
        cmp(cpu.a, 0x01);
        if (cpu.a > 0x01) {
            carry(false);
            return;
        }
        if (cpu.a == 0x01) {
            lda(RD8(ZP->temp00));
            cmp(cpu.a, 0x50);
            if (cpu.a >= 0x50) {
                carry(false);
                return;
            }
        }
    }
    carry(true);
}

void abi_LevelXBefore150_0EBE(void)
{
    LevelXBefore150_0EBE(cpu.x);
}

/*
 * @routine 072C:37E CalmSlots11To26_072C
 * @run 70:BB7E
 * WHAT IT DOES: Keeps only bits $87 of OBJ_PACE in slots $11 to $26, stops this object, sets OBJ_WORK_I 3 and counts its substate
 * (secret stage 03's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A 3, Y $27, C set, N and Z from the new substate; X kept.
 */
void CalmSlots11To26_072C(uint8_t slot)
{
    ldy(0x11);
    do {
        lda(RD8(OBJ->pace[cpu.y]));
        lda((uint8_t)(cpu.a & 0x87));
        WR8(OBJ->pace[cpu.y], cpu.a);
        ldy((uint8_t)(cpu.y + 1));
        cmp(cpu.y, 0x27);
    } while (cpu.y < 0x27);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    lda(0x03);
    WR8(OBJ->work_i[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_CalmSlots11To26_072C(void)
{
    CalmSlots11To26_072C(cpu.x);
}

/*
 * @routine 161D:6A6 CountThenState4_161D
 * @run 71:C6A6
 * WHAT IT DOES: Counts OBJ_WORK_H down (C clear); at zero goes to state 4 and loads the next count by OBJ_WORK_G (C set)
 * (Stage 0E's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: C whether the state changed; X kept.
 */
void CountThenState4_161D(uint8_t slot)
{
    lda(RD8(OBJ->work_h[slot]));
    if (cpu.a != 0) {
        abi_nz(DEC8(OBJ->work_h[slot]));
        carry(false);
        return;
    }
    to_state(slot, 4);
    lda(RD8(OBJ->work_g[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xC6C6 + cpu.y)));
    WR8(OBJ->work_h[slot], cpu.a);
    carry(true);
}

void abi_CountThenState4_161D(void)
{
    CountThenState4_161D(cpu.x);
}

/*
 * @routine 12DC:6C2 LaunchUpByFacing12DC
 * @run 70:BEC2
 * WHAT IT DOES: The X speed of its facing (both bytes the same table byte), an upward speed of -7.25, the substate counted on
 * (Stage 0C's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A $C0, N and Z from the new substate, Y twice the facing; X kept.
 */
void LaunchUpByFacing12DC(uint8_t slot)
{
    lda(RD8(OBJ->facing_left[slot]));
    asl();
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xBEE2 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xBEE2 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(0xF8);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0xC0);
    WR8(OBJ->vy_lo[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_LaunchUpByFacing12DC(void)
{
    LaunchUpByFacing12DC(cpu.x);
}

/*
 * @routine 0EBF:676 DriftWithScroll0EBF
 * @run 71:D676
 * WHAT IT DOES: While the screen scrolls along X, moves the object by the table's word for the direction (Stage 09's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the add, or A 0 when still; Y the direction's offset; X kept.
 */
void DriftWithScroll0EBF(uint8_t slot)
{
    lda(rd8(ScrollSpeedX));
    if (cpu.a == 0) {
        return;
    }
    rol();
    rol();
    lda((uint8_t)(cpu.a & 0x01));
    asl();
    cpu.y = cpu.a;
    lda(RD8(OBJ->x_lo[slot]));
    carry(false);
    adc(rd8((uint16_t)(0xD695 + cpu.y)));
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(RD8(OBJ->x_hi[slot]));
    adc(rd8((uint16_t)(0xD696 + cpu.y)));
    WR8(OBJ->x_hi[slot], cpu.a);
}

void abi_DriftWithScroll0EBF(void)
{
    DriftWithScroll0EBF(cpu.x);
}

/*
 * @routine 0EBB:12F StartByRoute0EBB
 * @run 70:B12F
 * WHAT IT DOES: While zField8C's bit 7 is clear: the state counted on and OBJ_WORK_D $40; start arguments 2 to 5 count it once more
 * (Stage 09's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare; X kept.
 */
void StartByRoute0EBB(uint8_t slot)
{
    lda(RD8(ZP->field8c));
    if ((cpu.a & 0x80) != 0) {
        return;
    }
    INC8(OBJ->state[slot]);
    lda(0x40);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    lda((uint8_t)(cpu.a & 0x0F));
    cmp(cpu.a, 0x02);
    if (cpu.a < 0x02) {
        return;
    }
    cmp(cpu.a, 0x06);
    if (cpu.a >= 0x06) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_StartByRoute0EBB(void)
{
    StartByRoute0EBB(cpu.x);
}

/*
 * @routine 088B:458 ClampAngle088B
 * @run 70:B458
 * WHAT IT DOES: Narrows an angle in A: under $40 to 0, $40 to $7F to $80, $A0 to $BF to $A0, $C0 to $DF to $E0 (secret stage 04's
 * overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: Exit: A the angle; C from the last compare; X and Y kept.
 */
void ClampAngle088B(void)
{
    cmp(cpu.a, 0xC0);
    if (cpu.a >= 0xC0) {
        cmp(cpu.a, 0xE0);
        if (cpu.a < 0xE0) {
            lda(0xE0);
        }
        return;
    }
    cmp(cpu.a, 0x40);
    if (cpu.a < 0x40) {
        cpu.a = 0;
        return;
    }
    cmp(cpu.a, 0x80);
    if (cpu.a < 0x80) {
        lda(0x80);
        return;
    }
    cmp(cpu.a, 0xA0);
    if (cpu.a >= 0xA0) {
        lda(0xA0);
    }
}

void abi_ClampAngle088B(void)
{
    ClampAngle088B();
}

/*
 * @routine 0EBC:673 NotePlaceAbovePrevious0EBC
 * @run 70:BE73
 * WHAT IT DOES: Keeps the place 18 dots above the slot before in OBJ_WORK_D to G (Stage 09's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A the X high byte, N and Z from it, C and V from the Y add; X kept.
 */
void NotePlaceAbovePrevious0EBC(uint8_t slot)
{
    lda(0xEE);
    carry(false);
    adc(RD8_IDX((uint16_t)(COL(OBJ->y_lo) - 1), slot));
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0xFF);
    adc(RD8_IDX((uint16_t)(COL(OBJ->y_hi) - 1), slot));
    WR8(OBJ->work_e[slot], cpu.a);
    lda(RD8_IDX((uint16_t)(COL(OBJ->x_lo) - 1), slot));
    WR8(OBJ->work_f[slot], cpu.a);
    lda(RD8_IDX((uint16_t)(COL(OBJ->x_hi) - 1), slot));
    WR8(OBJ->work_g[slot], cpu.a);
}

void abi_NotePlaceAbovePrevious0EBC(void)
{
    NotePlaceAbovePrevious0EBC(cpu.x);
}

/*
 * @routine 0EBC:0FE BobByTable0EBC
 * @run 70:B8FE
 * WHAT IT DOES: While OBJ_WORK_E's bit 7 is set, moves the object down by the table's step and counts it on, 16 steps (Stage 09's
 * overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the compare or the add; Y the step; X kept.
 */
void BobByTable0EBC(uint8_t slot)
{
    lda(RD8(OBJ->work_e[slot]));
    if ((cpu.a & 0x80) == 0) {
        return;
    }
    lda((uint8_t)(cpu.a & 0x7F));
    ldy(cpu.a);
    cmp(cpu.y, 0x10);
    if (cpu.y == 0x10) {
        WR8(OBJ->work_e[slot], 0);
        return;
    }
    lda(rd8((uint16_t)(0xB91C + cpu.y)));
    carry(false);
    adc(RD8(OBJ->y_lo[slot]));
    WR8(OBJ->y_lo[slot], cpu.a);
    abi_nz(INC8(OBJ->work_e[slot]));
}

void abi_BobByTable0EBC(void)
{
    BobByTable0EBC(cpu.x);
}

/*
 * @routine 18DB:394 HitBoxByAnimationStep18DB
 * @run 70:B394
 * WHAT IT DOES: The hit box on only in animation steps 6 to 11; else off with attribute $10 (Stage 10's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A 2 or $10; X and Y kept.
 */
void HitBoxByAnimationStep18DB(uint8_t slot)
{
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x06);
    if (cpu.a >= 0x06) {
        cmp(cpu.a, 0x0C);
        if (cpu.a < 0x0C) {
            lda(0x02);
            WR8(OBJ->hitbox_on[slot], cpu.a);
            WR8(OBJ->sprite_attr[slot], 0);
            return;
        }
    }
    WR8(OBJ->hitbox_on[slot], 0);
    lda(0x10);
    WR8(OBJ->sprite_attr[slot], cpu.a);
}

void abi_HitBoxByAnimationStep18DB(void)
{
    HitBoxByAnimationStep18DB(cpu.x);
}

/*
 * @routine 101B:107 WaitThenState3_101B
 * @run 70:B107
 * WHAT IT DOES: Counts OBJ_WORK_H down; at zero restarts the animation, sets the count 104 and goes to state 3 (Stage 0A's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load or compare left them; X kept.
 */
void WaitThenState3_101B(uint8_t slot)
{
    lda(RD8(OBJ->work_h[slot]));
    if (cpu.a != 0) {
        abi_nz(DEC8(OBJ->work_h[slot]));
        return;
    }
    restart_animation(slot);
    lda(0x68);
    WR8(OBJ->work_h[slot], cpu.a);
    to_state(slot, 3);
}

void abi_WaitThenState3_101B(void)
{
    WaitThenState3_101B(cpu.x);
}

/*
 * @routine 12DC:4DB CountToState4_12DC
 * @run 70:BCDB
 * WHAT IT DOES: Counts OBJ_WORK_K down (C clear); at zero sets it to 128, clears OBJ_WORK_P and goes to state 4 (C set).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: C whether the state changed; X kept.
 */
void CountToState4_12DC(uint8_t slot)
{
    countdown_to_state4(slot, 0x80);
}

void abi_CountToState4_12DC(void)
{
    CountToState4_12DC(cpu.x);
}

/*
 * @routine 101E:1EB CountToState4_101E
 * @run 71:C9EB
 * WHAT IT DOES: Counts OBJ_WORK_K down (C clear); at zero sets it to 192, clears OBJ_WORK_P and goes to state 4 (C set).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: C whether the state changed; X kept.
 */
void CountToState4_101E(uint8_t slot)
{
    countdown_to_state4(slot, 0xC0);
}

void abi_CountToState4_101E(void)
{
    CountToState4_101E(cpu.x);
}

/*
 * @routine 1020:23E MoveWithScrollY1020
 * @run 71:DA3E
 * WHAT IT DOES: Adds this frame's scroll speed along Y to the object's Y (Stage 0A's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A the Y high byte, flags from its add; X and Y kept.
 */
void MoveWithScrollY1020(uint8_t slot)
{
    WR8(ZP->temp00, 0);
    lda(rd8(ScrollSpeedY));
    if ((cpu.a & 0x80) != 0) {
        abi_nz(DEC8(ZP->temp00));
    }
    lda(RD8(OBJ->y_lo[slot]));
    carry(false);
    adc(rd8(ScrollSpeedY));
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(RD8(OBJ->y_hi[slot]));
    adc(RD8(ZP->temp00));
    WR8(OBJ->y_hi[slot], cpu.a);
}

void abi_MoveWithScrollY1020(void)
{
    MoveWithScrollY1020(cpu.x);
}

/*
 * @routine 117F:65D NextWaitToState3_117F
 * @run 71:D65D
 * WHAT IT DOES: State 3 with the animation restarted and the next of four counts in OBJ_WORK_E (Stage 0B's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A the count; Y the index; X kept.
 */
void NextWaitToState3_117F(uint8_t slot)
{
    restart_animation(slot);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    INC8(OBJ->work_g[slot]);
    lda(RD8(OBJ->work_g[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xD678 + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
}

void abi_NextWaitToState3_117F(void)
{
    NextWaitToState3_117F(cpu.x);
}

/*
 * @routine 101B:580 SlowByFacing101B
 * @run 70:B580
 * WHAT IT DOES: Takes the facing's step off the X speed (Stage 0A's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A the high byte, flags from its subtract; Y the facing's offset.
 */
void SlowByFacing101B(uint8_t slot)
{
    lda(RD8(OBJ->facing_left[slot]));
    lda((uint8_t)(cpu.a & 0x01));
    asl();
    cpu.y = cpu.a;
    lda(RD8(OBJ->vx_lo[slot]));
    carry(true);
    sbc(rd8((uint16_t)(0xB59B + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    sbc(rd8((uint16_t)(0xB59C + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
}

void abi_SlowByFacing101B(void)
{
    SlowByFacing101B(cpu.x);
}

/*
 * @routine 18DB:30D StartWhenSlot18Step7_18DB
 * @run 70:B30D
 * WHAT IT DOES: When slot $18 reaches animation step 7: frame 1, attribute $10, OBJ_WORK_E 16, substate on (Stage 10's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load or compare left them; X kept.
 */
void StartWhenSlot18Step7_18DB(uint8_t slot)
{
    lda(RD8(OBJ->anim_step[0x18]));
    cmp(cpu.a, 0x07);
    if (cpu.a != 0x07) {
        return;
    }
    lda(0x01);
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->sprite_attr[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_StartWhenSlot18Step7_18DB(void)
{
    StartWhenSlot18Step7_18DB(cpu.x);
}

/*
 * @routine 18DB:203 StartAtLevelX100_18DB
 * @run 70:B203
 * WHAT IT DOES: When the scroll is exactly at level X $100, stops it there (ScrollXMaxHi 1), sets OBJ_WORK_E 96, substate on
 * (Stage 10's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load or compare left them; X kept.
 */
void StartAtLevelX100_18DB(uint8_t slot)
{
    lda(rd8(ScrollXHiScreen));
    cmp(cpu.a, 0x01);
    if (cpu.a != 0x01) {
        return;
    }
    lda(rd8(ScrollXLo));
    if (cpu.a != 0) {
        return;
    }
    lda(0x01);
    wr8(ScrollXMaxHi, cpu.a);
    lda(0x60);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_StartAtLevelX100_18DB(void)
{
    StartAtLevelX100_18DB(cpu.x);
}

/*
 * @routine 13FB:2F5 HitToState5_13FB
 * @run 70:B2F5
 * WHAT IT DOES: Under state 5, once hit: OBJ_WORK_L cleared, state 5 (secret stage 0C's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load or compare left them; X kept.
 */
void HitToState5_13FB(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a >= 0x05) {
        return;
    }
    lda(RD8(OBJ->hit_status[slot]));
    lda((uint8_t)(cpu.a & 0x80));
    if (cpu.a == 0) {
        return;
    }
    WR8(OBJ->work_l[slot], 0);
    to_state(slot, 5);
}

void abi_HitToState5_13FB(void)
{
    HitToState5_13FB(cpu.x);
}

/*
 * @routine 12DB:687 RasterWhenLow12DB
 * @run 70:B687
 * WHAT IT DOES: Raster effect $0B from line $C0 while the object is low on the first screen, else none (Stage 0C's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load or compare left them; X kept.
 */
void RasterWhenLow12DB(uint8_t slot)
{
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a == 0) {
        lda(RD8(OBJ->y_lo[slot]));
        cmp(cpu.a, 0x7E);
        if (cpu.a >= 0x7E) {
            lda(0x0B);
            WR8(ZP->raster_effect, cpu.a);
            lda(0xC0);
            WR8(ZP->raster_arg, cpu.a);
            return;
        }
    }
    WR8(ZP->raster_effect, 0);
    WR8(ZP->raster_arg, 0);
}

void abi_RasterWhenLow12DB(void)
{
    RasterWhenLow12DB(cpu.x);
}

/*
 * @routine 0CAC:0B4 InWindow0CAC
 * @run 70:B8B4
 * WHAT IT DOES: C set when the object's place is within X $7F-$81 and Y $1F-$21 (secret stage 07's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: C the answer; A the byte last read.
 */
void InWindow0CAC(uint8_t slot)
{
    lda(RD8(OBJ->x_lo[slot]));
    cmp(cpu.a, 0x7F);
    if (cpu.a >= 0x7F) {
        cmp(cpu.a, 0x82);
        if (cpu.a < 0x82) {
            lda(RD8(OBJ->y_lo[slot]));
            cmp(cpu.a, 0x1F);
            if (cpu.a >= 0x1F) {
                cmp(cpu.a, 0x22);
                if (cpu.a < 0x22) {
                    carry(true);
                    return;
                }
            }
        }
    }
    carry(false);
}

void abi_InWindow0CAC(void)
{
    InWindow0CAC(cpu.x);
}

/*
 * @routine 12DC:62F ThrowUp12DC
 * @run 70:BE2F
 * WHAT IT DOES: No X speed, an upward speed of -7, OBJ_WORK_L 48, substate on (Stage 0C's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A $30; N and Z from the new substate; X kept.
 */
void ThrowUp12DC(uint8_t slot)
{
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    lda(0xF9);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x00);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(0x30);
    WR8(OBJ->work_l[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ThrowUp12DC(void)
{
    ThrowUp12DC(cpu.x);
}

/*
 * @routine 12DC:548 NextSubstateOfEight12DC
 * @run 70:BD48
 * WHAT IT DOES: The next substate of a cycle of eight (OBJ_WORK_N), the animation restarted (Stage 0C's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A the substate; Y the place in the cycle.
 */
void NextSubstateOfEight12DC(uint8_t slot)
{
    restart_animation(slot);
    WR8(OBJ->work_p[slot], 0);
    lda(RD8(OBJ->work_n[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xBD61 + cpu.y)));
    WR8(OBJ->substate[slot], cpu.a);
    abi_nz(INC8(OBJ->work_n[slot]));
}

void abi_NextSubstateOfEight12DC(void)
{
    NextSubstateOfEight12DC(cpu.x);
}

/*
 * @routine 117F:486 CountOnScreenToState4_117F
 * @run 71:D486
 * WHAT IT DOES: On the first screen, counts OBJ_WORK_E down (C clear); at zero state 4 (C set) (Stage 0B's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: C whether the state changed.
 */
void CountOnScreenToState4_117F(uint8_t slot)
{
    lda(RD8(OBJ->x_hi[slot]));
    if (cpu.a != 0) {
        carry(false);
        return;
    }
    lda(RD8(OBJ->work_e[slot]));
    if (cpu.a != 0) {
        abi_nz(DEC8(OBJ->work_e[slot]));
        carry(false);
        return;
    }
    to_state(slot, 4);
    carry(true);
}

void abi_CountOnScreenToState4_117F(void)
{
    CountOnScreenToState4_117F(cpu.x);
}

/*
 * @routine 0EBB:3E4 FallUpToSpeed7_0EBB
 * @run 70:B3E4
 * WHAT IT DOES: Gravity of $48 a frame until the Y speed reaches 7 (Stage 09's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the compare or the add; X and Y kept.
 */
void FallUpToSpeed7_0EBB(uint8_t slot)
{
    lda(RD8(OBJ->vy_hi[slot]));
    if ((cpu.a & 0x80) == 0) {
        cmp(cpu.a, 0x07);
        if (cpu.a >= 0x07) {
            return;
        }
    }
    lda(RD8(OBJ->vy_lo[slot]));
    carry(false);
    adc(0x48);
    WR8(OBJ->vy_lo[slot], cpu.a);
    if ((cpu.p & P_C) != 0) {
        abi_nz(INC8(OBJ->vy_hi[slot]));
    }
}

void abi_FallUpToSpeed7_0EBB(void)
{
    FallUpToSpeed7_0EBB(cpu.x);
}

/*
 * @routine 12DC:648 SpinCount12DC
 * @run 70:BE48
 * WHAT IT DOES: Adds 6 to OBJ_WORK_D and counts OBJ_WORK_L; at zero sets it to 28 and counts the substate (Stage 0C's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load or compare left them; X kept.
 */
void SpinCount12DC(uint8_t slot)
{
    lda(RD8(OBJ->work_d[slot]));
    carry(false);
    adc(0x06);
    WR8(OBJ->work_d[slot], cpu.a);
    {
        uint8_t left = DEC8(OBJ->work_l[slot]);

        abi_nz(left);
        if (left != 0) {
            return;
        }
    }
    lda(0x1C);
    WR8(OBJ->work_l[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SpinCount12DC(void)
{
    SpinCount12DC(cpu.x);
}

/*
 * @routine 12DB:559 NextSubstateOrState2_12DB
 * @run 70:B559
 * WHAT IT DOES: The next substate of a cycle of eight (OBJ_WORK_I); a 0 in the cycle means state 2 (Stage 0C's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A the substate or 2; Y the place in the cycle.
 */
void NextSubstateOrState2_12DB(uint8_t slot)
{
    lda(RD8(OBJ->work_i[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xB570 + cpu.y)));
    WR8(OBJ->substate[slot], cpu.a);
    if (cpu.a == 0) {
        lda(0x02);
        WR8(OBJ->state[slot], cpu.a);
    }
    abi_nz(INC8(OBJ->work_i[slot]));
}

void abi_NextSubstateOrState2_12DB(void)
{
    NextSubstateOrState2_12DB(cpu.x);
}

/*
 * @routine 10CC:539 RestartWhenAboveSlot12_10CC
 * @run 70:BD39
 * WHAT IT DOES: Once slot $12 is on the first screen below this object, restarts the animation and counts the substate (secret
 * stage 0A's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load or compare left them; X kept.
 */
void RestartWhenAboveSlot12_10CC(uint8_t slot)
{
    lda(RD8(OBJ->y_hi[0x12]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, RD8(OBJ->y_lo[0x12]));
    if ((cpu.p & P_C) != 0) {
        return;
    }
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_RestartWhenAboveSlot12_10CC(void)
{
    RestartWhenAboveSlot12_10CC(cpu.x);
}
