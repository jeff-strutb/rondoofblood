/*
 * More checks of stages $09 to $10 and the secret stages on their objects.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l8.h"
#include "hit_checks_b2.h"

static void lda(uint8_t value) { cpu.a = value; abi_nz(value); }
static void ldy(uint8_t value) { cpu.y = value; abi_nz(value); }
static void iny(void) { ldy((uint8_t)(cpu.y + 1)); }

static void cmp(uint8_t reg, uint8_t value)
{
    abi_flag(P_C, reg >= value);
    abi_nz((uint8_t)(reg - value));
}

static void clear_slot_in_y(uint16_t ret)
{
    jsr_push(ret);
    ClearObjectInY(cpu.y);
    jsr_pop();
}

/* LDA of OBJ_HIT_STATUS and AND #$80: true when bit 7 is set. */
static bool hit_bit(uint8_t slot)
{
    lda(RD8(OBJ->hit_status[slot]));
    lda((uint8_t)(cpu.a & 0x80));
    return cpu.a != 0;
}

/* LDA of OBJ_STATE and CMP: true when the state is under the limit. */
static bool state_under(uint8_t slot, uint8_t limit)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, limit);
    return cpu.a < limit;
}

static void sta_state(uint8_t slot, uint8_t value)
{
    lda(value);
    WR8(OBJ->state[slot], cpu.a);
}


/*
 * @routine 0EBC:5FF ToState6WhenHitOrAway0EBC
 * @run 70:BDFF
 * WHAT IT DOES: To state 6 once hit, or off the screen's height: Y high positive with Y low from $80, or negative with Y low
 * under $80 (Stage 09's overlay).
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test or load left them; X kept.
 */
void ToState6WhenHitOrAway0EBC(uint8_t slot)
{
    if (!hit_bit(slot)) {
        lda(RD8(OBJ->y_hi[slot]));
        if (cpu.a == 0) {
            return;
        }
        if ((cpu.a & 0x80) == 0) {
            lda(RD8(OBJ->y_lo[slot]));
            if ((cpu.a & 0x80) == 0) {
                return;
            }
        } else {
            lda(RD8(OBJ->y_lo[slot]));
            if ((cpu.a & 0x80) != 0) {
                return;
            }
        }
    }
    sta_state(slot, 6);
}

void abi_ToState6WhenHitOrAway0EBC(void)
{
    ToState6WhenHitOrAway0EBC(cpu.x);
}

/*
 * @routine 0EBD:248 HitBecomesKind38_0EBD
 * @run 71:C248
 * WHAT IT DOES: Once hit (Stage 09's overlay): the slot before records its event and is cleared; this one becomes kind $38 in
 * state 6 and goes on at the routine loaded at $C97E.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the routine at $C97E, or A 0 and Z when not hit; X kept.
 */
void HitBecomesKind38_0EBD(uint8_t slot)
{
    uint8_t stored, event, byte;
    bool marked;

    if (!hit_bit(slot)) {
        return;
    }
    cpu.x = (uint8_t)(cpu.x - 1);
    abi_nz(cpu.x);
    jsr_push(0xC253);
    marked = RecordObjectEvent(cpu.x, &stored, &event, &byte);
    abi_ended(l8_of_event(marked, stored, event, byte));
    jsr_pop();
    jsr_push(0xC256);
    ClearObject(cpu.x);
    jsr_pop();
    cpu.x = (uint8_t)(cpu.x + 1);
    abi_nz(cpu.x);
    lda(0x38);
    WR8(OBJ->kind[cpu.x], cpu.a);
    sta_state(cpu.x, 6);
    WR8(OBJ->frame[cpu.x], 0);
    run_routine(0xC97E);
}

void abi_HitBecomesKind38_0EBD(void)
{
    HitBecomesKind38_0EBD(cpu.x);
}

/*
 * @routine 13FC:1EA KeepNearScreen13FC
 * @run 70:B9EA
 * WHAT IT DOES: Brings the object back to the screen next to the first one: an X or Y high byte of -2 becomes -1, of 2 becomes 1;
 * one at a time (secret stage 0C's overlay).
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A the high byte or its new value, as the last compare; X kept.
 */
void KeepNearScreen13FC(uint8_t slot)
{
    lda(RD8(OBJ->x_hi[slot]));
    cmp(cpu.a, 0xFE);
    if (cpu.a == 0xFE) {
        lda(0xFF);
        WR8(OBJ->x_hi[slot], cpu.a);
        return;
    }
    cmp(cpu.a, 0x02);
    if (cpu.a == 0x02) {
        lda(0x01);
        WR8(OBJ->x_hi[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->y_hi[slot]));
    cmp(cpu.a, 0xFE);
    if (cpu.a == 0xFE) {
        lda(0xFF);
        WR8(OBJ->y_hi[slot], cpu.a);
        return;
    }
    cmp(cpu.a, 0x02);
    if (cpu.a == 0x02) {
        lda(0x01);
        WR8(OBJ->y_hi[slot], cpu.a);
    }
}

void abi_KeepNearScreen13FC(void)
{
    KeepNearScreen13FC(cpu.x);
}

/*
 * @routine 13FC:218 ToState4WhenSlot16Marked13FC
 * @run 70:BA18
 * WHAT IT DOES: Under state 4, once bit 7 of slot $10's OBJ_HIT_STATUS is set: state 4, substate 0 (secret stage 0C's overlay).
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test or load left them; X kept.
 */
void ToState4WhenSlot16Marked13FC(uint8_t slot)
{
    if (!state_under(slot, 4)) {
        return;
    }
    lda(RD8(OBJ->hit_status[0x10]));
    lda((uint8_t)(cpu.a & 0x80));
    if (cpu.a == 0) {
        return;
    }
    sta_state(slot, 4);
    WR8(OBJ->substate[slot], 0);
}

void abi_ToState4WhenSlot16Marked13FC(void)
{
    ToState4WhenSlot16Marked13FC(cpu.x);
}

/*
 * @routine 13FC:22F HitBoxByFrameRange13FC
 * @run 70:BA2F
 * WHAT IT DOES: A long hit box below the object while its frame is $44 to $47, else a small one (secret stage 0C's overlay).
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A the last value stored, N and Z from it; C set only for frames from $48; X kept.
 */
void HitBoxByFrameRange13FC(uint8_t slot)
{
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x44);
    if (cpu.a >= 0x44) {
        cmp(cpu.a, 0x48);
        if (cpu.a < 0x48) {
            lda(0x50);
            WR8(OBJ->hitbox_dy[slot], cpu.a);
            lda(0x0C);
            WR8(OBJ->hitbox_half_w[slot], cpu.a);
            lda(0x40);
            WR8(OBJ->hitbox_half_h[slot], cpu.a);
            return;
        }
    }
    WR8(OBJ->hitbox_dy[slot], 0);
    lda(0x08);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_HitBoxByFrameRange13FC(void)
{
    HitBoxByFrameRange13FC(cpu.x);
}

/*
 * @routine 161D:02A StartByPlayerSide161D
 * @run 71:C02A
 * WHAT IT DOES: When the player is on the object's side (X low from $80 with OBJ_START_ARG 0, under $70 else) and the table at $C058
 * allows its state: OBJ_WORK_M counted, the animation restarted, state 3 (Stage 0E's overlay).
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test or load left them; X kept.
 */
void StartByPlayerSide161D(uint8_t slot)
{
    lda(RD8(OBJ->start_arg[slot]));
    if (cpu.a == 0) {
        lda(RD8(OBJ->x_lo[0]));
        cmp(cpu.a, 0x80);
        if (cpu.a < 0x80) {
            return;
        }
    } else {
        lda(RD8(OBJ->x_lo[0]));
        cmp(cpu.a, 0x70);
        if (cpu.a >= 0x70) {
            return;
        }
    }
    cpu.y = RD8(OBJ->state[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xC058 + cpu.y)));
    if (cpu.a == 0) {
        return;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    INC8(OBJ->work_m[slot]);
    sta_state(slot, 3);
    WR8(OBJ->substate[slot], 0);
}

void abi_StartByPlayerSide161D(void)
{
    StartByPlayerSide161D(cpu.x);
}

/*
 * @routine 161D:060 HitStopToState5_161D
 * @run 71:C060
 * WHAT IT DOES: Under state 5, once hit (Stage 0E's overlay): no vertical speed, no hit box, state 5.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test or load left them; X kept.
 */
void HitStopToState5_161D(uint8_t slot)
{
    if (!state_under(slot, 5) || !hit_bit(slot)) {
        return;
    }
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
    sta_state(slot, 5);
    WR8(OBJ->substate[slot], 0);
}

void abi_HitStopToState5_161D(void)
{
    HitStopToState5_161D(cpu.x);
}

/*
 * @routine 161D:5CE FaceThePlayerEvery8_161D
 * @run 71:C5CE
 * WHAT IT DOES: Every eighth count of OBJ_WORK_F, in the states the table at $C5E8 marks, turns the object to the player
 * (Stage 0E's overlay).
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test or load left them; X kept.
 */
void FaceThePlayerEvery8_161D(uint8_t slot)
{
    lda(RD8(OBJ->work_f[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    if (cpu.a != 0) {
        return;
    }
    cpu.y = RD8(OBJ->state[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xC5E8 + cpu.y)));
    if (cpu.a == 0) {
        return;
    }
    jsr_push(0xC5E0);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x01));
    lda((uint8_t)(cpu.a ^ 0x01));
    WR8(OBJ->facing_left[slot], cpu.a);
}

void abi_FaceThePlayerEvery8_161D(void)
{
    FaceThePlayerEvery8_161D(cpu.x);
}

/*
 * @routine 161D:5F8 HalveWhenHit161D
 * @run 71:C5F8
 * WHAT IT DOES: Outside state 4, a hit of 5 or more halves OBJ_WORK_H (Stage 0E's overlay).
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare or the shift; X kept.
 */
void HalveWhenHit161D(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x04);
    if (cpu.a == 0x04) {
        return;
    }
    lda(RD8(OBJ->hit_taken[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a >= 0x05) {
        uint8_t old = RD8(OBJ->work_h[slot]);

        WR8(OBJ->work_h[slot], (uint8_t)(old >> 1));
        abi_flag(P_C, (old & 1) != 0);
        abi_nz((uint8_t)(old >> 1));
    }
}

void abi_HalveWhenHit161D(void)
{
    HalveWhenHit161D(cpu.x);
}

/*
 * @routine 161D:60A HitStopAndClearNext161D
 * @run 71:C60A
 * WHAT IT DOES: Once hit, under state 5 (Stage 0E's overlay): the next slot cleared, the hit box off, the animation restarted,
 * state 5.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test or load; Y the next slot when taken; X kept.
 */
void HitStopAndClearNext161D(uint8_t slot)
{
    if (!hit_bit(slot) || !state_under(slot, 5)) {
        return;
    }
    lda(cpu.x);
    ldy(cpu.a);
    iny();
    clear_slot_in_y(0xC61E);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    sta_state(slot, 5);
    WR8(OBJ->substate[slot], 0);
}

void abi_HitStopAndClearNext161D(void)
{
    HitStopAndClearNext161D(cpu.x);
}

/*
 * @routine 18DB:2C4 CountStateWhenHit18DB
 * @run 70:B2C4
 * WHAT IT DOES: Once hit, the object's state is counted on with substate 0 (Stage 10's overlay); C says whether.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: C set when taken; N and Z from the new state, else A 0 and Z; X kept.
 */
void CountStateWhenHit18DB(uint8_t slot)
{
    if (hit_bit(slot)) {
        abi_nz(INC8(OBJ->state[slot]));
        WR8(OBJ->substate[slot], 0);
        abi_flag(P_C, true);
        return;
    }
    abi_flag(P_C, false);
}

void abi_CountStateWhenHit18DB(void)
{
    CountStateWhenHit18DB(cpu.x);
}
