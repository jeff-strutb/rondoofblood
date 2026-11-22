/*
 * The shared object pool's routines (the stage overlays' common object code), as the enemy
 * files call them. A body takes the object's slot and returns, as plain values, how it ended;
 * its abi_ wrapper turns that into the registers and flags the assembly leaves.
 */
#ifndef RONDO_COMMON_OBJECTS_H
#define RONDO_COMMON_OBJECTS_H

#include "machine.h"

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

/* Not yet typed: the vector that goes to LoadObjectRecord (spawn.c), entered with X the object. */
void abi_VecLoadObjectRecord(void);

#endif
