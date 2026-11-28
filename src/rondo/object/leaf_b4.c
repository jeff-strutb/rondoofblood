/*
 * The smallest routines of stages $09 to $10 and the secret stages that call nothing: tile handler steps, counts,
 * tests and state changes.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "leaf_b4.h"

static void lda(uint8_t value) { cpu.a = value; abi_nz(value); }
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

static void asl(void)
{
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
}

static void lsr_flags(uint8_t old)
{
    abi_flag(P_C, (old & 1) != 0);
    abi_nz((uint8_t)(old >> 1));
}

/* A tile handler's step: zTemp02 as a row of 8 and zTemp00 the column in the table; A $81 on the way out. */
static void tile_row_step(uint16_t table)
{
    lda(RD8(ZP->temp02));
    asl();
    asl();
    asl();
    carry(false);
    adc(RD8(ZP->temp00));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(table + cpu.y)));
    WR8(ZP->temp02, cpu.a);
    lda(0x81);
}

/* A tile handler's step: the column's byte added to zTemp02; A $81. */
static void tile_column_add(uint16_t table)
{
    cpu.y = RD8(ZP->temp00);
    lda(rd8((uint16_t)(table + cpu.y)));
    carry(false);
    adc(RD8(ZP->temp02));
    WR8(ZP->temp02, cpu.a);
    lda(0x81);
}

/*
 * @routine 161D:3E3 CountDown161D_3E3
 * @run 71:C3E3
 * WHAT IT DOES: Counts OBJ_WORK_D; at zero restarts the animation and goes to state 2 (Stage 0E's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void CountDown161D_3E3(uint8_t slot)
{
    uint8_t left = DEC8(OBJ->work_d[slot]);

    abi_nz(left);
    if (left != 0) {
        return;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(2);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_CountDown161D_3E3(void)
{
    CountDown161D_3E3(cpu.x);
}

/*
 * @routine 088B:573 CountDown088B_573
 * @run 70:B573
 * WHAT IT DOES: Counts OBJ_WORK_F; at zero the hit box off and state $12 (secret stage 04's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void CountDown088B_573(uint8_t slot)
{
    uint8_t left = DEC8(OBJ->work_f[slot]);

    abi_nz(left);
    if (left != 0) {
        return;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    lda(0x12);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_CountDown088B_573(void)
{
    CountDown088B_573(cpu.x);
}

/*
 * @routine 088C:035 CountDown088C_035
 * @run 70:B835
 * WHAT IT DOES: Counts OBJ_WORK_F; at zero sets it to 32 and counts the state (secret stage 04's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void CountDown088C_035(uint8_t slot)
{
    uint8_t left = DEC8(OBJ->work_f[slot]);

    abi_nz(left);
    if (left != 0) {
        return;
    }
    lda(0x20);
    WR8(OBJ->work_f[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_CountDown088C_035(void)
{
    CountDown088C_035(cpu.x);
}

/*
 * @routine 18DB:21D CountDown18DB_21D
 * @run 70:B21D
 * WHAT IT DOES: Counts OBJ_WORK_E; at zero zRasterStage 4 and the substate on (Stage 10's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void CountDown18DB_21D(uint8_t slot)
{
    uint8_t left = DEC8(OBJ->work_e[slot]);

    abi_nz(left);
    if (left != 0) {
        return;
    }
    lda(4);
    wr8(zRasterStage, cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_CountDown18DB_21D(void)
{
    CountDown18DB_21D(cpu.x);
}

/*
 * @routine 18DB:63F CountDown18DB_63F
 * @run 70:B63F
 * WHAT IT DOES: Counts OBJ_WORK_E; at zero the substate on (Stage 10's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void CountDown18DB_63F(uint8_t slot)
{
    uint8_t left = DEC8(OBJ->work_e[slot]);

    abi_nz(left);
    if (left != 0) {
        return;
    }
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_CountDown18DB_63F(void)
{
    CountDown18DB_63F(cpu.x);
}

/*
 * @routine 18DD:1DF CountDown18DD_1DF
 * @run 71:C1DF
 * WHAT IT DOES: Counts OBJ_WORK_E; at zero the state and the raster effect on (Stage 10's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void CountDown18DD_1DF(uint8_t slot)
{
    uint8_t left = DEC8(OBJ->work_e[slot]);

    abi_nz(left);
    if (left != 0) {
        return;
    }
    INC8(OBJ->state[slot]);
    abi_nz(INC8(ZP->raster_effect));
}

void abi_CountDown18DD_1DF(void)
{
    CountDown18DD_1DF(cpu.x);
}

/*
 * @routine 12D0:431 TileRowStep12D0_431
 * @run 6B:9C31
 * WHAT IT DOES: A tile handler of Stage 0C's overlay: zTemp02 from the table at $9C42 by row zTemp02 and column zTemp00.
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: Exit: A $81, N 1, Z 0; Y the index; C and V from the index add; X kept.
 */
void TileRowStep12D0_431(void)
{
    tile_row_step(0x9C42);
}

void abi_TileRowStep12D0_431(void)
{
    TileRowStep12D0_431();
}

/*
 * @routine 12D0:2DE TileRowStep12D0_2DE
 * @run 6B:9ADE
 * WHAT IT DOES: A tile handler of Stage 0C's overlay: zTemp02 from the table at $9BCF by row zTemp02 and column zTemp00.
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: Exit: A $81, N 1, Z 0; Y the index; C and V from the index add; X kept.
 */
void TileRowStep12D0_2DE(void)
{
    tile_row_step(0x9BCF);
}

void abi_TileRowStep12D0_2DE(void)
{
    TileRowStep12D0_2DE();
}

/*
 * @routine 12D0:2CD TileRowStep12D0_2CD
 * @run 6B:9ACD
 * WHAT IT DOES: A tile handler of Stage 0C's overlay: zTemp02 from the table at $9B8F by row zTemp02 and column zTemp00.
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: Exit: A $81, N 1, Z 0; Y the index; C and V from the index add; X kept.
 */
void TileRowStep12D0_2CD(void)
{
    tile_row_step(0x9B8F);
}

void abi_TileRowStep12D0_2CD(void)
{
    TileRowStep12D0_2CD();
}

/*
 * @routine 12D0:282 TileRowStep12D0_282
 * @run 6B:9A82
 * WHAT IT DOES: A tile handler of Stage 0C's overlay: zTemp02 from the table at $9B47 by row zTemp02 and column zTemp00.
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: Exit: A $81, N 1, Z 0; Y the index; C and V from the index add; X kept.
 */
void TileRowStep12D0_282(void)
{
    tile_row_step(0x9B47);
}

void abi_TileRowStep12D0_282(void)
{
    TileRowStep12D0_282();
}

/*
 * @routine 12D0:271 TileRowStep12D0_271
 * @run 6B:9A71
 * WHAT IT DOES: A tile handler of Stage 0C's overlay: zTemp02 from the table at $9B07 by row zTemp02 and column zTemp00.
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: Exit: A $81, N 1, Z 0; Y the index; C and V from the index add; X kept.
 */
void TileRowStep12D0_271(void)
{
    tile_row_step(0x9B07);
}

void abi_TileRowStep12D0_271(void)
{
    TileRowStep12D0_271();
}

/*
 * @routine 12D0:2C0 TileColumnAdd12D0_2C0
 * @run 6B:9AC0
 * WHAT IT DOES: A tile handler of Stage 0C's overlay: adds the column's byte of $9B87 to zTemp02.
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: Exit: A $81; Y the column; C and V from the add; X kept.
 */
void TileColumnAdd12D0_2C0(void)
{
    tile_column_add(0x9B87);
}

void abi_TileColumnAdd12D0_2C0(void)
{
    TileColumnAdd12D0_2C0();
}

/*
 * @routine 12D0:264 TileColumnAdd12D0_264
 * @run 6B:9A64
 * WHAT IT DOES: A tile handler of Stage 0C's overlay: adds the column's byte of $9AFF to zTemp02.
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: Exit: A $81; Y the column; C and V from the add; X kept.
 */
void TileColumnAdd12D0_264(void)
{
    tile_column_add(0x9AFF);
}

void abi_TileColumnAdd12D0_264(void)
{
    TileColumnAdd12D0_264();
}

/*
 * @routine 12DD:0FF AddWhileNotAt12DD
 * @run 71:C0FF
 * WHAT IT DOES: Steps OBJ_WORK_I by zTemp01 until it equals zTemp00 (Stage 0C's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void AddWhileNotAt12DD(uint8_t slot)
{
    lda(RD8(OBJ->work_i[slot]));
    cmp(cpu.a, RD8(ZP->temp00));
    if ((cpu.p & P_Z) != 0) {
        return;
    }
    lda(RD8(OBJ->work_i[slot]));
    carry(false);
    adc(RD8(ZP->temp01));
    WR8(OBJ->work_i[slot], cpu.a);
}

void abi_AddWhileNotAt12DD(void)
{
    AddWhileNotAt12DD(cpu.x);
}

/*
 * @routine 13FB:3CA StartOnScreen3_13FB
 * @run 70:B3CA
 * WHAT IT DOES: When the scroll reaches screen 3: OBJ_WORK_E 64, substate on (secret stage 0C's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void StartOnScreen3_13FB(uint8_t slot)
{
    lda(rd8(ScrollXHiScreen));
    cmp(cpu.a, 0x03);
    if (cpu.a != 0x03) {
        return;
    }
    lda(0x40);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_StartOnScreen3_13FB(void)
{
    StartOnScreen3_13FB(cpu.x);
}

/*
 * @routine 0B4D:02B StartOnSlot16Frame0B4D
 * @run 71:C02B
 * WHAT IT DOES: When slot $10 shows frame $39: OBJ_WORK_D $2E, state on (secret stage 06's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void StartOnSlot16Frame0B4D(uint8_t slot)
{
    lda(RD8(OBJ->frame[0x10]));
    cmp(cpu.a, 0x39);
    if (cpu.a != 0x39) {
        return;
    }
    lda(0x2E);
    WR8(OBJ->work_d[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_StartOnSlot16Frame0B4D(void)
{
    StartOnSlot16Frame0B4D(cpu.x);
}

/*
 * @routine 13FB:5A0 SubstateByWorkK13FB
 * @run 70:B5A0
 * WHAT IT DOES: Substate 4 while OBJ_WORK_K is set, else the substate on (secret stage 0C's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void SubstateByWorkK13FB(uint8_t slot)
{
    lda(RD8(OBJ->work_k[slot]));
    if (cpu.a != 0) {
        lda(0x04);
        WR8(OBJ->substate[slot], cpu.a);
        return;
    }
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SubstateByWorkK13FB(void)
{
    SubstateByWorkK13FB(cpu.x);
}

/*
 * @routine 12DF:016 Slot16FrameIn74To76_12DF
 * @run 71:D016
 * WHAT IT DOES: C set while slot $10's frame is $74 to $76 (Stage 0C's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: Exit: C the answer; A the frame; X, Y kept.
 */
void Slot16FrameIn74To76_12DF(void)
{
    lda(RD8(OBJ->frame[0x10]));
    cmp(cpu.a, 0x74);
    if (cpu.a >= 0x74) {
        cmp(cpu.a, 0x77);
        if (cpu.a < 0x77) {
            carry(true);
            return;
        }
    }
    carry(false);
}

void abi_Slot16FrameIn74To76_12DF(void)
{
    Slot16FrameIn74To76_12DF();
}

/*
 * @routine 10CC:350 Slot18HitIs6Or9_10CC
 * @run 70:BB50
 * WHAT IT DOES: C set while slot $12's hit is 6 or 9 (secret stage 0A's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: Exit: C the answer; A the hit; X, Y kept.
 */
void Slot18HitIs6Or9_10CC(void)
{
    lda(RD8(OBJ->hit_taken[0x12]));
    cmp(cpu.a, 0x06);
    if (cpu.a != 0x06) {
        cmp(cpu.a, 0x09);
        if (cpu.a != 0x09) {
            carry(false);
            return;
        }
    }
    carry(true);
}

void abi_Slot18HitIs6Or9_10CC(void)
{
    Slot18HitIs6Or9_10CC();
}

/*
 * @routine 16CB:63C Frame5Wait128_16CB
 * @run 70:B63C
 * WHAT IT DOES: OBJ_WORK_D 128, frame 5, substate on (secret stage 0E's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void Frame5Wait128_16CB(uint8_t slot)
{
    lda(0x80);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0x05);
    WR8(OBJ->frame[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_Frame5Wait128_16CB(void)
{
    Frame5Wait128_16CB(cpu.x);
}

/*
 * @routine 101E:087 HitToState5_101E
 * @run 71:C887
 * WHAT IT DOES: Once hit, state 5 (Stage 0A's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void HitToState5_101E(uint8_t slot)
{
    lda(RD8(OBJ->hit_status[slot]));
    lda((uint8_t)(cpu.a & 0x80));
    if (cpu.a != 0) {
        lda(0x05);
        WR8(OBJ->state[slot], cpu.a);
    }
}

void abi_HitToState5_101E(void)
{
    HitToState5_101E(cpu.x);
}

/*
 * @routine 0EBC:060 RasterByScreenSide0EBC
 * @run 70:B860
 * WHAT IT DOES: Raster effect $17 unless the object is left of the screen (Stage 09's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void RasterByScreenSide0EBC(uint8_t slot)
{
    lda(RD8(OBJ->x_hi[slot]));
    if ((cpu.a & 0x80) != 0) {
        WR8(ZP->raster_effect, 0);
        return;
    }
    lda(0x17);
    WR8(ZP->raster_effect, cpu.a);
}

void abi_RasterByScreenSide0EBC(void)
{
    RasterByScreenSide0EBC(cpu.x);
}

/*
 * @routine 088B:76A SetWorkDOfSlots18To21_088B
 * @run 70:B76A
 * WHAT IT DOES: OBJ_WORK_D 2 in slots $12 to $15 (secret stage 04's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: Exit: A 2, Y $16, N 0, Z 1, C 1; X kept.
 */
void SetWorkDOfSlots18To21_088B(void)
{
    cpu.y = 0x12;
    abi_nz(cpu.y);
    lda(0x02);
    do {
        WR8(OBJ->work_d[cpu.y], cpu.a);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x16);
    } while (cpu.y < 0x16);
}

void abi_SetWorkDOfSlots18To21_088B(void)
{
    SetWorkDOfSlots18To21_088B();
}

/*
 * @routine 18DD:0C5 LeftOfSlot16_18DD
 * @run 71:C0C5
 * WHAT IT DOES: A 1 when the object's X low is under slot $10's, else 0 (Stage 10's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A the answer, N 0, Z from it; C from the ROL; X kept.
 */
void LeftOfSlot16_18DD(uint8_t slot)
{
    uint8_t bit;

    lda(RD8(OBJ->x_lo[slot]));
    cmp(cpu.a, RD8(OBJ->x_lo[0x10]));
    bit = (cpu.p & P_C) != 0 ? 1 : 0;
    carry((cpu.a & 0x80) != 0);
    lda((uint8_t)((cpu.a << 1) | bit));
    lda((uint8_t)(cpu.a & 0x01));
    lda((uint8_t)(cpu.a ^ 0x01));
}

void abi_LeftOfSlot16_18DD(void)
{
    LeftOfSlot16_18DD(cpu.x);
}

/*
 * @routine 12DD:5C2 RasterFromLineBF_12DD
 * @run 71:C5C2
 * WHAT IT DOES: Raster effect $0B from line $BF, substate on (Stage 0C's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void RasterFromLineBF_12DD(uint8_t slot)
{
    lda(0x0B);
    WR8(ZP->raster_effect, cpu.a);
    lda(0xBF);
    WR8(ZP->raster_arg, cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_RasterFromLineBF_12DD(void)
{
    RasterFromLineBF_12DD(cpu.x);
}

/*
 * @routine 101E:07C HalveWorkKOnHit101E
 * @run 71:C87C
 * WHAT IT DOES: A hit of 5 or more halves OBJ_WORK_K (Stage 0A's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void HalveWorkKOnHit101E(uint8_t slot)
{
    lda(RD8(OBJ->hit_taken[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a >= 0x05) {
        uint8_t old = RD8(OBJ->work_k[slot]);

        WR8(OBJ->work_k[slot], (uint8_t)(old >> 1));
        lsr_flags(old);
    }
}

void abi_HalveWorkKOnHit101E(void)
{
    HalveWorkKOnHit101E(cpu.x);
}

/*
 * @routine 0EBD:714 LowerSlotInY20_0EBD
 * @run 71:C714
 * WHAT IT DOES: Moves the slot in Y 20 dots down, Y low only (Stage 09's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: Exit: A the new Y low, flags from the add; X, Y kept.
 */
void LowerSlotInY20_0EBD(void)
{
    lda(RD8(OBJ->y_lo[cpu.y]));
    carry(false);
    adc(0x14);
    WR8(OBJ->y_lo[cpu.y], cpu.a);
}

void abi_LowerSlotInY20_0EBD(void)
{
    LowerSlotInY20_0EBD();
}

/*
 * @routine 13FC:663 WaitThenSubstate13FC_663
 * @run 70:BE63
 * WHAT IT DOES: OBJ_WORK_I 240, substate on.
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void WaitThenSubstate13FC_663(uint8_t slot)
{
    lda(0xF0);
    WR8(OBJ->work_i[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_WaitThenSubstate13FC_663(void)
{
    WaitThenSubstate13FC_663(cpu.x);
}

/*
 * @routine 13FC:53A WaitThenSubstate13FC_53A
 * @run 70:BD3A
 * WHAT IT DOES: OBJ_WORK_I 64, substate on.
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void WaitThenSubstate13FC_53A(uint8_t slot)
{
    lda(0x40);
    WR8(OBJ->work_i[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_WaitThenSubstate13FC_53A(void)
{
    WaitThenSubstate13FC_53A(cpu.x);
}

/*
 * @routine 12DB:578 WaitThenSubstate12DB_578
 * @run 70:B578
 * WHAT IT DOES: OBJ_WORK_E 64, substate on.
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void WaitThenSubstate12DB_578(uint8_t slot)
{
    lda(0x40);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_WaitThenSubstate12DB_578(void)
{
    WaitThenSubstate12DB_578(cpu.x);
}

/*
 * @routine 101F:68F StateOnWhenSlot30Free101F
 * @run 71:D68F
 * WHAT IT DOES: Counts the state on once slot $1E is empty (Stage 0A's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void StateOnWhenSlot30Free101F(uint8_t slot)
{
    lda(RD8(OBJ->kind[0x1E]));
    if (cpu.a == 0) {
        abi_nz(INC8(OBJ->state[slot]));
    }
}

void abi_StateOnWhenSlot30Free101F(void)
{
    StateOnWhenSlot30Free101F(cpu.x);
}

/*
 * @routine 0EBC:18C LoopFlagBit00EBC_18C
 * @run 70:B98C
 * WHAT IT DOES: Clears bit 0 of OBJ_LOOP_FLAGS (Stage 09's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void LoopFlagBit00EBC_18C(uint8_t slot)
{
    lda(RD8(OBJ->loop_flags[slot]));
    lda((uint8_t)(cpu.a & 0xFE));
    WR8(OBJ->loop_flags[slot], cpu.a);
}

void abi_LoopFlagBit00EBC_18C(void)
{
    LoopFlagBit00EBC_18C(cpu.x);
}

/*
 * @routine 0EBC:183 LoopFlagBit00EBC_183
 * @run 70:B983
 * WHAT IT DOES: Sets bit 0 of OBJ_LOOP_FLAGS (Stage 09's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void LoopFlagBit00EBC_183(uint8_t slot)
{
    lda(RD8(OBJ->loop_flags[slot]));
    lda((uint8_t)(cpu.a | 0x01));
    WR8(OBJ->loop_flags[slot], cpu.a);
}

void abi_LoopFlagBit00EBC_183(void)
{
    LoopFlagBit00EBC_183(cpu.x);
}

/*
 * @routine 0EBC:0D5 CountWord_FG_0EBC
 * @run 70:B8D5
 * WHAT IT DOES: Counts the word OBJ_WORK_F-G on (Stage 09's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void CountWord_FG_0EBC(uint8_t slot)
{
    uint8_t low = INC8(OBJ->work_f[slot]);

    abi_nz(low);
    if (low == 0) {
        abi_nz(INC8(OBJ->work_g[slot]));
    }
}

void abi_CountWord_FG_0EBC(void)
{
    CountWord_FG_0EBC(cpu.x);
}

/*
 * @routine 18DD:165 Raster1A_18DD
 * @run 71:C165
 * WHAT IT DOES: Raster effect $1A, state on (Stage 10's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void Raster1A_18DD(uint8_t slot)
{
    lda(0x1A);
    WR8(ZP->raster_effect, cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_Raster1A_18DD(void)
{
    Raster1A_18DD(cpu.x);
}

/*
 * @routine 18DD:16D RasterBack_18DD
 * @run 71:C16D
 * WHAT IT DOES: The raster effect counted down, state on (Stage 10's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void RasterBack_18DD(uint8_t slot)
{
    DEC8(ZP->raster_effect);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_RasterBack_18DD(void)
{
    RasterBack_18DD(cpu.x);
}

/*
 * @routine 12DD:578 SubstateOn12DD
 * @run 71:C578
 * WHAT IT DOES: The substate counted on (Stage 0C's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last load, compare or count left them; X kept.
 */
void SubstateOn12DD(uint8_t slot)
{
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SubstateOn12DD(void)
{
    SubstateOn12DD(cpu.x);
}

/*
 * @routine 0E0C:0BC StoreYHigh0E0C
 * @run 70:B8BC
 * WHAT IT DOES: Stores A as the object's Y high (secret stage 08's overlay).
 * HOW: Loads, compares and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: all registers and flags kept.
 */
void StoreYHigh0E0C(uint8_t slot)
{
    WR8(OBJ->y_hi[slot], cpu.a);
}

void abi_StoreYHigh0E0C(void)
{
    StoreYHigh0E0C(cpu.x);
}
