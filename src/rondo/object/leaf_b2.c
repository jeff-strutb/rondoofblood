/*
 * More routines of single stage overlays that call nothing: speeds and hit boxes from tables, a place from the level
 * position, a screen tile's address.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "leaf_b2.h"

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

/* ASL of A. */
static void asl(void)
{
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
}

/*
 * @routine 117B:14A PlaceAtLevelWhenCount117B
 * @run 70:B14A
 * WHAT IT DOES: With OBJ_WORK_F 0 and OBJ_WORK_G-H at the count the table at $B112 gives for its start argument, puts the object at
 * the level X kept in OBJ_WORK_J-K (Stage 0B's overlay).
 * HOW: Loads, adds and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare or the subtract; Y twice the start argument.
 */
void PlaceAtLevelWhenCount117B(uint8_t slot)
{
    lda(RD8(OBJ->work_f[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    asl();
    cpu.y = cpu.a;
    lda(RD8(OBJ->work_h[slot]));
    cmp(cpu.a, rd8((uint16_t)(0xB112 + cpu.y)));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(RD8(OBJ->work_g[slot]));
    cmp(cpu.a, rd8((uint16_t)(0xB113 + cpu.y)));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(RD8(OBJ->work_k[slot]));
    abi_flag(P_C, true);
    sbc(rd8(ScrollXLo));
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(RD8(OBJ->work_j[slot]));
    sbc(rd8(ScrollXHiScreen));
    WR8(OBJ->x_hi[slot], cpu.a);
}

void abi_PlaceAtLevelWhenCount117B(void)
{
    PlaceAtLevelWhenCount117B(cpu.x);
}

/*
 * @routine 0EBE:107 SpeedByFacingAndStep0EBE
 * @run 71:C907
 * WHAT IT DOES: Gives the object the speeds of its facing and OBJ_WORK_D from the tables at $C934, and counts its state on
 * (Stage 09's overlay).
 * HOW: Loads, adds and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A the Y speed's low byte, N and Z from the new state, Y the row.
 */
void SpeedByFacingAndStep0EBE(uint8_t slot)
{
    cpu.y = RD8(OBJ->facing_left[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xC934 + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    asl();
    asl();
    abi_flag(P_C, false);
    adc(RD8(ZP->temp00));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xC937 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC936 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xC939 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC938 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_SpeedByFacingAndStep0EBE(void)
{
    SpeedByFacingAndStep0EBE(cpu.x);
}

/*
 * @routine 13FD:3F2 SwingStep13FD
 * @run 71:C3F2
 * WHAT IT DOES: Counts OBJ_WORK_F on by 4, gives zero page $9E the byte at $C41E its bit 7 picks, and adds half a step to
 * OBJ_WORK_G-H up to $18 (secret stage 0C's overlay).
 * HOW: Loads, adds and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the compare or the add; Y 0 or 1.
 */
void SwingStep13FD(uint8_t slot)
{
    lda(RD8(OBJ->work_f[slot]));
    abi_flag(P_C, false);
    adc(0x04);
    WR8(OBJ->work_f[slot], cpu.a);
    {
        bool out = (cpu.a & 0x80) != 0;

        lda((uint8_t)((cpu.a << 1) | ((cpu.p & P_C) != 0 ? 1 : 0)));
        abi_flag(P_C, out);
        out = (cpu.a & 0x80) != 0;
        lda((uint8_t)((cpu.a << 1) | ((cpu.p & P_C) != 0 ? 1 : 0)));
        abi_flag(P_C, out);
    }
    lda((uint8_t)(cpu.a & 0x01));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xC41E + cpu.y)));
    wr8(zRasterStage, cpu.a);
    lda(RD8(OBJ->work_g[slot]));
    cmp(cpu.a, 0x18);
    if (cpu.a >= 0x18) {
        return;
    }
    lda(RD8(OBJ->work_h[slot]));
    abi_flag(P_C, false);
    adc(0x80);
    WR8(OBJ->work_h[slot], cpu.a);
    lda(RD8(OBJ->work_g[slot]));
    adc(0x00);
    WR8(OBJ->work_g[slot], cpu.a);
}

void abi_SwingStep13FD(void)
{
    SwingStep13FD(cpu.x);
}

/*
 * @routine 12DC:35C HitStopToState6_12DC
 * @run 70:BB5C
 * WHAT IT DOES: Under state 6, once hit: stopped with no hit box, the animation restarted, state 6 (Stage 0C's overlay).
 * HOW: Loads, adds and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test or load; C from the state's compare.
 */
void HitStopToState6_12DC(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x06);
    if (cpu.a >= 0x06) {
        return;
    }
    lda(RD8(OBJ->hit_status[slot]));
    lda((uint8_t)(cpu.a & 0x80));
    if (cpu.a == 0) {
        return;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x06);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_HitStopToState6_12DC(void)
{
    HitStopToState6_12DC(cpu.x);
}

/*
 * @routine 117B:26F SpeedXTowardTarget117B
 * @run 70:B26F
 * WHAT IT DOES: Adds the step the table at $B1D0 gives for OBJ_WORK_F to the X speed until it is the target at $B1CC (Stage 0B's
 * overlay).
 * HOW: Loads, adds and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare or the add; Y twice OBJ_WORK_F.
 */
void SpeedXTowardTarget117B(uint8_t slot)
{
    lda(RD8(OBJ->work_f[slot]));
    asl();
    cpu.y = cpu.a;
    lda(RD8(OBJ->vx_lo[slot]));
    cmp(cpu.a, rd8((uint16_t)(0xB1CC + cpu.y)));
    if ((cpu.p & P_Z) != 0) {
        lda(RD8(OBJ->vx_hi[slot]));
        cmp(cpu.a, rd8((uint16_t)(0xB1CD + cpu.y)));
        if ((cpu.p & P_Z) != 0) {
            return;
        }
    }
    lda(RD8(OBJ->vx_lo[slot]));
    abi_flag(P_C, false);
    adc(rd8((uint16_t)(0xB1D0 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    adc(rd8((uint16_t)(0xB1D1 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
}

void abi_SpeedXTowardTarget117B(void)
{
    SpeedXTowardTarget117B(cpu.x);
}

/*
 * @routine 117F:2D4 HitBoxByFrameRow117F
 * @run 71:D2D4
 * WHAT IT DOES: Gives the object the hit box of its frame's row, 8 wider (Stage 0B's overlay).
 * HOW: Loads, adds and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A the half height, N and Z from it, Y the row's offset, C and V from the add.
 */
void HitBoxByFrameRow117F(uint8_t slot)
{
    lda(RD8(OBJ->frame[slot]));
    abi_flag(P_C, true);
    sbc(0x02);
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xD2FD + cpu.y)));
    asl();
    asl();
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xD8A6 + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(rd8((uint16_t)(0xD8A7 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    lda(rd8((uint16_t)(0xD8A8 + cpu.y)));
    abi_flag(P_C, false);
    adc(0x08);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(rd8((uint16_t)(0xD8A9 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_HitBoxByFrameRow117F(void)
{
    HitBoxByFrameRow117F(cpu.x);
}

/*
 * @routine 18DB:483 LaunchByFacing18DB
 * @run 70:B483
 * WHAT IT DOES: The hit box off, the animation restarted, frame $11, OBJ_WORK_E 32 and the X speed of its facing; the substate is
 * counted on (Stage 10's overlay).
 * HOW: Loads, adds and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A the speed's high byte, N and Z from the new substate, Y twice the facing.
 */
void LaunchByFacing18DB(uint8_t slot)
{
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    lda(0x11);
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x20);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(RD8(OBJ->facing_left[slot]));
    asl();
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xB4AB + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xB4AC + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_LaunchByFacing18DB(void)
{
    LaunchByFacing18DB(cpu.x);
}

/*
 * @routine 0ECB:558 ScreenTileAddress0ECB
 * @run 74:D558
 * WHAT IT DOES: The tile map address (zTemp07 high, zTemp05 low) of the screen point zTemp00-01 along X, zTemp02-03 along Y, with
 * the scroll added (Stage 09's overlay).
 * HOW: Loads, adds and stores in turn.
 * WAITS: no.
 * Entry: Exit: A zTemp05's byte, N, Z, C and V from its add; X and Y kept.
 */
void ScreenTileAddress0ECB(void)
{
    uint8_t old;

    lda(RD8(ZP->temp00));
    abi_flag(P_C, false);
    adc(rd8(ScrollXLo));
    WR8(ZP->temp04, cpu.a);
    lda(RD8(ZP->temp01));
    adc(rd8(ScrollXHi));
    lda((uint8_t)(cpu.a & 0x01));
    abi_flag(P_C, (cpu.a & 0x01) != 0);
    lda((uint8_t)(cpu.a >> 1));
    lda(RD8(ZP->temp04));
    old = cpu.a;
    lda((uint8_t)((cpu.a >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
    abi_flag(P_C, (old & 0x01) != 0);
    abi_flag(P_C, (cpu.a & 0x01) != 0);
    lda((uint8_t)(cpu.a >> 1));
    abi_flag(P_C, (cpu.a & 0x01) != 0);
    lda((uint8_t)(cpu.a >> 1));
    lda((uint8_t)(cpu.a & 0x3F));
    WR8(ZP->temp05, cpu.a);
    lda(RD8(ZP->temp02));
    abi_flag(P_C, false);
    adc(rd8(ScrollYLo));
    WR8(ZP->temp06, cpu.a);
    lda(RD8(ZP->temp03));
    adc(rd8(ScrollYHi));
    lda((uint8_t)(cpu.a & 0x01));
    WR8(ZP->temp07, cpu.a);
    lda(RD8(ZP->temp06));
    for (int i = 0; i < 3; i++) {
        asl();
        old = RD8(ZP->temp07);
        WR8(ZP->temp07, (uint8_t)((old << 1) | ((cpu.p & P_C) != 0 ? 1 : 0)));
        abi_nz((uint8_t)((old << 1) | ((cpu.p & P_C) != 0 ? 1 : 0)));
        abi_flag(P_C, (old & 0x80) != 0);
    }
    lda((uint8_t)(cpu.a & 0xC0));
    abi_flag(P_C, false);
    adc(RD8(ZP->temp05));
    WR8(ZP->temp05, cpu.a);
}

void abi_ScreenTileAddress0ECB(void)
{
    ScreenTileAddress0ECB();
}
