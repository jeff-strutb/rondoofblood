/*
 * Calling routines of Stage 06's overlays: object states that start, move, animate, land and remove objects, pick
 * moves at random, ride or follow other slots, and wipe blocks of VRAM bit by bit.
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
#include "stage06_calls_c.h"

/*
 * @routine 0A8F:002 StepSlots54And55ThenMask0A8F
 * @run 6B:9002
 * @stop 904A
 * WHAT IT DOES: Drives slots $54 and $55: moves $55 by its Y speed; counts $54's OBJ_WORK_E down and on zero steps
 * its OBJ_WORK_D on, taking a new X speed and count from two tables; moves $54 by its X speed; while $55's
 * OBJ_WORK_D is 6, copies $55's OBJ_WORK_E into $54's OBJ_WORK_F (clearing $54's frame at $1F); once $54's
 * OBJ_WORK_D is 6, goes on into the VRAM mask with $54's OBJ_WORK_F (Stage 06's $8000 overlay).
 * HOW: Fixed slots; a DEC and two table reads by the step; the mask by a fall into $9066.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    StepSlots54And55ThenMask0A8FSpeeds = 0x9049,   /* the X speed per step: high byte in bits 1-7, bit 0 the low byte's $80 */
    StepSlots54And55ThenMask0A8FCounts = 0x904F,   /* the frames each step lasts */
};

void StepSlots54And55ThenMask0A8F(void)
{
    op_lda(RD8(OBJ->work_d[0x55]));
    op_cmp(0x06);
    if (flag_z()) {
        op_lda(RD8(OBJ->work_e[0x55]));
        op_cmp(0x1F);
        if (flag_z()) {
            WR8(OBJ->frame[0x54], 0);
        }
        WR8(OBJ->work_f[0x54], cpu.a);
    }
    op_ldx(0x55);
    jsr_push(0x901B);
    axis_move_exit_shim(ApplyVelocityY(cpu.x));
    jsr_pop();
    RMW8(OBJ->work_e[0x54], v, f_dec(v));
    if (flag_z()) {
        RMW8(OBJ->work_d[0x54], v, f_inc(v));
        op_ldy(RD8(OBJ->work_d[0x54]));
        op_lda(rd8((uint16_t)(StepSlots54And55ThenMask0A8FSpeeds + cpu.y)));
        cpu.a = f_lsr(cpu.a);
        WR8(OBJ->vx_hi[0x54], cpu.a);
        op_cla();
        cpu.a = f_ror(cpu.a);
        WR8(OBJ->vx_lo[0x54], cpu.a);
        op_lda(rd8((uint16_t)(StepSlots54And55ThenMask0A8FCounts + cpu.y)));
        WR8(OBJ->work_e[0x54], cpu.a);
    }
    op_ldx(0x54);
    jsr_push(0x903D);
    axis_move_exit_shim(ApplyVelocityX(cpu.x));
    jsr_pop();
    op_lda(RD8(OBJ->work_d[0x54]));
    op_cmp(0x06);
    if (!flag_z()) {
        return;
    }
    op_lda(RD8(OBJ->work_f[0x54]));
    MaskVramRowsByLevel0A8F();
}

void abi_StepSlots54And55ThenMask0A8F(void)
{
    StepSlots54And55ThenMask0A8F();
}

/*
 * @routine 0A8F:066 MaskVramRowsByLevel0A8F
 * @run 6B:9066
 * @stop 90B1
 * WHAT IT DOES: From the level in A (folded down when $20 or more) sets the mask's depth (multiplier high, $10 less
 * the low four bits) and picks one of four runs of VRAM rows; for each row takes a random number and masks the row
 * (Stage 06's $8000 overlay).
 * HOW: Bits 4-5 of the folded level index four-byte records; a loop that adds the step to the 16-bit address.
 * WAITS: no.
 * Entry: A the level. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    MaskVramRowsByLevel0A8FRuns = 0x9056,   /* VRAM address low, high, row count, address step per run */
};

void MaskVramRowsByLevel0A8F(void)
{
    WR8(ZP->multiplier.hi, cpu.a);
    op_cmp(0x20);
    if (flag_c()) {
        op_and(0x1F);
        cpu.a = f_lsr(cpu.a);
        if (flag_c()) {
            op_clc();
            op_adc(0x10);
        }
        op_adc(0x20);
    }
    op_pha();
    op_and(0x0F);
    WR8(ZP->temp10, cpu.a);
    op_lda(0x10);
    op_sec();
    op_sbc(RD8(ZP->temp10));
    WR8(ZP->multiplier.hi, cpu.a);
    op_pla();
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    op_and(0x0C);
    op_tax();
    op_lda(rd8((uint16_t)(MaskVramRowsByLevel0A8FRuns + cpu.x)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(MaskVramRowsByLevel0A8FRuns + 1 + cpu.x)));
    WR8(ZP->temp11, cpu.a);
    op_lda(rd8((uint16_t)(MaskVramRowsByLevel0A8FRuns + 2 + cpu.x)));
    WR8(ZP->temp12, cpu.a);
    op_lda(rd8((uint16_t)(MaskVramRowsByLevel0A8FRuns + 3 + cpu.x)));
    WR8(ZP->product.lo, cpu.a);
    do {
        jsr_push(0x909F);
        random_exit_shim(NextRandom());
        jsr_pop();
        call_routine(0x90A1, 0x90B1);
        op_lda(RD8(ZP->temp10));
        op_clc();
        op_adc(RD8(ZP->product.lo));
        WR8(ZP->temp10, cpu.a);
        if (flag_c()) {
            RMW8(ZP->temp11, v, f_inc(v));
        }
        RMW8(ZP->temp12, v, f_dec(v));
    } while (!flag_z());
}

void abi_MaskVramRowsByLevel0A8F(void)
{
    MaskVramRowsByLevel0A8F();
}

/*
 * @routine 0A9A:6C4 SpawnKind2AWhenRoom0A9A
 * @run 70:AEC4
 * WHAT IT DOES: State 0 loads the record; then counts OBJ_WORK_D down and on zero looks for seven empty slots in a
 * row at $10, $17 or $1E: with them, starts a kind $2A in the first and waits $A0 frames; without, waits $40 (Stage
 * 06's overlay).
 * HOW: A DEC; an OR of seven kinds per group; StartObjectAtPlace.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void SpawnKind2AWhenRoom0A9A(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
        }
        return;
    }
    op_lda(RD8(OBJ->work_d[slot]));
    if (!flag_z()) {
        RMW8(OBJ->work_d[slot], v, f_dec(v));
        return;
    }
    op_ldy(0x10);
at_AED7:
    op_lda(RD8(OBJ->kind[cpu.y]));
    op_ora(RD8(OBJ->kind[cpu.y + 1]));
    op_ora(RD8(OBJ->kind[cpu.y + 2]));
    op_ora(RD8(OBJ->kind[cpu.y + 3]));
    op_ora(RD8(OBJ->kind[cpu.y + 4]));
    op_ora(RD8(OBJ->kind[cpu.y + 5]));
    op_ora(RD8(OBJ->kind[cpu.y + 6]));
    if (!flag_z()) {
        op_tya();
        op_clc();
        op_adc(0x07);
        op_tay();
        op_cpy(0x25);
        if (!flag_z()) {
            goto at_AED7;
        }
        op_lda(0x40);
        WR8(OBJ->work_d[slot], cpu.a);
        return;
    }
    jsr_push(0xAF00);
    ClearObjectInY(cpu.y);
    jsr_pop();
    op_lda(0x2A);
    jsr_push(0xAF05);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
    jsr_pop();
    op_lda(0xA0);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_SpawnKind2AWhenRoom0A9A(void)
{
    SpawnKind2AWhenRoom0A9A(cpu.x);
}

/*
 * @routine 0A9A:70B PlaceAndStartSixKind2B0A9A
 * @run 70:AF0B
 * WHAT IT DOES: State 0 puts the object at one of eight places chosen by PlayerDistanceSum, drifts it with the
 * scroll, starts six kind $2B objects in the next six slots (OBJ_WORK_F from the place's record, OBJ_WORK_D 0, 4 ..
 * $14, OBJ_WORK_I 0 to 5) and loads its record; later it clears itself once the next slot no longer holds kind $2B
 * (Stage 06's overlay).
 * HOW: Four times the random's low three bits indexes four-byte records; a loop of six starts.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    PlaceAndStartSixKind2B0A9APlaces = 0xAF74,   /* X low, Y high, Y low, the parts' OBJ_WORK_F per place */
};

void PlaceAndStartSixKind2B0A9A(void)
{
    op_lda(RD8(OBJ->state[cpu.x]));
    if (flag_z()) {
        WR8(OBJ->x_hi[cpu.x], 0);
        jsr_push(0xAF16);
        distance_sum_exit_shim(PlayerDistanceSum(cpu.x, (cpu.p & P_C) != 0));
        jsr_pop();
        op_and(0x07);
        cpu.a = f_asl(cpu.a);
        cpu.a = f_asl(cpu.a);
        op_tay();
        op_lda(rd8((uint16_t)(PlaceAndStartSixKind2B0A9APlaces + cpu.y)));
        WR8(OBJ->x_lo[cpu.x], cpu.a);
        op_lda(rd8((uint16_t)(PlaceAndStartSixKind2B0A9APlaces + 1 + cpu.y)));
        WR8(OBJ->y_hi[cpu.x], cpu.a);
        op_lda(rd8((uint16_t)(PlaceAndStartSixKind2B0A9APlaces + 2 + cpu.y)));
        WR8(OBJ->y_lo[cpu.x], cpu.a);
        op_lda(rd8((uint16_t)(PlaceAndStartSixKind2B0A9APlaces + 3 + cpu.y)));
        WR8(ZP->temp01, cpu.a);
        call_routine(0xAF35, 0xAF94);
        op_txa();
        op_tay();
        op_iny();
        op_lda(0x06);
        WR8(ZP->temp00, cpu.a);
        WR8(ZP->temp02, 0);
        WR8(ZP->temp03, 0);
        do {
            jsr_push(0xAF43);
            ClearObjectInY(cpu.y);
            jsr_pop();
            op_lda(0x2B);
            jsr_push(0xAF48);
            copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
            jsr_pop();
            op_lda(RD8(ZP->temp01));
            WR8(OBJ->work_f[cpu.y], cpu.a);
            op_lda(RD8(ZP->temp02));
            WR8(OBJ->work_d[cpu.y], cpu.a);
            op_lda(RD8(ZP->temp03));
            WR8(OBJ->work_i[cpu.y], cpu.a);
            op_iny();
            op_lda(RD8(ZP->temp02));
            op_clc();
            op_adc(0x04);
            WR8(ZP->temp02, cpu.a);
            RMW8(ZP->temp03, v, f_inc(v));
            RMW8(ZP->temp00, v, f_dec(v));
        } while (!flag_z());
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xAF68);
            last = LoadObjectRecord(cpu.x, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        return;
    }
    op_lda(RD8(OBJ->kind[cpu.x + 1]));
    op_cmp(0x2B);
    if (!flag_z()) {
        ClearObject(cpu.x);
        return;
    }
}

void abi_PlaceAndStartSixKind2B0A9A(void)
{
    PlaceAndStartSixKind2B0A9A();
}

/*
 * @routine 0A9A:7D1 LoadAndArmLastPart0A9A
 * @run 70:AFD1
 * WHAT IT DOES: Loads the record; the part with OBJ_WORK_I 5 also takes OBJ_WORK_J from a table by
 * PlayerDistanceSum, its hit box on, hit reaction $44 and its hit status cleared (Stage 06's overlay).
 * HOW: A compare, a table read by the random's low two bits, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    LoadAndArmLastPart0A9AWorkJ = 0xAFF5,   /* four values of OBJ_WORK_J */
};

void LoadAndArmLastPart0A9A(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xAFD4);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(RD8(OBJ->work_i[slot]));
    op_cmp(0x05);
    if (flag_z()) {
        jsr_push(0xAFDE);
        distance_sum_exit_shim(PlayerDistanceSum(slot, (cpu.p & P_C) != 0));
        jsr_pop();
        op_and(0x03);
        op_tay();
        op_lda(rd8((uint16_t)(LoadAndArmLastPart0A9AWorkJ + cpu.y)));
        WR8(OBJ->work_j[slot], cpu.a);
        op_lda(0x02);
        WR8(OBJ->hitbox_on[slot], cpu.a);
        op_lda(0x44);
        WR8(OBJ->hit_reaction[slot], cpu.a);
        WR8(OBJ->hit_status[slot], 0);
    }
}

void abi_LoadAndArmLastPart0A9A(void)
{
    LoadAndArmLastPart0A9A(cpu.x);
}

/*
 * @routine 0A9B:04A FollowPrevThenMove0A9B
 * @run 70:B04A
 * WHAT IT DOES: Puts the object at the place of the slot before it, then moves it as its direction gives (Stage
 * 06's overlay).
 * HOW: CopyObjectPlaceBack from X - 1, then a fall into $B050.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FollowPrevThenMove0A9B(void)
{
    op_txa();
    op_tay();
    op_dey();
    jsr_push(0xB050);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, cpu.x));
    jsr_pop();
    /* falls into $B050 */
    MoveByDirectionFrame0A9B(cpu.x);
}

void abi_FollowPrevThenMove0A9B(void)
{
    FollowPrevThenMove0A9B();
}

/*
 * @routine 0A9B:050 MoveByDirectionFrame0A9B
 * @run 70:B050
 * @stop B0B5
 * WHAT IT DOES: Part 5 (OBJ_WORK_I 5) turns and drops at the player instead; the others take X and Y speeds for the
 * direction in OBJ_WORK_F (bit 7 reverses both), move, and show frame $70 plus the direction's top four bits (Stage
 * 06's overlay).
 * HOW: Twice OBJ_WORK_F indexes the high and low speed bytes; NegateVelocityX and Y; bit 6 picks the rounding
 * added.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    MoveByDirectionFrame0A9BRound = 0xB0B5,   /* the rounding added to OBJ_WORK_F, by its bit 6 */
    MoveByDirectionFrame0A9BSpeedsHi = 0xB0D8,   /* X speed high, Y speed high per direction */
    MoveByDirectionFrame0A9BSpeedsLo = 0xB1D8,   /* X speed low, Y speed low per direction */
};

void MoveByDirectionFrame0A9B(uint8_t slot)
{
    op_lda(RD8(OBJ->work_i[slot]));
    op_cmp(0x05);
    if (flag_z()) {
        TurnAndDropAtPlayer0A9B(slot);
        return;
    }
    op_lda(RD8(OBJ->work_f[slot]));
    cpu.a = f_asl(cpu.a);
    if (!flag_c()) {
        op_tay();
        op_lda(rd8((uint16_t)(MoveByDirectionFrame0A9BSpeedsHi + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        op_lda(rd8((uint16_t)(MoveByDirectionFrame0A9BSpeedsLo + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        op_lda(rd8((uint16_t)(MoveByDirectionFrame0A9BSpeedsHi + 1 + cpu.y)));
        WR8(OBJ->vy_hi[slot], cpu.a);
        op_lda(rd8((uint16_t)(MoveByDirectionFrame0A9BSpeedsLo + 1 + cpu.y)));
        WR8(OBJ->vy_lo[slot], cpu.a);
    } else {
        op_tay();
        op_lda(rd8((uint16_t)(MoveByDirectionFrame0A9BSpeedsHi + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        op_lda(rd8((uint16_t)(MoveByDirectionFrame0A9BSpeedsLo + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        op_lda(rd8((uint16_t)(MoveByDirectionFrame0A9BSpeedsHi + 1 + cpu.y)));
        WR8(OBJ->vy_hi[slot], cpu.a);
        op_lda(rd8((uint16_t)(MoveByDirectionFrame0A9BSpeedsLo + 1 + cpu.y)));
        WR8(OBJ->vy_lo[slot], cpu.a);
        jsr_push(0xB094);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
        jsr_push(0xB097);
        negate_exit_shim(NegateVelocityY(slot));
        jsr_pop();
    }
    jsr_push(0xB09A);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    op_lda(RD8(OBJ->work_f[slot]));
    cpu.a = f_rol(cpu.a);
    cpu.a = f_rol(cpu.a);
    cpu.a = f_rol(cpu.a);
    op_and(0x01);
    op_tay();
    op_lda(RD8(OBJ->work_f[slot]));
    op_clc();
    op_adc(rd8((uint16_t)(MoveByDirectionFrame0A9BRound + cpu.y)));
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    op_clc();
    op_adc(0x70);
    WR8(OBJ->frame[slot], cpu.a);
}

void abi_MoveByDirectionFrame0A9B(void)
{
    MoveByDirectionFrame0A9B(cpu.x);
}

/*
 * @routine 0A9B:0B7 TurnAndDropAtPlayer0A9B
 * @run 70:B0B7
 * WHAT IT DOES: Shows frame $6B, faces the player, takes the X speed of that facing and a Y speed of $17 down, and
 * moves (Stage 06's overlay).
 * HOW: SideOfPlayer, a table read by the facing, ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    TurnAndDropAtPlayer0A9BSpeeds = 0xB0D6,   /* the X speed high byte by facing */
};

void TurnAndDropAtPlayer0A9B(uint8_t slot)
{
    op_lda(0x6B);
    WR8(OBJ->frame[slot], cpu.a);
    jsr_push(0xB0BF);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    op_and(0x01);
    WR8(OBJ->facing_left[slot], cpu.a);
    op_ldy(RD8(OBJ->facing_left[slot]));
    op_lda(rd8((uint16_t)(TurnAndDropAtPlayer0A9BSpeeds + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(0x17);
    WR8(OBJ->vy_hi[slot], cpu.a);
    jsr_push(0xB0D5);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
}

void abi_TurnAndDropAtPlayer0A9B(void)
{
    TurnAndDropAtPlayer0A9B(cpu.x);
}

/*
 * @routine 0A9B:327 BecomeKind2CInY0A9B
 * @run 70:B327
 * WHAT IT DOES: Starts a kind $2C in slot Y at the object's place (clearing it again if its X high byte is not 0)
 * and removes the object (Stage 06's overlay).
 * HOW: StartObjectAtPlace, a test, ClearObjectInY, ClearObject.
 * WAITS: no.
 * Entry: X the object; Y the slot for the new object. Exit: A, X, Y and the flags as its last instruction or call
 * left them.
 */
void BecomeKind2CInY0A9B(uint8_t slot)
{
    op_lda(0x2C);
    jsr_push(0xB32C);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
    jsr_pop();
    op_lda(0xFC);
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_lda(RD8(OBJ->x_hi[cpu.y]));
    if (!flag_z()) {
        jsr_push(0xB339);
        ClearObjectInY(cpu.y);
        jsr_pop();
    }
    ClearObject(slot);
}

void abi_BecomeKind2CInY0A9B(void)
{
    BecomeKind2CInY0A9B(cpu.x);
}

/*
 * @routine 0A9B:397 SoundWhenSlot29Low0A9B
 * @run 70:B397
 * WHAT IT DOES: Once slot $29's Y position is $00B0 or more, plays sound $4E and steps the state on (Stage 06's
 * overlay).
 * HOW: Two compares, SoundCommand, INC.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void SoundWhenSlot29Low0A9B(uint8_t slot)
{
    op_lda(RD8(OBJ->y_hi[0x29]));
    if (flag_z()) {
        op_lda(RD8(OBJ->y_lo[0x29]));
        op_cmp(0xB0);
        if (flag_c()) {
            op_lda(0x4E);
            {
                uint8_t command;

                jsr_push(0xB3A8);
                command = cpu.a;
                sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
                jsr_pop();
            }
            RMW8(OBJ->state[slot], v, f_inc(v));
        }
    }
}

void abi_SoundWhenSlot29Low0A9B(void)
{
    SoundWhenSlot29Low0A9B(cpu.x);
}

/*
 * @routine 0A9B:3AC SetLevelYSteps0A9B
 * @run 70:B3AC
 * WHAT IT DOES: Sets the level's Y word (LEVELW) to the value of step OBJ_WORK_D and steps it on; at step 4 removes
 * the object (Stage 06's overlay).
 * HOW: A compare, twice the step indexes a word table.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    SetLevelYSteps0A9BValues = 0xB3C8,   /* the level's Y word for steps 0 to 3 */
};

void SetLevelYSteps0A9B(uint8_t slot)
{
    op_lda(RD8(OBJ->work_d[slot]));
    op_cmp(0x04);
    if (flag_z()) {
        ClearObject(slot);
        return;
    }
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(SetLevelYSteps0A9BValues + cpu.y)));
    WR8(LEVELW->y.lo, cpu.a);
    op_lda(rd8((uint16_t)(SetLevelYSteps0A9BValues + 1 + cpu.y)));
    WR8(LEVELW->y.hi, cpu.a);
    RMW8(OBJ->work_d[slot], v, f_inc(v));
}

void abi_SetLevelYSteps0A9B(void)
{
    SetLevelYSteps0A9B(cpu.x);
}

/*
 * @routine 0A9B:3E8 LoadFrameByArg0A9B
 * @run 70:B3E8
 * WHAT IT DOES: Loads the record and shows frame 1 plus OBJ_START_ARG (Stage 06's overlay).
 * HOW: LoadObjectRecord and an add.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void LoadFrameByArg0A9B(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xB3EB);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x01);
    op_clc();
    op_adc(RD8(OBJ->start_arg[slot]));
    WR8(OBJ->frame[slot], cpu.a);
}

void abi_LoadFrameByArg0A9B(void)
{
    LoadFrameByArg0A9B(cpu.x);
}

/*
 * @routine 0A9B:3F5 RecordThenSpeedByArg0A9B
 * @run 70:B3F5
 * WHAT IT DOES: With its X high byte 0 and slot $10 not holding kind $36, records the object's event, takes an X
 * speed by OBJ_START_ARG and steps the state on (Stage 06's overlay).
 * HOW: Two tests, RecordObjectEvent, a table read.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RecordThenSpeedByArg0A9BSpeeds = 0xB411,   /* the X speed high byte by OBJ_START_ARG */
};

void RecordThenSpeedByArg0A9B(uint8_t slot)
{
    op_lda(RD8(OBJ->x_hi[slot]));
    if (flag_z()) {
        op_lda(RD8(OBJ->kind[0x10]));
        op_cmp(0x36);
        if (!flag_z()) {
            {
                uint8_t stored;
                uint8_t event;
                uint8_t byte;

                jsr_push(0xB404);
                if (!RecordObjectEvent(slot, &stored, &event, &byte)) {
                    cpu.a = 0;
                    abi_nz(0);
                } else {
                    record_event_exit_shim(event, stored, byte);
                }
                jsr_pop();
            }
            op_ldy(RD8(OBJ->start_arg[slot]));
            op_lda(rd8((uint16_t)(RecordThenSpeedByArg0A9BSpeeds + cpu.y)));
            WR8(OBJ->vx_hi[slot], cpu.a);
            RMW8(OBJ->state[slot], v, f_inc(v));
        }
    }
}

void abi_RecordThenSpeedByArg0A9B(void)
{
    RecordThenSpeedByArg0A9B(cpu.x);
}

/*
 * @routine 0A9B:433 ContactWhilePlayerMovesY0A9B
 * @run 70:B433
 * WHAT IT DOES: Sets sprite attribute $87; when the object touches the player while the player's Y speed is not 0,
 * keeps the side the player is on in OBJ_WORK_E, sets OBJ_WORK_D 5 and steps the state on (Stage 06's overlay).
 * HOW: ContactWithPlayer with a $1C by $1A box, a test of slot 0, SideOfPlayer.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ContactWhilePlayerMovesY0A9B(void)
{
    op_lda(0x87);
    WR8(OBJ->sprite_attr[cpu.x], cpu.a);
    op_lda(0x1C);
    WR8(ZP->temp08, cpu.a);
    op_lda(0x1A);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xB443, 0xAA35);
    if (flag_c()) {
        op_lda(RD8(OBJ->vy_hi[0x00]));
        if (!flag_z()) {
            jsr_push(0xB44D);
            side_of_player_exit_shim(SideOfPlayer(cpu.x));
            jsr_pop();
            op_and(0x01);
            WR8(OBJ->work_e[cpu.x], cpu.a);
            op_lda(0x05);
            WR8(OBJ->work_d[cpu.x], cpu.a);
            RMW8(OBJ->state[cpu.x], v, f_inc(v));
        }
    }
}

void abi_ContactWhilePlayerMovesY0A9B(void)
{
    ContactWhilePlayerMovesY0A9B();
}

/*
 * @routine 0A9B:45B ContactThenCountDown0A9B
 * @run 70:B45B
 * WHAT IT DOES: Lets the player touch the object, counts OBJ_WORK_D down and on zero steps the state on (Stage 06's
 * overlay).
 * HOW: ContactWithPlayer with a $1C by $1A box, DEC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ContactThenCountDown0A9B(void)
{
    op_lda(0x1C);
    WR8(ZP->temp08, cpu.a);
    op_lda(0x1A);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xB466, 0xAA35);
    RMW8(OBJ->work_d[cpu.x], v, f_dec(v));
    if (flag_z()) {
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
    }
}

void abi_ContactThenCountDown0A9B(void)
{
    ContactThenCountDown0A9B();
}

/*
 * @routine 0A9B:46F PlayListByWorkE0A9B
 * @run 70:B46F
 * WHAT IT DOES: Plays the frame list OBJ_WORK_E chooses; when it ends, goes to state 1 (Stage 06's overlay).
 * HOW: Twice OBJ_WORK_E indexes a word table of lists; StepObjectFrames.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    PlayListByWorkE0A9BLists = 0xB489,   /* the frame list address per OBJ_WORK_E */
};

void PlayListByWorkE0A9B(uint8_t slot)
{
    op_lda(RD8(OBJ->work_e[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(PlayListByWorkE0A9BLists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(PlayListByWorkE0A9BLists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xB481);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        op_lda(0x01);
        WR8(OBJ->state[slot], cpu.a);
    }
}

void abi_PlayListByWorkE0A9B(void)
{
    PlayListByWorkE0A9B(cpu.x);
}

/*
 * @routine 0A9B:52B KnockBackAndBurst0A9B
 * @run 70:B52B
 * WHAT IT DOES: Runs the wait-to-be-hit step; then, unless the object is kind $CE, gives it a speed of $0180 across
 * and $FC80 up, plays sound $19 and starts a debris burst (Stage 06's overlay).
 * HOW: StepReleaseOnHit, a compare of the kind, stores, SoundCommand, StartDebrisBurst with A 1.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void KnockBackAndBurst0A9B(void)
{
    call_routine(0xB52E, 0xA329);
    op_lda(RD8(OBJ->kind[cpu.x]));
    op_cmp(0xCE);
    if (!flag_z()) {
        op_lda(0x80);
        WR8(OBJ->vx_lo[cpu.x], cpu.a);
        op_lda(0x01);
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        op_lda(0x80);
        WR8(OBJ->vy_lo[cpu.x], cpu.a);
        op_lda(0xFC);
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        op_lda(0x19);
        {
            uint8_t command;

            jsr_push(0xB54E);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
            jsr_pop();
        }
        op_lda(0x01);
        call_routine(0xB553, 0xA90B);
    }
}

void abi_KnockBackAndBurst0A9B(void)
{
    KnockBackAndBurst0A9B();
}

/*
 * @routine 0A9B:578 LoadBoxAndWaitByArg0A9B
 * @run 70:B578
 * WHAT IT DOES: Loads the record, shows frame 5, and takes its hit box and a wait (into OBJ_WORK_D and OBJ_WORK_E)
 * by OBJ_START_ARG (Stage 06's overlay).
 * HOW: Twice OBJ_START_ARG indexes the box bytes; OBJ_START_ARG indexes the waits.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    LoadBoxAndWaitByArg0A9BWaits = 0xB556,   /* the wait per OBJ_START_ARG */
    LoadBoxAndWaitByArg0A9BBoxes = 0xB558,   /* half width, half height, X offset, Y offset, two bytes apart */
};

void LoadBoxAndWaitByArg0A9B(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xB57B);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x05);
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(RD8(OBJ->start_arg[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(LoadBoxAndWaitByArg0A9BBoxes + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    op_lda(rd8((uint16_t)(LoadBoxAndWaitByArg0A9BBoxes + 1 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
    op_lda(rd8((uint16_t)(LoadBoxAndWaitByArg0A9BBoxes + 2 + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    op_lda(rd8((uint16_t)(LoadBoxAndWaitByArg0A9BBoxes + 3 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    op_ldy(RD8(OBJ->start_arg[slot]));
    op_lda(rd8((uint16_t)(LoadBoxAndWaitByArg0A9BWaits + cpu.y)));
    WR8(OBJ->work_d[slot], cpu.a);
    WR8(OBJ->work_e[slot], cpu.a);
}

void abi_LoadBoxAndWaitByArg0A9B(void)
{
    LoadBoxAndWaitByArg0A9B(cpu.x);
}

/*
 * @routine 0A9B:5EB FrameAndWaitByArg0A9B
 * @run 70:B5EB
 * WHAT IT DOES: Takes a frame and OBJ_WORK_E by OBJ_START_ARG, plays sound $3A and steps the substate on (Stage
 * 06's overlay).
 * HOW: Two table reads, SoundCommand, INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FrameAndWaitByArg0A9BValues = 0xB554,   /* the frame, then OBJ_WORK_E, per OBJ_START_ARG */
};

void FrameAndWaitByArg0A9B(uint8_t slot)
{
    op_ldy(RD8(OBJ->start_arg[slot]));
    op_lda(rd8((uint16_t)(FrameAndWaitByArg0A9BValues + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(rd8((uint16_t)(FrameAndWaitByArg0A9BValues + 1 + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(0x3A);
    {
        uint8_t command;

        jsr_push(0xB5FF);
        command = cpu.a;
        sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
        jsr_pop();
    }
    RMW8(OBJ->substate[slot], v, f_inc(v));
}

void abi_FrameAndWaitByArg0A9B(void)
{
    FrameAndWaitByArg0A9B(cpu.x);
}

/*
 * @routine 0A9B:603 RiseThenBecomeKind0A9B
 * @run 70:B603
 * WHAT IT DOES: Moves the object up a pixel a frame while OBJ_WORK_E counts down; on zero records its event and
 * turns it into the kind OBJ_START_ARG chooses, in state 0 with its hit box offsets and Y speed cleared (Stage 06's
 * overlay).
 * HOW: Two DECs, RecordObjectEvent, a table read, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RiseThenBecomeKind0A9BKinds = 0xB557,   /* the new kind per OBJ_START_ARG */
};

void RiseThenBecomeKind0A9B(uint8_t slot)
{
    RMW8(OBJ->y_lo[slot], v, f_dec(v));
    RMW8(OBJ->work_e[slot], v, f_dec(v));
    if (flag_z()) {
        {
            uint8_t stored;
            uint8_t event;
            uint8_t byte;

            jsr_push(0xB60E);
            if (!RecordObjectEvent(slot, &stored, &event, &byte)) {
                cpu.a = 0;
                abi_nz(0);
            } else {
                record_event_exit_shim(event, stored, byte);
            }
            jsr_pop();
        }
        op_ldy(RD8(OBJ->start_arg[slot]));
        op_lda(rd8((uint16_t)(RiseThenBecomeKind0A9BKinds + cpu.y)));
        WR8(OBJ->kind[slot], cpu.a);
        WR8(OBJ->state[slot], 0);
        WR8(OBJ->substate[slot], 0);
        WR8(OBJ->hitbox_dx[slot], 0);
        WR8(OBJ->hitbox_dy[slot], 0);
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->vy_lo[slot], 0);
    }
}

void abi_RiseThenBecomeKind0A9B(void)
{
    RiseThenBecomeKind0A9B(cpu.x);
}

/*
 * @routine 0A9B:6E7 BreakAfterFiveHits0A9B
 * @run 70:B6E7
 * WHAT IT DOES: State 0 loads the record with half height $10, sprite set 6 and frame $14; later, once hit 5 times
 * or more, records its event, becomes kind $8B in state 0, plays sound $19 and starts a debris burst (Stage 06's
 * overlay).
 * HOW: A test of the state; a compare of OBJ_HIT_TAKEN; stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void BreakAfterFiveHits0A9B(void)
{
    op_lda(RD8(OBJ->state[cpu.x]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xB6EF);
            last = LoadObjectRecord(cpu.x, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(0x10);
        WR8(OBJ->hitbox_half_h[cpu.x], cpu.a);
        op_lda(0x06);
        WR8(OBJ->sprite_set[cpu.x], cpu.a);
        op_lda(0x14);
        WR8(OBJ->frame[cpu.x], cpu.a);
        return;
    }
    op_lda(RD8(OBJ->hit_taken[cpu.x]));
    op_cmp(0x05);
    if (flag_c()) {
        {
            uint8_t stored;
            uint8_t event;
            uint8_t byte;

            jsr_push(0xB709);
            if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
                cpu.a = 0;
                abi_nz(0);
            } else {
                record_event_exit_shim(event, stored, byte);
            }
            jsr_pop();
        }
        op_lda(0x8B);
        WR8(OBJ->kind[cpu.x], cpu.a);
        WR8(OBJ->state[cpu.x], 0);
        op_lda(0x19);
        {
            uint8_t command;

            jsr_push(0xB716);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
            jsr_pop();
        }
        op_lda(0x01);
        call_routine(0xB71B, 0xA90B);
    }
}

void abi_BreakAfterFiveHits0A9B(void)
{
    BreakAfterFiveHits0A9B();
}

/*
 * @routine 0A9B:738 PlayFixedList0A9B
 * @run 70:B738
 * WHAT IT DOES: Plays the frame list at $B7B8 (Stage 06's overlay).
 * HOW: StepObjectFrames.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void PlayFixedList0A9B(void)
{
    op_lda(0xB8);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xB7);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xB743);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
}

void abi_PlayFixedList0A9B(void)
{
    PlayFixedList0A9B();
}

/*
 * @routine 0A9B:7A5 FreeSlotFrom38_0A9B
 * @run 70:B7A5
 * WHAT IT DOES: Finds the first empty slot from $38 to $3F and clears it (Stage 06's overlay).
 * HOW: A search loop, ClearObjectInY.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: C set with Y the slot found; C clear when none is empty.
 */
void FreeSlotFrom38_0A9B(void)
{
    op_ldy(0x38);
    do {
        op_lda(RD8(OBJ->kind[cpu.y]));
        if (flag_z()) {
            goto at_B7B3;
        }
        op_iny();
        op_cpy(0x40);
    } while (!flag_c());
    op_clc();
    return;
at_B7B3:
    jsr_push(0xB7B6);
    ClearObjectInY(cpu.y);
    jsr_pop();
    op_sec();
}

void abi_FreeSlotFrom38_0A9B(void)
{
    FreeSlotFrom38_0A9B();
}

/*
 * @routine 0A9B:7BD ArcAndShedKind9B0A9B
 * @run 70:B7BD
 * WHAT IT DOES: State 0 loads the record with sprite set 6 and the frame and X and Y speeds OBJ_WORK_D chooses;
 * later moves it with gravity and every fourth frame starts a kind $9B (slot from $20) with sprite attribute $10
 * (Stage 06's overlay).
 * HOW: Table reads by OBJ_WORK_D and twice it; the moves; a test of zWaveCounter; FindFreeSlotFrom20.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    ArcAndShedKind9B0A9BFrames = 0xB80F,   /* the frame per OBJ_WORK_D */
    ArcAndShedKind9B0A9BSpeedsX = 0xB812,   /* the X speed per OBJ_WORK_D, a word each */
    ArcAndShedKind9B0A9BSpeedsY = 0xB818,   /* the Y speed per OBJ_WORK_D, a word each */
};

void ArcAndShedKind9B0A9B(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xB7C5);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
        op_ldy(RD8(OBJ->work_d[slot]));
        op_lda(rd8((uint16_t)(ArcAndShedKind9B0A9BFrames + cpu.y)));
        WR8(OBJ->frame[slot], cpu.a);
        op_lda(RD8(OBJ->work_d[slot]));
        cpu.a = f_asl(cpu.a);
        op_tay();
        op_lda(rd8((uint16_t)(ArcAndShedKind9B0A9BSpeedsX + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        op_lda(rd8((uint16_t)(ArcAndShedKind9B0A9BSpeedsX + 1 + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        op_lda(rd8((uint16_t)(ArcAndShedKind9B0A9BSpeedsY + cpu.y)));
        WR8(OBJ->vy_lo[slot], cpu.a);
        op_lda(rd8((uint16_t)(ArcAndShedKind9B0A9BSpeedsY + 1 + cpu.y)));
        WR8(OBJ->vy_hi[slot], cpu.a);
        return;
    }
    jsr_push(0xB7F4);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
    jsr_push(0xB7F7);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
    jsr_push(0xB7FA);
    gravity_exit_shim(ApplyGravity(slot));
    jsr_pop();
    op_tst(0x03, RD8(ZP->wave_counter));
    if (flag_z()) {
        {
            uint8_t a;
            bool found;
            uint8_t found_slot;

            jsr_push(0xB802);
            found_slot = FindFreeSlotFrom20(&a, &found);
            find_slot_exit_shim(found_slot, a, found);
            jsr_pop();
        }
        if (flag_c()) {
            op_lda(0x9B);
            jsr_push(0xB809);
            copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
            jsr_pop();
            op_lda(0x10);
            WR8(OBJ->sprite_attr[cpu.y], cpu.a);
        }
    }
}

void abi_ArcAndShedKind9B0A9B(void)
{
    ArcAndShedKind9B0A9B(cpu.x);
}

/*
 * @routine 0A9C:01E SteerAroundSlotPoint0A9C
 * @run 70:B81E
 * WHAT IT DOES: State 0 sets sprite set 6 and OBJ_PACE bits 3-6. Later: waits for or finds its target, plays its
 * list, faces the player, every $10 to $1F frames picks one of eight points around the slot in OBJ_START_ARG
 * (another eight round the player when it is 0), moves at angle OBJ_WORK_D and speed OBJ_WORK_E:F, turns the angle
 * $10 toward the point, and speeds up by $20 to $0410 while within $10 of it or slows by $20 (not below $50) when
 * off (Stage 06's overlay).
 * HOW: WaitForSlotThenFind0A9C, NextRandom, SetVelocityFromAngle, AngleToPoint from a table of offsets,
 * TurnAngleToward, 16-bit adds.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    SteerAroundSlotPoint0A9COffsets = 0xB950,   /* X offset word, Y offset word per point */
};

void SteerAroundSlotPoint0A9C(void)
{
    op_lda(RD8(OBJ->state[cpu.x]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xB826);
            last = LoadObjectRecord(cpu.x, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(0x06);
        WR8(OBJ->sprite_set[cpu.x], cpu.a);
        op_lda(0x78);
        op_ora(RD8(OBJ->pace[cpu.x]));
        WR8(OBJ->pace[cpu.x], cpu.a);
        return;
    }
    call_routine(0xB837, 0xB918);
    op_lda(0x4B);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xB9);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xB842);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    jsr_push(0xB845);
    side_of_player_exit_shim(SideOfPlayer(cpu.x));
    jsr_pop();
    op_and(0x01);
    WR8(OBJ->facing_left[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_h[cpu.x]));
    if (flag_z()) {
        jsr_push(0xB852);
        random_exit_shim(NextRandom());
        jsr_pop();
        op_and(0x0F);
        op_clc();
        op_adc(0x10);
        WR8(OBJ->work_h[cpu.x], cpu.a);
        jsr_push(0xB85D);
        random_exit_shim(NextRandom());
        jsr_pop();
        op_and(0x07);
        WR8(OBJ->work_g[cpu.x], cpu.a);
    }
    RMW8(OBJ->work_h[cpu.x], v, f_dec(v));
    op_lda(RD8(OBJ->work_f[cpu.x]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_e[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_d[cpu.x]));
    jsr_push(0xB875);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, cpu.x));
    jsr_pop();
    jsr_push(0xB878);
    axis_move_exit_shim(ApplyVelocity(cpu.x));
    jsr_pop();
    op_lda(RD8(OBJ->work_g[cpu.x]));
    op_tst(0xFF, RD8(OBJ->start_arg[cpu.x]));
    if (flag_z()) {
        op_clc();
        op_adc(0x08);
    }
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(RD8(OBJ->start_arg[cpu.x]));
    op_phx();
    op_tax();
    op_lda(RD8(OBJ->x_lo[cpu.x]));
    op_clc();
    op_adc(rd8((uint16_t)(SteerAroundSlotPoint0A9COffsets + cpu.y)));
    WR8(ZP->temp08, cpu.a);
    op_lda(RD8(OBJ->x_hi[cpu.x]));
    op_adc(rd8((uint16_t)(SteerAroundSlotPoint0A9COffsets + 1 + cpu.y)));
    WR8(ZP->temp09, cpu.a);
    op_lda(RD8(OBJ->y_lo[cpu.x]));
    op_clc();
    op_adc(rd8((uint16_t)(SteerAroundSlotPoint0A9COffsets + 2 + cpu.y)));
    WR8(ZP->temp0a, cpu.a);
    op_lda(RD8(OBJ->y_hi[cpu.x]));
    op_adc(rd8((uint16_t)(SteerAroundSlotPoint0A9COffsets + 3 + cpu.y)));
    WR8(ZP->temp0b, cpu.a);
    op_plx();
    jsr_push(0xB8B2);
    angle_exit_shim(AngleToPoint(cpu.x));
    jsr_pop();
    WR8(ZP->temp02, cpu.a);
    op_lda(0x10);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_d[cpu.x]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xB8C0);
    turn_angle_exit_shim(TurnAngleToward());
    jsr_pop();
    op_and(0xF0);
    WR8(OBJ->work_d[cpu.x], cpu.a);
    op_sec();
    op_sbc(RD8(ZP->temp02));
    op_clc();
    op_adc(0x10);
    op_cmp(0x20);
    if (flag_c()) {
        goto at_B8FA;
    }
    op_lda(RD8(OBJ->work_e[cpu.x]));
    op_cmp(0x04);
    if (flag_c()) {
        op_lda(RD8(OBJ->work_f[cpu.x]));
        op_cmp(0x10);
        if (flag_c()) {
            op_lda(0x04);
            WR8(OBJ->work_e[cpu.x], cpu.a);
            op_lda(0x10);
            WR8(OBJ->work_f[cpu.x], cpu.a);
            return;
        }
    }
    op_lda(RD8(OBJ->work_f[cpu.x]));
    op_clc();
    op_adc(0x20);
    WR8(OBJ->work_f[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_e[cpu.x]));
    op_adc(0x00);
    WR8(OBJ->work_e[cpu.x], cpu.a);
at_B8F9:
    return;
at_B8FA:
    op_lda(RD8(OBJ->work_e[cpu.x]));
    if (flag_z()) {
        op_lda(RD8(OBJ->work_f[cpu.x]));
        op_cmp(0x50);
        if (!flag_c()) {
            goto at_B8F9;
        }
    }
    op_lda(RD8(OBJ->work_f[cpu.x]));
    op_sec();
    op_sbc(0x20);
    WR8(OBJ->work_f[cpu.x], cpu.a);
    op_lda(RD8(OBJ->work_e[cpu.x]));
    op_sbc(0x00);
    WR8(OBJ->work_e[cpu.x], cpu.a);
}

void abi_SteerAroundSlotPoint0A9C(void)
{
    SteerAroundSlotPoint0A9C();
}

/*
 * @routine 0A9C:190 FlipPlayerBySide0A9C
 * @run 70:B990
 * WHAT IT DOES: In any state but 0, sets the player's sprite flip by the side the player is on, as OBJ_START_ARG
 * turns it (Stage 06's overlay).
 * HOW: SideOfPlayer EOR OBJ_START_ARG, bit 0 indexes a table.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FlipPlayerBySide0A9CFlips = 0xB9A8,   /* the player's sprite flip byte by side */
};

void FlipPlayerBySide0A9C(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
        }
        return;
    }
    jsr_push(0xB99B);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    op_eor(RD8(OBJ->start_arg[slot]));
    op_and(0x01);
    op_tay();
    op_lda(rd8((uint16_t)(FlipPlayerBySide0A9CFlips + cpu.y)));
    WR8(OBJ->sprite_flip[0x00], cpu.a);
}

void abi_FlipPlayerBySide0A9C(void)
{
    FlipPlayerBySide0A9C(cpu.x);
}

/*
 * @routine 0A9C:1BE LoadAndLockScroll0A9C
 * @run 70:B9BE
 * WHAT IT DOES: State 0 loads the record and sets the scroll's X limit high byte to one screen past its low limit;
 * other states go on at $B9CE (Stage 06's overlay).
 * HOW: A test of the state; LoadObjectRecord; INC and a store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LoadAndLockScroll0A9C(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (!flag_z()) {
        WaitForFifthHit0A9C(slot);
        return;
    }
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xB9C6);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(RD8(SCROLLLIM->x_min_hi));
    cpu.a = f_inc(cpu.a);
    WR8(SCROLLLIM->x_max.hi, cpu.a);
    /* falls into $B9CD */
    JustReturn0A9C();
}

void abi_LoadAndLockScroll0A9C(void)
{
    LoadAndLockScroll0A9C(cpu.x);
}

/*
 * @routine 0A9C:1CD JustReturn0A9C
 * @run 70:B9CD
 * WHAT IT DOES: Returns at once (Stage 06's overlay).
 * HOW: RTS.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void JustReturn0A9C(void)
{
}

void abi_JustReturn0A9C(void)
{
    JustReturn0A9C();
}

/*
 * @routine 0A9C:1CE WaitForFifthHit0A9C
 * @run 70:B9CE
 * @stop B9EA
 * WHAT IT DOES: In state 1, once the object has been hit 5 times or more and a slot from $30 is free, keeps the hit
 * count in OBJ_WORK_E and steps the state on; other states go on at $BA1D (Stage 06's overlay).
 * HOW: DEC of the state, a compare of OBJ_HIT_TAKEN, FindFreeSlotFrom30.
 * WAITS: no.
 * Entry: X the object; A its state. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void WaitForFifthHit0A9C(uint8_t slot)
{
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        PlayListSoundThenSpawn0A9C(slot);
        return;
    }
    op_lda(RD8(OBJ->hit_taken[slot]));
    op_cmp(0x05);
    if (!flag_c()) {
        JustReturn0A9C();
        return;
    }
    WR8(OBJ->work_e[slot], cpu.a);
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xB9DE);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (!flag_c()) {
        JustReturn0A9C();
        return;
    }
    op_lda(RD8(OBJ->work_d[slot]));
    if (flag_z()) {
        StepStateOn0A9C(slot);
        return;
    }
    op_lda(RD8(OBJ->hit_taken[slot]));
    StepStateOn0A9C(slot);
}

void abi_WaitForFifthHit0A9C(void)
{
    WaitForFifthHit0A9C(cpu.x);
}

/*
 * @routine 0A9C:1EC StartKindInYGuarded0A9C
 * @run 70:B9EC
 * WHAT IT DOES: Starts an object of kind A in slot Y at the object's place, with OBJ_INVULN_TIMER $10 and
 * OBJ_START_ARG 2, and steps the object's state on (Stage 06's overlay).
 * HOW: StartObjectAtPlace, two stores, a fall into $B9F9.
 * WAITS: no.
 * Entry: X the object; Y the free slot; A the kind. Exit: A, X, Y and the flags as its last instruction or call
 * left them.
 */
void StartKindInYGuarded0A9C(void)
{
    jsr_push(0xB9EF);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
    jsr_pop();
    op_lda(0x10);
    WR8(OBJ->invuln_timer[cpu.y], cpu.a);
    op_lda(0x02);
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    /* falls into $B9F9 */
    StepStateOn0A9C(cpu.x);
}

void abi_StartKindInYGuarded0A9C(void)
{
    StartKindInYGuarded0A9C();
}

/*
 * @routine 0A9C:1F9 StepStateOn0A9C
 * @run 70:B9F9
 * WHAT IT DOES: Steps the object's state on (Stage 06's overlay).
 * HOW: INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StepStateOn0A9C(uint8_t slot)
{
    RMW8(OBJ->state[slot], v, f_inc(v));
}

void abi_StepStateOn0A9C(void)
{
    StepStateOn0A9C(cpu.x);
}

/*
 * @routine 0A9C:1FD SpawnNextKind0A9C
 * @run 70:B9FD
 * @stop BA10
 * WHAT IT DOES: With a free slot from $30, puts the object back to state 0 and starts kind $8B there when
 * OBJ_WORK_E is $FF, or a kind chosen at random otherwise (Stage 06's overlay).
 * HOW: FindFreeSlotFrom30, a compare, then $B9EC or $BA3F.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SpawnNextKind0A9C(uint8_t slot)
{
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xBA00);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (!flag_c()) {
        JustReturn0A9C();
        return;
    }
    WR8(OBJ->state[slot], 0);
    op_lda(RD8(OBJ->work_e[slot]));
    op_cmp(0xFF);
    if (!flag_z()) {
        StartRandomKind0A9C();
        return;
    }
    op_lda(0x8B);
    StartKindInYGuarded0A9C();
}

void abi_SpawnNextKind0A9C(void)
{
    SpawnNextKind0A9C(cpu.x);
}

/*
 * @routine 0A9C:21D PlayListSoundThenSpawn0A9C
 * @run 70:BA1D
 * WHAT IT DOES: Plays the frame list at $BA5C, playing sound $A7 as steps 2 and 6 begin; when the list ends, spawns
 * the next object (Stage 06's overlay).
 * HOW: StepObjectFrames, compares of the animation timer and step, SoundCommand; $B9FD at the end.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void PlayListSoundThenSpawn0A9C(uint8_t slot)
{
    op_lda(0xBA);
    WR8(ZP->temp11, cpu.a);
    op_lda(0x5C);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xBA28);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        SpawnNextKind0A9C(slot);
        return;
    }
    op_lda(RD8(OBJ->anim_timer[slot]));
    if (!flag_z()) {
        JustReturn0A9C();
        return;
    }
    op_lda(RD8(OBJ->anim_step[slot]));
    op_cmp(0x02);
    if (!flag_z()) {
        op_cmp(0x06);
        if (!flag_z()) {
            JustReturn0A9C();
            return;
        }
    }
    op_lda(0xA7);
    {
        uint8_t command;

        command = cpu.a;
        sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
    }
}

void abi_PlayListSoundThenSpawn0A9C(void)
{
    PlayListSoundThenSpawn0A9C(cpu.x);
}

/*
 * @routine 0A9C:23F StartRandomKind0A9C
 * @run 70:BA3F
 * WHAT IT DOES: Starts one of eight kinds, chosen at random, in slot Y (Stage 06's overlay).
 * HOW: NextRandom's low three bits index a table; then $B9EC.
 * WAITS: no.
 * Entry: X the object; Y the free slot. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    StartRandomKind0A9CKinds = 0xBA54,   /* eight kinds to choose from */
};

void StartRandomKind0A9C(void)
{
    jsr_push(0xBA42);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x07);
    op_phy();
    op_tay();
    op_lda(rd8((uint16_t)(StartRandomKind0A9CKinds + cpu.y)));
    op_ply();
    StartKindInYGuarded0A9C();
}

void abi_StartRandomKind0A9C(void)
{
    StartRandomKind0A9C();
}

/*
 * @routine 0A9C:26F BreakIntoArgKind0A9C
 * @run 70:BA6F
 * WHAT IT DOES: State 0 loads the record with hit box Y offset $D0; state 1 plays the list at $BAB4 and, once hit 5
 * times or more with a slot from $30 free, starts the kind in OBJ_START_ARG there (OBJ_INVULN_TIMER $10), shows
 * frame $0D, steps the state on, starts a debris burst and plays sound $19 (Stage 06's overlay).
 * HOW: A test of the state; StepObjectFrames; a compare; FindFreeSlotFrom30, StartObjectAtPlace, StartDebrisBurst.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void BreakIntoArgKind0A9C(void)
{
    op_lda(RD8(OBJ->state[cpu.x]));
    if (!flag_z()) {
        goto at_BA7D;
    }
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xBA77);
        last = LoadObjectRecord(cpu.x, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0xD0);
    WR8(OBJ->hitbox_dy[cpu.x], cpu.a);
at_BA7C:
    return;
at_BA7D:
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        goto at_BA7C;
    }
    op_lda(0xBA);
    WR8(ZP->temp11, cpu.a);
    op_lda(0xB4);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xBA8B);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    op_lda(RD8(OBJ->hit_taken[cpu.x]));
    op_cmp(0x05);
    if (!flag_c()) {
        goto at_BA7C;
    }
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xBA95);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (!flag_c()) {
        goto at_BA7C;
    }
    op_lda(RD8(OBJ->start_arg[cpu.x]));
    jsr_push(0xBA9D);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
    jsr_pop();
    op_lda(0x10);
    WR8(OBJ->invuln_timer[cpu.y], cpu.a);
    op_lda(0x0D);
    WR8(OBJ->frame[cpu.x], cpu.a);
    RMW8(OBJ->state[cpu.x], v, f_inc(v));
    op_lda(0x01);
    call_routine(0xBAAF, 0xA90B);
    op_lda(0x19);
    {
        uint8_t command;

        command = cpu.a;
        sound_command_exit_shim(command, SoundCommand(command, cpu.x, cpu.y));
    }
}

void abi_BreakIntoArgKind0A9C(void)
{
    BreakIntoArgKind0A9C();
}

/*
 * @routine 0A9C:2BF TurnOnFifthHit0A9C
 * @run 70:BABF
 * WHAT IT DOES: State 0 loads the record. State 1: with OBJ_START_ARG set and it and both neighbours facing left,
 * starts a kind $88 (slot from $30, OBJ_INVULN_TIMER $10) and puts all three in state 3; else on its fifth hit
 * turns round, waits $20 frames in state 2 with sound $3A, then goes back to state 1 (Stage 06's overlay).
 * HOW: Two DECs of the state; an AND of three facings; compares; FindFreeSlotFrom30.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void TurnOnFifthHit0A9C(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
        }
        return;
    }
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        goto at_BAF6;
    }
    op_lda(RD8(OBJ->start_arg[slot]));
    if (!flag_z()) {
        op_lda(RD8(OBJ->facing_left[slot]));
        op_and(RD8(OBJ->facing_left[slot + 1]));
        op_and(rd8((uint16_t)(COL(OBJ->facing_left) - 1 + slot)));
        if (!flag_z()) {
            goto at_BB02;
        }
    }
at_BADA:
    op_lda(RD8(OBJ->hit_taken[slot]));
    op_cmp(0x05);
    if (!flag_z()) {
        goto at_BB01;
    }
    op_lda(RD8(OBJ->facing_left[slot]));
    op_eor(0x01);
    WR8(OBJ->facing_left[slot], cpu.a);
    op_lda(0x20);
    WR8(OBJ->work_d[slot], cpu.a);
    RMW8(OBJ->state[slot], v, f_inc(v));
    op_lda(0x3A);
    {
        uint8_t command;

        command = cpu.a;
        sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
    }
    return;
at_BAF6:
    cpu.a = f_dec(cpu.a);
    if (flag_z()) {
        RMW8(OBJ->work_d[slot], v, f_dec(v));
        if (flag_z()) {
            RMW8(OBJ->state[slot], v, f_dec(v));
        }
    }
at_BB01:
    return;
at_BB02:
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xBB05);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (!flag_c()) {
        goto at_BADA;
    }
    op_lda(0x88);
    jsr_push(0xBB0C);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
    jsr_pop();
    op_lda(0x10);
    WR8(OBJ->invuln_timer[cpu.y], cpu.a);
    op_lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->state[slot + 1], cpu.a);
    wr8((uint16_t)(COL(OBJ->state) - 1 + slot), cpu.a);
}

void abi_TurnOnFifthHit0A9C(void)
{
    TurnOnFifthHit0A9C(cpu.x);
}

/*
 * @routine 0A9C:45A RandomMinusOneToOne0A9C
 * @run 70:BC5A
 * WHAT IT DOES: Gives a random step of -1, 0 or 1 (Stage 06's overlay).
 * HOW: NextRandom's bit 0, plus 1 on odd frames of zWaveCounter, less 1.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A $FF, 0 or 1; N, Z and C from the SBC.
 */
void RandomMinusOneToOne0A9C(void)
{
    jsr_push(0xBC5D);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x01);
    if (!((RD8(ZP->wave_counter) & 0x01) == 0)) {
        cpu.a = f_inc(cpu.a);
    }
    op_sec();
    op_sbc(0x01);
}

void abi_RandomMinusOneToOne0A9C(void)
{
    RandomMinusOneToOne0A9C();
}

/*
 * @routine 0A9C:467 SoundOnce0A9C
 * @run 70:BC67
 * WHAT IT DOES: The first time only (OBJ_WORK_F 0), sets OBJ_WORK_F and plays sound $AB (Stage 06's overlay).
 * HOW: A test, INC, SoundCommand.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void SoundOnce0A9C(uint8_t slot)
{
    op_lda(RD8(OBJ->work_f[slot]));
    if (flag_z()) {
        RMW8(OBJ->work_f[slot], v, f_inc(v));
        op_lda(0xAB);
        {
            uint8_t command;

            jsr_push(0xBC74);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
            jsr_pop();
        }
    }
}

void abi_SoundOnce0A9C(void)
{
    SoundOnce0A9C(cpu.x);
}

/*
 * @routine 0A9C:522 RandomMinusTwoToTwo0A9C
 * @run 70:BD22
 * WHAT IT DOES: Gives a random step of -2 to 2 (Stage 06's overlay).
 * HOW: NextRandom's low two bits, plus 1 on odd frames of zWaveCounter, less 2.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A $FE to 2; N, Z and C from the SBC.
 */
void RandomMinusTwoToTwo0A9C(void)
{
    jsr_push(0xBD25);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x03);
    if (!((RD8(ZP->wave_counter) & 0x01) == 0)) {
        cpu.a = f_inc(cpu.a);
    }
    op_sec();
    op_sbc(0x02);
}

void abi_RandomMinusTwoToTwo0A9C(void)
{
    RandomMinusTwoToTwo0A9C();
}

/*
 * @routine 0A9D:62E EnterFromScreenEdge0A9D
 * @run 71:C62E
 * WHAT IT DOES: State 0 loads the record and puts the object $0C below the player at a screen edge: at X -$10
 * moving right at $0120 (facing left set) when the player is at X $C0 or more, or from $40 up facing left; else at
 * X $0110 moving left at $0120, one time in eight with OBJ_WORK_C $21, OBJ_WORK_F 1 and sprite attribute $8D. Later
 * states go on at $C69F (Stage 06's overlay).
 * HOW: Compares of the player's X and facing; stores; PlayerDistanceSum's low three bits.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void EnterFromScreenEdge0A9D(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (!flag_z()) {
        BobAlongCosine0A9D(slot);
        return;
    }
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xC639);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(RD8(OBJ->y_lo[0x00]));
    op_clc();
    op_adc(0x0C);
    WR8(OBJ->y_lo[slot], cpu.a);
    op_lda(RD8(OBJ->y_hi[0x00]));
    op_adc(0x00);
    WR8(OBJ->y_hi[slot], cpu.a);
    op_lda(RD8(OBJ->x_lo[0x00]));
    op_cmp(0x40);
    if (flag_c()) {
        op_cmp(0xC0);
        if (!flag_c()) {
            op_lda(RD8(OBJ->facing_left[0x00]));
            if (flag_z()) {
                goto at_C674;
            }
        }
        op_lda(0x01);
        WR8(OBJ->facing_left[slot], cpu.a);
        op_lda(0xFF);
        WR8(OBJ->x_hi[slot], cpu.a);
        op_lda(0xF0);
        WR8(OBJ->x_lo[slot], cpu.a);
        op_lda(0x01);
        WR8(OBJ->vx_hi[slot], cpu.a);
        op_lda(0x20);
        WR8(OBJ->vx_lo[slot], cpu.a);
        return;
    }
at_C674:
    op_lda(0x01);
    WR8(OBJ->x_hi[slot], cpu.a);
    op_lda(0x10);
    WR8(OBJ->x_lo[slot], cpu.a);
    op_lda(0xFE);
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(0xE0);
    WR8(OBJ->vx_lo[slot], cpu.a);
    jsr_push(0xC68B);
    distance_sum_exit_shim(PlayerDistanceSum(slot, (cpu.p & P_C) != 0));
    jsr_pop();
    op_and(0x07);
    if (flag_z()) {
        op_lda(0x21);
        WR8(OBJ->work_c[slot], cpu.a);
        op_lda(0x01);
        WR8(OBJ->work_f[slot], cpu.a);
        op_lda(0x8D);
        WR8(OBJ->sprite_attr[slot], cpu.a);
    }
}

void abi_EnterFromScreenEdge0A9D(void)
{
    EnterFromScreenEdge0A9D(cpu.x);
}

/*
 * @routine 0A9D:69F BobAlongCosine0A9D
 * @run 71:C69F
 * WHAT IT DOES: Plays the list at $C6EF (two frames on while rising), moves across by its X speed, and takes its Y
 * speed from the cosine of OBJ_WORK_D at an amplitude of $0240, stepping OBJ_WORK_D:E on by $0280 a frame (Stage
 * 06's overlay).
 * HOW: StepObjectFrames, ApplyVelocityX, ScaleByCosine, ApplyVelocityY, a 16-bit add.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void BobAlongCosine0A9D(uint8_t slot)
{
    op_lda(0xC6);
    WR8(ZP->temp11, cpu.a);
    op_lda(0xEF);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC6AA);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(RD8(OBJ->vy_hi[slot]));
        if (flag_n()) {
            op_lda(RD8(OBJ->frame[slot]));
            op_clc();
            op_adc(0x02);
            WR8(OBJ->frame[slot], cpu.a);
        }
    }
    jsr_push(0xC6BD);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
    op_lda(0x40);
    WR8(ZP->temp00, cpu.a);
    op_lda(0x02);
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    jsr_push(0xC6CB);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    op_lda(RD8(ZP->product.hi));
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_lda(RD8(ZP->product.mid));
    WR8(OBJ->vy_lo[slot], cpu.a);
    jsr_push(0xC6D8);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
    op_lda(RD8(OBJ->work_f[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(RD8(OBJ->work_e[slot]));
    op_clc();
    op_adc(0x80);
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(RD8(OBJ->work_d[slot]));
    op_adc(0x02);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_BobAlongCosine0A9D(void)
{
    BobAlongCosine0A9D(cpu.x);
}

/*
 * @routine 0A9D:716 FlipThenRunState0A9D
 * @run 71:C716
 * WHAT IT DOES: Sets the sprite flip by place, then runs the routine of the object's state (Stage 06's overlay).
 * HOW: FlipBelowRow50AtRight0A9D; a jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FlipThenRunState0A9DStates = 0xC72B,   /* the routine address per state */
};

void FlipThenRunState0A9D(void)
{
    call_routine(0xC719, 0xC737);
    op_lda(RD8(OBJ->state[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(FlipThenRunState0A9DStates + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(FlipThenRunState0A9DStates + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_FlipThenRunState0A9D(void)
{
    FlipThenRunState0A9D();
}

/*
 * @routine 0A9D:76D LoadThenStateByArg0A9D
 * @run 71:C76D
 * WHAT IT DOES: Loads the record; with OBJ_START_ARG 1 faces left and goes to state 2 with speeds $0140 across and
 * $00A0 down; with any other nonzero OBJ_START_ARG goes to state 5 (Stage 06's overlay).
 * HOW: Two compares, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void LoadThenStateByArg0A9D(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xC770);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(RD8(OBJ->start_arg[slot]));
    if (!flag_z()) {
        op_cmp(0x01);
        if (flag_z()) {
            goto at_C77F;
        }
        op_lda(0x05);
        WR8(OBJ->state[slot], cpu.a);
    }
    return;
at_C77F:
    op_lda(0x01);
    WR8(OBJ->facing_left[slot], cpu.a);
    op_lda(0x01);
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(0x40);
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(0x00);
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_lda(0xA0);
    WR8(OBJ->vy_lo[slot], cpu.a);
    op_lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_LoadThenStateByArg0A9D(void)
{
    LoadThenStateByArg0A9D(cpu.x);
}

/*
 * @routine 0A9D:79E PlayFaceUntilNear0A9D
 * @run 71:C79E
 * WHAT IT DOES: Plays the list at $C7BE, faces the player, and goes to state 3 when the player is less than $60
 * away across (Stage 06's overlay).
 * HOW: StepObjectFrames, SideOfPlayer, DistanceXToPlayer and a compare.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void PlayFaceUntilNear0A9D(uint8_t slot)
{
    op_lda(0xC7);
    WR8(ZP->temp11, cpu.a);
    op_lda(0xBE);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xC7A9);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    jsr_push(0xC7AC);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    op_and(0x01);
    WR8(OBJ->facing_left[slot], cpu.a);
    jsr_push(0xC7B4);
    distance_exit_shim(DistanceXToPlayer(slot));
    jsr_pop();
    op_cmp(0x60);
    if (!flag_c()) {
        op_lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
    }
}

void abi_PlayFaceUntilNear0A9D(void)
{
    PlayFaceUntilNear0A9D(cpu.x);
}

/*
 * @routine 0A9D:7C3 MoveUntilFarAndPast0A9D
 * @run 71:C7C3
 * WHAT IT DOES: Moves; when the player is $4C or more away across and the object's stage X has high byte $0A or
 * more and low byte $7C or more, steps the state on (Stage 06's overlay).
 * HOW: ApplyVelocity, DistanceXToPlayer, the scroll added to X, two compares.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void MoveUntilFarAndPast0A9D(uint8_t slot)
{
    jsr_push(0xC7C6);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    jsr_push(0xC7C9);
    distance_exit_shim(DistanceXToPlayer(slot));
    jsr_pop();
    op_cmp(0x4C);
    if (flag_c()) {
        op_lda(RD8(OBJ->x_lo[slot]));
        op_clc();
        op_adc(RD8(SCROLL->x_lo[0x00]));
        WR8(ZP->temp00, cpu.a);
        op_lda(RD8(OBJ->x_hi[slot]));
        op_adc(RD8(SCROLLE->x_hi_screen[0x00]));
        op_cmp(0x0A);
        if (!flag_c()) {
            goto at_C7E9;
        }
        op_lda(RD8(ZP->temp00));
        op_cmp(0x7C);
        if (!flag_c()) {
            goto at_C7E9;
        }
    }
    RMW8(OBJ->state[slot], v, f_inc(v));
at_C7E9:
    ;
}

void abi_MoveUntilFarAndPast0A9D(void)
{
    MoveUntilFarAndPast0A9D(cpu.x);
}

/*
 * @routine 0A9D:7FE ChooseByPlayerDistance0A9D
 * @run 71:C7FE
 * WHAT IT DOES: Hit 1 to 4 times, sets OBJ_WORK_D 2; otherwise runs the choice made for the player's distance
 * across, in bands of 16 pixels (Stage 06's overlay).
 * HOW: The distance's top four bits index a choice; twice the choice indexes a jump table.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    ChooseByPlayerDistance0A9DChoices = 0xC824,   /* the choice per band of 16 pixels */
    ChooseByPlayerDistance0A9DRoutines = 0xC834,   /* the routine address per choice */
};

void ChooseByPlayerDistance0A9D(uint8_t slot)
{
    op_lda(RD8(OBJ->hit_taken[slot]));
    if (!flag_z()) {
        op_cmp(0x05);
        if (!flag_c()) {
            SetWorkD2_0A9E(slot);
            return;
        }
    }
    jsr_push(0xC80D);
    distance_exit_shim(DistanceXToPlayer(slot));
    jsr_pop();
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ChooseByPlayerDistance0A9DChoices + cpu.y)));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(ChooseByPlayerDistance0A9DRoutines + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(ChooseByPlayerDistance0A9DRoutines + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_ChooseByPlayerDistance0A9D(void)
{
    ChooseByPlayerDistance0A9D(cpu.x);
}

/*
 * @routine 0A9E:03A PickWorkDByChance0A9E
 * @run 71:C83A
 * WHAT IT DOES: Sets OBJ_WORK_D at random: 3 for two chances in eight, 2 for one, 1 for one, 0 for the other four
 * (Stage 06's overlay).
 * HOW: PlayerDistanceSum's low three bits and compares.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void PickWorkDByChance0A9E(void)
{
    jsr_push(0xC83D);
    distance_sum_exit_shim(PlayerDistanceSum(cpu.x, (cpu.p & P_C) != 0));
    jsr_pop();
    op_and(0x07);
    op_cmp(0x02);
    if (!flag_c()) {
        SetWorkD3_0A9E(cpu.x);
        return;
    }
    op_cmp(0x02);
    if (flag_z()) {
        SetWorkD2_0A9E(cpu.x);
        return;
    }
    op_cmp(0x03);
    if (flag_z()) {
        SetWorkD1_0A9E(cpu.x);
        return;
    }
    /* falls into $C84B */
    ClearWorkD0A9E(cpu.x);
}

void abi_PickWorkDByChance0A9E(void)
{
    PickWorkDByChance0A9E();
}

/*
 * @routine 0A9E:04B ClearWorkD0A9E
 * @run 71:C84B
 * WHAT IT DOES: Sets OBJ_WORK_D to 0 (Stage 06's overlay).
 * HOW: One store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ClearWorkD0A9E(uint8_t slot)
{
    WR8(OBJ->work_d[slot], 0);
}

void abi_ClearWorkD0A9E(void)
{
    ClearWorkD0A9E(cpu.x);
}

/*
 * @routine 0A9E:04F PickWorkDByChanceAndFacing0A9E
 * @run 71:C84F
 * WHAT IT DOES: Sets OBJ_WORK_D at random: 3, 2 or 0 for one chance in eight each; otherwise 1 when the object
 * faces the way the player faces, 0 when not (Stage 06's overlay).
 * HOW: PlayerDistanceSum's low three bits, compares, an EOR of the facings.
 * WAITS: no.
 * Entry: X the object; A as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
void PickWorkDByChanceAndFacing0A9E(uint8_t slot)
{
    jsr_push(0xC852);
    distance_sum_exit_shim(PlayerDistanceSum(slot, (cpu.p & P_C) != 0));
    jsr_pop();
    op_and(0x07);
    if (flag_z()) {
        SetWorkD3_0A9E(slot);
        return;
    }
    op_cmp(0x01);
    if (flag_z()) {
        SetWorkD2_0A9E(slot);
        return;
    }
    op_cmp(0x02);
    if (flag_z()) {
        ClearWorkD0A9E(slot);
        return;
    }
    op_lda(RD8(OBJ->facing_left[slot]));
    op_eor(RD8(OBJ->facing_left[0x00]));
    if (!flag_z()) {
        ClearWorkD0A9E(slot);
        return;
    }
    /* falls into $C866 */
    SetWorkD1_0A9E(slot);
}

void abi_PickWorkDByChanceAndFacing0A9E(void)
{
    PickWorkDByChanceAndFacing0A9E(cpu.x);
}

/*
 * @routine 0A9E:066 SetWorkD1_0A9E
 * @run 71:C866
 * WHAT IT DOES: Sets OBJ_WORK_D to 1 (Stage 06's overlay).
 * HOW: One store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SetWorkD1_0A9E(uint8_t slot)
{
    op_lda(0x01);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_SetWorkD1_0A9E(void)
{
    SetWorkD1_0A9E(cpu.x);
}

/*
 * @routine 0A9E:06C SetWorkD2_0A9E
 * @run 71:C86C
 * WHAT IT DOES: Sets OBJ_WORK_D to 2 (Stage 06's overlay).
 * HOW: One store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SetWorkD2_0A9E(uint8_t slot)
{
    op_lda(0x02);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_SetWorkD2_0A9E(void)
{
    SetWorkD2_0A9E(cpu.x);
}

/*
 * @routine 0A9E:072 SetWorkD3_0A9E
 * @run 71:C872
 * WHAT IT DOES: Sets OBJ_WORK_D to 3 (Stage 06's overlay).
 * HOW: One store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void SetWorkD3_0A9E(uint8_t slot)
{
    op_lda(0x03);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_SetWorkD3_0A9E(void)
{
    SetWorkD3_0A9E(cpu.x);
}

/*
 * @routine 0A9E:0D5 FallFrameThenLand0A9E
 * @run 71:C8D5
 * WHAT IT DOES: Shows the rising or falling frame by the sign of its Y speed, falls with height 6, and on landing
 * goes to state 3, substate 0 (Stage 06's overlay).
 * HOW: Bit 7 of the Y speed indexes a table; StopOnLanding.
 * WAITS: no.
 * Entry: X the object; C as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    FallFrameThenLand0A9EFrames = 0xC8F3,   /* the frame while rising, then while falling */
};

void FallFrameThenLand0A9E(uint8_t slot)
{
    op_lda(RD8(OBJ->vy_hi[slot]));
    cpu.a = f_rol(cpu.a);
    cpu.a = f_rol(cpu.a);
    op_and(0x01);
    op_tay();
    op_lda(rd8((uint16_t)(FallFrameThenLand0A9EFrames + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
    op_lda(0x06);
    jsr_push(0xC8E8);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        op_lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
    }
}

void abi_FallFrameThenLand0A9E(void)
{
    FallFrameThenLand0A9E(cpu.x);
}

/*
 * @routine 0A9E:0F5 HitCheckThenRunState0A9E
 * @run 71:C8F5
 * WHAT IT DOES: Goes to state 6 when hit, then runs the routine of the object's state (Stage 06's overlay).
 * HOW: The call at $C918; a jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    HitCheckThenRunState0A9EStates = 0xC90A,   /* the routine address per state */
};

void HitCheckThenRunState0A9E(uint8_t slot)
{
    jsr_push(0xC8F8);
    State6WhenHitBit7_0A9E(slot);
    jsr_pop();
    op_lda(RD8(OBJ->state[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(HitCheckThenRunState0A9EStates + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(HitCheckThenRunState0A9EStates + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_HitCheckThenRunState0A9E(void)
{
    HitCheckThenRunState0A9E(cpu.x);
}

/*
 * @routine 0A9E:118 State6WhenHitBit7_0A9E
 * @run 71:C918
 * WHAT IT DOES: Puts the object in state 6 when bit 7 of OBJ_HIT_STATUS is set (Stage 06's overlay).
 * HOW: An AND and a store.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void State6WhenHitBit7_0A9E(uint8_t slot)
{
    op_lda(RD8(OBJ->hit_status[slot]));
    op_and(0x80);
    if (!flag_z()) {
        op_lda(0x06);
        WR8(OBJ->state[slot], cpu.a);
    }
}

void abi_State6WhenHitBit7_0A9E(void)
{
    State6WhenHitBit7_0A9E(cpu.x);
}

/*
 * @routine 0A9E:134 LandThenStateOn0A9E
 * @run 71:C934
 * WHAT IT DOES: Falls with height 8 and on landing steps the state on (Stage 06's overlay).
 * HOW: StopOnLanding, INC.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
void LandThenStateOn0A9E(uint8_t slot)
{
    op_lda(0x08);
    jsr_push(0xC939);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_LandThenStateOn0A9E(void)
{
    LandThenStateOn0A9E(cpu.x);
}

/*
 * @routine 0A9E:16E FacePlayerPlayList0A9E
 * @run 71:C96E
 * WHAT IT DOES: Faces the player and plays the frame list that Y chooses (Stage 06's overlay).
 * HOW: SideOfPlayer; Y indexes a word table of lists; StepObjectFrames.
 * WAITS: no.
 * Entry: X the object; Y twice the list's number. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    FacePlayerPlayList0A9ELists = 0xC983,   /* the frame list addresses */
};

void FacePlayerPlayList0A9E(uint8_t slot)
{
    jsr_push(0xC971);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    op_and(0x01);
    WR8(OBJ->facing_left[slot], cpu.a);
    op_lda(rd8((uint16_t)(FacePlayerPlayList0A9ELists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(FacePlayerPlayList0A9ELists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
    }
}

void abi_FacePlayerPlayList0A9E(void)
{
    FacePlayerPlayList0A9E(cpu.x);
}

/*
 * @routine 0A9E:2F9 LandThenSubstateOn0A9E
 * @run 71:CAF9
 * WHAT IT DOES: Falls with height 8 and on landing steps the substate on (Stage 06's overlay).
 * HOW: StopOnLanding, INC.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
void LandThenSubstateOn0A9E(uint8_t slot)
{
    op_lda(0x08);
    jsr_push(0xCAFE);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_LandThenSubstateOn0A9E(void)
{
    LandThenSubstateOn0A9E(cpu.x);
}

/*
 * @routine 0A9E:31A BurstIntoSixKind2E0A9E
 * @run 71:CB1A
 * WHAT IT DOES: Starts up to six kind $2E objects (slots from $30) at the object's place with its facing, each with
 * OBJ_WORK_D, OBJ_WORK_E and X and Y speeds from its record, then removes the object (Stage 06's overlay).
 * HOW: A loop of six with eight times the count indexing the records; ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    BurstIntoSixKind2E0A9EParts = 0xCB65,   /* OBJ_WORK_D, OBJ_WORK_E, X speed high, low, Y speed high, low; eight bytes apart */
};

void BurstIntoSixKind2E0A9E(void)
{
    op_cly();
at_CB1B:
    WR8(ZP->hit_slot, cpu.y);
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xCB20);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0x2E);
        jsr_push(0xCB27);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
        jsr_pop();
        op_lda(RD8(OBJ->facing_left[cpu.x]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        WR8(ZP->richter_object, cpu.x);
        op_lda(RD8(ZP->hit_slot));
        cpu.a = f_asl(cpu.a);
        cpu.a = f_asl(cpu.a);
        cpu.a = f_asl(cpu.a);
        op_tax();
        op_lda(rd8((uint16_t)(BurstIntoSixKind2E0A9EParts + cpu.x)));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(BurstIntoSixKind2E0A9EParts + 1 + cpu.x)));
        WR8(OBJ->work_e[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(BurstIntoSixKind2E0A9EParts + 2 + cpu.x)));
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(BurstIntoSixKind2E0A9EParts + 3 + cpu.x)));
        WR8(OBJ->vx_lo[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(BurstIntoSixKind2E0A9EParts + 4 + cpu.x)));
        WR8(OBJ->vy_hi[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(BurstIntoSixKind2E0A9EParts + 5 + cpu.x)));
        WR8(OBJ->vy_lo[cpu.y], cpu.a);
        op_ldx(RD8(ZP->richter_object));
        op_ldy(RD8(ZP->hit_slot));
        op_iny();
        op_cpy(0x06);
        if (!flag_c()) {
            goto at_CB1B;
        }
    }
    ClearObject(cpu.x);
}

void abi_BurstIntoSixKind2E0A9E(void)
{
    BurstIntoSixKind2E0A9E();
}

/*
 * @routine 0A9E:395 LeapTowardPlayer0A9E
 * @run 71:CB95
 * @stop CBD0
 * WHAT IT DOES: State 0 loads the record, lifts the object 16 pixels, and gives it a speed of $FB80 up and an X
 * speed toward the player that grows with the distance; later states go on at $CBE0 (Stage 06's overlay).
 * HOW: ApplyVelocityY; the distance over 32 indexes a word table; NegateVelocityX by SideOfPlayer.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    LeapTowardPlayer0A9ESpeeds = 0xCBD0,   /* the X speed per 32 pixels of distance, a word each */
};

void LeapTowardPlayer0A9E(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (!flag_z()) {
        FlyListWithGravity0A9E(slot);
        return;
    }
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xCB9D);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0xF0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    jsr_push(0xCBA5);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
    jsr_push(0xCBA8);
    distance_exit_shim(DistanceXToPlayer(slot));
    jsr_pop();
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(LeapTowardPlayer0A9ESpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(LeapTowardPlayer0A9ESpeeds + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(0xFB);
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_lda(0x80);
    WR8(OBJ->vy_lo[slot], cpu.a);
    jsr_push(0xCBC8);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    op_and(0x01);
    if (!flag_z()) {
        jsr_push(0xCBCF);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
    }
}

void abi_LeapTowardPlayer0A9E(void)
{
    LeapTowardPlayer0A9E(cpu.x);
}

/*
 * @routine 0A9E:3E0 FlyListWithGravity0A9E
 * @run 71:CBE0
 * WHAT IT DOES: Plays the list at $C9B9, adds $24 to the Y speed and moves (Stage 06's overlay).
 * HOW: StepObjectFrames, a 16-bit add, ApplyVelocity.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void FlyListWithGravity0A9E(uint8_t slot)
{
    op_lda(0xC9);
    WR8(ZP->temp11, cpu.a);
    op_lda(0xB9);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xCBEB);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    op_lda(RD8(OBJ->vy_lo[slot]));
    op_clc();
    op_adc(0x24);
    WR8(OBJ->vy_lo[slot], cpu.a);
    op_lda(RD8(OBJ->vy_hi[slot]));
    op_adc(0x00);
    WR8(OBJ->vy_hi[slot], cpu.a);
    axis_move_exit_shim(ApplyVelocity(slot));
}

void abi_FlyListWithGravity0A9E(void)
{
    FlyListWithGravity0A9E(cpu.x);
}

/*
 * @routine 0A9E:3FF FlyThenBecomeKind9B0A9E
 * @run 71:CBFF
 * WHAT IT DOES: State 0 loads the record and turns its X speed toward the player; later, while OBJ_WORK_E counts
 * down, plays the list OBJ_WORK_D chooses and moves with gravity; on zero becomes kind $9B in state 0 (Stage 06's
 * overlay).
 * HOW: DEC; twice OBJ_WORK_D indexes a word table; ApplyGravityUnlimited, ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FlyThenBecomeKind9B0A9ELists = 0xCC38,   /* the frame list address per OBJ_WORK_D */
};

void FlyThenBecomeKind9B0A9E(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xCC07);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        jsr_push(0xCC0A);
        side_of_player_exit_shim(SideOfPlayer(slot));
        jsr_pop();
        op_and(0x01);
        if (!flag_z()) {
            jsr_push(0xCC11);
            negate_exit_shim(NegateVelocityX(slot));
            jsr_pop();
        }
        return;
    }
    RMW8(OBJ->work_e[slot], v, f_dec(v));
    if (flag_z()) {
        op_lda(0x9B);
        WR8(OBJ->kind[slot], cpu.a);
        WR8(OBJ->state[slot], 0);
        return;
    }
    op_lda(RD8(OBJ->work_d[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(FlyThenBecomeKind9B0A9ELists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(FlyThenBecomeKind9B0A9ELists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xCC32);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    jsr_push(0xCC35);
    gravity_exit_shim(ApplyGravityUnlimited(slot));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocity(slot));
}

void abi_FlyThenBecomeKind9B0A9E(void)
{
    FlyThenBecomeKind9B0A9E(cpu.x);
}

/*
 * @routine 0A9E:440 RunStateUnlessSlowed0A9E
 * @run 71:CC40
 * WHAT IT DOES: Lets the player touch the object and starts its hit state; unless time is slowed this frame, hops,
 * sets its hit box and runs the routine of its state (Stage 06's overlay).
 * HOW: The calls at $CC6E, $CC79 and $CC93; HopAndSettle, SetHitboxByFrame; a jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RunStateUnlessSlowed0A9EStates = 0xCC64,   /* the routine address per state */
};

void RunStateUnlessSlowed0A9E(void)
{
    jsr_push(0xCC43);
    ContactBox14By20_0A9E();
    jsr_pop();
    jsr_push(0xCC46);
    State4WhenHitBit7_0A9E(cpu.x);
    jsr_pop();
    jsr_push(0xCC49);
    SkipFrameWhenSlowed0A9E(cpu.x);
    jsr_pop();
    if (flag_c()) {
        return;
    }
    call_routine(0xCC4F, 0xCCC3);
    call_routine(0xCC52, 0xCD1B);
    op_lda(RD8(OBJ->state[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RunStateUnlessSlowed0A9EStates + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(RunStateUnlessSlowed0A9EStates + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_RunStateUnlessSlowed0A9E(void)
{
    RunStateUnlessSlowed0A9E();
}

/*
 * @routine 0A9E:46E ContactBox14By20_0A9E
 * @run 71:CC6E
 * WHAT IT DOES: Tests the object's touch with the player with a $14 by $20 box (Stage 06's overlay).
 * HOW: Two stores, then ContactWithPlayer.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void ContactBox14By20_0A9E(void)
{
    op_lda(0x14);
    WR8(ZP->temp08, cpu.a);
    op_lda(0x20);
    WR8(ZP->temp09, cpu.a);
    run_routine(0xAA35);
}

void abi_ContactBox14By20_0A9E(void)
{
    ContactBox14By20_0A9E();
}

/*
 * @routine 0A9E:479 State4WhenHitBit7_0A9E
 * @run 71:CC79
 * WHAT IT DOES: Outside state 4, when bit 7 of OBJ_HIT_STATUS is set, restarts the animation in state 4 (Stage 06's
 * overlay).
 * HOW: A compare, an AND, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void State4WhenHitBit7_0A9E(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    op_cmp(0x04);
    if (!flag_z()) {
        op_lda(RD8(OBJ->hit_status[slot]));
        op_and(0x80);
        if (!flag_z()) {
            WR8(OBJ->anim_step[slot], 0);
            WR8(OBJ->anim_timer[slot], 0);
            op_lda(0x04);
            WR8(OBJ->state[slot], cpu.a);
        }
    }
}

void abi_State4WhenHitBit7_0A9E(void)
{
    State4WhenHitBit7_0A9E(cpu.x);
}

/*
 * @routine 0A9E:493 SkipFrameWhenSlowed0A9E
 * @run 71:CC93
 * WHAT IT DOES: In states other than 0 and 4, while time is slowed (TimeSlowMode not 0), says whether to skip this
 * frame by the pattern of the mode (mode 2 has its own) (Stage 06's overlay).
 * HOW: zWaveCounter's low three bits plus $18 index a pattern table.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: C set to skip the frame; C clear to run it.
 */
enum {
    SkipFrameWhenSlowed0A9EPattern = 0xA146,   /* nonzero for the frames skipped, at +$18 by zWaveCounter */
    SkipFrameWhenSlowed0A9EPattern2 = 0xA166,   /* for mode 2: zero for the frames run, at +$18 by zWaveCounter */
};

void SkipFrameWhenSlowed0A9E(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (!flag_z()) {
        op_cmp(0x04);
        if (!flag_z()) {
            op_lda(RD8(PVARS->time_slow_mode));
            if (!flag_z()) {
                op_cmp(0x02);
                if (!flag_z()) {
                    goto at_CCB4;
                }
                op_lda(RD8(ZP->wave_counter));
                op_and(0x07);
                op_clc();
                op_adc(0x18);
                op_tay();
                op_lda(rd8((uint16_t)(SkipFrameWhenSlowed0A9EPattern2 + cpu.y)));
                if (flag_z()) {
                    goto at_CCC1;
                }
at_CCB2:
                op_sec();
                return;
at_CCB4:
                op_lda(RD8(ZP->wave_counter));
                op_and(0x07);
                op_clc();
                op_adc(0x18);
                op_tay();
                op_lda(rd8((uint16_t)(SkipFrameWhenSlowed0A9EPattern + cpu.y)));
                if (!flag_z()) {
                    goto at_CCB2;
                }
            }
        }
    }
at_CCC1:
    op_clc();
}

void abi_SkipFrameWhenSlowed0A9E(void)
{
    SkipFrameWhenSlowed0A9E(cpu.x);
}

/*
 * @routine 0A9E:64F PlayAlternateListThenState1_0A9E
 * @run 71:CE4F
 * WHAT IT DOES: Plays one of two lists by bit 0 of OBJ_WORK_H; when it ends, steps OBJ_WORK_H on and goes to state
 * 1 (Stage 06's overlay).
 * HOW: Bit 0 of OBJ_WORK_H indexes a word table; StepObjectFrames.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    PlayAlternateListThenState1_0A9ELists = 0xCE6E,   /* two frame list addresses */
};

void PlayAlternateListThenState1_0A9E(uint8_t slot)
{
    op_lda(RD8(OBJ->work_h[slot]));
    op_and(0x01);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(PlayAlternateListThenState1_0A9ELists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(PlayAlternateListThenState1_0A9ELists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xCE63);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        RMW8(OBJ->work_h[slot], v, f_inc(v));
        op_lda(0x01);
        WR8(OBJ->state[slot], cpu.a);
    }
}

void abi_PlayAlternateListThenState1_0A9E(void)
{
    PlayAlternateListThenState1_0A9E(cpu.x);
}

/*
 * @routine 0A9E:69E PickMoveAtRandom0A9E
 * @run 71:CE9E
 * WHAT IT DOES: Goes back to state 1 when the player is on the side away from its facing (as OBJ_WORK_H's bit 0
 * turns it); else picks OBJ_WORK_I and then OBJ_WORK_K at random, takes OBJ_WORK_J by bit 0 of OBJ_WORK_H and steps
 * the substate on (Stage 06's overlay).
 * HOW: SideOfPlayer and a compare; NextRandom's low four and two bits index tables.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    PickMoveAtRandom0A9EMoves = 0xCEE6,   /* sixteen values of OBJ_WORK_I */
    PickMoveAtRandom0A9EWorkK = 0xCEF6,   /* four values of OBJ_WORK_K per OBJ_WORK_I */
    PickMoveAtRandom0A9EWorkJ = 0xCF02,   /* OBJ_WORK_J by bit 0 of OBJ_WORK_H */
};

void PickMoveAtRandom0A9E(uint8_t slot)
{
    op_lda(RD8(OBJ->facing_left[slot]));
    op_eor(RD8(OBJ->work_h[slot]));
    op_and(0x01);
    WR8(ZP->temp08, cpu.a);
    jsr_push(0xCEAB);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    op_and(0x01);
    op_cmp(RD8(ZP->temp08));
    if (!flag_z()) {
        op_lda(0x01);
        WR8(OBJ->state[slot], cpu.a);
        return;
    }
    jsr_push(0xCEBA);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x0F);
    op_tay();
    op_lda(rd8((uint16_t)(PickMoveAtRandom0A9EMoves + cpu.y)));
    WR8(OBJ->work_i[slot], cpu.a);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    WR8(ZP->temp00, cpu.a);
    jsr_push(0xCECA);
    random_exit_shim(NextRandom());
    jsr_pop();
    op_and(0x03);
    op_clc();
    op_adc(RD8(ZP->temp00));
    op_tay();
    op_lda(rd8((uint16_t)(PickMoveAtRandom0A9EWorkK + cpu.y)));
    WR8(OBJ->work_k[slot], cpu.a);
    op_lda(RD8(OBJ->work_h[slot]));
    op_and(0x01);
    op_tay();
    op_lda(rd8((uint16_t)(PickMoveAtRandom0A9EWorkJ + cpu.y)));
    WR8(OBJ->work_j[slot], cpu.a);
    RMW8(OBJ->substate[slot], v, f_inc(v));
}

void abi_PickMoveAtRandom0A9E(void)
{
    PickMoveAtRandom0A9E(cpu.x);
}

/*
 * @routine 0A9E:704 ThrowKindByWorkI0A9E
 * @run 71:CF04
 * WHAT IT DOES: With a slot from $30, starts the kind OBJ_WORK_I chooses there with OBJ_WORK_D its OBJ_WORK_I, the
 * object's facing and slot (OBJ_WORK_F), and an X speed of $F8, or $EE and the facing reversed when OBJ_WORK_J is
 * set; steps the substate on. With none, stands in state 1 (Stage 06's overlay).
 * HOW: FindFreeSlotFrom30, a table read, StartObjectAtPlace, stores; else $CF96.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    ThrowKindByWorkI0A9EKinds = 0xCF46,   /* the kind per OBJ_WORK_I */
};

void ThrowKindByWorkI0A9E(uint8_t slot)
{
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xCF07);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (!flag_c()) {
        StandInState1_0A9E(slot);
        return;
    }
    WR8(ZP->hit_slot, cpu.y);
    op_ldy(RD8(OBJ->work_i[slot]));
    op_lda(rd8((uint16_t)(ThrowKindByWorkI0A9EKinds + cpu.y)));
    op_ldy(RD8(ZP->hit_slot));
    jsr_push(0xCF19);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
    jsr_pop();
    op_lda(RD8(OBJ->work_i[slot]));
    WR8(OBJ->work_d[cpu.y], cpu.a);
    op_lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    op_txa();
    WR8(OBJ->work_f[cpu.y], cpu.a);
    op_lda(RD8(OBJ->work_j[slot]));
    if (flag_z()) {
        op_lda(0xF8);
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
    } else {
        op_lda(0xEE);
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        op_lda(RD8(OBJ->facing_left[cpu.y]));
        op_eor(0x01);
        WR8(OBJ->facing_left[cpu.y], cpu.a);
    }
    RMW8(OBJ->substate[slot], v, f_inc(v));
}

void abi_ThrowKindByWorkI0A9E(void)
{
    ThrowKindByWorkI0A9E(cpu.x);
}

/*
 * @routine 0A9E:749 PlayListByWorkIJ0A9E
 * @run 71:CF49
 * WHAT IT DOES: Plays the list OBJ_WORK_I and OBJ_WORK_J choose; when it ends, steps the substate on (Stage 06's
 * overlay).
 * HOW: Twice OBJ_WORK_I plus OBJ_WORK_J indexes a word table; StepObjectFrames.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    PlayListByWorkIJ0A9ELists = 0xCF65,   /* the frame list addresses */
};

void PlayListByWorkIJ0A9E(uint8_t slot)
{
    op_lda(RD8(OBJ->work_i[slot]));
    cpu.a = f_asl(cpu.a);
    op_clc();
    op_adc(RD8(OBJ->work_j[slot]));
    op_tay();
    op_lda(rd8((uint16_t)(PlayListByWorkIJ0A9ELists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(PlayListByWorkIJ0A9ELists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xCF5F);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_PlayListByWorkIJ0A9E(void)
{
    PlayListByWorkIJ0A9E(cpu.x);
}

/*
 * @routine 0A9E:796 StandInState1_0A9E
 * @run 71:CF96
 * WHAT IT DOES: Puts the object in state 1 with frame $80, or $83 when OBJ_WORK_J is set (Stage 06's overlay).
 * HOW: Stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StandInState1_0A9E(uint8_t slot)
{
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

void abi_StandInState1_0A9E(void)
{
    StandInState1_0A9E(cpu.x);
}

/*
 * @routine 0A9E:7AB RenumberKind30ThenClear0A9E
 * @run 71:CFAB
 * WHAT IT DOES: Plays one of two lists by bit 0 of OBJ_WORK_H; when it ends, numbers the kind $30 objects below
 * state 4 in slots OBJ_WORK_M to OBJ_WORK_N from 0 (OBJ_START_ARG) and gives each the first one's slot
 * (OBJ_WORK_M), then removes the object (Stage 06's overlay).
 * HOW: A word table by bit 0; a loop over the slots; ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    RenumberKind30ThenClear0A9ELists = 0xCFF6,   /* two frame list addresses */
};

void RenumberKind30ThenClear0A9E(uint8_t slot)
{
    op_lda(RD8(OBJ->work_h[slot]));
    op_and(0x01);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(RenumberKind30ThenClear0A9ELists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(RenumberKind30ThenClear0A9ELists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xCFBF);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        op_ldy(RD8(OBJ->work_m[slot]));
        WR8(ZP->temp00, 0);
        op_lda(RD8(OBJ->work_n[slot]));
        WR8(ZP->temp01, cpu.a);
        RMW8(ZP->temp01, v, f_inc(v));
        do {
            op_lda(RD8(OBJ->kind[cpu.y]));
            op_cmp(0x30);
            if (flag_z()) {
                op_lda(RD8(OBJ->state[cpu.y]));
                op_cmp(0x04);
                if (!flag_c()) {
                    op_lda(RD8(ZP->temp00));
                    if (flag_z()) {
                        WR8(ZP->temp03, cpu.y);
                    }
                    op_lda(RD8(ZP->temp00));
                    WR8(OBJ->start_arg[cpu.y], cpu.a);
                    op_lda(RD8(ZP->temp03));
                    WR8(OBJ->work_m[cpu.y], cpu.a);
                    RMW8(ZP->temp00, v, f_inc(v));
                }
            }
            op_iny();
            op_cpy(RD8(ZP->temp01));
        } while (!flag_c());
        jsr_push(0xCFF5);
        ClearObject(slot);
        jsr_pop();
    }
}

void abi_RenumberKind30ThenClear0A9E(void)
{
    RenumberKind30ThenClear0A9E(cpu.x);
}

/*
 * @routine 0A9F:028 SetFlightAndSpawnKind33_0A9F
 * @run 71:D028
 * WHAT IT DOES: Loads the record with OBJ_WORK_E $28, moves across (X speed reversed facing left), takes X and Y
 * speeds by OBJ_WORK_D and the facing, and starts a kind $33 (slot from $50) with its OBJ_WORK_F (Stage 06's
 * overlay).
 * HOW: Eight times OBJ_WORK_D plus four times the facing indexes four-byte records; FindFreeSlotFrom50.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    SetFlightAndSpawnKind33_0A9FSpeeds = 0xD075,   /* X speed low, high, Y speed low, high */
};

void SetFlightAndSpawnKind33_0A9F(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xD02B);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x28);
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(RD8(OBJ->facing_left[slot]));
    if (!flag_z()) {
        jsr_push(0xD038);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
    }
    jsr_push(0xD03B);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
    op_lda(RD8(OBJ->work_d[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->facing_left[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_clc();
    op_adc(RD8(ZP->temp00));
    op_tay();
    op_lda(rd8((uint16_t)(SetFlightAndSpawnKind33_0A9FSpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(SetFlightAndSpawnKind33_0A9FSpeeds + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(rd8((uint16_t)(SetFlightAndSpawnKind33_0A9FSpeeds + 3 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(SetFlightAndSpawnKind33_0A9FSpeeds + 2 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xD067);
        found_slot = FindFreeSlotFrom50(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0x33);
        jsr_push(0xD06E);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        op_lda(RD8(OBJ->work_f[slot]));
        WR8(OBJ->work_f[cpu.y], cpu.a);
    }
}

void abi_SetFlightAndSpawnKind33_0A9F(void)
{
    SetFlightAndSpawnKind33_0A9F(cpu.x);
}

/*
 * @routine 0A9F:085 WaitThenArmFollowY0A9F
 * @run 71:D085
 * WHAT IT DOES: Counts OBJ_WORK_E down; on zero plays sound $25, turns the hit box on, restarts the animation and
 * steps the state on; then follows its parent's Y (Stage 06's overlay).
 * HOW: DEC, SoundCommand, stores, then $D0A0.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void WaitThenArmFollowY0A9F(uint8_t slot)
{
    RMW8(OBJ->work_e[slot], v, f_dec(v));
    if (flag_z()) {
        op_lda(0x25);
        {
            uint8_t command;

            jsr_push(0xD08F);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
            jsr_pop();
        }
        op_lda(0x02);
        WR8(OBJ->hitbox_on[slot], cpu.a);
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
    FollowParentYOrClear0A9F(slot);
}

void abi_WaitThenArmFollowY0A9F(void)
{
    WaitThenArmFollowY0A9F(cpu.x);
}

/*
 * @routine 0A9F:0A0 FollowParentYOrClear0A9F
 * @run 71:D0A0
 * WHAT IT DOES: Takes the Y position of the slot in OBJ_WORK_F, and removes the object once that slot is in state 4
 * (Stage 06's overlay).
 * HOW: A copy and a compare; ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FollowParentYOrClear0A9F(uint8_t slot)
{
    op_ldy(RD8(OBJ->work_f[slot]));
    op_lda(RD8(OBJ->y_lo[cpu.y]));
    WR8(OBJ->y_lo[slot], cpu.a);
    op_lda(RD8(OBJ->state[cpu.y]));
    op_cmp(0x04);
    if (flag_z()) {
        ClearObject(slot);
        return;
    }
}

void abi_FollowParentYOrClear0A9F(void)
{
    FollowParentYOrClear0A9F(cpu.x);
}

/*
 * @routine 0A9F:0B4 PlayListMoveFall0A9F
 * @run 71:D0B4
 * WHAT IT DOES: Plays the list OBJ_WORK_D chooses and moves; with OBJ_WORK_D 1 also adds $18 to the Y speed (Stage
 * 06's overlay).
 * HOW: Twice OBJ_WORK_D indexes a word table; StepObjectFrames, ApplyVelocity, a 16-bit add.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    PlayListMoveFall0A9FLists = 0xD0E2,   /* the frame list address per OBJ_WORK_D */
};

void PlayListMoveFall0A9F(uint8_t slot)
{
    op_lda(RD8(OBJ->work_d[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(PlayListMoveFall0A9FLists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(PlayListMoveFall0A9FLists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD0C6);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    jsr_push(0xD0C9);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    op_lda(RD8(OBJ->work_d[slot]));
    op_cmp(0x01);
    if (flag_z()) {
        op_lda(RD8(OBJ->vy_lo[slot]));
        op_clc();
        op_adc(0x18);
        WR8(OBJ->vy_lo[slot], cpu.a);
        op_lda(RD8(OBJ->vy_hi[slot]));
        op_adc(0x00);
        WR8(OBJ->vy_hi[slot], cpu.a);
    }
}

void abi_PlayListMoveFall0A9F(void)
{
    PlayListMoveFall0A9F(cpu.x);
}

/*
 * @routine 0A9F:10E MoveAndSpawnKind33_0A9F
 * @run 71:D10E
 * WHAT IT DOES: Loads the record with OBJ_WORK_E $40, moves across (X speed reversed facing left), starts a kind
 * $33 (slot from $50) with OBJ_WORK_D 1 and its OBJ_WORK_F, then takes the X speed of its facing and moves across
 * again (Stage 06's overlay).
 * HOW: NegateVelocityX, ApplyVelocityX, FindFreeSlotFrom50, a table read by the facing.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    MoveAndSpawnKind33_0A9FSpeeds = 0xD143,   /* the X speed high byte by facing */
};

void MoveAndSpawnKind33_0A9F(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xD111);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x40);
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(RD8(OBJ->facing_left[slot]));
    if (!flag_z()) {
        jsr_push(0xD11E);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
    }
    jsr_push(0xD121);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xD124);
        found_slot = FindFreeSlotFrom50(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_lda(0x33);
        jsr_push(0xD12B);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        op_lda(0x01);
        WR8(OBJ->work_d[cpu.y], cpu.a);
        op_lda(RD8(OBJ->work_f[slot]));
        WR8(OBJ->work_f[cpu.y], cpu.a);
    }
    op_ldy(RD8(OBJ->facing_left[slot]));
    op_lda(rd8((uint16_t)(MoveAndSpawnKind33_0A9FSpeeds + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    jsr_push(0xD142);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
}

void abi_MoveAndSpawnKind33_0A9F(void)
{
    MoveAndSpawnKind33_0A9F(cpu.x);
}

/*
 * @routine 0A9F:15D PlayBoxedListThenClear0A9F
 * @run 71:D15D
 * WHAT IT DOES: Plays the list at $D174 with the hit box of each frame, and removes the object when OBJ_WORK_E runs
 * out (Stage 06's overlay).
 * HOW: StepObjectFrames, HitBoxByFrame0A9F_197, DEC, ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void PlayBoxedListThenClear0A9F(void)
{
    op_lda(0xD1);
    WR8(ZP->temp11, cpu.a);
    op_lda(0x74);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD168);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    call_routine(0xD16B, 0xD197);
    RMW8(OBJ->work_e[cpu.x], v, f_dec(v));
    if (flag_z()) {
        ClearObject(cpu.x);
        return;
    }
}

void abi_PlayBoxedListThenClear0A9F(void)
{
    PlayBoxedListThenClear0A9F();
}

/*
 * @routine 0A9F:1D5 TimedListFollowParent0A9F
 * @run 71:D1D5
 * WHAT IT DOES: State 0 loads the record with sprite set OBJ_WORK_D + 3 and OBJ_WORK_E $28; later plays the list
 * OBJ_WORK_D chooses, follows its parent's Y, and is removed when OBJ_WORK_E runs out (Stage 06's overlay).
 * HOW: Twice OBJ_WORK_D indexes a word table; StepObjectFrames, DEC, ClearObject or $D0A0.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    TimedListFollowParent0A9FLists = 0xD209,   /* the frame list address per OBJ_WORK_D */
};

void TimedListFollowParent0A9F(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xD1DD);
            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(RD8(OBJ->work_d[slot]));
        op_clc();
        op_adc(0x03);
        WR8(OBJ->sprite_set[slot], cpu.a);
        op_lda(0x28);
        WR8(OBJ->work_e[slot], cpu.a);
        return;
    }
    op_lda(RD8(OBJ->work_d[slot]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(TimedListFollowParent0A9FLists + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    op_lda(rd8((uint16_t)(TimedListFollowParent0A9FLists + 1 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD1FE);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    RMW8(OBJ->work_e[slot], v, f_dec(v));
    if (flag_z()) {
        ClearObject(slot);
        return;
    }
    FollowParentYOrClear0A9F(slot);
}

void abi_TimedListFollowParent0A9F(void)
{
    TimedListFollowParent0A9F(cpu.x);
}

/*
 * @routine 0A9F:24D CarrySlotInWorkE0A9F
 * @run 71:D24D
 * WHAT IT DOES: Carries the slot in OBJ_WORK_E: puts it at the object's place, moves it by the offset of the
 * object's frame (X reversed facing left), holds its speeds and hits at 0, and puts it back to state 2 when it is
 * in state 2 or more (Stage 06's overlay).
 * HOW: CopyObjectPlace; twice the frame less $9C indexes two-byte records; ApplyVelocity on that slot; stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    CarrySlotInWorkE0A9FOffsets = 0xD2A0,   /* X speed high, Y speed high per frame from $9C */
};

void CarrySlotInWorkE0A9F(void)
{
    op_ldy(RD8(OBJ->work_e[cpu.x]));
    if (!flag_z()) {
        jsr_push(0xD255);
        copy_place_exit_shim(CopyObjectPlace(cpu.x, cpu.y));
        jsr_pop();
        WR8(ZP->richter_object, cpu.x);
        op_lda(RD8(OBJ->frame[cpu.x]));
        op_sec();
        op_sbc(0x9C);
        cpu.a = f_asl(cpu.a);
        op_tax();
        op_lda(rd8((uint16_t)(CarrySlotInWorkE0A9FOffsets + cpu.x)));
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        op_lda(rd8((uint16_t)(CarrySlotInWorkE0A9FOffsets + 1 + cpu.x)));
        WR8(OBJ->vy_hi[cpu.y], cpu.a);
        op_ldx(RD8(ZP->richter_object));
        op_lda(RD8(OBJ->facing_left[cpu.x]));
        if (!flag_z()) {
            op_lda(RD8(OBJ->vx_hi[cpu.y]));
            op_eor(0xFF);
            cpu.a = f_inc(cpu.a);
            WR8(OBJ->vx_hi[cpu.y], cpu.a);
        }
        op_sxy();
        jsr_push(0xD27F);
        axis_move_exit_shim(ApplyVelocity(cpu.x));
        jsr_pop();
        WR8(OBJ->vx_hi[cpu.x], 0);
        WR8(OBJ->vx_lo[cpu.x], 0);
        WR8(OBJ->vy_hi[cpu.x], 0);
        WR8(OBJ->vy_lo[cpu.x], 0);
        WR8(OBJ->hit_taken[cpu.x], 0);
        op_sxy();
        op_lda(RD8(OBJ->state[cpu.y]));
        op_cmp(0x02);
        if (flag_c()) {
            op_cla();
            WR8(OBJ->hit_taken[cpu.y], cpu.a);
            op_lda(0x02);
            WR8(OBJ->state[cpu.y], cpu.a);
        }
    }
}

void abi_CarrySlotInWorkE0A9F(void)
{
    CarrySlotInWorkE0A9F();
}

/*
 * @routine 0A9F:2C0 RandomWorkDOnZeroG0A9F
 * @run 71:D2C0
 * WHAT IT DOES: When OBJ_WORK_G's low seven bits are 0, sets OBJ_WORK_D to a random 0 to 3 (Stage 06's overlay).
 * HOW: An AND, PlayerDistanceSum's low two bits.
 * WAITS: no.
 * Entry: X the object; C as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void RandomWorkDOnZeroG0A9F(uint8_t slot)
{
    op_lda(RD8(OBJ->work_g[slot]));
    op_and(0x7F);
    if (flag_z()) {
        jsr_push(0xD2CA);
        distance_sum_exit_shim(PlayerDistanceSum(slot, (cpu.p & P_C) != 0));
        jsr_pop();
        op_and(0x03);
        WR8(OBJ->work_d[slot], cpu.a);
    }
}

void abi_RandomWorkDOnZeroG0A9F(void)
{
    RandomWorkDOnZeroG0A9F(cpu.x);
}

/*
 * @routine 0A9F:2FF SpawnDropByField93_0A9F
 * @run 71:D2FF
 * WHAT IT DOES: Starts kind $84, or with zField93 set the kind it chooses (setting PlayerItemFirstUse and clearing
 * zField93; kind 0 starts nothing), in a slot from $30, keeps that slot in OBJ_WORK_E and plays sound $A8 (Stage
 * 06's overlay).
 * HOW: A table read by zField93, FindFreeSlotFrom30, StartObjectAtPlace, SoundCommand.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    SpawnDropByField93_0A9FKinds = 0xD328,   /* the kind per value of zField93 */
};

void SpawnDropByField93_0A9F(uint8_t slot)
{
    op_lda(RD8(ZP->field93));
    if (flag_z()) {
        op_lda(0x84);
    } else {
        op_tay();
        op_lda(0x01);
        WR8(PVARS->item_first_use, cpu.a);
        WR8(ZP->field93, 0);
        op_lda(rd8((uint16_t)(SpawnDropByField93_0A9FKinds + cpu.y)));
        if (flag_z()) {
            goto at_D327;
        }
    }
    op_pha();
    {
        uint8_t a;
        bool found;
        uint8_t found_slot;

        jsr_push(0xD318);
        found_slot = FindFreeSlotFrom30(&a, &found);
        find_slot_exit_shim(found_slot, a, found);
        jsr_pop();
    }
    if (flag_c()) {
        op_pla();
        jsr_push(0xD31E);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        op_tya();
        WR8(OBJ->work_e[slot], cpu.a);
        op_lda(0xA8);
        {
            uint8_t command;

            jsr_push(0xD327);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
            jsr_pop();
        }
    }
at_D327:
    ;
}

void abi_SpawnDropByField93_0A9F(void)
{
    SpawnDropByField93_0A9F(cpu.x);
}

/*
 * @routine 0A9F:337 LandThenStateOn0A9F
 * @run 71:D337
 * WHAT IT DOES: Falls with height $0C and on landing steps the state on (Stage 06's overlay).
 * HOW: StopOnLanding, INC.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
void LandThenStateOn0A9F(uint8_t slot)
{
    op_lda(0x0C);
    jsr_push(0xD33C);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_LandThenStateOn0A9F(void)
{
    LandThenStateOn0A9F(cpu.x);
}

/*
 * @routine 0A9F:342 PlayFaceThenWalk0A9F
 * @run 71:D342
 * WHAT IT DOES: Plays the list at $D3BC, faces the player, and walks (Stage 06's overlay).
 * HOW: StepObjectFrames, SideOfPlayer, then $D358.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void PlayFaceThenWalk0A9F(uint8_t slot)
{
    op_lda(0xD3);
    WR8(ZP->temp11, cpu.a);
    op_lda(0xBC);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD34D);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    jsr_push(0xD350);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    op_and(0x01);
    WR8(OBJ->facing_left[slot], cpu.a);
    WalkUntilEdge0A9F(slot);
}

void abi_PlayFaceThenWalk0A9F(void)
{
    PlayFaceThenWalk0A9F(cpu.x);
}

/*
 * @routine 0A9F:358 WalkUntilEdge0A9F
 * @run 71:D358
 * WHAT IT DOES: Walks at the speed of OBJ_WORK_D and its facing; goes to state 3, substate 0 when the move stops
 * it, when the terrain on the side it moves to has bit 0 clear, one time in 32, or while the player is in substate
 * 1 (Stage 06's overlay).
 * HOW: Four times OBJ_WORK_D plus twice the facing indexes a word table; MoveAndLand, ProbeTerrain,
 * PlayerDistanceSum.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    WalkUntilEdge0A9FSpeeds = 0xD3AC,   /* the X speed per OBJ_WORK_D and facing, a word each */
};

void WalkUntilEdge0A9F(uint8_t slot)
{
    op_lda(RD8(OBJ->work_d[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->facing_left[slot]));
    cpu.a = f_asl(cpu.a);
    op_clc();
    op_adc(RD8(ZP->temp00));
    op_tay();
    op_lda(rd8((uint16_t)(WalkUntilEdge0A9FSpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(WalkUntilEdge0A9FSpeeds + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(0x0C);
    jsr_push(0xD378);
    mover_exit_shim(MoveAndLand(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_WALL);
    jsr_pop();
    if (!flag_c()) {
        op_lda(0x0C);
        WR8(ZP->product.mid, cpu.a);
        op_lda(0x01);
        jsr_push(0xD383);
        probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
        jsr_pop();
        op_lda(RD8(OBJ->vx_hi[slot]));
        cpu.a = f_rol(cpu.a);
        cpu.a = f_rol(cpu.a);
        op_and(0x01);
        op_eor(0x01);
        cpu.a = f_inc(cpu.a);
        op_tay();
        op_lda(RD8(LEVEL->tile_hit[cpu.y]));
        op_and(0x01);
        if (!flag_z()) {
            jsr_push(0xD398);
            distance_sum_exit_shim(PlayerDistanceSum(slot, (cpu.p & P_C) != 0));
            jsr_pop();
            op_and(0x1F);
            if (!flag_z()) {
                op_lda(RD8(OBJ->substate[0x00]));
                op_cmp(0x01);
                if (!flag_z()) {
                    goto at_D3AB;
                }
            }
        }
    }
    op_lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
at_D3AB:
    ;
}

void abi_WalkUntilEdge0A9F(void)
{
    WalkUntilEdge0A9F(cpu.x);
}

/*
 * @routine 0A9F:3C3 JumpAtRandomThenLand0A9F
 * @run 71:D3C3
 * @stop D3FA
 * WHAT IT DOES: Substate 0 takes one of four X and Y speeds at random (X reversed facing left), restarts the
 * animation and steps the substate on; later ones land (Stage 06's overlay).
 * HOW: Four times PlayerDistanceSum's low two bits indexes four-byte records; else $D40A.
 * WAITS: no.
 * Entry: X the object; C as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    JumpAtRandomThenLand0A9FSpeeds = 0xD3FA,   /* X speed low, high, Y speed low, high */
};

void JumpAtRandomThenLand0A9F(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    if (!flag_z()) {
        PlayListUntilLanded0A9F(slot);
        return;
    }
    jsr_push(0xD3CB);
    distance_sum_exit_shim(PlayerDistanceSum(slot, (cpu.p & P_C) != 0));
    jsr_pop();
    op_and(0x03);
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(JumpAtRandomThenLand0A9FSpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(JumpAtRandomThenLand0A9FSpeeds + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(rd8((uint16_t)(JumpAtRandomThenLand0A9FSpeeds + 3 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(JumpAtRandomThenLand0A9FSpeeds + 2 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    RMW8(OBJ->substate[slot], v, f_inc(v));
    op_lda(RD8(OBJ->facing_left[slot]));
    if (!flag_z()) {
        jsr_push(0xD3F9);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
    }
}

void abi_JumpAtRandomThenLand0A9F(void)
{
    JumpAtRandomThenLand0A9F(cpu.x);
}

/*
 * @routine 0A9F:40A PlayListUntilLanded0A9F
 * @run 71:D40A
 * WHAT IT DOES: Plays the list at $D42F and falls with height $0C; on landing restarts the animation in state 2,
 * substate 0, with OBJ_WORK_H 0 (Stage 06's overlay).
 * HOW: StepObjectFrames, StopOnLandingFrontColumn, stores.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void PlayListUntilLanded0A9F(uint8_t slot)
{
    op_lda(0xD4);
    WR8(ZP->temp11, cpu.a);
    op_lda(0x2F);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD415);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    op_lda(0x0C);
    jsr_push(0xD41A);
    mover_exit_shim(StopOnLandingFrontColumn(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        op_lda(0x02);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
        WR8(OBJ->work_h[slot], 0);
        return;
    }
}

void abi_PlayListUntilLanded0A9F(void)
{
    PlayListUntilLanded0A9F(cpu.x);
}

/*
 * @routine 0A9F:436 WalkRightCounted0A9F
 * @run 71:D436
 * WHAT IT DOES: Plays the list at $D493 facing right and walks at the speed of OBJ_WORK_D; steps the state on
 * (substate 0, animation restarted) when the move stops it, when the terrain ahead has bit 0 clear, or when
 * OBJ_WORK_F counts out (Stage 06's overlay).
 * HOW: Four times OBJ_WORK_D indexes a word table; MoveAndLand, ProbeTerrain, DEC.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    WalkRightCounted0A9FSpeeds = 0xD3AC,   /* the X speed per OBJ_WORK_D and facing, a word each */
};

void WalkRightCounted0A9F(uint8_t slot)
{
    op_lda(0xD4);
    WR8(ZP->temp11, cpu.a);
    op_lda(0x93);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD441);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    WR8(OBJ->facing_left[slot], 0);
    op_lda(RD8(OBJ->work_d[slot]));
    cpu.a = f_asl(cpu.a);
    cpu.a = f_asl(cpu.a);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->facing_left[slot]));
    cpu.a = f_asl(cpu.a);
    op_clc();
    op_adc(RD8(ZP->temp00));
    op_tay();
    op_lda(rd8((uint16_t)(WalkRightCounted0A9FSpeeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(WalkRightCounted0A9FSpeeds + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(0x0C);
    jsr_push(0xD464);
    mover_exit_shim(MoveAndLand(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_WALL);
    jsr_pop();
    if (!flag_c()) {
        op_lda(0x0C);
        WR8(ZP->product.mid, cpu.a);
        op_lda(0x01);
        jsr_push(0xD46F);
        probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
        jsr_pop();
        op_lda(RD8(OBJ->vx_hi[slot]));
        cpu.a = f_rol(cpu.a);
        cpu.a = f_rol(cpu.a);
        op_and(0x01);
        op_eor(0x01);
        cpu.a = f_inc(cpu.a);
        op_tay();
        op_lda(RD8(LEVEL->tile_hit[cpu.y]));
        op_and(0x01);
        if (!flag_z()) {
            RMW8(OBJ->work_f[slot], v, f_dec(v));
            if (!flag_z()) {
                goto at_D492;
            }
        }
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    RMW8(OBJ->state[slot], v, f_inc(v));
    WR8(OBJ->substate[slot], 0);
at_D492:
    ;
}

void abi_WalkRightCounted0A9F(void)
{
    WalkRightCounted0A9F(cpu.x);
}

/*
 * @routine 0A9F:4B2 PlayListThenRise0A9F_4B2
 * @run 71:D4B2
 * WHAT IT DOES: Plays the list at $D51F; when it ends, stops across, takes a Y speed of $FA00, restarts the
 * animation and steps the substate on (Stage 06's overlay).
 * HOW: StepObjectFrames, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void PlayListThenRise0A9F_4B2(uint8_t slot)
{
    op_lda(0xD5);
    WR8(ZP->temp11, cpu.a);
    op_lda(0x1F);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD4BD);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        WR8(OBJ->vx_hi[slot], 0);
        WR8(OBJ->vx_lo[slot], 0);
        op_lda(0xFA);
        WR8(OBJ->vy_hi[slot], cpu.a);
        WR8(OBJ->vy_lo[slot], 0);
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_PlayListThenRise0A9F_4B2(void)
{
    PlayListThenRise0A9F_4B2(cpu.x);
}

/*
 * @routine 0A9F:4D7 FlyListThenSpawnKind35_0A9F
 * @run 71:D4D7
 * WHAT IT DOES: Moves with gravity and plays the list at $D528; when it ends, starts a kind $35 (slot from $30),
 * takes a Y speed of $FE00, restarts the animation and steps the substate on (Stage 06's overlay).
 * HOW: ApplyGravity, ApplyVelocity, StepObjectFrames, FindFreeSlotFrom30, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void FlyListThenSpawnKind35_0A9F(uint8_t slot)
{
    jsr_push(0xD4DA);
    gravity_exit_shim(ApplyGravity(slot));
    jsr_pop();
    jsr_push(0xD4DD);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    op_lda(0xD5);
    WR8(ZP->temp11, cpu.a);
    op_lda(0x28);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD4E8);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        {
            uint8_t a;
            bool found;
            uint8_t found_slot;

            jsr_push(0xD4ED);
            found_slot = FindFreeSlotFrom30(&a, &found);
            find_slot_exit_shim(found_slot, a, found);
            jsr_pop();
        }
        if (flag_c()) {
            op_lda(0x35);
            jsr_push(0xD4F4);
            copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
            jsr_pop();
            op_lda(0xFE);
            WR8(OBJ->vy_hi[slot], cpu.a);
            WR8(OBJ->vy_lo[slot], 0);
            WR8(OBJ->anim_step[slot], 0);
            WR8(OBJ->anim_timer[slot], 0);
            RMW8(OBJ->substate[slot], v, f_inc(v));
        }
    }
}

void abi_FlyListThenSpawnKind35_0A9F(void)
{
    FlyListThenSpawnKind35_0A9F(cpu.x);
}

/*
 * @routine 0A9F:506 PlayListMoveYClear0A9F
 * @run 71:D506
 * WHAT IT DOES: Plays the list at $D531, then moves up or down and is removed at Y high byte $FE (Stage 06's
 * overlay).
 * HOW: StepObjectFrames, a fall into $D511.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void PlayListMoveYClear0A9F(void)
{
    op_lda(0xD5);
    WR8(ZP->temp11, cpu.a);
    op_lda(0x31);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD511);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    /* falls into $D511 */
    MoveYClearAtFE0A9F(cpu.x);
}

void abi_PlayListMoveYClear0A9F(void)
{
    PlayListMoveYClear0A9F();
}

/*
 * @routine 0A9F:511 MoveYClearAtFE0A9F
 * @run 71:D511
 * WHAT IT DOES: Moves by its Y speed and removes the object once its Y high byte is $FE (Stage 06's overlay).
 * HOW: ApplyVelocityY, a compare, ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void MoveYClearAtFE0A9F(uint8_t slot)
{
    jsr_push(0xD514);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
    op_lda(RD8(OBJ->y_hi[slot]));
    op_cmp(0xFE);
    if (flag_z()) {
        ClearObject(slot);
        return;
    }
}

void abi_MoveYClearAtFE0A9F(void)
{
    MoveYClearAtFE0A9F(cpu.x);
}

/*
 * @routine 0A9F:536 StartHighThenRise0A9F
 * @run 71:D536
 * WHAT IT DOES: State 0 loads the record, moves the object up by $60 and leaves it a Y speed of $FE00; later states
 * move it and remove it at Y high byte $FE (Stage 06's overlay).
 * HOW: LoadObjectRecord, ApplyVelocityY with $A0, a store; else $D511.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartHighThenRise0A9F(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (!flag_z()) {
        MoveYClearAtFE0A9F(slot);
        return;
    }
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xD53E);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0xA0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    jsr_push(0xD546);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
    op_lda(0xFE);
    WR8(OBJ->vy_hi[slot], cpu.a);
}

void abi_StartHighThenRise0A9F(void)
{
    StartHighThenRise0A9F(cpu.x);
}

/*
 * @routine 0A9F:57F FacePlayerEvery16_0A9F
 * @run 71:D57F
 * WHAT IT DOES: In the states a table marks, faces the player when OBJ_WORK_I's low four bits are 0 (Stage 06's
 * overlay).
 * HOW: A table read by the state, an AND, SideOfPlayer.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FacePlayerEvery16_0A9FStates = 0xD597,   /* nonzero for the states that turn */
};

void FacePlayerEvery16_0A9F(uint8_t slot)
{
    op_ldy(RD8(OBJ->state[slot]));
    op_lda(rd8((uint16_t)(FacePlayerEvery16_0A9FStates + cpu.y)));
    if (!flag_z()) {
        op_lda(RD8(OBJ->work_i[slot]));
        op_and(0x0F);
        if (flag_z()) {
            jsr_push(0xD591);
            side_of_player_exit_shim(SideOfPlayer(slot));
            jsr_pop();
            op_and(0x01);
            WR8(OBJ->facing_left[slot], cpu.a);
        }
    }
}

void abi_FacePlayerEvery16_0A9F(void)
{
    FacePlayerEvery16_0A9F(cpu.x);
}

/*
 * @routine 0A9F:5D2 State8WhenHit0A9F
 * @run 71:D5D2
 * WHAT IT DOES: Below state 8, when bit 7 of OBJ_HIT_STATUS is set, plays sound $A6 (on the first screen), restarts
 * the animation and goes to state 8, substate 0 (Stage 06's overlay).
 * HOW: A compare, an AND, SoundIfOnFirstScreen, stores.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void State8WhenHit0A9F(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    op_cmp(0x08);
    if (!flag_c()) {
        op_lda(RD8(OBJ->hit_status[slot]));
        op_and(0x80);
        if (!flag_z()) {
            op_lda(0xA6);
            jsr_push(0xD5E5);
            sound_if_on_first_screen_exit_shim(SoundIfOnFirstScreen(cpu.a, slot, cpu.y));
            jsr_pop();
            WR8(OBJ->anim_step[slot], 0);
            WR8(OBJ->anim_timer[slot], 0);
            op_lda(0x08);
            WR8(OBJ->state[slot], cpu.a);
            WR8(OBJ->substate[slot], 0);
        }
    }
}

void abi_State8WhenHit0A9F(void)
{
    State8WhenHit0A9F(cpu.x);
}

/*
 * @routine 0A9F:621 StartPartsHoldDoors0A9F
 * @run 71:D621
 * WHAT IT DOES: Loads the record with OBJ_WORK_E $50 and sprite flip $80, holds the stage's doors shut
 * (StageDoorHeld 1), and starts kinds $37 and $39 in the next two slots (Stage 06's overlay).
 * HOW: Stores; ClearObjectInY and StartObjectAtPlace twice.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StartPartsHoldDoors0A9F(uint8_t slot)
{
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xD624);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_lda(0x50);
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(0x01);
    wr8((uint16_t)(StageDoorHeld), cpu.a);
    op_lda(0x80);
    WR8(OBJ->sprite_flip[slot], cpu.a);
    op_txa();
    op_tay();
    op_iny();
    jsr_push(0xD639);
    ClearObjectInY(cpu.y);
    jsr_pop();
    op_lda(0x37);
    jsr_push(0xD63E);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
    jsr_pop();
    op_iny();
    jsr_push(0xD642);
    ClearObjectInY(cpu.y);
    jsr_pop();
    op_lda(0x39);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
}

void abi_StartPartsHoldDoors0A9F(void)
{
    StartPartsHoldDoors0A9F(cpu.x);
}

/*
 * @routine 0A9F:647 StateOnWhenNear60_0A9F
 * @run 71:D647
 * WHAT IT DOES: Steps the state on when the player is less than $60 away across (Stage 06's overlay).
 * HOW: DistanceXToPlayer, a compare, INC.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void StateOnWhenNear60_0A9F(uint8_t slot)
{
    jsr_push(0xD64A);
    distance_exit_shim(DistanceXToPlayer(slot));
    jsr_pop();
    op_cmp(0x60);
    if (!flag_c()) {
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_StateOnWhenNear60_0A9F(void)
{
    StateOnWhenNear60_0A9F(cpu.x);
}

/*
 * @routine 0A9F:652 PlayListThenRise0A9F_652
 * @run 71:D652
 * WHAT IT DOES: Plays the list at $D66B; when it ends, clears the sprite flip, takes a Y speed high byte of $FD and
 * steps the state on (Stage 06's overlay).
 * HOW: StepObjectFrames, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void PlayListThenRise0A9F_652(uint8_t slot)
{
    op_lda(0x6B);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xD6);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD65D);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        WR8(OBJ->sprite_flip[slot], 0);
        op_lda(0xFD);
        WR8(OBJ->vy_hi[slot], cpu.a);
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_PlayListThenRise0A9F_652(void)
{
    PlayListThenRise0A9F_652(cpu.x);
}

/*
 * @routine 0A9F:67E LandSoundArm0A9F
 * @run 71:D67E
 * WHAT IT DOES: Falls with height $0F; on landing plays sound $4A (on the first screen), turns the hit box on and
 * steps the state on (Stage 06's overlay).
 * HOW: StopOnLanding, SoundIfOnFirstScreen, stores.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
void LandSoundArm0A9F(uint8_t slot)
{
    op_lda(0x0F);
    jsr_push(0xD683);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        op_lda(0x4A);
        jsr_push(0xD68A);
        sound_if_on_first_screen_exit_shim(SoundIfOnFirstScreen(cpu.a, slot, cpu.y));
        jsr_pop();
        op_lda(0x02);
        WR8(OBJ->hitbox_on[slot], cpu.a);
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_LandSoundArm0A9F(void)
{
    LandSoundArm0A9F(cpu.x);
}

/*
 * @routine 0A9F:6AC BounceByListStep0A9F
 * @run 71:D6AC
 * WHAT IT DOES: Plays the list at $D6CC; as each step begins, takes the X speed OBJ_WORK_F chooses and moves
 * across, bouncing at walls (Stage 06's overlay).
 * HOW: StepObjectFrames' carry, a table read, then $D6C2.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    BounceByListStep0A9FSpeeds = 0xD6CA,   /* the X speed high byte per OBJ_WORK_F */
};

void BounceByListStep0A9F(uint8_t slot)
{
    op_lda(0xCC);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xD6);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD6B7);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (!flag_c()) {
        JustReturn0A9F();
        return;
    }
    op_ldy(RD8(OBJ->work_f[slot]));
    op_lda(rd8((uint16_t)(BounceByListStep0A9FSpeeds + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    /* falls into $D6C2 */
    BounceAtWallsHeightF0A9F();
}

void abi_BounceByListStep0A9F(void)
{
    BounceByListStep0A9F(cpu.x);
}

/*
 * @routine 0A9F:6C2 BounceAtWallsHeightF0A9F
 * @run 71:D6C2
 * WHAT IT DOES: Moves across and bounces at walls with a height of $0F (Stage 06's overlay).
 * HOW: A store to zProduct mid, BounceAtWalls.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void BounceAtWallsHeightF0A9F(void)
{
    op_lda(0x0F);
    WR8(ZP->product.mid, cpu.a);
    call_routine(0xD6C9, 0xAB3A);
    /* falls into $D6C9 */
    JustReturn0A9F();
}

void abi_BounceAtWallsHeightF0A9F(void)
{
    BounceAtWallsHeightF0A9F();
}

/*
 * @routine 0A9F:6C9 JustReturn0A9F
 * @run 71:D6C9
 * WHAT IT DOES: Returns at once (Stage 06's overlay).
 * HOW: RTS.
 * WAITS: no.
 * Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void JustReturn0A9F(void)
{
}

void abi_JustReturn0A9F(void)
{
    JustReturn0A9F();
}

/*
 * @routine 0A9F:750 ThrowKind38AtFrameC3_0A9F
 * @run 71:D750
 * @stop D77C
 * WHAT IT DOES: Plays the list at $D77C; as frame $C3 begins, plays sound $1A and starts a kind $38 (slot from $30)
 * with the object's facing; when the list ends, goes on to the next round (Stage 06's overlay).
 * HOW: StepObjectFrames' Z and carry, a compare, FindFreeSlotFrom30; $D78D at the end.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void ThrowKind38AtFrameC3_0A9F(uint8_t slot)
{
    op_lda(0x7C);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xD7);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD75B);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        NextRoundToState4_0A9F(slot);
        return;
    }
    if (flag_c()) {
        op_lda(RD8(OBJ->frame[slot]));
        op_cmp(0xC3);
        if (flag_z()) {
            op_lda(0x1A);
            {
                uint8_t command;

                jsr_push(0xD76B);
                command = cpu.a;
                sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
                jsr_pop();
            }
            {
                uint8_t a;
                bool found;
                uint8_t found_slot;

                jsr_push(0xD76E);
                found_slot = FindFreeSlotFrom30(&a, &found);
                find_slot_exit_shim(found_slot, a, found);
                jsr_pop();
            }
            if (flag_c()) {
                op_lda(0x38);
                jsr_push(0xD775);
                copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
                jsr_pop();
                op_lda(RD8(OBJ->facing_left[slot]));
                WR8(OBJ->facing_left[cpu.y], cpu.a);
            }
        }
    }
}

void abi_ThrowKind38AtFrameC3_0A9F(void)
{
    ThrowKind38AtFrameC3_0A9F(cpu.x);
}

/*
 * @routine 0A9F:78D NextRoundToState4_0A9F
 * @run 71:D78D
 * WHAT IT DOES: Restarts the animation in state 4, steps OBJ_WORK_G on, takes OBJ_WORK_E for it and sets OBJ_WORK_C
 * 2 (Stage 06's overlay).
 * HOW: OBJ_WORK_G's low two bits index a table; stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    NextRoundToState4_0A9FWaits = 0xD7AD,   /* OBJ_WORK_E per round */
};

void NextRoundToState4_0A9F(uint8_t slot)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    op_lda(0x04);
    WR8(OBJ->state[slot], cpu.a);
    RMW8(OBJ->work_g[slot], v, f_inc(v));
    op_lda(RD8(OBJ->work_g[slot]));
    op_and(0x03);
    op_tay();
    op_lda(rd8((uint16_t)(NextRoundToState4_0A9FWaits + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(0x02);
    WR8(OBJ->work_c[slot], cpu.a);
}

void abi_NextRoundToState4_0A9F(void)
{
    NextRoundToState4_0A9F(cpu.x);
}

/*
 * @routine 0A9F:7B1 SoundA5ThenSubstateOn0A9F
 * @run 71:D7B1
 * WHAT IT DOES: Plays sound $A5 and steps the substate on (Stage 06's overlay).
 * HOW: SoundCommand, INC.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void SoundA5ThenSubstateOn0A9F(uint8_t slot)
{
    op_lda(0xA5);
    {
        uint8_t command;

        jsr_push(0xD7B6);
        command = cpu.a;
        sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
        jsr_pop();
    }
    RMW8(OBJ->substate[slot], v, f_inc(v));
}

void abi_SoundA5ThenSubstateOn0A9F(void)
{
    SoundA5ThenSubstateOn0A9F(cpu.x);
}

/*
 * @routine 0A9F:7FE LeapByFacingWithSound0A9F
 * @run 71:D7FE
 * WHAT IT DOES: Plays sound $A5, takes the X speed of its facing and a Y speed of $FA00, sets OBJ_WORK_D $60 and
 * steps the substate on (Stage 06's overlay).
 * HOW: SoundCommand, a table read by the facing, stores.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    LeapByFacingWithSound0A9FSpeeds = 0xD820,   /* the X speed high byte by facing */
};

void LeapByFacingWithSound0A9F(uint8_t slot)
{
    op_lda(0xA5);
    {
        uint8_t command;

        jsr_push(0xD803);
        command = cpu.a;
        sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
        jsr_pop();
    }
    op_ldy(RD8(OBJ->facing_left[slot]));
    op_lda(rd8((uint16_t)(LeapByFacingWithSound0A9FSpeeds + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    WR8(OBJ->vx_lo[slot], 0);
    op_lda(0xFA);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->vy_lo[slot], 0);
    op_lda(0x60);
    WR8(OBJ->work_d[slot], cpu.a);
    RMW8(OBJ->substate[slot], v, f_inc(v));
}

void abi_LeapByFacingWithSound0A9F(void)
{
    LeapByFacingWithSound0A9F(cpu.x);
}

/*
 * @routine 0AA0:022 WaitFallLand0AA0
 * @run 71:D822
 * WHAT IT DOES: Counts OBJ_WORK_D down; then plays the list at $D870, stops across once its stage X has high byte 3
 * or more and low byte $D0 or more, falls with height $0F, and on landing plays sound $4A, sets OBJ_WORK_C 2,
 * restarts the animation and steps the substate on (Stage 06's overlay).
 * HOW: DEC; StepObjectFrames; the scroll added to X and two compares; StopOnLanding, SoundCommand.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; D as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
void WaitFallLand0AA0(uint8_t slot)
{
    op_lda(RD8(OBJ->work_d[slot]));
    if (!flag_z()) {
        RMW8(OBJ->work_d[slot], v, f_dec(v));
        return;
    }
    op_lda(0x70);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xD8);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD836);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    op_lda(RD8(OBJ->x_lo[slot]));
    op_clc();
    op_adc(RD8(SCROLL->x_lo[0x00]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->x_hi[slot]));
    op_adc(RD8(SCROLLE->x_hi_screen[0x00]));
    op_cmp(0x03);
    if (flag_c()) {
        op_lda(RD8(ZP->temp00));
        op_cmp(0xD0);
        if (flag_c()) {
            WR8(OBJ->vx_hi[slot], 0);
            WR8(OBJ->vx_lo[slot], 0);
        }
    }
    op_lda(0x0F);
    jsr_push(0xD85A);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        op_lda(0x4A);
        {
            uint8_t command;

            jsr_push(0xD861);
            command = cpu.a;
            sound_command_exit_shim(command, SoundCommand(command, slot, cpu.y));
            jsr_pop();
        }
        op_lda(0x02);
        WR8(OBJ->work_c[slot], cpu.a);
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_WaitFallLand0AA0(void)
{
    WaitFallLand0AA0(cpu.x);
}

/*
 * @routine 0AA0:085 PlayListThenNextRound0AA0
 * @run 71:D885
 * WHAT IT DOES: Plays the list at $D896; when it ends, goes on to the next round (Stage 06's overlay).
 * HOW: StepObjectFrames, then $D78D.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void PlayListThenNextRound0AA0(void)
{
    op_lda(0x96);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xD8);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD890);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        NextRoundToState4_0A9F(cpu.x);
        return;
    }
}

void abi_PlayListThenNextRound0AA0(void)
{
    PlayListThenNextRound0AA0();
}

/*
 * @routine 0AA0:0BB QuarterWorkEThenState5_0AA0
 * @run 71:D8BB
 * WHAT IT DOES: Plays the list at $D8E6; when it ends, quarters OBJ_WORK_E, restarts the animation in state 5 and
 * gives it OBJ_INVULN_TIMER of at least 3 (Stage 06's overlay).
 * HOW: StepObjectFrames, two LSRs, stores, a compare.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void QuarterWorkEThenState5_0AA0(uint8_t slot)
{
    op_lda(0xE6);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xD8);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD8C6);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        RMW8(OBJ->work_e[slot], v, f_lsr(v));
        RMW8(OBJ->work_e[slot], v, f_lsr(v));
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        op_lda(0x05);
        WR8(OBJ->state[slot], cpu.a);
        op_lda(RD8(OBJ->invuln_timer[slot]));
        op_cmp(0x03);
        if (!flag_c()) {
            op_lda(0x03);
            WR8(OBJ->invuln_timer[slot], cpu.a);
        }
    }
}

void abi_QuarterWorkEThenState5_0AA0(void)
{
    QuarterWorkEThenState5_0AA0(cpu.x);
}

/*
 * @routine 0AA0:0ED LandThenWipe0AA0
 * @run 71:D8ED
 * @stop D90D
 * WHAT IT DOES: Falls with height $0F; in substate 0 plays the list at $D90D and when it ends sets OBJ_WORK_D $50
 * and steps the substate on; later substates wipe the VRAM blocks a step a frame (Stage 06's overlay).
 * HOW: StopOnLanding, StepObjectFrames; $D926 after.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
void LandThenWipe0AA0(uint8_t slot)
{
    op_lda(0x0F);
    jsr_push(0xD8F2);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    op_lda(RD8(OBJ->substate[slot]));
    if (!flag_z()) {
        WipeStepThenOpenDoors0AA0();
        return;
    }
    op_lda(0x0D);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xD9);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD902);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        op_lda(0x50);
        WR8(OBJ->work_d[slot], cpu.a);
        RMW8(OBJ->substate[slot], v, f_inc(v));
    }
}

void abi_LandThenWipe0AA0(void)
{
    LandThenWipe0AA0(cpu.x);
}

/*
 * @routine 0AA0:126 WipeStepThenOpenDoors0AA0
 * @run 71:D926
 * WHAT IT DOES: Wipes the VRAM blocks by step OBJ_WORK_J (low six bits) and steps it on; when OBJ_WORK_D runs out,
 * removes the object, sets slot $20's OBJ_HIT_TAKEN $FF and lets the doors open (StageDoorHeld 0) (Stage 06's
 * overlay).
 * HOW: The call at $DB94, INC, DEC, ClearObject, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void WipeStepThenOpenDoors0AA0(void)
{
    op_lda(RD8(OBJ->work_j[cpu.x]));
    op_and(0x3F);
    jsr_push(0xD92E);
    WipeVramBlocks0AA0();
    jsr_pop();
    RMW8(OBJ->work_j[cpu.x], v, f_inc(v));
    RMW8(OBJ->work_d[cpu.x], v, f_dec(v));
    if (flag_z()) {
        jsr_push(0xD939);
        ClearObject(cpu.x);
        jsr_pop();
        op_lda(0xFF);
        WR8(OBJ->hit_taken[0x20], cpu.a);
        wr8((uint16_t)(StageDoorHeld), 0);
    }
}

void abi_WipeStepThenOpenDoors0AA0(void)
{
    WipeStepThenOpenDoors0AA0();
}

/*
 * @routine 0AA0:142 HopTowardPlayer0AA0
 * @run 71:D942
 * @stop D97F
 * WHAT IT DOES: State 0 loads the record, moves by the X speed of its facing and $D0 up, then takes an X speed
 * toward the player that grows with the distance and a Y speed high byte of $FC; later states fly (Stage 06's
 * overlay).
 * HOW: A table read by the facing, ApplyVelocity; the distance over 32 indexes a word table; NegateVelocityX facing
 * left.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    HopTowardPlayer0AA0FirstSpeeds = 0xD97F,   /* the first X speed high byte by facing */
    HopTowardPlayer0AA0Speeds = 0xD981,   /* the X speed per 32 pixels of distance, a word each */
};

void HopTowardPlayer0AA0(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (!flag_z()) {
        FlyListWithGravity0AA0();
        return;
    }
    {
        uint8_t nz;
        bool carry;
        uint8_t last;

        jsr_push(0xD94A);
        last = LoadObjectRecord(slot, &nz, &carry);
        load_record_exit_shim(last, nz, carry);
        jsr_pop();
    }
    op_ldy(RD8(OBJ->facing_left[slot]));
    op_lda(rd8((uint16_t)(HopTowardPlayer0AA0FirstSpeeds + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(0xD0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    jsr_push(0xD95B);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    jsr_push(0xD95E);
    distance_exit_shim(DistanceXToPlayer(slot));
    jsr_pop();
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(HopTowardPlayer0AA0Speeds + 1 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    op_lda(rd8((uint16_t)(HopTowardPlayer0AA0Speeds + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    op_lda(0xFC);
    WR8(OBJ->vy_hi[slot], cpu.a);
    op_lda(RD8(OBJ->facing_left[slot]));
    if (!flag_z()) {
        jsr_push(0xD97E);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
    }
}

void abi_HopTowardPlayer0AA0(void)
{
    HopTowardPlayer0AA0(cpu.x);
}

/*
 * @routine 0AA0:191 FlyListWithGravity0AA0
 * @run 71:D991
 * WHAT IT DOES: Plays the list at $D9A2 and moves with gravity (Stage 06's overlay).
 * HOW: StepObjectFrames, ApplyGravityUnlimited, ApplyVelocity.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FlyListWithGravity0AA0(void)
{
    op_lda(0xA2);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xD9);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD99C);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    jsr_push(0xD99F);
    gravity_exit_shim(ApplyGravityUnlimited(cpu.x));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocity(cpu.x));
}

void abi_FlyListWithGravity0AA0(void)
{
    FlyListWithGravity0AA0();
}

/*
 * @routine 0AA0:1BB FollowKind36HitBox0AA0
 * @run 71:D9BB
 * WHAT IT DOES: State 0 loads the record. Later, while the slot before holds kind $36, copies its OBJ_WORK_C,
 * OBJ_WORK_B, hit box switch, facing and place, and takes the hit box of its frame $AD to $CC (none from $CD);
 * otherwise removes itself (Stage 06's overlay).
 * HOW: Copies from X - 1, CopyObjectPlaceBack; four times the frame less $AD indexes four-byte records.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
enum {
    FollowKind36HitBox0AA0Boxes = 0xDA18,   /* X offset, Y offset, half width, half height per frame */
};

void FollowKind36HitBox0AA0(uint8_t slot)
{
    op_lda(RD8(OBJ->state[slot]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            last = LoadObjectRecord(slot, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
        }
        return;
    }
    op_lda(rd8((uint16_t)(COL(OBJ->kind) - 1 + slot)));
    op_cmp(0x36);
    if (!flag_z()) {
        ClearObject(slot);
        return;
    }
    op_lda(rd8((uint16_t)(COL(OBJ->work_c) - 1 + slot)));
    WR8(OBJ->work_c[slot], cpu.a);
    op_lda(rd8((uint16_t)(COL(OBJ->work_b) - 1 + slot)));
    WR8(OBJ->work_b[slot], cpu.a);
    op_lda(rd8((uint16_t)(COL(OBJ->hitbox_on) - 1 + slot)));
    WR8(OBJ->hitbox_on[slot], cpu.a);
    op_lda(rd8((uint16_t)(COL(OBJ->facing_left) - 1 + slot)));
    WR8(OBJ->facing_left[slot], cpu.a);
    op_txa();
    op_tay();
    op_dey();
    jsr_push(0xD9EB);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    op_lda(rd8((uint16_t)(COL(OBJ->frame) - 1 + slot)));
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
    op_lda(rd8((uint16_t)(FollowKind36HitBox0AA0Boxes + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    op_lda(rd8((uint16_t)(FollowKind36HitBox0AA0Boxes + 1 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    op_lda(rd8((uint16_t)(FollowKind36HitBox0AA0Boxes + 2 + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    op_lda(rd8((uint16_t)(FollowKind36HitBox0AA0Boxes + 3 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_FollowKind36HitBox0AA0(void)
{
    FollowKind36HitBox0AA0(cpu.x);
}

/*
 * @routine 0AA0:2B4 FollowTwoBackOrHandOff0AA0
 * @run 71:DAB4
 * WHAT IT DOES: When the slot two before is in state 8, starts a kind $39 (slot from $50) in state 2 with
 * OBJ_WORK_E $A0 and the object's facing, and removes the object; otherwise, for that slot's frames $AD to $CC,
 * takes its place and facing, moves by the frame's offset (X reversed facing left) and lets the player touch it
 * (Stage 06's overlay).
 * HOW: A compare; FindFreeSlotFrom50; twice the frame less $AD indexes two-byte records; CopyObjectPlaceBack,
 * ApplyVelocity, ContactWithPlayer with a $11 by $1D box.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    FollowTwoBackOrHandOff0AA0Offsets = 0xDB17,   /* X speed high, Y speed high per frame from $AD */
};

void FollowTwoBackOrHandOff0AA0(void)
{
    op_lda(rd8((uint16_t)(COL(OBJ->state) - 2 + cpu.x)));
    op_cmp(0x08);
    if (flag_z()) {
        {
            uint8_t a;
            bool found;
            uint8_t found_slot;

            jsr_push(0xDABE);
            found_slot = FindFreeSlotFrom50(&a, &found);
            find_slot_exit_shim(found_slot, a, found);
            jsr_pop();
        }
        if (!flag_c()) {
            goto at_DB16;
        }
        op_lda(0x39);
        jsr_push(0xDAC5);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
        jsr_pop();
        op_lda(0xA0);
        WR8(OBJ->work_e[cpu.y], cpu.a);
        op_lda(RD8(OBJ->facing_left[cpu.x]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        op_lda(0x02);
        WR8(OBJ->state[cpu.y], cpu.a);
        ClearObject(cpu.x);
        return;
    }
    op_lda(rd8((uint16_t)(COL(OBJ->frame) - 2 + cpu.x)));
    op_cmp(0xCD);
    if (!flag_c()) {
        op_sec();
        op_sbc(0xAD);
        cpu.a = f_asl(cpu.a);
        op_tay();
        op_lda(rd8((uint16_t)(FollowTwoBackOrHandOff0AA0Offsets + cpu.y)));
        WR8(OBJ->vx_hi[cpu.x], cpu.a);
        op_lda(rd8((uint16_t)(FollowTwoBackOrHandOff0AA0Offsets + 1 + cpu.y)));
        WR8(OBJ->vy_hi[cpu.x], cpu.a);
        op_lda(rd8((uint16_t)(COL(OBJ->facing_left) - 2 + cpu.x)));
        WR8(OBJ->facing_left[cpu.x], cpu.a);
        if (!flag_z()) {
            op_lda(RD8(OBJ->vx_hi[cpu.x]));
            op_eor(0xFF);
            cpu.a = f_inc(cpu.a);
            WR8(OBJ->vx_hi[cpu.x], cpu.a);
        }
        op_txa();
        op_tay();
        op_dey();
        op_dey();
        jsr_push(0xDB08);
        copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, cpu.x));
        jsr_pop();
        jsr_push(0xDB0B);
        axis_move_exit_shim(ApplyVelocity(cpu.x));
        jsr_pop();
        op_lda(0x11);
        WR8(ZP->temp08, cpu.a);
        op_lda(0x1D);
        WR8(ZP->temp09, cpu.a);
        call_routine(0xDB16, 0xAA35);
    }
at_DB16:
    ;
}

void abi_FollowTwoBackOrHandOff0AA0(void)
{
    FollowTwoBackOrHandOff0AA0();
}

/*
 * @routine 0AA0:357 BounceToRest0AA0
 * @run 71:DB57
 * WHAT IT DOES: Shows frame $D6 and counts OBJ_WORK_E down (on zero steps the state on); falls with height 0 and on
 * each landing bounces with the Y speed of OBJ_WORK_D, stepping it on up to 6 (Stage 06's overlay).
 * HOW: DEC; StopOnLanding; twice OBJ_WORK_D indexes a word table.
 * WAITS: no.
 * Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
enum {
    BounceToRest0AA0Speeds = 0xDB85,   /* the Y speed per bounce, a word each */
};

void BounceToRest0AA0(uint8_t slot)
{
    op_lda(0xD6);
    WR8(OBJ->frame[slot], cpu.a);
    RMW8(OBJ->work_e[slot], v, f_dec(v));
    if (flag_z()) {
        RMW8(OBJ->state[slot], v, f_inc(v));
        return;
    }
    op_lda(0x00);
    jsr_push(0xDB6A);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), STAGE_C_PROBE_FLOOR);
    jsr_pop();
    if (flag_c()) {
        op_lda(RD8(OBJ->work_d[slot]));
        cpu.a = f_asl(cpu.a);
        op_tay();
        op_lda(rd8((uint16_t)(BounceToRest0AA0Speeds + 1 + cpu.y)));
        WR8(OBJ->vy_hi[slot], cpu.a);
        op_lda(rd8((uint16_t)(BounceToRest0AA0Speeds + cpu.y)));
        WR8(OBJ->vy_lo[slot], cpu.a);
        op_cpy(0x0C);
        if (!flag_z()) {
            RMW8(OBJ->work_d[slot], v, f_inc(v));
        }
    }
}

void abi_BounceToRest0AA0(void)
{
    BounceToRest0AA0(cpu.x);
}

/*
 * @routine 0AA0:394 WipeVramBlocks0AA0
 * @run 71:DB94
 * WHAT IT DOES: Clears one bit, in one of every four words, of each of ten blocks of $40 VRAM words, as the step in
 * A chooses (Stage 06's overlay).
 * HOW: The mask from $DBA3, then $DBCE for blocks 0 to 9.
 * WAITS: no.
 * Entry: A the step: bits 4-5 the word of four, bits 0-3 the bit. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
void WipeVramBlocks0AA0(void)
{
    op_phx();
    jsr_push(0xDB98);
    WipeMaskForStep0AA0();
    jsr_pop();
    op_clx();
    do {
        jsr_push(0xDB9C);
        WipeVramBlock0AA0();
        jsr_pop();
        op_inx();
        op_cpx(0x0A);
    } while (!flag_c());
    op_plx();
}

void abi_WipeVramBlocks0AA0(void)
{
    WipeVramBlocks0AA0();
}

/*
 * @routine 0AA0:3A3 WipeMaskForStep0AA0
 * @run 71:DBA3
 * WHAT IT DOES: Splits the step in A into the word of four (zTemp00, from its top four bits) and a mask with every
 * bit set but the one its low four bits name (zTemp01:zTemp02) (Stage 06's overlay).
 * HOW: LSRs, then a loop of 16-bit shifts and an EOR.
 * WAITS: no.
 * Entry: A the step. Exit: zTemp00 the word of four; zTemp01:zTemp02 the mask.
 */
void WipeMaskForStep0AA0(void)
{
    op_pha();
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    cpu.a = f_lsr(cpu.a);
    WR8(ZP->temp00, cpu.a);
    op_pla();
    op_and(0x0F);
    WR8(ZP->temp03, cpu.a);
    op_lda(0x01);
    WR8(ZP->temp01, cpu.a);
    WR8(ZP->temp02, 0);
at_DBB5:
    op_lda(RD8(ZP->temp03));
    if (!flag_z()) {
        RMW8(ZP->temp01, v, f_asl(v));
        RMW8(ZP->temp02, v, f_rol(v));
        RMW8(ZP->temp03, v, f_dec(v));
        goto at_DBB5;
    }
    op_lda(RD8(ZP->temp01));
    op_eor(0xFF);
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(ZP->temp02));
    op_eor(0xFF);
    WR8(ZP->temp02, cpu.a);
}

void abi_WipeMaskForStep0AA0(void)
{
    WipeMaskForStep0AA0();
}

/*
 * @routine 0AA0:3CE WipeVramBlock0AA0
 * @run 71:DBCE
 * WHAT IT DOES: Points at VRAM block X and wipes its $40 words with the mask (Stage 06's overlay).
 * HOW: Twice X indexes a word table into zBiosC; then $DBF3.
 * WAITS: no.
 * Entry: X the block; zTemp00 and zTemp01:zTemp02 as $DBA3 left them. Exit: A, X, Y and the flags as its last
 * instruction or call left them.
 */
enum {
    WipeVramBlock0AA0Addresses = 0xDBDF,   /* the VRAM address per block */
};

void WipeVramBlock0AA0(void)
{
    op_txa();
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(WipeVramBlock0AA0Addresses + cpu.y)));
    WR8(ZP->bios_c.lo, cpu.a);
    op_lda(rd8((uint16_t)(WipeVramBlock0AA0Addresses + 1 + cpu.y)));
    WR8(ZP->bios_c.hi, cpu.a);
    jsr_push(0xDBDE);
    MaskVramWords0AA0();
    jsr_pop();
}

void abi_WipeVramBlock0AA0(void)
{
    WipeVramBlock0AA0();
}

/*
 * @routine 0AA0:3F3 MaskVramWords0AA0
 * @run 71:DBF3
 * WHAT IT DOES: ANDs the mask zTemp01:zTemp02 into every fourth of the $40 VRAM words from zBiosC, the ones whose
 * number's low two bits equal zTemp00 (Stage 06's overlay).
 * HOW: A loop of $40 that sets the VDC's write and read addresses, reads, ANDs and writes the word.
 * WAITS: no.
 * Entry: zBiosC the VRAM address; zTemp00 the word of four; zTemp01:zTemp02 the mask. Exit: zBiosC $40 on; Y kept.
 */
void MaskVramWords0AA0(void)
{
    op_phy();
    op_cly();
    do {
        op_tya();
        op_and(0x03);
        op_cmp(RD8(ZP->temp00));
        if (flag_z()) {
            jsr_push(0xDBFF);
            SelectVramWriteAddress();
            cpu.a = STAGE_C_VDC_MAWR;
            jsr_pop();
            op_lda(RD8(ZP->bios_c.lo));
            WR8(VDC->data_lo, cpu.a);
            op_lda(RD8(ZP->bios_c.hi));
            WR8(VDC->data_hi, cpu.a);
            jsr_push(0xDC0C);
            SelectVramReadAddress();
            cpu.a = STAGE_C_VDC_MARR;
            abi_nz(STAGE_C_VDC_MARR);
            jsr_pop();
            op_lda(RD8(ZP->bios_c.lo));
            WR8(VDC->data_lo, cpu.a);
            op_lda(RD8(ZP->bios_c.hi));
            WR8(VDC->data_hi, cpu.a);
            jsr_push(0xDC19);
            SelectVramData();
            cpu.a = STAGE_C_VDC_VRAM;
            abi_nz(STAGE_C_VDC_VRAM);
            jsr_pop();
            op_lda(RD8(VDC->data_lo));
            op_and(RD8(ZP->temp01));
            WR8(VDC->data_lo, cpu.a);
            op_lda(RD8(VDC->data_hi));
            op_and(RD8(ZP->temp02));
            WR8(VDC->data_hi, cpu.a);
        }
        op_lda(RD8(ZP->bios_c.lo));
        op_clc();
        op_adc(0x01);
        WR8(ZP->bios_c.lo, cpu.a);
        op_lda(RD8(ZP->bios_c.hi));
        op_adc(0x00);
        WR8(ZP->bios_c.hi, cpu.a);
        op_iny();
        op_cpy(0x40);
    } while (!flag_c());
    op_ply();
}

void abi_MaskVramWords0AA0(void)
{
    MaskVramWords0AA0();
}

/*
 * @routine 0AA0:4BD HitCheckThenRunState0AA0
 * @run 71:DCBD
 * WHAT IT DOES: Goes to state 4 when hit twice or more, then runs the routine of the object's state (Stage 06's
 * overlay).
 * HOW: State4WhenHitTwice0AA0; a jump table by the state.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
enum {
    HitCheckThenRunState0AA0States = 0xDCD2,   /* the routine address per state */
};

void HitCheckThenRunState0AA0(void)
{
    call_routine(0xDCC0, 0xDCDC);
    op_lda(RD8(OBJ->state[cpu.x]));
    cpu.a = f_asl(cpu.a);
    op_tay();
    op_lda(rd8((uint16_t)(HitCheckThenRunState0AA0States + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    op_lda(rd8((uint16_t)(HitCheckThenRunState0AA0States + 1 + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    run_routine(RD16_LH2(ZP->temp00, ZP->temp01));
}

void abi_HitCheckThenRunState0AA0(void)
{
    HitCheckThenRunState0AA0();
}

/*
 * @routine 0AA0:506 PlayListThenArm0AA0
 * @run 71:DD06
 * WHAT IT DOES: Plays the list at $DD22; when it ends, turns the hit box on, restarts the animation and steps the
 * state on (Stage 06's overlay).
 * HOW: StepObjectFrames, stores.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void PlayListThenArm0AA0(uint8_t slot)
{
    op_lda(0x22);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xDD);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xDD11);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        op_lda(0x02);
        WR8(OBJ->hitbox_on[slot], cpu.a);
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        RMW8(OBJ->state[slot], v, f_inc(v));
    }
}

void abi_PlayListThenArm0AA0(void)
{
    PlayListThenArm0AA0(cpu.x);
}

/*
 * @routine 0AA0:52D TurnTowardPlayerSpeedUp0AA0
 * @run 71:DD2D
 * WHAT IT DOES: Plays the list at $DD89, faces the player, turns its angle (OBJ_WORK_E) one step toward the player,
 * speeds OBJ_WORK_F:G up by 4 a frame to $0180 and moves (Stage 06's overlay).
 * HOW: AngleToPlayer, TurnAngleToward, a 16-bit add, SetVelocityFromAngle, ApplyVelocity.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void TurnTowardPlayerSpeedUp0AA0(uint8_t slot)
{
    op_lda(0x89);
    WR8(ZP->temp10, cpu.a);
    op_lda(0xDD);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xDD38);
        y = cpu.y;
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    jsr_push(0xDD3B);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    op_and(0x01);
    WR8(OBJ->facing_left[slot], cpu.a);
    jsr_push(0xDD43);
    angle_exit_shim(AngleToPlayer(slot));
    jsr_pop();
    op_pha();
    op_lda(0x01);
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_e[slot]));
    WR8(ZP->temp01, cpu.a);
    op_pla();
    WR8(ZP->temp02, cpu.a);
    jsr_push(0xDD53);
    turn_angle_exit_shim(TurnAngleToward());
    jsr_pop();
    WR8(OBJ->work_e[slot], cpu.a);
    op_lda(RD8(OBJ->work_f[slot]));
    op_cmp(0x01);
    if (flag_c()) {
        op_lda(RD8(OBJ->work_g[slot]));
        op_cmp(0x80);
        if (flag_c()) {
            goto at_DD75;
        }
    }
    op_lda(RD8(OBJ->work_g[slot]));
    op_clc();
    op_adc(0x04);
    WR8(OBJ->work_g[slot], cpu.a);
    op_lda(RD8(OBJ->work_f[slot]));
    op_adc(0x00);
    WR8(OBJ->work_f[slot], cpu.a);
at_DD75:
    op_lda(RD8(OBJ->work_f[slot]));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_g[slot]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_e[slot]));
    jsr_push(0xDD85);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
    jsr_push(0xDD88);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
}

void abi_TurnTowardPlayerSpeedUp0AA0(void)
{
    TurnTowardPlayerSpeedUp0AA0(cpu.x);
}

/*
 * @routine 0AA0:592 BackAwayFromPlayer0AA0
 * @run 71:DD92
 * WHAT IT DOES: Substate 0 aims away from the player at a speed of $0180 for $14 frames and steps the substate on;
 * later moves that way and, when the count runs out, goes to state 3 with the speed 0 (Stage 06's overlay).
 * HOW: AngleToPlayer plus $80, stores; SetVelocityFromAngle, ApplyVelocity, DEC.
 * WAITS: no.
 * Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
 * them.
 */
void BackAwayFromPlayer0AA0(uint8_t slot)
{
    op_lda(RD8(OBJ->substate[slot]));
    if (flag_z()) {
        jsr_push(0xDD9A);
        angle_exit_shim(AngleToPlayer(slot));
        jsr_pop();
        op_clc();
        op_adc(0x80);
        WR8(OBJ->work_e[slot], cpu.a);
        op_lda(0x01);
        WR8(OBJ->work_f[slot], cpu.a);
        op_lda(0x80);
        WR8(OBJ->work_g[slot], cpu.a);
        op_lda(0x14);
        WR8(OBJ->work_h[slot], cpu.a);
        RMW8(OBJ->substate[slot], v, f_inc(v));
        return;
    }
    op_lda(RD8(OBJ->work_f[slot]));
    WR8(ZP->temp01, cpu.a);
    op_lda(RD8(OBJ->work_g[slot]));
    WR8(ZP->temp00, cpu.a);
    op_lda(RD8(OBJ->work_e[slot]));
    jsr_push(0xDDC3);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
    jsr_push(0xDDC6);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    RMW8(OBJ->work_h[slot], v, f_dec(v));
    if (flag_z()) {
        op_lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->work_f[slot], 0);
        WR8(OBJ->work_g[slot], 0);
    }
}

void abi_BackAwayFromPlayer0AA0(void)
{
    BackAwayFromPlayer0AA0(cpu.x);
}
