/* The stage objects that wait for a hit: they take their numbers from their record and start a step when hit. */
#include "enemy_objects.h"
#include "stage_map.h"

enum {
    RECORD_SLOT_FIRST = 0x80, HIT_LIMIT = 5, STEP_TO_RUN = 2, NIBBLE = 0x0F, FIVE_BITS = 0x1F, TWO_BITS = 0x03,
    TIMER_FIRST = 18, TIMER_COUNT = 4, FIRST_RECORD_RETURN = 0x8AAC, SECOND_RECORD_RETURN = 0x8B04
};

/* A, the carry and the byte: one ROL of A with a carry in. */
static uint8_t rol_a(uint8_t a, bool *carry)
{
    uint8_t out = (uint8_t)((a << 1) | (*carry ? 1 : 0));

    *carry = (a & 0x80) != 0;
    return out;
}

/* What the code at the second trigger's label leaves: the hit-taken byte cleared, A as it was. */
static Ended clear_hit_taken(uint8_t x, Ended e)
{
    WR8(OBJ->hit_taken[x], 0);
    return e;
}

/*
 * @routine 088B:784 ClearStageTimers
 * @run 70:B784
 * WHAT IT DOES: Clears four of the stage's object timers (slots 18 to 21 of the life timer column).
 * HOW: Four stores of zero.
 * WAITS: no.
 * Entry: nothing. Exit: all registers and flags kept.
 */
void ClearStageTimers(void)
{
    unsigned i;

    for (i = 0; i < TIMER_COUNT; i++) {
        WR8(OBJ->life_timer[TIMER_FIRST + i], 0);
    }
}

void abi_ClearStageTimers(void)
{
    ClearStageTimers();
}

/*
 * @routine 050E:2A4 StageTriggerObject
 * @run 6B:8AA4
 * WHAT IT DOES: A stage object that is started by a hit. Its first frame loads its record and
 * keeps the record's byte split in three (a number, an angle and a count); every later frame, when
 * the pad overrides allow, hit-taken is under five and the hit-taken byte is set, it hands its two
 * numbers to the stage (zField83, zField87) and starts step 2.
 * HOW: LoadObjectRecord, then rotates of the record's byte; the later frames read PadOverrides
 * by the count.
 * Five stage overlays hold this code.
 * WAITS: no.
 * Entry: X the object. Exit: as the last path left it.
 */
Ended StageTriggerObject(uint8_t x)
{
    Ended e;
    Sum sum;
    uint8_t a;
    uint8_t y;
    uint8_t last;
    uint8_t nz;
    bool carry;

    a = RD8(OBJ->state[x]);
    if (a == 0) {
        jsr_push(FIRST_RECORD_RETURN);
        last = LoadObjectRecord(x, &nz, &carry);
        jsr_pop();
        (void)last;
        a = RD8(OBJ->hitbox_on[x]);
        sum = add_bytes(RECORD_SLOT_FIRST, a, false);
        WR8(OBJ->hitbox_on[x], sum.value);
        a = RD8(OBJ->next_kind[x]);
        y = a;
        a = (uint8_t)(a & NIBBLE);
        WR8(OBJ->f3510[x], a);
        a = y;
        carry = sum.carry;
        a = rol_a(a, &carry);
        a = rol_a(a, &carry);
        a = rol_a(a, &carry);
        y = a;
        a = (uint8_t)(a & TWO_BITS);
        WR8(OBJ->angle_hi[x], a);
        a = y;
        a = rol_a(a, &carry);
        a = rol_a(a, &carry);
        a = (uint8_t)(a & TWO_BITS);
        WR8(OBJ->life_timer[x], a);
        e = ended_with_y(ended_with_overflow(ended_with_carry(ended_load(a), carry), sum.overflow), y);
        return e;
    }
    WR8(OBJ->timer[x], 0);
    y = RD8(OBJ->life_timer[x]);
    a = rd8((uint16_t)(PadOverrides + y));
    e = ended_with_y(ended_load(a), y);
    if (a == 0) {
        return clear_hit_taken(x, e);
    }
    a = RD8(OBJ->hit_taken[x]);
    e = ended_with_y(ended_load(a), y);
    if (a == 0) {
        return clear_hit_taken(x, e);
    }
    e = ended_with_y(ended_compare(a, HIT_LIMIT), y);
    if (a >= HIT_LIMIT) {
        return clear_hit_taken(x, e);
    }
    a = RD8(OBJ->f3510[x]);
    WR8(ZP->field83, a);
    a = RD8(OBJ->angle_hi[x]);
    WR8(ZP->field87, a);
    WR8(ZP->field8c, 1);
    WR8(ZP->sub_step, STEP_TO_RUN);
    return ended_with_y(ended_with_carry(ended_load(STEP_TO_RUN), false), y);
}

void abi_StageTriggerObject(void)
{
    abi_ended(StageTriggerObject(cpu.x));
}

/*
 * @routine 092E:2FC StageTriggerObjectB
 * @run 6B:8AFC
 * WHAT IT DOES: StageTriggerObject's sibling for the stages that keep a five-bit number: the same first
 * frame (a five-bit number and a count) and, later, it starts step 2 with a clear flag when the player
 * may leave the top, and stops the object's speed.
 * HOW: As StageTriggerObject, with PlayerMayLeaveTop in place of PadOverrides.
 * Two stage overlays hold this code.
 * WAITS: no.
 * Entry: X the object. Exit: as the last path left it.
 */
Ended StageTriggerObjectB(uint8_t x)
{
    Ended e;
    Sum sum;
    uint8_t a;
    uint8_t y;
    uint8_t last;
    uint8_t nz;
    bool carry;

    a = RD8(OBJ->state[x]);
    if (a == 0) {
        jsr_push(SECOND_RECORD_RETURN);
        last = LoadObjectRecord(x, &nz, &carry);
        jsr_pop();
        (void)last;
        a = RD8(OBJ->hitbox_on[x]);
        sum = add_bytes(RECORD_SLOT_FIRST, a, false);
        WR8(OBJ->hitbox_on[x], sum.value);
        a = RD8(OBJ->next_kind[x]);
        y = a;
        a = (uint8_t)(a & FIVE_BITS);
        WR8(OBJ->f3510[x], a);
        a = y;
        carry = sum.carry;
        a = rol_a(a, &carry);
        a = rol_a(a, &carry);
        a = rol_a(a, &carry);
        a = (uint8_t)(a & TWO_BITS);
        WR8(OBJ->angle_hi[x], a);
        return ended_with_y(ended_with_overflow(ended_with_carry(ended_load(a), carry), sum.overflow), y);
    }
    WR8(OBJ->timer[x], 0);
    a = RD8(OBJ->next_kind[x]);
    e = ended_load(a);
    if ((a & 0x80) != 0) {
        /* Untraced: a negative number takes the same exit as a missing hit. */
        a = rd8(PlayerByte3BD7);
        e = ended_load(a);
        if (a == 0) {
            return clear_hit_taken(x, e);
        }
    }
    a = rd8(PlayerMayLeaveTop);
    a = (uint8_t)(a - 1);
    e = ended_load(a);
    if (a == 0) {
        return e;
    }
    a = RD8(OBJ->hit_taken[x]);
    e = ended_load(a);
    if (a == 0) {
        return clear_hit_taken(x, e);
    }
    e = ended_compare(a, HIT_LIMIT);
    if (a >= HIT_LIMIT) {
        return clear_hit_taken(x, e);
    }
    a = RD8(OBJ->f3510[x]);
    WR8(ZP->field83, a);
    a = RD8(OBJ->angle_hi[x]);
    WR8(ZP->field87, a);
    WR8(ZP->field8c, 0);
    WR8(ZP->sub_step, STEP_TO_RUN);
    WR8(OBJ->vx_hi[0], 0);
    WR8(OBJ->vx_lo[0], 0);
    return ended_with_carry(ended_load(STEP_TO_RUN), false);
}

void abi_StageTriggerObjectB(void)
{
    abi_ended(StageTriggerObjectB(cpu.x));
}
