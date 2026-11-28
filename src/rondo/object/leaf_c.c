/*
 * Routines of the overlays of stages 06, 07 and 08 that call nothing: hit boxes and speeds by frame or step, counters
 * that step a state, checks of other slots' states, and the steps of the stage's scroll byte.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "ops6280.h"
#include "routines.h"
#include "stage_ops_c.h"
#include "leaf_c.h"

/*
 * @routine 0A8D:66A StepField82Table0A8D
 * @run 6B:866A
 * WHAT IT DOES: Steps zField82 on and copies the byte of the overlay's table at that step into zField83 (Stage 06's
 * $8000 overlay).
 * HOW: INC, then a table read by the new step.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    StepField82Table0A8DBytes = 0x9CAC,   /* the byte for each step of zField82 */
};

void StepField82Table0A8D(void)
{
    RMW8(ZP->field82, v, f_inc(v));
    op_lda(RD8(ZP->field82));
    op_tay();
    op_lda(rd8((uint16_t)(StepField82Table0A8DBytes + cpu.y)));
    WR8(ZP->field83, cpu.a);
}

void abi_StepField82Table0A8D(void)
{
    StepField82Table0A8D();
}

/*
 * @routine 0A99:6BD BecomeKindInA0A99
 * @run 70:A6BD
 * WHAT IT DOES: Turns the object into the kind in A, with OBJ_WORK_D 3 and its state back to 0 (Stage 06's
 * overlay).
 * HOW: Three stores.
 * WAITS: no.
 * Entry: X the object; A the new kind. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void BecomeKindInA0A99(uint8_t slot)
{
    WR8(OBJ->kind[slot], cpu.a);
    op_lda(0x03);
    WR8(OBJ->work_d[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
}

void abi_BecomeKindInA0A99(void)
{
    BecomeKindInA0A99(cpu.x);
}

/*
 * @routine 0A9A:794 DriftWithScrollSpeed0A9A
 * @run 70:AF94
 * WHAT IT DOES: While the scroll moves (ScrollSpeedX not 0), moves the object's X by a step chosen by the speed's
 * sign (Stage 06's overlay).
 * HOW: Bit 7 of the speed rolled down to bit 1 gives the word's index; a 16-bit add to the X position.
 * WAITS: no.
 * Entry: X the object; C as the caller left it; D as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
enum {
    DriftWithScrollSpeed0A9ASteps = 0xAFB3,   /* two X steps, a word each: for a speed of 0 to $7F, then for $80 up */
};

void DriftWithScrollSpeed0A9A(uint8_t slot)
{
    op_lda(RD8(SCROLLE->speed_x[0x00]));
    if (!flag_z()) {
        cpu.a = f_rol(cpu.a);
        cpu.a = f_rol(cpu.a);
        op_and(0x01);
        cpu.a = f_asl(cpu.a);
        op_tay();
        op_lda(RD8(OBJ->x_lo[slot]));
        op_clc();
        op_adc(rd8((uint16_t)(DriftWithScrollSpeed0A9ASteps + cpu.y)));
        WR8(OBJ->x_lo[slot], cpu.a);
        op_lda(RD8(OBJ->x_hi[slot]));
        op_adc(rd8((uint16_t)(DriftWithScrollSpeed0A9ASteps + 1 + cpu.y)));
        WR8(OBJ->x_hi[slot], cpu.a);
    }
}

void abi_DriftWithScrollSpeed0A9A(void)
{
    DriftWithScrollSpeed0A9A(cpu.x);
}

/*
 * @routine 0A9A:7F9 CountDownThenState0A9A
 * @run 70:AFF9
 * WHAT IT DOES: Counts OBJ_WORK_D down; once it is zero, steps the object's state on (Stage 06's overlay).
 * HOW: A load, then DEC or INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CountDownThenState0A9A(uint8_t slot)
{
    op_lda(RD8(OBJ->work_d[slot]));
    if (!flag_z()) {
        RMW8(OBJ->work_d[slot], v, f_dec(v));
        return;
    }
    RMW8(OBJ->state[slot], v, f_inc(v));
}

void abi_CountDownThenState0A9A(void)
{
    CountDownThenState0A9A(cpu.x);
}

/*
 * @routine 0A9B:312 CountKind2CFrom30_0A9B
 * @run 70:B312
 * WHAT IT DOES: Counts the objects of kind $2C in slots $30 to $3F into zTemp00 and leaves the count in A (Stage
 * 06's overlay).
 * HOW: A loop over the slots with a compare of each kind.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A and zTemp00 the count, N and Z from it; Y $40; C set from the last
 * compare.
 */
void CountKind2CFrom30_0A9B(void)
{
    WR8(ZP->temp00, 0);
    op_ldy(0x30);
    do {
        op_lda(RD8(OBJ->kind[cpu.y]));
        op_cmp(0x2C);
        if (flag_z()) {
            RMW8(ZP->temp00, v, f_inc(v));
        }
        op_iny();
        op_cpy(0x40);
    } while (!flag_c());
    op_lda(RD8(ZP->temp00));
}

void abi_CountKind2CFrom30_0A9B(void)
{
    CountKind2CFrom30_0A9B();
}

/*
 * @routine 0A9B:33F CheckPartnerOrHit0A9B
 * @run 70:B33F
 * WHAT IT DOES: With OBJ_WORK_I not 5, puts the object in state 3 unless the slot 5 less OBJ_WORK_I after it holds
 * kind $2B; with 5, once it has been hit (OBJ_HIT_STATUS bits 5 to 7) it becomes kind $2C in state 0 (Stage 06's
 * overlay).
 * HOW: A compare of OBJ_WORK_I; the slot found by an add of X; stores.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void CheckPartnerOrHit0A9B(uint8_t slot)
{
    op_lda(RD8(OBJ->work_i[slot]));
    op_cmp(0x05);
    if (flag_z()) {
        goto at_B360;
    }
    op_lda(0x05);
    op_sec();
    op_sbc(RD8(OBJ->work_i[slot]));
    WR8(ZP->temp00, cpu.a);
    op_txa();
    op_clc();
    op_adc(RD8(ZP->temp00));
    op_tay();
    op_lda(RD8(OBJ->kind[cpu.y]));
    op_cmp(0x2B);
    if (!flag_z()) {
        op_lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
    }
at_B35F:
    return;
at_B360:
    op_lda(RD8(OBJ->hit_status[slot]));
    op_and(0xE0);
    if (flag_z()) {
        goto at_B35F;
    }
    op_lda(0x2C);
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
}

void abi_CheckPartnerOrHit0A9B(void)
{
    CheckPartnerOrHit0A9B(cpu.x);
}

/*
 * @routine 0A9C:118 WaitForSlotThenFind0A9C
 * @run 70:B918
 * WHAT IT DOES: In substate 0 waits for the slot in OBJ_START_ARG to be empty, then counts OBJ_WORK_P down from 0;
 * when the count runs out looks in slots $10 to $4F for kind $C9 and keeps its slot in OBJ_START_ARG (Stage 06's
 * overlay).
 * HOW: Two arms by the substate; a search loop.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void WaitForSlotThenFind0A9C(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    if (flag_z()) {
        op_ldy(RD8(OBJ->start_arg[slot]));
        op_lda(RD8(OBJ->kind[cpu.y]));
        if (flag_z()) {
            WR8(OBJ->start_arg[slot], 0);
            WR8(OBJ->work_p[slot], 0);
            RMW8(OBJ->substate[slot], v, f_inc(v));
        }
        return;
    }
    RMW8(OBJ->work_p[slot], v, f_dec(v));
    if (flag_z()) {
        op_ldy(0x10);
        do {
            op_lda(RD8(OBJ->kind[cpu.y]));
            op_cmp(0xC9);
            if (flag_z()) {
                goto at_B943;
            }
            op_iny();
            op_cpy(0x50);
        } while (!flag_z());
    }
    return;
at_B943:
    op_tya();
    WR8(OBJ->start_arg[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_WaitForSlotThenFind0A9C(void)
{
    WaitForSlotThenFind0A9C(cpu.x);
}

/*
 * @routine 0A9D:737 FlipBelowRow50AtRight0A9D
 * @run 71:C737
 * WHAT IT DOES: Clears OBJ_SPRITE_FLIP unless the object is at Y $50 or more and its screen X is $830 or more; then
 * sets it from a table by the state (Stage 06's overlay).
 * HOW: A compare of Y, a 16-bit screen X from the scroll and two compares, a table read.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    FlipBelowRow50AtRight0A9DFlips = 0xC767,   /* the sprite flip byte by state */
};

void FlipBelowRow50AtRight0A9D(uint8_t slot)
{
    op_lda(RD8(OBJ->y_lo[slot]));
    op_cmp(0x50);
    if (flag_c()) {
        goto at_C742;
    }
at_C73E:
    WR8(OBJ->sprite_flip[slot], 0);
    return;
at_C742:
    op_lda(RD8(OBJ->x_lo[slot]));
    op_clc();
    op_adc(RD8(SCROLL->x_lo[0x00]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->x_hi[slot]));
    op_adc(RD8(SCROLLE->x_hi_screen[0x00]));
    op_cmp(0x08);
    if (!flag_c()) {
        goto at_C73E;
    }
    if (flag_z()) {
        op_lda(RD8(ZP->temp00));
        op_cmp(0x30);
        if (!flag_c()) {
            goto at_C73E;
        }
    }
    op_ldy(RD8(OBJ->state[slot]));
    op_lda(rd8((uint16_t)(FlipBelowRow50AtRight0A9DFlips + cpu.y)));
    WR8(OBJ->sprite_flip[slot], cpu.a);
}

void abi_FlipBelowRow50AtRight0A9D(void)
{
    FlipBelowRow50AtRight0A9D(cpu.x);
}

/*
 * @routine 0A9E:07E SetSpeedByFacingAndStep0A9E
 * @run 71:C87E
 * WHAT IT DOES: Sets the object's X and Y speeds from a table of four-byte records chosen by its facing and
 * OBJ_WORK_D, then steps the state on (Stage 06's overlay).
 * HOW: The facing's base plus four times OBJ_WORK_D indexes the records.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    SetSpeedByFacingAndStep0A9EBases = 0xC8AB,   /* the record index for each facing */
    SetSpeedByFacingAndStep0A9ESpeeds = 0xC8AD,   /* X speed low, high, Y speed low, high per record */
};

void SetSpeedByFacingAndStep0A9E(uint8_t slot)
{
    op_ldy(RD8(OBJ->facing_left[slot]));
    op_lda(rd8((uint16_t)(SetSpeedByFacingAndStep0A9EBases + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_clc();
    op_adc(RD8(ZP->temp00));
    op_tay();
    op_lda(rd8((uint16_t)(SetSpeedByFacingAndStep0A9ESpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(SetSpeedByFacingAndStep0A9ESpeeds + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(rd8((uint16_t)(SetSpeedByFacingAndStep0A9ESpeeds + 3 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(SetSpeedByFacingAndStep0A9ESpeeds + 2 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    RMW8(OBJ->state[slot], v, f_inc(v));
}

void abi_SetSpeedByFacingAndStep0A9E(void)
{
    SetSpeedByFacingAndStep0A9E(cpu.x);
}

/*
 * @routine 0A9E:785 RepeatOrStand0A9E
 * @run 71:CF85
 * WHAT IT DOES: Counts OBJ_WORK_K down: while it runs, restarts the animation in substate 1; when it ends, state 1
 * with frame $80, or $83 when OBJ_WORK_J is set (Stage 06's overlay).
 * HOW: DEC and two arms of stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void RepeatOrStand0A9E(uint8_t slot)
{
    RMW8(OBJ->work_k[slot], v, f_dec(v));
    if (!flag_z()) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        op_lda(0x01);
        WR8(OBJ->substate[slot], cpu.a);
        return;
    }
    op_lda(0x01);
    WR8(OBJ->state[slot], cpu.a);
    op_lda(0x80);
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(RD8(OBJ->work_j[slot]));
    if (!flag_z()) {
        op_lda(0x83);
        WR8(OBJ->frame[slot], cpu.a);
    }
}

void abi_RepeatOrStand0A9E(void)
{
    RepeatOrStand0A9E(cpu.x);
}

/*
 * @routine 0A9F:197 HitBoxByFrame0A9F_197
 * @run 71:D197
 * WHAT IT DOES: Gives the object the hit box of its frame from $92 on (Stage 06's overlay).
 * HOW: Four times the frame less $92 indexes four-byte records: X and Y offset, half width, half height.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    HitBoxByFrame0A9F_197Boxes = 0xD1B9,   /* X offset, Y offset, half width, half height per frame */
};

void HitBoxByFrame0A9F_197(uint8_t slot)
{
    op_lda(RD8(OBJ->frame[slot]));
    op_sec();
    op_sbc(0x92);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(HitBoxByFrame0A9F_197Boxes + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    op_lda(rd8((uint16_t)(HitBoxByFrame0A9F_197Boxes + 1 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    op_lda(rd8((uint16_t)(HitBoxByFrame0A9F_197Boxes + 2 + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    op_lda(rd8((uint16_t)(HitBoxByFrame0A9F_197Boxes + 3 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_HitBoxByFrame0A9F_197(void)
{
    HitBoxByFrame0A9F_197(cpu.x);
}

/*
 * @routine 0A9F:5A7 StaggerWhenHit0A9F
 * @run 71:D5A7
 * WHAT IT DOES: In the states a table marks, an object hit 5 times or more restarts its animation in state 7 (Stage
 * 06's overlay).
 * HOW: A table read by the state, a compare of OBJ_HIT_TAKEN, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    StaggerWhenHit0A9FStates = 0xD5C2,   /* nonzero for the states in which a hit counts */
};

void StaggerWhenHit0A9F(uint8_t slot)
{
    op_ldy(RD8(OBJ->state[slot]));
    op_lda(rd8((uint16_t)(StaggerWhenHit0A9FStates + cpu.y)));
    if (!flag_z()) {
        op_lda(RD8(OBJ->hit_taken[slot]));
        op_cmp(0x05);
        if (flag_c()) {
            WR8(OBJ->anim_step[slot], 0);
            WR8(OBJ->anim_timer[slot], 0);
            op_lda(0x07);
            WR8(OBJ->state[slot], cpu.a);
        }
    }
}

void abi_StaggerWhenHit0A9F(void)
{
    StaggerWhenHit0A9F(cpu.x);
}

/*
 * @routine 0A9F:5F4 HitBoxByFrame0A9F_5F4
 * @run 71:D5F4
 * WHAT IT DOES: Gives the object the hit box of its frame $AD to $CC; from frame $CD on it has none (Stage 06's
 * overlay).
 * HOW: A compare, then four times the frame less $AD indexes four-byte records.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    HitBoxByFrame0A9F_5F4Boxes = 0xDC3D,   /* X offset, Y offset, half width, half height per frame */
};

void HitBoxByFrame0A9F_5F4(uint8_t slot)
{
    op_lda(RD8(OBJ->frame[slot]));
    op_cmp(0xCD);
    if (flag_c()) {
        WR8(OBJ->hitbox_half_w[slot], 0);
        WR8(OBJ->hitbox_half_h[slot], 0);
        return;
    }
    op_sec();
    op_sbc(0xAD);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(HitBoxByFrame0A9F_5F4Boxes + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    op_lda(rd8((uint16_t)(HitBoxByFrame0A9F_5F4Boxes + 1 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    op_lda(rd8((uint16_t)(HitBoxByFrame0A9F_5F4Boxes + 2 + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    op_lda(rd8((uint16_t)(HitBoxByFrame0A9F_5F4Boxes + 3 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_HitBoxByFrame0A9F_5F4(void)
{
    HitBoxByFrame0A9F_5F4(cpu.x);
}

/*
 * @routine 0A9F:726 NextPatternStep0A9F
 * @run 71:D726
 * WHAT IT DOES: Restarts the animation and takes the next step of an eight-step pattern: its substate and
 * OBJ_WORK_C by OBJ_WORK_K (Stage 06's overlay).
 * HOW: OBJ_WORK_K's low three bits index the substates; the substate indexes OBJ_WORK_C's values.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    NextPatternStep0A9FSubstates = 0xD743,   /* the substate for each of the eight steps */
    NextPatternStep0A9FWorkC = 0xD74B,   /* OBJ_WORK_C for each substate */
};

void NextPatternStep0A9F(uint8_t slot)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    op_lda(RD8(OBJ->work_k[slot]));
    op_and(0x07);
    op_tay();
    op_lda(rd8((uint16_t)(NextPatternStep0A9FSubstates + cpu.y)));
    WR8(OBJ->substate[slot], cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(NextPatternStep0A9FWorkC + cpu.y)));
    WR8(OBJ->work_c[slot], cpu.a);
    RMW8(OBJ->work_k[slot], v, f_inc(v));
}

void abi_NextPatternStep0A9F(void)
{
    NextPatternStep0A9F(cpu.x);
}

/*
 * @routine 0AA0:4DC State4WhenHitTwice0AA0
 * @run 71:DCDC
 * WHAT IT DOES: Puts an object hit twice or more in state 4, substate 0, unless it is in state 4 already (Stage
 * 06's overlay).
 * HOW: Two compares, two stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void State4WhenHitTwice0AA0(uint8_t slot)
{
    op_lda(RD8(OBJ->hit_taken[slot]));
    op_cmp(0x02);
    if (flag_c()) {
        op_lda(RD8(OBJ->state[slot]));
        op_cmp(0x04);
        if (!flag_z()) {
            op_lda(0x04);
            WR8(OBJ->state[slot], cpu.a);
            WR8(OBJ->substate[slot], 0);
        }
    }
}

void abi_State4WhenHitTwice0AA0(void)
{
    State4WhenHitTwice0AA0(cpu.x);
}

/*
 * @routine 0AA0:4F3 StateOnAtColumn0AA0
 * @run 71:DCF3
 * WHAT IT DOES: Steps the state on when the object stands in the first screen column its OBJ_START_ARG names (X
 * high byte 0, X low byte's top four bits equal to it) (Stage 06's overlay).
 * HOW: Two compares and INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StateOnAtColumn0AA0(uint8_t slot)
{
    op_lda(RD8(OBJ->x_hi[slot]));
    if (flag_z()) {
        op_lda(RD8(OBJ->x_lo[slot]));
        op_and(0xF0);
        op_cmp(RD8(OBJ->start_arg[slot]));
        if (flag_z()) {
            RMW8(OBJ->state[slot], v, f_inc(v));
        }
    }
}

void abi_StateOnAtColumn0AA0(void)
{
    StateOnAtColumn0AA0(cpu.x);
}

/*
 * @routine 0BED:66A StepField82Table0BED
 * @run 6B:866A
 * WHAT IT DOES: Steps zField82 on and copies the byte of the overlay's table at that step into zField83 (Stage 07's
 * $8000 overlay).
 * HOW: INC, then a table read by the new step.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    StepField82Table0BEDBytes = 0x9CC4,   /* the byte for each step of zField82 */
};

void StepField82Table0BED(void)
{
    RMW8(ZP->field82, v, f_inc(v));
    op_lda(RD8(ZP->field82));
    op_tay();
    op_lda(rd8((uint16_t)(StepField82Table0BEDBytes + cpu.y)));
    WR8(ZP->field83, cpu.a);
}

void abi_StepField82Table0BED(void)
{
    StepField82Table0BED();
}

/*
 * @routine 0BFB:006 ReturnCarrySet0BFB
 * @run 70:B006
 * WHAT IT DOES: Returns with the carry set (Stage 07's overlay).
 * HOW: SEC.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: C set; A, X, Y and the other flags kept.
 */
void ReturnCarrySet0BFB(void)
{
    op_sec();
}

void abi_ReturnCarrySet0BFB(void)
{
    ReturnCarrySet0BFB();
}

/*
 * @routine 0BFB:51F OpenWhenGuardsDown0BFB
 * @run 70:B51F
 * WHAT IT DOES: In states 4 and 5, once no object of kind $2D in slots $49 to $4B is in a state other than 2, takes
 * the hit box off, steps the next three slots' states on and goes to state 6, substate 0 (Stage 07's overlay).
 * HOW: Two compares of the state, a search loop, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void OpenWhenGuardsDown0BFB(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    op_cmp(0x04);
    if (flag_c()) {
        op_cmp(0x06);
        if (!flag_c()) {
            op_ldy(0x49);
            do {
                op_lda(RD8(OBJ->kind[cpu.y]));
                op_cmp(0x2D);
                if (flag_z()) {
                    op_lda(RD8(OBJ->state[cpu.y]));
                    op_cmp(0x02);
                    if (!flag_z()) {
                        return;
                    }
                }
                op_iny();
                op_cpy(0x4C);
            } while (!flag_c());
            WR8(OBJ->hitbox_on[slot], 0);
            RMW8(OBJ->state[slot + 1], v, f_inc(v));
            RMW8(OBJ->state[slot + 2], v, f_inc(v));
            RMW8(OBJ->state[slot + 3], v, f_inc(v));
            op_lda(0x06);
            WR8(OBJ->state[slot], cpu.a);
            WR8(OBJ->substate[slot], 0);
        }
    }
}

void abi_OpenWhenGuardsDown0BFB(void)
{
    OpenWhenGuardsDown0BFB(cpu.x);
}

/*
 * @routine 0BFB:555 State5WhenHitInState4_0BFB
 * @run 70:B555
 * WHAT IT DOES: An object hit twice or more while in state 4 goes to state 5, substate 0 (Stage 07's overlay).
 * HOW: Two compares, two stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void State5WhenHitInState4_0BFB(uint8_t slot)
{
    op_lda(RD8(OBJ->hit_taken[slot]));
    op_cmp(0x02);
    if (flag_c()) {
        op_lda(RD8(OBJ->state[slot]));
        op_cmp(0x04);
        if (flag_z()) {
            op_lda(0x05);
            WR8(OBJ->state[slot], cpu.a);
            WR8(OBJ->substate[slot], 0);
        }
    }
}

void abi_State5WhenHitInState4_0BFB(void)
{
    State5WhenHitInState4_0BFB(cpu.x);
}

/*
 * @routine 0BFB:570 RiseAtScreen3_0BFB
 * @run 70:B570
 * WHAT IT DOES: When the scroll reaches screen 3, holds the scroll's X limits at screen 3, starts the object rising
 * at -$60 a frame and steps its state on (Stage 07's overlay).
 * HOW: A compare of ScrollXHiScreen, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void RiseAtScreen3_0BFB(uint8_t slot)
{
    op_lda(RD8(SCROLLE->x_hi_screen[0x00]));
    op_cmp(0x03);
    if (flag_z()) {
        op_lda(0x03);
        WR8(SCROLLLIM->x_min_hi, cpu.a);
        WR8(SCROLLLIM->x_max.hi, cpu.a);
        op_lda(0xFF);
        WR8(OBJ->vy_hi[slot], cpu.a);
        op_lda(0xA0);
        WR8(OBJ->vy_lo[slot], cpu.a);
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_RiseAtScreen3_0BFB(void)
{
    RiseAtScreen3_0BFB(cpu.x);
}

/*
 * @routine 0BFB:732 CountThenFall0BFB
 * @run 70:B732
 * WHAT IT DOES: Counts OBJ_WORK_H down; when it runs out, starts the object falling at 1 pixel a frame and steps
 * the substate on (Stage 07's overlay).
 * HOW: DEC, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CountThenFall0BFB(uint8_t slot)
{
    RMW8(OBJ->work_h[slot], v, f_dec(v));
    if (flag_z()) {
        op_lda(0x01);
        WR8(OBJ->vy_hi[slot], cpu.a);
        WR8(OBJ->vy_lo[slot], 0);
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_CountThenFall0BFB(void)
{
    CountThenFall0BFB(cpu.x);
}

/*
 * @routine 0BFD:03B HitBoxXByFrame0BFD
 * @run 71:C03B
 * WHAT IT DOES: Turns the hit box on with an X offset by frame for frames up to $61; from $62 on, the box is off
 * (Stage 07's overlay).
 * HOW: A compare of the frame, a table read by the frame less $5C.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    HitBoxXByFrame0BFDOffsets = 0xC059,   /* the hit box's X offset for frames $5C to $61 */
};

void HitBoxXByFrame0BFD(uint8_t slot)
{
    op_lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    op_lda(RD8(OBJ->frame[slot]));
    op_cmp(0x62);
    if (!flag_c()) {
        op_sec();
        op_sbc(0x5C);
        op_tay();
        op_lda(rd8((uint16_t)(HitBoxXByFrame0BFDOffsets + cpu.y)));
        WR8(OBJ->hitbox_dx[slot], cpu.a);
        return;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->hitbox_dx[slot], 0);
}

void abi_HitBoxXByFrame0BFD(void)
{
    HitBoxXByFrame0BFD(cpu.x);
}

/*
 * @routine 0BFD:2A5 RepeatOrStand0BFD
 * @run 71:C2A5
 * WHAT IT DOES: Counts OBJ_WORK_K down: while it runs, restarts the animation in substate 1; when it ends, state 1
 * with frame $5C, or $5F when OBJ_WORK_J is set (Stage 07's overlay).
 * HOW: DEC and two arms of stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void RepeatOrStand0BFD(uint8_t slot)
{
    RMW8(OBJ->work_k[slot], v, f_dec(v));
    if (!flag_z()) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        op_lda(0x01);
        WR8(OBJ->substate[slot], cpu.a);
        return;
    }
    op_lda(0x01);
    WR8(OBJ->state[slot], cpu.a);
    op_lda(0x5C);
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(RD8(OBJ->work_j[slot]));
    if (!flag_z()) {
        op_lda(0x5F);
        WR8(OBJ->frame[slot], cpu.a);
    }
}

void abi_RepeatOrStand0BFD(void)
{
    RepeatOrStand0BFD(cpu.x);
}

/*
 * @routine 0BFD:4B7 HitBoxByFrame0BFD
 * @run 71:C4B7
 * WHAT IT DOES: Gives the object the hit box of its frame from $6E on (Stage 07's overlay).
 * HOW: Four times the frame less $6E indexes four-byte records.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    HitBoxByFrame0BFDBoxes = 0xC4D9,   /* X offset, Y offset, half width, half height per frame */
};

void HitBoxByFrame0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->frame[slot]));
    op_sec();
    op_sbc(0x6E);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(HitBoxByFrame0BFDBoxes + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    op_lda(rd8((uint16_t)(HitBoxByFrame0BFDBoxes + 1 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    op_lda(rd8((uint16_t)(HitBoxByFrame0BFDBoxes + 2 + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    op_lda(rd8((uint16_t)(HitBoxByFrame0BFDBoxes + 3 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_HitBoxByFrame0BFD(void)
{
    HitBoxByFrame0BFD(cpu.x);
}

/*
 * @routine 0BFD:57D MarkPartsByBlink0BFD
 * @run 71:C57D
 * WHAT IT DOES: In state 1 writes 2 (while OBJ_INVULN_TIMER runs) or 0 into OBJ_WORK_J of the object's slot and the
 * nine after it, and halves OBJ_WORK_O once it has been hit 5 times or more (Stage 07's overlay).
 * HOW: A loop of ten stores from Y = X; a compare and LSR.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void MarkPartsByBlink0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    op_cmp(0x01);
    if (flag_z()) {
        WR8(ZP->temp00, 0);
        op_lda(RD8(OBJ->invuln_timer[slot]));
        if (!flag_z()) {
            op_lda(0x02);
            WR8(ZP->temp00, cpu.a);
        }
        op_txa();
        op_tay();
        op_lda(0x0A);
        WR8(ZP->temp01, cpu.a);
        do {
            op_lda(RD8(ZP->temp00));
            WR8(OBJ->work_j[cpu.y], cpu.a);
            op_iny();
            RMW8(ZP->temp01, v, f_dec(v));
        } while (!flag_z());
        op_lda(RD8(OBJ->hit_taken[slot]));
        op_cmp(0x05);
        if (flag_c()) {
            RMW8(OBJ->work_o[slot], v, f_lsr(v));
        }
    }
}

void abi_MarkPartsByBlink0BFD(void)
{
    MarkPartsByBlink0BFD(cpu.x);
}

/*
 * @routine 0BFD:77B EaseWorkFToTarget0BFD
 * @run 71:C77B
 * WHAT IT DOES: Moves the 16-bit value OBJ_WORK_F:OBJ_WORK_G toward the target of its step (OBJ_WORK_J) by that
 * step's speed, up or down, and stops on the target (Stage 07's overlay).
 * HOW: Twice OBJ_WORK_J indexes the targets and the speeds; a compare picks the direction; 16-bit adds or
 * subtracts.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    EaseWorkFToTarget0BFDTargets = 0xC7CF,   /* the target high byte per step, two bytes apart */
    EaseWorkFToTarget0BFDSpeeds = 0xC7D4,   /* the speed per step, a word each */
};

void EaseWorkFToTarget0BFD(uint8_t slot)
{
    op_lda(RD8(OBJ->work_j[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(RD8(OBJ->work_f[slot]));
    op_cmp(rd8((uint16_t)(EaseWorkFToTarget0BFDTargets + cpu.y)));
    if (flag_c()) {
        if (!flag_z()) {
            goto at_C7AC;
        }
        op_lda(RD8(OBJ->work_g[slot]));
        if (!flag_z()) {
            goto at_C7AC;
        }
        return;
    }
    op_lda(RD8(OBJ->work_g[slot]));
    op_clc();
    op_adc(rd8((uint16_t)(EaseWorkFToTarget0BFDSpeeds + cpu.y)));
    WR8(OBJ->work_g[slot], cpu.a);
    op_lda(RD8(OBJ->work_f[slot]));
    op_adc(rd8((uint16_t)(EaseWorkFToTarget0BFDSpeeds + 1 + cpu.y)));
    WR8(OBJ->work_f[slot], cpu.a);
    op_cmp(rd8((uint16_t)(EaseWorkFToTarget0BFDTargets + cpu.y)));
    if (flag_z()) {
        WR8(OBJ->work_g[slot], 0);
    }
at_C7AB:
    return;
at_C7AC:
    op_lda(RD8(OBJ->work_g[slot]));
    op_sec();
    op_sbc(rd8((uint16_t)(EaseWorkFToTarget0BFDSpeeds + cpu.y)));
    WR8(OBJ->work_g[slot], cpu.a);
    op_lda(RD8(OBJ->work_f[slot]));
    op_sbc(rd8((uint16_t)(EaseWorkFToTarget0BFDSpeeds + 1 + cpu.y)));
    WR8(OBJ->work_f[slot], cpu.a);
    op_cmp(rd8((uint16_t)(EaseWorkFToTarget0BFDTargets + cpu.y)));
    if (flag_c()) {
        goto at_C7AB;
    }
    op_lda(rd8((uint16_t)(EaseWorkFToTarget0BFDTargets + cpu.y)));
    WR8(OBJ->work_f[slot], cpu.a);
    WR8(OBJ->work_g[slot], 0);
}

void abi_EaseWorkFToTarget0BFD(void)
{
    EaseWorkFToTarget0BFD(cpu.x);
}

/*
 * @routine 0BFE:02D SetNineAfterToState2_0BFE
 * @run 71:C82D
 * WHAT IT DOES: Puts the nine slots after the object's in state 2, substate 0 (Stage 07's overlay).
 * HOW: A loop from Y = X + 1.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SetNineAfterToState2_0BFE(void)
{
    op_txa();
    op_tay();
    op_iny();
    op_lda(0x09);
    WR8(ZP->temp01, cpu.a);
    do {
        op_lda(0x02);
        WR8(OBJ->state[cpu.y], cpu.a);
        op_cla();
        WR8(OBJ->substate[cpu.y], cpu.a);
        op_iny();
        RMW8(ZP->temp01, v, f_dec(v));
    } while (!flag_z());
}

void abi_SetNineAfterToState2_0BFE(void)
{
    SetNineAfterToState2_0BFE();
}

/*
 * @routine 0BFE:096 ClampA50ToB0_0BFE
 * @run 71:C896
 * WHAT IT DOES: Clamps A to $50 to $B0 (Stage 07's overlay).
 * HOW: Two compares.
 * WAITS: no.
 * Entry: A the value. Exit: A the value clamped; N, Z and C from the last compare or load; X and Y kept.
 */
void ClampA50ToB0_0BFE(void)
{
    op_cmp(0x50);
    if (flag_c()) {
        op_cmp(0xB0);
        if (flag_c()) {
            goto at_C8A2;
        }
        return;
    }
    op_lda(0x50);
    return;
at_C8A2:
    op_lda(0xB0);
}

void abi_ClampA50ToB0_0BFE(void)
{
    ClampA50ToB0_0BFE();
}

/*
 * @routine 0BFE:124 SetNineAfterToState3_0BFE
 * @run 71:C924
 * WHAT IT DOES: Puts the nine slots after the object's in state 3 with the object's OBJ_START_ARG in their
 * OBJ_WORK_L (Stage 07's overlay).
 * HOW: Y = X + 1, then the loop that follows.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SetNineAfterToState3_0BFE(void)
{
    op_txa();
    op_tay();
    op_iny();
    /* falls into $C927 */
    SetNineFromYToState3_0BFE(cpu.x);
}

void abi_SetNineAfterToState3_0BFE(void)
{
    SetNineAfterToState3_0BFE();
}

/*
 * @routine 0BFE:127 SetNineFromYToState3_0BFE
 * @run 71:C927
 * WHAT IT DOES: Puts nine slots from Y on in state 3, substate 0, with the object's OBJ_START_ARG in their
 * OBJ_WORK_L (Stage 07's overlay).
 * HOW: A loop of nine.
 * WAITS: no.
 * Entry: X the object; Y the first slot. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SetNineFromYToState3_0BFE(uint8_t slot)
{
    op_lda(0x09);
    WR8(ZP->temp01, cpu.a);
    do {
        op_lda(0x03);
        WR8(OBJ->state[cpu.y], cpu.a);
        op_lda(RD8(OBJ->start_arg[slot]));
        WR8(OBJ->work_l[cpu.y], cpu.a);
        op_cla();
        WR8(OBJ->substate[cpu.y], cpu.a);
        op_iny();
        RMW8(ZP->temp01, v, f_dec(v));
    } while (!flag_z());
}

void abi_SetNineFromYToState3_0BFE(void)
{
    SetNineFromYToState3_0BFE(cpu.x);
}

/*
 * @routine 0BFE:1E6 WorkIByStep0BFE
 * @run 71:C9E6
 * WHAT IT DOES: Sets OBJ_WORK_I from a table by OBJ_WORK_K and steps the substate on (Stage 07's overlay).
 * HOW: A table read.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    WorkIByStep0BFEValues = 0xC9F3,   /* OBJ_WORK_I for each value of OBJ_WORK_K */
};

void WorkIByStep0BFE(uint8_t slot)
{
    op_ldy(RD8(OBJ->work_k[slot]));
    op_lda(rd8((uint16_t)(WorkIByStep0BFEValues + cpu.y)));
    WR8(OBJ->work_i[slot], cpu.a);
    RMW8(OBJ->substate[slot], v, f_inc(v));
}

void abi_WorkIByStep0BFE(void)
{
    WorkIByStep0BFE(cpu.x);
}

/*
 * @routine 0BFE:229 CountThenRewind0BFE
 * @run 71:CA29
 * WHAT IT DOES: Counts OBJ_WORK_I down; when it runs out, reloads it with $30, sets OBJ_WORK_J and steps the
 * substate on (Stage 07's overlay).
 * HOW: DEC, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void CountThenRewind0BFE(uint8_t slot)
{
    RMW8(OBJ->work_i[slot], v, f_dec(v));
    if (flag_z()) {
        op_lda(0x30);
        WR8(OBJ->work_i[slot], cpu.a);
        op_lda(0x01);
        WR8(OBJ->work_j[slot], cpu.a);
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_CountThenRewind0BFE(void)
{
    CountThenRewind0BFE(cpu.x);
}

/*
 * @routine 0BFE:27C LaunchByStep0BFE
 * @run 71:CA7C
 * WHAT IT DOES: Sets the object's X and Y speeds from a four-byte record by OBJ_WORK_K, takes its hit box off,
 * clears OBJ_PACE bits 3 to 6 and steps the substate on (Stage 07's overlay).
 * HOW: Four times OBJ_WORK_K indexes the records.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    LaunchByStep0BFESpeeds = 0xCAA9,   /* X speed low, high, Y speed low, high per step */
};

void LaunchByStep0BFE(uint8_t slot)
{
    op_lda(RD8(OBJ->work_k[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(LaunchByStep0BFESpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(LaunchByStep0BFESpeeds + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(rd8((uint16_t)(LaunchByStep0BFESpeeds + 3 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(LaunchByStep0BFESpeeds + 2 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    WR8(OBJ->hitbox_on[slot], 0);
    op_lda(RD8(OBJ->pace[slot]));
    op_and(0x87);
    WR8(OBJ->pace[slot], cpu.a);
    RMW8(OBJ->substate[slot], v, f_inc(v));
}

void abi_LaunchByStep0BFE(void)
{
    LaunchByStep0BFE(cpu.x);
}

/*
 * @routine 0BFE:58A PickPhaseOfFour0BFE
 * @run 71:CD8A
 * WHAT IT DOES: Takes the substate and OBJ_WORK_H of the next of four phases by OBJ_WORK_F, and steps OBJ_WORK_F on
 * (Stage 07's overlay).
 * HOW: OBJ_WORK_F's low two bits index two tables.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    PickPhaseOfFour0BFESubstates = 0xCDA0,   /* the substate of each phase */
    PickPhaseOfFour0BFEWorkH = 0xCDA4,   /* OBJ_WORK_H of each phase */
};

void PickPhaseOfFour0BFE(uint8_t slot)
{
    op_lda(RD8(OBJ->work_f[slot]));
    op_and(0x03);
    op_tay();
    op_lda(rd8((uint16_t)(PickPhaseOfFour0BFESubstates + cpu.y)));
    WR8(OBJ->substate[slot], cpu.a);
    op_lda(rd8((uint16_t)(PickPhaseOfFour0BFEWorkH + cpu.y)));
    WR8(OBJ->work_h[slot], cpu.a);
    RMW8(OBJ->work_f[slot], v, f_inc(v));
}

void abi_PickPhaseOfFour0BFE(void)
{
    PickPhaseOfFour0BFE(cpu.x);
}

/*
 * @routine 0BFE:5A8 SetFrame89Wait28_0BFE
 * @run 71:CDA8
 * WHAT IT DOES: Sets frame $89 and a wait of $28 in OBJ_WORK_D, and steps the substate on (Stage 07's overlay).
 * HOW: Stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SetFrame89Wait28_0BFE(uint8_t slot)
{
    op_lda(0x28);
    WR8(OBJ->work_d[slot], cpu.a);
    op_lda(0x89);
    WR8(OBJ->frame[slot], cpu.a);
    RMW8(OBJ->substate[slot], v, f_inc(v));
}

void abi_SetFrame89Wait28_0BFE(void)
{
    SetFrame89Wait28_0BFE(cpu.x);
}

/*
 * @routine 0BFE:785 HitBoxOffWait40_0BFE
 * @run 71:CF85
 * WHAT IT DOES: Takes the hit box off, sets OBJ_WORK_K to $40 and steps the substate on (Stage 07's overlay).
 * HOW: Stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void HitBoxOffWait40_0BFE(uint8_t slot)
{
    WR8(OBJ->hitbox_on[slot], 0);
    op_lda(0x40);
    WR8(OBJ->work_k[slot], cpu.a);
    RMW8(OBJ->substate[slot], v, f_inc(v));
}

void abi_HitBoxOffWait40_0BFE(void)
{
    HitBoxOffWait40_0BFE(cpu.x);
}

/*
 * @routine 0BFF:4C1 StateOnAndMarkField8C0BFF
 * @run 71:D4C1
 * WHAT IT DOES: Steps the state on and sets bit 7 of zField8C (Stage 07's overlay).
 * HOW: INC, ORA.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StateOnAndMarkField8C0BFF(uint8_t slot)
{
    RMW8(OBJ->state[slot], v, f_inc(v));
    op_lda(RD8(ZP->field8c));
    op_ora(0x80);
    WR8(ZP->field8c, cpu.a);
}

void abi_StateOnAndMarkField8C0BFF(void)
{
    StateOnAndMarkField8C0BFF(cpu.x);
}

/*
 * @routine 0D2D:29C SetUpByStartArg0D2D
 * @run 81:C29C
 * WHAT IT DOES: Starts the object: sprite set 3, hit box on, health $80, state on; the record five on when
 * zCharacter is not 0, or when it is 0 and zField93's byte (by a table) is the record's key; then its hit box,
 * OBJ_WORK_D and OBJ_WORK_E from the record of OBJ_START_ARG (Stage 08's overlay).
 * HOW: The record index in OBJ_START_ARG; a compare through two tables; table reads by twice the index and by the
 * index.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    SetUpByStartArg0D2DWaits = 0xC1BC,   /* OBJ_WORK_D and OBJ_WORK_E per record */
    SetUpByStartArg0D2DKeys = 0xC1D0,   /* the byte each record is kept for */
    SetUpByStartArg0D2DSizes = 0xC1E4,   /* half width, half height per record */
    SetUpByStartArg0D2DOffsets = 0xC20C,   /* X offset, Y offset per record */
    SetUpByStartArg0D2DByField93 = 0xC374,   /* the byte compared for each value of zField93 */
};

void SetUpByStartArg0D2D(void)
{
    op_lda(0x03);
    WR8(OBJ->sprite_set[cpu.x], cpu.a);
    op_lda(0x01);
    WR8(OBJ->hitbox_on[cpu.x], cpu.a);
    op_lda(0x80);
    WR8(OBJ->health[cpu.x], cpu.a);
    RMW8(OBJ->state[cpu.x], v, f_inc(v));
    op_lda(RD8(ZP->character));
    if (flag_z()) {
        op_ldy(RD8(OBJ->start_arg[cpu.x]));
        op_phx();
        op_ldx(RD8(ZP->field93));
        op_lda(rd8((uint16_t)(SetUpByStartArg0D2DByField93 + cpu.x)));
        op_plx();
        op_cmp(rd8((uint16_t)(SetUpByStartArg0D2DKeys + cpu.y)));
        if (!flag_z()) {
            goto at_C2CA;
        }
    }
    op_lda(RD8(OBJ->start_arg[cpu.x]));
    op_clc();
    op_adc(0x05);
    WR8(OBJ->start_arg[cpu.x], cpu.a);
at_C2CA:
    op_lda(RD8(OBJ->start_arg[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(SetUpByStartArg0D2DSizes + cpu.y)));
    WR8(OBJ->hitbox_half_w[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(SetUpByStartArg0D2DSizes + 1 + cpu.y)));
    WR8(OBJ->hitbox_half_h[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(SetUpByStartArg0D2DOffsets + cpu.y)));
    WR8(OBJ->hitbox_dx[cpu.x], cpu.a);
    op_lda(rd8((uint16_t)(SetUpByStartArg0D2DOffsets + 1 + cpu.y)));
    WR8(OBJ->hitbox_dy[cpu.x], cpu.a);
    op_ldy(RD8(OBJ->start_arg[cpu.x]));
    op_lda(rd8((uint16_t)(SetUpByStartArg0D2DWaits + cpu.y)));
    WR8(OBJ->work_d[cpu.x], cpu.a);
    WR8(OBJ->work_e[cpu.x], cpu.a);
}

void abi_SetUpByStartArg0D2D(void)
{
    SetUpByStartArg0D2D();
}

/*
 * @routine 0D4D:66A StepField82Table0D4D
 * @run 6B:866A
 * WHAT IT DOES: Steps zField82 on and copies the byte of the overlay's table at that step into zField83 (Stage 08's
 * $8000 overlay).
 * HOW: INC, then a table read by the new step.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    StepField82Table0D4DBytes = 0x9CE7,   /* the byte for each step of zField82 */
};

void StepField82Table0D4D(void)
{
    RMW8(ZP->field82, v, f_inc(v));
    op_lda(RD8(ZP->field82));
    op_tay();
    op_lda(rd8((uint16_t)(StepField82Table0D4DBytes + cpu.y)));
    WR8(ZP->field83, cpu.a);
}

void abi_StepField82Table0D4D(void)
{
    StepField82Table0D4D();
}

/*
 * @routine 0D5B:312 State4WhenHit0D5B
 * @run 70:B312
 * WHAT IT DOES: Once the object is hit (OBJ_HIT_STATUS bit 7) in a state below 4, puts it in state 4, substate 0
 * (Stage 08's overlay).
 * HOW: Two tests, two stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void State4WhenHit0D5B(uint8_t slot)
{
    op_lda(RD8(OBJ->hit_status[slot]));
    op_and(0x80);
    if (!flag_z()) {
        op_lda(RD8(OBJ->state[slot]));
        op_cmp(0x04);
        if (!flag_c()) {
            op_lda(0x04);
            WR8(OBJ->state[slot], cpu.a);
            WR8(OBJ->substate[slot], 0);
        }
    }
}

void abi_State4WhenHit0D5B(void)
{
    State4WhenHit0D5B(cpu.x);
}

/*
 * @routine 0D5B:527 EaseWorkFToTarget0D5B
 * @run 70:B527
 * WHAT IT DOES: Moves the 16-bit value OBJ_WORK_F:OBJ_WORK_G toward the target of its step (OBJ_WORK_J) by that
 * step's speed, up or down, and stops on the target (Stage 08's overlay).
 * HOW: Twice OBJ_WORK_J indexes the targets and the speeds; a compare picks the direction; 16-bit adds or
 * subtracts.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    EaseWorkFToTarget0D5BTargets = 0xB57B,   /* the target high byte per step, two bytes apart */
    EaseWorkFToTarget0D5BSpeeds = 0xB580,   /* the speed per step, a word each */
};

void EaseWorkFToTarget0D5B(uint8_t slot)
{
    op_lda(RD8(OBJ->work_j[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(RD8(OBJ->work_f[slot]));
    op_cmp(rd8((uint16_t)(EaseWorkFToTarget0D5BTargets + cpu.y)));
    if (flag_c()) {
        if (!flag_z()) {
            goto at_B558;
        }
        op_lda(RD8(OBJ->work_g[slot]));
        if (!flag_z()) {
            goto at_B558;
        }
        return;
    }
    op_lda(RD8(OBJ->work_g[slot]));
    op_clc();
    op_adc(rd8((uint16_t)(EaseWorkFToTarget0D5BSpeeds + cpu.y)));
    WR8(OBJ->work_g[slot], cpu.a);
    op_lda(RD8(OBJ->work_f[slot]));
    op_adc(rd8((uint16_t)(EaseWorkFToTarget0D5BSpeeds + 1 + cpu.y)));
    WR8(OBJ->work_f[slot], cpu.a);
    op_cmp(rd8((uint16_t)(EaseWorkFToTarget0D5BTargets + cpu.y)));
    if (flag_z()) {
        WR8(OBJ->work_g[slot], 0);
    }
at_B557:
    return;
at_B558:
    op_lda(RD8(OBJ->work_g[slot]));
    op_sec();
    op_sbc(rd8((uint16_t)(EaseWorkFToTarget0D5BSpeeds + cpu.y)));
    WR8(OBJ->work_g[slot], cpu.a);
    op_lda(RD8(OBJ->work_f[slot]));
    op_sbc(rd8((uint16_t)(EaseWorkFToTarget0D5BSpeeds + 1 + cpu.y)));
    WR8(OBJ->work_f[slot], cpu.a);
    op_cmp(rd8((uint16_t)(EaseWorkFToTarget0D5BTargets + cpu.y)));
    if (flag_c()) {
        goto at_B557;
    }
    op_lda(rd8((uint16_t)(EaseWorkFToTarget0D5BTargets + cpu.y)));
    WR8(OBJ->work_f[slot], cpu.a);
    WR8(OBJ->work_g[slot], 0);
}

void abi_EaseWorkFToTarget0D5B(void)
{
    EaseWorkFToTarget0D5B(cpu.x);
}

/*
 * @routine 0D5B:79B WorkIByStep0D5B
 * @run 70:B79B
 * WHAT IT DOES: Sets OBJ_WORK_I from a table by OBJ_WORK_K and steps the substate on (Stage 08's overlay).
 * HOW: A table read.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    WorkIByStep0D5BValues = 0xB7A8,   /* OBJ_WORK_I for each value of OBJ_WORK_K */
};

void WorkIByStep0D5B(uint8_t slot)
{
    op_ldy(RD8(OBJ->work_k[slot]));
    op_lda(rd8((uint16_t)(WorkIByStep0D5BValues + cpu.y)));
    WR8(OBJ->work_i[slot], cpu.a);
    RMW8(OBJ->substate[slot], v, f_inc(v));
}

void abi_WorkIByStep0D5B(void)
{
    WorkIByStep0D5B(cpu.x);
}

/*
 * @routine 0D5C:031 LaunchByStep0D5C
 * @run 70:B831
 * WHAT IT DOES: Sets the object's X and Y speeds from a four-byte record by OBJ_WORK_K, takes its hit box off,
 * clears OBJ_PACE bits 3 to 6 and steps the substate on (Stage 08's overlay).
 * HOW: Four times OBJ_WORK_K indexes the records.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    LaunchByStep0D5CSpeeds = 0xB85E,   /* X speed low, high, Y speed low, high per step */
};

void LaunchByStep0D5C(uint8_t slot)
{
    op_lda(RD8(OBJ->work_k[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(LaunchByStep0D5CSpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(LaunchByStep0D5CSpeeds + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(rd8((uint16_t)(LaunchByStep0D5CSpeeds + 3 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(LaunchByStep0D5CSpeeds + 2 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    WR8(OBJ->hitbox_on[slot], 0);
    op_lda(RD8(OBJ->pace[slot]));
    op_and(0x87);
    WR8(OBJ->pace[slot], cpu.a);
    RMW8(OBJ->substate[slot], v, f_inc(v));
}

void abi_LaunchByStep0D5C(void)
{
    LaunchByStep0D5C(cpu.x);
}

/*
 * @routine 0D5C:0BC BecomeWorkLKind0D5C
 * @run 70:B8BC
 * WHAT IT DOES: Turns the object into the kind in its OBJ_WORK_L, state 0 (Stage 08's overlay).
 * HOW: Two stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void BecomeWorkLKind0D5C(uint8_t slot)
{
    op_lda(RD8(OBJ->work_l[slot]));
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
}

void abi_BecomeWorkLKind0D5C(void)
{
    BecomeWorkLKind0D5C(cpu.x);
}

/*
 * @routine 0D5C:0E8 BreakUnlessLeaderAlive0D5C
 * @run 70:B8E8
 * WHAT IT DOES: Unless the slot in OBJ_WORK_D holds a kind $22 in a state other than 4, turns the object into kind
 * $9C, state 0, and returns with C set; else with C clear (Stage 08's overlay).
 * HOW: Two compares, stores.
 * WAITS: no.
 * Entry: X the object. Exit: C set when the object was turned; A, N and Z from the last compare or store; X kept.
 */
void BreakUnlessLeaderAlive0D5C(uint8_t slot)
{
    op_ldy(RD8(OBJ->work_d[slot]));
    op_lda(RD8(OBJ->kind[cpu.y]));
    op_cmp(0x22);
    if (flag_z()) {
        op_lda(RD8(OBJ->state[cpu.y]));
        op_cmp(0x04);
        if (!flag_z()) {
            goto at_B903;
        }
    }
    op_lda(0x9C);
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
    op_sec();
    return;
at_B903:
    op_clc();
}

void abi_BreakUnlessLeaderAlive0D5C(void)
{
    BreakUnlessLeaderAlive0D5C(cpu.x);
}

/*
 * @routine 0D5C:20E BreakUpWhenHit0D5C
 * @run 70:BA0E
 * WHAT IT DOES: Once the object is hit (OBJ_HIT_STATUS bit 7) outside state 4: the next slot becomes kind $9C, the
 * six after it get OBJ_WORK_N from a table, their hit boxes off and OBJ_PACE bits 3 to 6 cleared; the object loses
 * its hit box and frame and goes to state 4 (Stage 08's overlay).
 * HOW: Two tests, then a loop of six over X with Y the table's index, X kept on the stack.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    BreakUpWhenHit0D5CWorkN = 0xBA4C,   /* OBJ_WORK_N for each of the six pieces */
};

void BreakUpWhenHit0D5C(void)
{
    op_lda(RD8(OBJ->hit_status[cpu.x]));
    op_and(0x80);
    if (!flag_z()) {
        op_lda(RD8(OBJ->state[cpu.x]));
        op_cmp(0x04);
        if (!flag_z()) {
            op_phx();
            op_inx();
            op_lda(0x9C);
            WR8(OBJ->kind[cpu.x], cpu.a);
            WR8(OBJ->state[cpu.x], 0);
            op_inx();
            op_cly();
            do {
                op_lda(rd8((uint16_t)(BreakUpWhenHit0D5CWorkN + cpu.y)));
                WR8(OBJ->work_n[cpu.x], cpu.a);
                WR8(OBJ->hitbox_on[cpu.x], 0);
                op_lda(RD8(OBJ->pace[cpu.x]));
                op_and(0x87);
                WR8(OBJ->pace[cpu.x], cpu.a);
                op_inx();
                op_iny();
                op_cpy(0x06);
            } while (!flag_c());
            op_plx();
            WR8(OBJ->hitbox_on[cpu.x], 0);
            WR8(OBJ->frame[cpu.x], 0);
            op_lda(0x04);
            WR8(OBJ->state[cpu.x], cpu.a);
        }
    }
}

void abi_BreakUpWhenHit0D5C(void)
{
    BreakUpWhenHit0D5C();
}

/*
 * @routine 0D5C:28B TurnBackPastColumn0D5C
 * @run 70:BA8B
 * WHAT IT DOES: On the first screen, once the object's X reaches the column of the record in slot Y's
 * OBJ_START_ARG, sets its X speed to -$20 and steps the substate on (Stage 08's overlay).
 * HOW: A table read by the record's low seven bits, a compare, stores.
 * WAITS: no.
 * Entry: X the object; Y the slot whose record counts. Exit: A, X, Y and the flags as its last instruction or call
 * left them.
 */
enum {
    TurnBackPastColumn0D5CColumns = 0xBAAC,   /* the X where the object turns, by record */
};

void TurnBackPastColumn0D5C(uint8_t slot)
{
    op_lda(RD8(OBJ->x_hi[slot]));
    if (flag_z()) {
        op_lda(RD8(OBJ->start_arg[cpu.y]));
        op_and(0x7F);
        op_tay();
        op_lda(RD8(OBJ->x_lo[slot]));
        op_cmp(rd8((uint16_t)(TurnBackPastColumn0D5CColumns + cpu.y)));
        if (flag_c()) {
            op_lda(0xFF);
            WR8(OBJ->vx_hi[slot], cpu.a);
            op_lda(0xE0);
            WR8(OBJ->vx_lo[slot], cpu.a);
            RMW8(OBJ->substate[slot], v, f_inc(v));
        }
    }
}

void abi_TurnBackPastColumn0D5C(void)
{
    TurnBackPastColumn0D5C(cpu.x);
}

/*
 * @routine 0D5C:577 SetFrame60_0D5C
 * @run 70:BD77
 * WHAT IT DOES: Sets frame $60 and clears OBJ_WORK_C (Stage 08's overlay).
 * HOW: Two stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SetFrame60_0D5C(uint8_t slot)
{
    op_lda(0x60);
    WR8(OBJ->frame[slot], cpu.a);
    WR8(OBJ->work_c[slot], 0);
}

void abi_SetFrame60_0D5C(void)
{
    SetFrame60_0D5C(cpu.x);
}

/*
 * @routine 0D5D:145 WakeSlotInStartArg0D5D
 * @run 71:C145
 * WHAT IT DOES: Steps the slot named by OBJ_START_ARG's low seven bits from state 1 to state 2 (Stage 08's
 * overlay).
 * HOW: A test, a compare, INC A and a store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void WakeSlotInStartArg0D5D(uint8_t slot)
{
    op_lda(RD8(OBJ->start_arg[slot]));
    op_and(0x7F);
    if (!flag_z()) {
        op_tay();
        op_lda(RD8(OBJ->state[cpu.y]));
        op_cmp(0x01);
        if (flag_z()) {
            cpu.a = f_inc(cpu.a);
            WR8(OBJ->state[cpu.y], cpu.a);
        }
    }
}

void abi_WakeSlotInStartArg0D5D(void)
{
    WakeSlotInStartArg0D5D(cpu.x);
}

/*
 * @routine 0D5D:27F OthersInState2_0D5D
 * @run 71:C27F
 * WHAT IT DOES: For an object in slot $33, $34 or $35, tests whether the other two of those slots are in state 2;
 * returns Z set when one is (Stage 08's overlay).
 * HOW: Two compares of X choose the slots; compares of their states.
 * WAITS: no.
 * Entry: X the slot. Exit: Z set when another of slots $33 to $35 is in state 2; A that slot's state; C from the
 * compare; X and Y kept.
 */
void OthersInState2_0D5D(void)
{
    op_cpx(0x33);
    if (flag_z()) {
        goto at_C294;
    }
    op_cpx(0x34);
    if (flag_z()) {
        goto at_C299;
    }
    op_lda(RD8(OBJ->state[0x33]));
at_C28A:
    op_cmp(0x02);
    if (!flag_z()) {
        op_lda(RD8(OBJ->state[0x34]));
        op_cmp(0x02);
    }
at_C293:
    return;
at_C294:
    op_lda(RD8(OBJ->state[0x35]));
    goto at_C28A;
at_C299:
    op_lda(RD8(OBJ->state[0x33]));
    op_cmp(0x02);
    if (flag_z()) {
        goto at_C293;
    }
    op_lda(RD8(OBJ->state[0x35]));
    op_cmp(0x02);
}

void abi_OthersInState2_0D5D(void)
{
    OthersInState2_0D5D();
}

/*
 * @routine 0D5D:2A6 PassRecordFromState2_0D5D
 * @run 71:C2A6
 * WHAT IT DOES: For slot $35 with OBJ_START_ARG set: finds the first of slots $33 to $35 in state 2 and gives its
 * OBJ_START_ARG to the occupied slots from $10 on, up to $1A (Stage 08's overlay).
 * HOW: Three compares, then a copy loop that stops at an empty slot.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void PassRecordFromState2_0D5D(uint8_t slot)
{
    op_cpx(0x35);
    if (flag_z()) {
        op_lda(RD8(OBJ->start_arg[slot]));
        if (!flag_z()) {
            op_ldy(0x33);
            op_lda(RD8(OBJ->state[cpu.y]));
            op_cmp(0x02);
            if (flag_z()) {
                goto at_C2C9;
            }
            op_iny();
            op_lda(RD8(OBJ->state[cpu.y]));
            op_cmp(0x02);
            if (flag_z()) {
                goto at_C2C9;
            }
            op_iny();
            op_lda(RD8(OBJ->state[cpu.y]));
            op_cmp(0x02);
            if (flag_z()) {
                goto at_C2C9;
            }
        }
    }
    return;
at_C2C9:
    op_lda(RD8(OBJ->start_arg[cpu.y]));
    WR8(ZP->temp00, cpu.a);
    op_ldy(0x10);
at_C2D0:
    op_lda(RD8(OBJ->kind[cpu.y]));
    if (!flag_z()) {
        op_lda(RD8(OBJ->start_arg[slot]));
        op_lda(RD8(ZP->temp00));
        WR8(OBJ->start_arg[cpu.y], cpu.a);
        op_iny();
        op_cpy(0x1B);
        if (!flag_z()) {
            goto at_C2D0;
        }
    }
}

void abi_PassRecordFromState2_0D5D(void)
{
    PassRecordFromState2_0D5D(cpu.x);
}

/*
 * @routine 0D5D:44A HitBoxWidthByFrame0D5D
 * @run 71:C44A
 * WHAT IT DOES: Sets the hit box's half width to 8, or $13 for frame $2F (Stage 08's overlay).
 * HOW: A compare, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void HitBoxWidthByFrame0D5D(uint8_t slot)
{
    op_lda(0x08);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    op_lda(RD8(OBJ->frame[slot]));
    op_cmp(0x2F);
    if (flag_z()) {
        op_lda(0x13);
        WR8(OBJ->hitbox_half_w[slot], cpu.a);
    }
}

void abi_HitBoxWidthByFrame0D5D(void)
{
    HitBoxWidthByFrame0D5D(cpu.x);
}

/*
 * @routine 0D5D:4FD WaitThenStepPattern0D5D
 * @run 71:C4FD
 * WHAT IT DOES: Counts OBJ_WORK_D down, returning with C clear; once it is zero, takes the next wait from the table
 * by OBJ_START_ARG and OBJ_WORK_I, restarts the animation in state 4, substate 0 and returns with C set (Stage 08's
 * overlay).
 * HOW: DEC, or eight times OBJ_START_ARG plus OBJ_WORK_I's low three bits indexing the waits.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: C set when the pattern stepped, clear while waiting; A and N,
 * Z as the last load or DEC left them; X kept.
 */
enum {
    WaitThenStepPattern0D5DWaits = 0xC52E,   /* eight waits for each record */
};

void WaitThenStepPattern0D5D(uint8_t slot)
{
    op_lda(RD8(OBJ->work_d[slot]));
    if (!flag_z()) {
        RMW8(OBJ->work_d[slot], v, f_dec(v));
        op_clc();
        return;
    }
    op_lda(RD8(OBJ->start_arg[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_i[slot]));
    op_and(0x07);
    op_clc();
    op_adc(RD8(ZP->temp00));
    op_tay();
    op_lda(rd8((uint16_t)(WaitThenStepPattern0D5DWaits + cpu.y)));
    WR8(OBJ->work_d[slot], cpu.a);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    op_lda(0x04);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
    op_sec();
}

void abi_WaitThenStepPattern0D5D(void)
{
    WaitThenStepPattern0D5D(cpu.x);
}

/*
 * @routine 0D5E:112 LeapByNextFacing0D5E
 * @run 71:C912
 * WHAT IT DOES: Starts the object upward at -2 with an X speed by the facing of the next slot, and steps the state
 * on (Stage 08's overlay).
 * HOW: A table read by the next slot's OBJ_FACING_LEFT.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    LeapByNextFacing0D5EXSpeeds = 0xC924,   /* the X speed high byte for each facing */
};

void LeapByNextFacing0D5E(uint8_t slot)
{
    op_lda(0xFE);
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_ldy(RD8(OBJ->facing_left[slot + 1]));
    op_lda(rd8((uint16_t)(LeapByNextFacing0D5EXSpeeds + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    RMW8(OBJ->state[slot], v, f_inc(v));
}

void abi_LeapByNextFacing0D5E(void)
{
    LeapByNextFacing0D5E(cpu.x);
}

/*
 * @routine 0D5F:255 HitBoxXByFrame0D5F
 * @run 71:D255
 * WHAT IT DOES: Turns the hit box on with an X offset by frame for frames up to $8F; from $90 on, the box is off
 * (Stage 08's overlay).
 * HOW: A compare of the frame, a table read by the frame less $8A.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    HitBoxXByFrame0D5FOffsets = 0xD273,   /* the hit box's X offset for frames $8A to $8F */
};

void HitBoxXByFrame0D5F(uint8_t slot)
{
    op_lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    op_lda(RD8(OBJ->frame[slot]));
    op_cmp(0x90);
    if (!flag_c()) {
        op_sec();
        op_sbc(0x8A);
        op_tay();
        op_lda(rd8((uint16_t)(HitBoxXByFrame0D5FOffsets + cpu.y)));
        WR8(OBJ->hitbox_dx[slot], cpu.a);
        return;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->hitbox_dx[slot], 0);
}

void abi_HitBoxXByFrame0D5F(void)
{
    HitBoxXByFrame0D5F(cpu.x);
}

/*
 * @routine 0D5F:4BF RepeatOrStand0D5F
 * @run 71:D4BF
 * WHAT IT DOES: Counts OBJ_WORK_K down: while it runs, restarts the animation in substate 1; when it ends, state 1
 * with frame $8A, or $8D when OBJ_WORK_J is set (Stage 08's overlay).
 * HOW: DEC and two arms of stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void RepeatOrStand0D5F(uint8_t slot)
{
    RMW8(OBJ->work_k[slot], v, f_dec(v));
    if (!flag_z()) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        op_lda(0x01);
        WR8(OBJ->substate[slot], cpu.a);
        return;
    }
    op_lda(0x01);
    WR8(OBJ->state[slot], cpu.a);
    op_lda(0x8A);
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(RD8(OBJ->work_j[slot]));
    if (!flag_z()) {
        op_lda(0x8D);
        WR8(OBJ->frame[slot], cpu.a);
    }
}

void abi_RepeatOrStand0D5F(void)
{
    RepeatOrStand0D5F(cpu.x);
}

/*
 * @routine 0D5F:6D1 HitBoxByFrame0D5F
 * @run 71:D6D1
 * WHAT IT DOES: Gives the object the hit box of its frame from $9C on (Stage 08's overlay).
 * HOW: Four times the frame less $9C indexes four-byte records.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    HitBoxByFrame0D5FBoxes = 0xD6F3,   /* X offset, Y offset, half width, half height per frame */
};

void HitBoxByFrame0D5F(uint8_t slot)
{
    op_lda(RD8(OBJ->frame[slot]));
    op_sec();
    op_sbc(0x9C);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(HitBoxByFrame0D5FBoxes + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    op_lda(rd8((uint16_t)(HitBoxByFrame0D5FBoxes + 1 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    op_lda(rd8((uint16_t)(HitBoxByFrame0D5FBoxes + 2 + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    op_lda(rd8((uint16_t)(HitBoxByFrame0D5FBoxes + 3 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_HitBoxByFrame0D5F(void)
{
    HitBoxByFrame0D5F(cpu.x);
}

/*
 * @routine 0D60:27E FallFasterTo4_0D60
 * @run 71:DA7E
 * WHAT IT DOES: Adds $10 to the object's Y speed, up to 4 pixels a frame (Stage 08's overlay).
 * HOW: A 16-bit add and a clamp.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void FallFasterTo4_0D60(uint8_t slot)
{
    op_lda(RD8(OBJ->vy_lo[slot]));
    op_clc();
    op_adc(0x10);
    WR8(OBJ->vy_lo[slot], cpu.a);
    op_lda(RD8(OBJ->vy_hi[slot]));
    op_adc(0x00);
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_cmp(0x04);
    if (flag_c()) {
        op_lda(0x00);
        WR8(OBJ->vy_lo[slot], cpu.a);
        op_lda(0x04);
        WR8(OBJ->vy_hi[slot], cpu.a);
    }
}

void abi_FallFasterTo4_0D60(void)
{
    FallFasterTo4_0D60(cpu.x);
}

/*
 * @routine 0D60:30E RiseByTemp02_0D60
 * @run 71:DB0E
 * WHAT IT DOES: Moves the object up by zTemp02 pixels and clears its Y fraction (Stage 08's overlay).
 * HOW: A 16-bit subtract.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void RiseByTemp02_0D60(uint8_t slot)
{
    op_lda(RD8(OBJ->y_lo[slot]));
    op_sec();
    op_sbc(RD8(ZP->temp02));
    WR8(OBJ->y_lo[slot], cpu.a);
    op_lda(RD8(OBJ->y_hi[slot]));
    op_sbc(0x00);
    WR8(OBJ->y_hi[slot], cpu.a);
    WR8(OBJ->y_frac[slot], 0);
}

void abi_RiseByTemp02_0D60(void)
{
    RiseByTemp02_0D60(cpu.x);
}

/*
 * @routine 0D6C:256 StartWhenWorkH0D6C
 * @run 74:DA56
 * WHAT IT DOES: Once OBJ_WORK_H is set, sets OBJ_WORK_F to $FF, OBJ_WORK_E to $0C and steps the state on (Stage
 * 08's overlay).
 * HOW: A test, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartWhenWorkH0D6C(uint8_t slot)
{
    op_lda(RD8(OBJ->work_h[slot]));
    if (!flag_z()) {
        op_lda(0xFF);
        WR8(OBJ->work_f[slot], cpu.a);
        op_lda(0x0C);
        WR8(OBJ->work_e[slot], cpu.a);
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_StartWhenWorkH0D6C(void)
{
    StartWhenWorkH0D6C(cpu.x);
}

/*
 * @routine 0D6C:546 SetUpSmallPiece0D6C
 * @run 74:DD46
 * WHAT IT DOES: Sets sprite set 6, frame $62 and a 4 by 4 hit box offset by -$0C and 8 (Stage 08's overlay).
 * HOW: Stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SetUpSmallPiece0D6C(uint8_t slot)
{
    op_lda(0x06);
    WR8(OBJ->sprite_set[slot], cpu.a);
    op_lda(0x62);
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(0x04);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    op_lda(0x04);
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
    op_lda(0xF4);
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    op_lda(0x08);
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    RMW8(OBJ->state[slot], v, f_inc(v));
}

void abi_SetUpSmallPiece0D6C(void)
{
    SetUpSmallPiece0D6C(cpu.x);
}
