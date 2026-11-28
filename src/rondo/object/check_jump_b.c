/*
 * Object dispatchers of stages $09 to $0E and the secret stages that make a check before the state's handler, and
 * the checks of single overlays.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "richter.h"      /* axis_move_exit_shim */
#include "check_jump_b.h"

static void lda(uint8_t value) { cpu.a = value; abi_nz(value); }
static void ldx(uint8_t value) { cpu.x = value; abi_nz(value); }
static void inx(void) { cpu.x = (uint8_t)(cpu.x + 1); abi_nz(cpu.x); }

static void cmp(uint8_t reg, uint8_t value)
{
    abi_flag(P_C, reg >= value);
    abi_nz((uint8_t)(reg - value));
}

static void flags_of(Sum sum)
{
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
}

/* The check, which is whatever the loaded overlay holds at its address, then the state's (or phase's) handler. */
static Jump check_then_jump(uint8_t slot, uint16_t check, uint16_t table, bool by_phase, uint16_t ret)
{
    JumpTableLayout layout = { table };

    call_routine(ret, check);
    return by_phase ? JumpByPhaseTableBody(slot, &layout) : JumpByStateTableBody(slot, &layout);
}

/*
 * @routine 0EBE:1A1 ToState6WhenHitBit7
 * @run 71:C9A1
 * WHAT IT DOES: Takes the object to state 6 once bit 7 of its OBJ_HIT_STATUS is set.
 * HOW: One test and store. Stage 0E's overlay holds the same bytes at $B7C6.
 * WAITS: no.
 * Entry: X object. Exit: A 0 (Z set) or 6; X, Y, C and V kept.
 */
void ToState6WhenHitBit7(uint8_t slot)
{
    lda(RD8(OBJ->hit_status[slot]));
    lda((uint8_t)(cpu.a & 0x80));
    if (cpu.a != 0) {
        lda(0x06);
        WR8(OBJ->state[slot], cpu.a);
    }
}

void abi_ToState6WhenHitBit7(void)
{
    ToState6WhenHitBit7(cpu.x);
}

/*
 * @routine 10CC:136 HitBoxByFrame10CC
 * @run 70:B936
 * WHAT IT DOES: Gives the object the hit box of its frame, from $94 on (secret stage 0A's overlay).
 * HOW: Four bytes of the table at $B958 by four times the frame less $94.
 * WAITS: no.
 * Entry: X object. Exit: A the half height, N and Z from it, Y the row's offset, C from the second shift, V from the
 * subtract.
 */
void HitBoxByFrame10CC(uint8_t slot)
{
    lda(RD8(OBJ->frame[slot]));
    flags_of(sub_bytes(cpu.a, 0x94, true));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xB958 + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(rd8((uint16_t)(0xB959 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    lda(rd8((uint16_t)(0xB95A + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(rd8((uint16_t)(0xB95B + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_HitBoxByFrame10CC(void)
{
    HitBoxByFrame10CC(cpu.x);
}

/*
 * @routine 0EC0:281 RiseWhenHitBit7_0EC0
 * @run 71:DA81
 * WHAT IT DOES: In states under 3, once bit 7 of its OBJ_HIT_STATUS is set: the hit box off, sprite set 3, frame 0,
 * an upward speed and state 3 (Stage 09's overlay).
 * HOW: Two tests and the stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last test, or A 3; C from the state's compare; X, Y and V kept.
 */
void RiseWhenHitBit7_0EC0(uint8_t slot)
{
    lda(RD8(OBJ->hit_status[slot]));
    lda((uint8_t)(cpu.a & 0x80));
    if (cpu.a == 0) {
        return;
    }
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x03);
    if (cpu.a >= 0x03) {
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
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_RiseWhenHitBit7_0EC0(void)
{
    RiseWhenHitBit7_0EC0(cpu.x);
}

/*
 * @routine 101D:2A2 HoldWhenHitBit7_101D
 * @run 71:C2A2
 * WHAT IT DOES: In states under 3, once bit 7 of its OBJ_HIT_STATUS is set: OBJ_WORK_D $40, frame $AA, state 3 and
 * the hit box off (Stage 0A's overlay).
 * HOW: Two tests and the stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last test, or A 3; C from the state's compare; X, Y and V kept.
 */
void HoldWhenHitBit7_101D(uint8_t slot)
{
    lda(RD8(OBJ->hit_status[slot]));
    lda((uint8_t)(cpu.a & 0x80));
    if (cpu.a == 0) {
        return;
    }
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x03);
    if (cpu.a >= 0x03) {
        return;
    }
    lda(0x40);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0xAA);
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->hitbox_on[slot], 0);
}

void abi_HoldWhenHitBit7_101D(void)
{
    HoldWhenHitBit7_101D(cpu.x);
}

/*
 * @routine 117F:016 StartPiecesWhenHit117F
 * @run 71:D016
 * WHAT IT DOES: Once bit 7 of its OBJ_HIT_STATUS is set (Stage 0B's overlay): the next slot becomes kind $9C, the six
 * after it get OBJ_WORK_N from the table at $D054 with their hit boxes off and pace bits kept to $87, and the object
 * goes to state 4 with no frame or hit box.
 * HOW: A loop over six slots, X kept on the stack.
 * WAITS: no.
 * Entry: X object, not yet in state 4 for anything to happen. Exit: A 4 or as the test; X kept, Y 6, C set.
 */
void StartPiecesWhenHit117F(uint8_t slot)
{
    lda(RD8(OBJ->hit_status[slot]));
    lda((uint8_t)(cpu.a & 0x80));
    if (cpu.a == 0) {
        return;
    }
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x04);
    if (cpu.a == 0x04) {
        return;
    }
    push8(cpu.x);
    inx();
    lda(0x9C);
    WR8(OBJ->kind[cpu.x], cpu.a);
    WR8(OBJ->state[cpu.x], 0);
    inx();
    cpu.y = 0;
    do {
        lda(rd8((uint16_t)(0xD054 + cpu.y)));
        WR8(OBJ->work_n[cpu.x], cpu.a);
        WR8(OBJ->hitbox_on[cpu.x], 0);
        lda(RD8(OBJ->pace[cpu.x]));
        lda((uint8_t)(cpu.a & 0x87));
        WR8(OBJ->pace[cpu.x], cpu.a);
        inx();
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x06);
    } while (cpu.y < 0x06);
    ldx(pull8());
    WR8(OBJ->hitbox_on[cpu.x], 0);
    WR8(OBJ->frame[cpu.x], 0);
    lda(0x04);
    WR8(OBJ->state[cpu.x], cpu.a);
}

void abi_StartPiecesWhenHit117F(void)
{
    StartPiecesWhenHit117F(cpu.x);
}

/*
 * @routine 13FB:7CE HoldNearLevelX13FB
 * @run 70:B7CE
 * WHAT IT DOES: Keeps the object's level X near $03xx (secret stage 0C's overlay): below $033D it moves half a dot
 * right, on another screen from $44 of the low byte half a dot left; its X velocity is then cleared.
 * HOW: The level X from the scroll and the object's screen X; one ApplyVelocityX.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare or the move; X kept.
 */
void HoldNearLevelX13FB(uint8_t slot)
{
    Sum sum;

    lda(rd8(ScrollXLo));
    flags_of(add_bytes(cpu.a, RD8(OBJ->x_lo[slot]), false));
    WR8(ZP->temp00, cpu.a);
    lda(rd8(ScrollXHiScreen));
    sum = add_bytes(cpu.a, RD8(OBJ->x_hi[slot]), (cpu.p & P_C) != 0);
    flags_of(sum);
    cmp(cpu.a, 0x03);
    if (cpu.a != 0x03) {
        lda(RD8(ZP->temp00));
        cmp(cpu.a, 0x44);
        if (cpu.a >= 0x44) {
            lda(0x80);
            WR8(OBJ->vx_lo[slot], cpu.a);
            lda(0xFF);
            WR8(OBJ->vx_hi[slot], cpu.a);
            jsr_push(0xB7F4);
            axis_move_exit_shim(ApplyVelocityX(slot));
            jsr_pop();
        }
    } else {
        lda(RD8(ZP->temp00));
        cmp(cpu.a, 0x3D);
        if (cpu.a < 0x3D) {
            lda(0x80);
            WR8(OBJ->vx_lo[slot], cpu.a);
            WR8(OBJ->vx_hi[slot], 0);
            jsr_push(0xB807);
            axis_move_exit_shim(ApplyVelocityX(slot));
            jsr_pop();
        }
    }
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
}

void abi_HoldNearLevelX13FB(void)
{
    HoldNearLevelX13FB(cpu.x);
}

/*
 * @routine 16CC:112 PlaceChain16CC
 * @run 70:B912
 * WHAT IT DOES: Places the chain of slots $15 down to $11 (secret stage 0A's overlay): the first from slot $16 with a
 * Y offset by slot $16's frame, each next from the slot above 8 dots down, each then moved by its angle's velocity.
 * HOW: CopyObjectPlaceBack, ApplyVelocityY, SetVelocityFromAngle with the angle OBJ_WORK_D (speed zTemp00-01), the
 * X velocity halved with its sign, ApplyVelocity.
 * WAITS: no.
 * Entry: nothing but X to keep. Exit: X restored, N and Z from it; C set; A, Y and V as the last calls left them.
 */
void PlaceChain16CC(uint8_t slot)
{
    uint8_t last;
    ScaleResult scaled;
    uint8_t old;

    push8(cpu.x);
    ldx(0x15);
    do {
        cmp(cpu.x, 0x15);
        if (cpu.x == 0x15) {
            cpu.y = 0x16;
            abi_nz(cpu.y);
            jsr_push(0xB91E);
            last = CopyObjectPlaceBack(cpu.y, cpu.x);
            copy_place_exit_shim(last);
            jsr_pop();
            lda(RD8(OBJ->frame[0x16]));
            flags_of(sub_bytes(cpu.a, 0x0C, true));
            cpu.y = cpu.a;
            lda(rd8((uint16_t)(0xB95B + cpu.y)));
            WR8(OBJ->vy_hi[cpu.x], cpu.a);
        } else {
            lda(cpu.x);
            cpu.y = cpu.a;
            cpu.y = (uint8_t)(cpu.y + 1);
            abi_nz(cpu.y);
            jsr_push(0xB933);
            last = CopyObjectPlaceBack(cpu.y, cpu.x);
            copy_place_exit_shim(last);
            jsr_pop();
            lda(0x08);
            WR8(OBJ->vy_hi[cpu.x], cpu.a);
        }
        jsr_push(0xB93B);
        axis_move_exit_shim(ApplyVelocityY(cpu.x));
        jsr_pop();
        WR8(ZP->temp00, 0);
        lda(0x06);
        WR8(ZP->temp01, cpu.a);
        lda(RD8(OBJ->work_d[cpu.x]));
        jsr_push(0xB947);
        scaled = SetVelocityFromAngle(cpu.a, cpu.x);
        scale_exit_shim(scaled);
        jsr_pop();
        lda(RD8(OBJ->vx_hi[cpu.x]));
        abi_flag(P_C, (cpu.a & 0x80) != 0);
        lda((uint8_t)(cpu.a << 1));
        old = RD8(OBJ->vx_hi[cpu.x]);
        WR8(OBJ->vx_hi[cpu.x], (uint8_t)((old >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
        abi_flag(P_C, (old & 0x01) != 0);
        old = RD8(OBJ->vx_lo[cpu.x]);
        WR8(OBJ->vx_lo[cpu.x], (uint8_t)((old >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
        abi_nz((uint8_t)((old >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
        abi_flag(P_C, (old & 0x01) != 0);
        jsr_push(0xB954);
        axis_move_exit_shim(ApplyVelocity(cpu.x));
        jsr_pop();
        cpu.x = (uint8_t)(cpu.x - 1);
        abi_nz(cpu.x);
        cmp(cpu.x, 0x10);
    } while (cpu.x != 0x10);
    ldx(pull8());
    (void)slot;
}

void abi_PlaceChain16CC(void)
{
    PlaceChain16CC(cpu.x);
}

/*
 * @routine 05CB:382 CheckThenJump05CB382
 * @run 70:B382
 * WHAT IT DOES: An object's dispatcher (Secret stage 02): the overlay's check at $B58D, then the handler of its
 * phase from the table at $B397.
 * HOW: JSR to the routine loaded at the check's address, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckThenJump05CB382(uint8_t slot)
{
    return check_then_jump(slot, 0xB58D, 0xB397, true, 0xB385);
}

void abi_CheckThenJump05CB382(void)
{
    Jump jump = CheckThenJump05CB382(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 09EC:03C CheckThenJump09EC03C
 * @run 70:B83C
 * WHAT IT DOES: An object's dispatcher (Secret stage 05): the overlay's check at $B859, then the handler of its
 * state from the table at $B851.
 * HOW: JSR to the routine loaded at the check's address, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckThenJump09EC03C(uint8_t slot)
{
    return check_then_jump(slot, 0xB859, 0xB851, false, 0xB83F);
}

void abi_CheckThenJump09EC03C(void)
{
    Jump jump = CheckThenJump09EC03C(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0E0C:0CC CheckThenJump0E0C0CC
 * @run 70:B8CC
 * WHAT IT DOES: An object's dispatcher (Secret stage 08): the overlay's check at $B8F1, then the handler of its
 * state from the table at $B8E1.
 * HOW: JSR to the routine loaded at the check's address, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckThenJump0E0C0CC(uint8_t slot)
{
    return check_then_jump(slot, 0xB8F1, 0xB8E1, false, 0xB8CF);
}

void abi_CheckThenJump0E0C0CC(void)
{
    Jump jump = CheckThenJump0E0C0CC(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0EBE:17E CheckThenJump0EBE17E
 * @run 71:C97E
 * WHAT IT DOES: An object's dispatcher (Stage 09): the overlay's check at $C9A1, then the handler of its
 * state from the table at $C993.
 * HOW: JSR to the routine loaded at the check's address, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckThenJump0EBE17E(uint8_t slot)
{
    return check_then_jump(slot, 0xC9A1, 0xC993, false, 0xC981);
}

void abi_CheckThenJump0EBE17E(void)
{
    Jump jump = CheckThenJump0EBE17E(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0EBE:652 CheckThenJump0EBE652
 * @run 71:CE52
 * WHAT IT DOES: An object's dispatcher (Stage 09): the overlay's check at $CE6D, then the handler of its
 * state from the table at $CE67.
 * HOW: JSR to the routine loaded at the check's address, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckThenJump0EBE652(uint8_t slot)
{
    return check_then_jump(slot, 0xCE6D, 0xCE67, false, 0xCE55);
}

void abi_CheckThenJump0EBE652(void)
{
    Jump jump = CheckThenJump0EBE652(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0EBF:472 CheckThenJump0EBF472
 * @run 71:D472
 * WHAT IT DOES: An object's dispatcher (Stage 09): the overlay's check at $D491, then the handler of its
 * state from the table at $D487.
 * HOW: JSR to the routine loaded at the check's address, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckThenJump0EBF472(uint8_t slot)
{
    return check_then_jump(slot, 0xD491, 0xD487, false, 0xD475);
}

void abi_CheckThenJump0EBF472(void)
{
    Jump jump = CheckThenJump0EBF472(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0EC0:264 CheckThenJump0EC0264
 * @run 71:DA64
 * WHAT IT DOES: An object's dispatcher (Stage 09): the overlay's check at $DA81, then the handler of its
 * state from the table at $DA79.
 * HOW: JSR to the routine loaded at the check's address, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckThenJump0EC0264(uint8_t slot)
{
    return check_then_jump(slot, 0xDA81, 0xDA79, false, 0xDA67);
}

void abi_CheckThenJump0EC0264(void)
{
    Jump jump = CheckThenJump0EC0264(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 101D:285 CheckThenJump101D285
 * @run 71:C285
 * WHAT IT DOES: An object's dispatcher (Stage 0A): the overlay's check at $C2A2, then the handler of its
 * state from the table at $C29A.
 * HOW: JSR to the routine loaded at the check's address, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckThenJump101D285(uint8_t slot)
{
    return check_then_jump(slot, 0xC2A2, 0xC29A, false, 0xC288);
}

void abi_CheckThenJump101D285(void)
{
    Jump jump = CheckThenJump101D285(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 10CC:10D CheckThenJump10CC10D
 * @run 70:B90D
 * WHAT IT DOES: An object's dispatcher (Secret stage 0A): the overlay's check at $B936, then the handler of its
 * phase from the table at $B922.
 * HOW: JSR to the routine loaded at the check's address, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckThenJump10CC10D(uint8_t slot)
{
    return check_then_jump(slot, 0xB936, 0xB922, true, 0xB910);
}

void abi_CheckThenJump10CC10D(void)
{
    Jump jump = CheckThenJump10CC10D(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117C:0CB CheckThenJump117C0CB
 * @run 70:B8CB
 * WHAT IT DOES: An object's dispatcher (Stage 0B): the overlay's check at $B8EA, then the handler of its
 * state from the table at $B8E0.
 * HOW: JSR to the routine loaded at the check's address, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckThenJump117C0CB(uint8_t slot)
{
    return check_then_jump(slot, 0xB8EA, 0xB8E0, false, 0xB8CE);
}

void abi_CheckThenJump117C0CB(void)
{
    Jump jump = CheckThenJump117C0CB(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117D:39D CheckThenJump117D39D
 * @run 71:C39D
 * WHAT IT DOES: An object's dispatcher (Stage 0B): the overlay's check at $C3B6, then the handler of its
 * state from the table at $C3B2.
 * HOW: JSR to the routine loaded at the check's address, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckThenJump117D39D(uint8_t slot)
{
    return check_then_jump(slot, 0xC3B6, 0xC3B2, false, 0xC3A0);
}

void abi_CheckThenJump117D39D(void)
{
    Jump jump = CheckThenJump117D39D(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117E:7F7 CheckThenJump117E7F7
 * @run 71:CFF7
 * WHAT IT DOES: An object's dispatcher (Stage 0B): the overlay's check at $D016, then the handler of its
 * state from the table at $D00C.
 * HOW: JSR to the routine loaded at the check's address, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckThenJump117E7F7(uint8_t slot)
{
    return check_then_jump(slot, 0xD016, 0xD00C, false, 0xCFFA);
}

void abi_CheckThenJump117E7F7(void)
{
    Jump jump = CheckThenJump117E7F7(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 13FB:7AF CheckThenJump13FB7AF
 * @run 70:B7AF
 * WHAT IT DOES: An object's dispatcher (Secret stage 0C): the overlay's check at $B7CE, then the handler of its
 * phase from the table at $B7C4.
 * HOW: JSR to the routine loaded at the check's address, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckThenJump13FB7AF(uint8_t slot)
{
    return check_then_jump(slot, 0xB7CE, 0xB7C4, true, 0xB7B2);
}

void abi_CheckThenJump13FB7AF(void)
{
    Jump jump = CheckThenJump13FB7AF(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 161B:7A3 CheckThenJump161B7A3
 * @run 70:B7A3
 * WHAT IT DOES: An object's dispatcher (Stage 0E): the overlay's check at $B7C6, then the handler of its
 * state from the table at $B7B8.
 * HOW: JSR to the routine loaded at the check's address, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckThenJump161B7A3(uint8_t slot)
{
    return check_then_jump(slot, 0xB7C6, 0xB7B8, false, 0xB7A6);
}

void abi_CheckThenJump161B7A3(void)
{
    Jump jump = CheckThenJump161B7A3(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 16CC:165 CheckThenJump16CC165
 * @run 70:B965
 * WHAT IT DOES: An object's dispatcher (Secret stage 0E): the overlay's check at $B912, then the handler of its
 * phase from the table at $B97A.
 * HOW: JSR to the routine loaded at the check's address, then the table jump.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
 */
Jump CheckThenJump16CC165(uint8_t slot)
{
    return check_then_jump(slot, 0xB912, 0xB97A, true, 0xB968);
}

void abi_CheckThenJump16CC165(void)
{
    Jump jump = CheckThenJump16CC165(cpu.x);

    jump_shim(jump.end, jump.target);
}
