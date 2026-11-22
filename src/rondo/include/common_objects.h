/*
 * The shared object pool's routines (the stage overlays' common object code), as the enemy
 * files call them. A body takes the object's slot and returns, as plain values, how it ended;
 * its abi_ wrapper turns that into the registers and flags the assembly leaves.
 */
#ifndef RONDO_COMMON_OBJECTS_H
#define RONDO_COMMON_OBJECTS_H

#include "machine.h"
#include "routines.h"

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
bool InitObjectFromRecord(uint8_t slot);

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
} FallResult;

/* The object moves, and on landing enters its next state; off deadly ground it bounces away. */
FallResult FallAndBounce(uint8_t slot);
/* 1 shifted left count times, as a byte. */
uint8_t BitMaskFromIndex(uint8_t count);

/* Not yet typed (register entries of routines that have C): the vector that goes to LoadObjectRecord (spawn.c), entered with X the object. */
/* Typed callees defined elsewhere, not declared in routines.h. */
uint8_t LoadObjectRecord(uint8_t object, uint8_t *exit_nz, bool *exit_c);  /* object/spawn.c */
void abi_VecApplyVelocity(void);
void abi_VecProbeTerrain(void);
void abi_VecScaleByCosine(void);

/* How EnterStateFourOnFlag ended. */
typedef enum { STATE4_ALREADY, STATE4_NO_FLAG, STATE4_ENTERED } State4End;
typedef struct {
    State4End end;
    bool above;           /* the state was above 4 (its compare's carry) */
} State4Result;

/* An object not yet in state 4 whose OBJ_31A0 bit 7 is set restarts its animation in state 4. */
State4Result EnterStateFourOnFlag(uint8_t slot);

/* How a table lookup by animation frame ended: what the wrapper needs of the last values. */
typedef struct {
    bool none;            /* the frame has no entry: the offsets were cleared */
    uint8_t frame_index;  /* the frame less 1 (Y at that point) */
    bool frame_zero;      /* the frame itself was 0: no table was read */
    uint8_t last;         /* the last byte loaded into A */
    uint8_t table_offset; /* Y at the end */
    bool carry;           /* C at the end */
    bool overflow;        /* V: the frame's subtract of 1 overflowed (only frame $80) */
} FrameLookup;

/* An object's four hit offsets are loaded from the table row picked by its animation frame. */
FrameLookup SetHitOffsetsFromFrame(uint8_t slot);


/* Moved unchanged from object/sensing.c, which still includes this header. */
/* What was done about a wall. */
typedef enum { NO_PUSH, PUSHED_RIGHT, PUSHED_LEFT } WallPushed;

/* How a probe went, step by step, for what the wrapper replays. */
typedef struct {
    uint8_t object;
    uint8_t column;
    uint8_t answer;
    bool floor_asked;
    LookupEnd floor;
    bool scanned;
    uint8_t scanned_to;
    bool lifted;
    uint16_t lift_before, lift_amount;
    bool wall_asked;
    LookupEnd wall;
    WallPushed pushed;
    uint16_t push_before, push_amount;
} Probe;

/* Exit shims exported from the files that define them, for callers' wrappers. */
void probe_exit_shim(Probe p);  /* object/sensing.c */
Alu cosine_exit_shim(uint8_t angle, uint16_t value, uint32_t result);  /* math/trig.c */

#endif
