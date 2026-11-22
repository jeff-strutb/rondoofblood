/*
 * The shared object pool's routines (the stage overlays' common object code), as the enemy
 * files call them. A body takes the object's slot and returns, as plain values, how it ended;
 * its abi_ wrapper turns that into the registers and flags the assembly leaves.
 */
#ifndef RONDO_COMMON_OBJECTS_H
#define RONDO_COMMON_OBJECTS_H

#include "machine.h"
#include "routines.h"

/*
 * An ADC as the processor does it: in decimal mode (D set in the P the caller left) the add is
 * BCD and V is not touched. Used where a routine adds after a call, whose exit flags the
 * original's ADC honours. overflow_set is false when V is to be left as it stands.
 */
static inline Sum add_bytes_in_mode(uint8_t a, uint8_t b, bool carry_in, bool *overflow_set)
{
    Sum sum;

    if (cpu.p & P_D) {
        unsigned total = (a & 0x0Fu) + (b & 0x0Fu) + (carry_in ? 1u : 0u);

        if (total >= 0x0A) {
            total += 0x06;
        }
        total += (a & 0xF0u) + (b & 0xF0u);
        if (total >= 0xA0) {
            total += 0x60;
        }
        sum.value = (uint8_t)total;
        sum.carry = (total & 0xFF00u) != 0;
        sum.overflow = false;
        *overflow_set = false;
        return sum;
    }
    *overflow_set = true;
    return add_bytes(a, b, carry_in);
}

/* How EnterHurtStateOnHit ended. */
typedef enum {
    HURT_NOT_ACTIVE,      /* the object's state is 0 */
    HURT_STATE_PAST,      /* the state is 5 or more: already hurt or later */
    HURT_NOT_HIT,         /* no hit is pending (the hit byte is 0) */
    HURT_HIT_PAST,        /* the pending hit is 5 or more: not a plain hit */
    HURT_ENTERED          /* the state became 5 */
} HurtEnd;

typedef struct {
    HurtEnd end;
    uint8_t value;        /* the last byte compared: the state, or the pending hit */
} HurtCheck;

/* How SetBlinkFlash ended. */
typedef enum {
    BLINK_TIMER_LOW,      /* the timer is under $18: flag left 0 */
    BLINK_WAVE_BIT,       /* bit 1 of the wave counter is set: flag left 0 */
    BLINK_SET             /* the flag became $86 */
} BlinkEnd;

typedef struct {
    BlinkEnd end;
    uint8_t value;        /* the last byte read: the timer, or the wave counter ANDed with 2 */
} BlinkCheck;

/* An object in states 1 to 4 with a pending hit of 1 to 4 enters state 5. */
HurtCheck EnterHurtStateOnHit(uint8_t slot);
/* The object's flash flag is cleared, then set to $86 on the frames the wave counter allows. */
BlinkCheck SetBlinkFlash(uint8_t slot);

/* An object's record is loaded, then its player-shared byte and timers are set (see the routine). */
void InitObjectFromRecord(uint8_t slot);

/* How FallAndBounce ended. */
typedef enum {
    FALL_DEADLY_GROUND,   /* the probe reported deadly ground (bit 4): nothing more done */
    FALL_BOUNCED          /* the bounce was set up */
} FallEnd;

typedef struct {
    FallEnd end;
    uint8_t probe_bit4;   /* FALL_DEADLY_GROUND: the answer ANDed with $10 */
    uint8_t angle_high;   /* FALL_BOUNCED: the object's angle high byte as stored */
    bool carry;           /* FALL_BOUNCED: the last add's carry out */
    bool overflow;        /* FALL_BOUNCED: the last add's signed overflow */
    bool overflow_set;    /* FALL_BOUNCED: false when the add was decimal and left V alone */
} FallResult;

/* The object moves, and on landing enters its next state; off deadly ground it bounces away. */
FallResult FallAndBounce(uint8_t slot);
/* 1 shifted left count times, as a byte. */
uint8_t BitMaskFromIndex(uint8_t count);

/* How StepReleaseOnHit ended. */
typedef enum {
    RELEASE_STARTED_UP,   /* state 0: the record was loaded */
    RELEASE_WAITING,      /* the pending hit is under 5 */
    RELEASE_NO_SLOT,      /* released, but no free slot was found */
    RELEASE_STARTED       /* released: the new object was started (tail call) */
} ReleaseEnd;

typedef struct {
    ReleaseEnd end;
    uint8_t hit;          /* RELEASE_WAITING: the pending hit */
} ReleaseResult;

/* How StepFallingItem ended. */
typedef enum {
    ITEM_SET_UP,          /* state 0: the record was loaded and its speed set from a table */
    ITEM_STEPPED,         /* moved and its frames stepped; still alive */
    ITEM_CLEARED          /* its frames ran out: the object was cleared (tail call) */
} ItemEnd;

typedef struct {
    ItemEnd end;
    uint8_t table_index;  /* ITEM_SET_UP: the table offset used */
    uint8_t table_value;  /* ITEM_SET_UP: the last byte read from the table */
    bool carry;           /* ITEM_SET_UP: the shifted kind's carry */
} ItemResult;

/* An object waits for a hit of 5 or more, then replaces itself and starts the object $9B. */
ReleaseResult StepReleaseOnHit(uint8_t slot);
/* An object of kinds $9B and up: at state 0 its speed is read from a table, then it falls. */
ItemResult StepFallingItem(uint8_t slot);

void abi_VecStepObjectFrames(void);
void abi_VecApplyVelocityY(void);
void abi_VecClearObject(void);
void abi_VecRecordObjectEvent(void);
void abi_VecFindFreeSlotFrom50(void);
void abi_VecStartObjectAtPlace(void);

/* Not yet typed (register entries of routines that have C): the vector that goes to LoadObjectRecord (spawn.c), entered with X the object. */
void abi_VecLoadObjectRecord(void);
void abi_VecApplyVelocity(void);
void abi_VecProbeTerrain(void);
void abi_VecScaleByCosine(void);

#endif
