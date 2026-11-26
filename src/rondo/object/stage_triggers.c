/* The stage objects that wait for a hit: they take their numbers from their record and start a step when hit. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
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

/* How a trigger's frame ended, which decides what its exit shows. */
typedef enum { TRIGGER_RECORD_LOADED, TRIGGER_NOT_ALLOWED, TRIGGER_NOT_HIT, TRIGGER_HIT_TOO_OFTEN, TRIGGER_STARTED } TriggerHow;

typedef struct {
    TriggerHow how;
    uint8_t a;       /* the byte the last load or compare worked on */
    uint8_t y;       /* Y where the path sets it */
    bool y_set;
    bool carry;      /* the carry the first frame's rotates left */
    bool overflow;   /* the overflow of the first frame's add */
} TriggerEnd;

/* What each way of ending leaves in A, N, Z, C, V and Y. */
static void trigger_exit_shim(TriggerEnd end)
{
    cpu.a = end.a;
    switch (end.how) {
    case TRIGGER_RECORD_LOADED:
        abi_nz(end.a);
        abi_flag(P_C, end.carry);
        abi_flag(P_V, end.overflow);
        break;
    case TRIGGER_NOT_ALLOWED:
    case TRIGGER_NOT_HIT:
        abi_nz(end.a);
        break;
    case TRIGGER_HIT_TOO_OFTEN:
        abi_nz((uint8_t)(end.a - HIT_LIMIT));
        abi_flag(P_C, true);
        break;
    case TRIGGER_STARTED:
        abi_nz(end.a);
        abi_flag(P_C, false);
        break;
    }
    if (end.y_set) {
        cpu.y = end.y;
    }
}

/* The first frame of both triggers: the record, the hitbox byte moved on, the record's byte split. */
static TriggerEnd load_trigger(uint8_t x, uint8_t number_mask, bool takes_count, uint16_t record_return)
{
    TriggerEnd end = { TRIGGER_RECORD_LOADED, 0, 0, false, false, false };
    Sum sum;
    uint8_t a;
    uint8_t y;
    uint8_t last;
    uint8_t nz;
    bool carry;

    jsr_push(record_return);
    last = LoadObjectRecord(x, &nz, &carry);
    jsr_pop();
    (void)last;
    a = RD8(OBJ->hitbox_on[x]);
    sum = add_bytes(RECORD_SLOT_FIRST, a, false);
    WR8(OBJ->hitbox_on[x], sum.value);
    a = RD8(OBJ->next_kind[x]);
    y = a;
    a = (uint8_t)(a & number_mask);
    WR8(OBJ->f3510[x], a);
    a = y;
    carry = sum.carry;
    a = rol_a(a, &carry);
    a = rol_a(a, &carry);
    a = rol_a(a, &carry);
    if (takes_count) {
        y = a;
        a = (uint8_t)(a & TWO_BITS);
        WR8(OBJ->angle_hi[x], a);
        a = y;
        a = rol_a(a, &carry);
        a = rol_a(a, &carry);
        a = (uint8_t)(a & TWO_BITS);
        WR8(OBJ->life_timer[x], a);
    } else {
        a = (uint8_t)(a & TWO_BITS);
        WR8(OBJ->angle_hi[x], a);
    }
    end.a = a;
    end.y = y;
    end.y_set = true;
    end.carry = carry;
    end.overflow = sum.overflow;
    return end;
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
TriggerEnd StageTriggerObject(uint8_t x)
{
    TriggerEnd end = { TRIGGER_NOT_ALLOWED, 0, 0, false, false, false };
    uint8_t a;
    uint8_t y;

    a = RD8(OBJ->state[x]);
    if (a == 0) {
        return load_trigger(x, NIBBLE, true, FIRST_RECORD_RETURN);
    }
    WR8(OBJ->timer[x], 0);
    y = RD8(OBJ->life_timer[x]);
    end.y = y;
    end.y_set = true;
    a = rd8((uint16_t)(PadOverrides + y));
    end.a = a;
    if (a == 0) {
        WR8(OBJ->hit_taken[x], 0);
        return end;
    }
    a = RD8(OBJ->hit_taken[x]);
    end.a = a;
    if (a == 0) {
        end.how = TRIGGER_NOT_HIT;
        WR8(OBJ->hit_taken[x], 0);
        return end;
    }
    if (a >= HIT_LIMIT) {
        end.how = TRIGGER_HIT_TOO_OFTEN;
        WR8(OBJ->hit_taken[x], 0);
        return end;
    }
    a = RD8(OBJ->f3510[x]);
    WR8(ZP->field83, a);
    a = RD8(OBJ->angle_hi[x]);
    WR8(ZP->field87, a);
    WR8(ZP->field8c, 1);
    WR8(ZP->sub_step, STEP_TO_RUN);
    end.how = TRIGGER_STARTED;
    end.a = STEP_TO_RUN;
    return end;
}

void abi_StageTriggerObject(void)
{
    trigger_exit_shim(StageTriggerObject(cpu.x));
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
TriggerEnd StageTriggerObjectB(uint8_t x)
{
    TriggerEnd end = { TRIGGER_NOT_ALLOWED, 0, 0, false, false, false };
    uint8_t a;

    a = RD8(OBJ->state[x]);
    if (a == 0) {
        return load_trigger(x, FIVE_BITS, false, SECOND_RECORD_RETURN);
    }
    WR8(OBJ->timer[x], 0);
    a = RD8(OBJ->next_kind[x]);
    end.a = a;
    if ((a & 0x80) != 0) {
        /* Untraced: a negative number takes the same exit as a missing hit. */
        a = rd8(PlayerByte3BD7);
        end.a = a;
        if (a == 0) {
            WR8(OBJ->hit_taken[x], 0);
            return end;
        }
    }
    a = rd8(PlayerMayLeaveTop);
    a = (uint8_t)(a - 1);
    end.a = a;
    if (a == 0) {
        return end;
    }
    a = RD8(OBJ->hit_taken[x]);
    end.a = a;
    if (a == 0) {
        end.how = TRIGGER_NOT_HIT;
        WR8(OBJ->hit_taken[x], 0);
        return end;
    }
    if (a >= HIT_LIMIT) {
        end.how = TRIGGER_HIT_TOO_OFTEN;
        WR8(OBJ->hit_taken[x], 0);
        return end;
    }
    a = RD8(OBJ->f3510[x]);
    WR8(ZP->field83, a);
    a = RD8(OBJ->angle_hi[x]);
    WR8(ZP->field87, a);
    WR8(ZP->field8c, 0);
    WR8(ZP->sub_step, STEP_TO_RUN);
    WR8(OBJ->vx_hi[0], 0);
    WR8(OBJ->vx_lo[0], 0);
    end.how = TRIGGER_STARTED;
    end.a = STEP_TO_RUN;
    return end;
}

void abi_StageTriggerObjectB(void)
{
    trigger_exit_shim(StageTriggerObjectB(cpu.x));
}
