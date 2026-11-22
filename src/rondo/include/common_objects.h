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

/* object/sensing.c: the probes (carry_in is the C the caller left; it reaches the overlay's lookup). */
Probe ProbeTerrain(uint8_t request, uint8_t object, uint8_t column, bool carry_in);
Probe ProbeTerrainAlt(uint8_t request, uint8_t object, uint8_t column, bool carry_in);
Probe ProbeTerrainFrontColumn(uint8_t request, uint8_t object, uint8_t column, bool carry_in);

/* Exit shims exported from the files that define them, for callers' wrappers. */
void probe_exit_shim(Probe p);  /* object/sensing.c */

/* What FallAndBounce did: the probe it made, the deadly bit, and when it bounced, the last add. */
typedef struct {
    bool bounced;         /* false: the ground was deadly and nothing more was done */
    Probe probe;
    uint8_t deadly;       /* the probe's byte ANDed with $10 */
    Sum angle;            /* the add that turned the angle on (when bounced) */
} FallResult;

/* The object moves, and on landing enters its next state; off deadly ground it bounces away. */
FallResult FallAndBounce(uint8_t slot, uint8_t column);

/* How StepReleaseOnHit ended. */
typedef enum { RELEASE_STARTED, RELEASE_STEPPED, RELEASE_NO_SLOT, RELEASE_SPAWNED } ReleaseEnd;
typedef struct {
    ReleaseEnd end;
    bool carry;           /* STARTED: the record's carry */
    uint8_t y;            /* Y as the step left it */
    uint8_t hit;          /* the hit byte compared */
    uint8_t slot;         /* the slot the search ended on */
    uint8_t last_type;    /* the last type the search read */
    uint8_t placed;       /* SPAWNED: the byte the place copy ended on */
} ReleaseResult;

/* An object that waits to be hit: on a hit of 5 or more it records the event and spawns a kind $9B beside it. */
ReleaseResult StepReleaseOnHit(uint8_t slot, uint8_t y);

/* What StepFallingItem left. */
typedef struct {
    bool started;         /* the object had no state: it was started from its record */
    FrameStep step;       /* else the animation's step */
    uint8_t y;            /* Y as it ends */
    uint8_t last;         /* started: the last table byte (A) */
    bool carry;           /* started: C */
    bool overflow;        /* V: the kind less $9B */
} FallingItem;

/* A thing that falls, with its own animation, then clears itself when the list ends. */
FallingItem StepFallingItem(uint8_t slot, uint8_t y);
/* What ChangeKindFromTable left. */
typedef struct {
    bool cleared;         /* the new kind was 0: the object was cleared */
    Sum table;            /* the kind less $80 */
    uint8_t y;            /* Y as it ends */
    uint8_t last;         /* the sideways speed's low byte (A when changed) */
    bool carry;           /* changed: the facing's top bit */
    Sum borrow;           /* changed: the Y position's high byte's subtract */
} KindChange;

/* The object turns into the kind a table names, thrown up at the player's place; a zero clears it. */
KindChange ChangeKindFromTable(uint8_t slot);
/* What AnimateKind88 left. */
typedef struct {
    bool stepped;         /* the kind was $88 and its list was stepped */
    uint8_t value;        /* the kind compared, or the frame copied */
    uint8_t y;            /* Y as it ends */
    FrameStep step;
} Kind88Step;

/* An object of kind $88 steps its animation list and keeps the frame in OBJ_3510. */
Kind88Step AnimateKind88(uint8_t slot, uint8_t y);

/* What StartKindFromCharacter left: the table byte, its index and the add that made it. */
typedef struct {
    uint8_t last;
    uint8_t index;
    Sum sum;
} KindStart;

/* An object's kind is settled, its record loaded, and OBJ_3510 set from a table by kind and character. */
KindStart StartKindFromCharacter(uint8_t slot);

/* What ClearOnLanding did. */
typedef struct {
    bool probed;          /* the object was not rising: the floor was asked */
    bool cleared;         /* the probe said floor: the object was cleared */
    Probe probe;
    uint8_t frame;        /* not cleared: the frame taken from OBJ_3510 */
    uint8_t state;        /* not cleared: the new state */
} LandClear;

/* A falling object that is cleared when the floor is reached, else moves on a state. */
LandClear ClearOnLanding(uint8_t slot, uint8_t column, bool carry_in);

/* 1 shifted left count times, as a byte. */
uint8_t BitMaskFromIndex(uint8_t count);

/* Not yet typed (register entries of routines that have C): the vector that goes to LoadObjectRecord (spawn.c), entered with X the object. */
/* Typed callees defined elsewhere, not declared in routines.h. */
uint8_t LoadObjectRecord(uint8_t object, uint8_t *exit_nz, bool *exit_c);  /* object/spawn.c */

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

/* An object's four hitbox are loaded from the table row picked by its animation frame. */
FrameLookup SetHitboxFromFrame(uint8_t slot);


/* What TurnAngleBack left. */
typedef struct {
    bool borrowed;        /* the 16-bit angle went below zero: the object moved on */
    uint8_t high;         /* the angle's high byte after the subtract (A on the plain path) */
    bool carry;           /* the high subtract's carry (no borrow) */
    bool overflow;        /* its signed overflow */
    uint8_t new_state;    /* when borrowed: the state after INC */
} TurnBack;

/* An object's angle falls by $0080; when it passes zero, the object enters its next state. */
TurnBack TurnAngleBack(uint8_t slot);

/* What AdvanceOnTimer left. */
typedef struct {
    bool waiting;         /* the timer has not run out */
    uint8_t timer;        /* the timer after its DEC */
    uint8_t loaded;       /* the last byte loaded from the row (A at the end) */
    uint8_t row_offset;   /* Y at the end */
    bool carry;           /* C at the end: bit 6 of the next kind */
    uint8_t new_state;    /* the state after INC */
} TimerAdvance;

/* When the object's life timer runs out, four bytes of a row picked by its next kind are loaded and it moves on. */
TimerAdvance AdvanceOnTimer(uint8_t slot);

/* What LaunchOnTimer left. */
typedef struct {
    bool waiting;         /* the timer has not run out */
    uint8_t timer;        /* the timer after its DEC */
    uint8_t loaded;       /* the last table byte (A at the end) */
    uint8_t row_offset;   /* Y at the end */
    bool carry;           /* C at the end: bit 7 of the next kind */
    bool overflow;        /* V: the add of $12 to the next kind overflowed */
    uint8_t new_state;    /* the state after INC */
} Launch;

/* When the life timer runs out, the object shows its next kind's frame, takes its two speeds from a table and moves on. */
Launch LaunchOnTimer(uint8_t slot);

/* What SetHitboxFromFrameFive left. */
typedef struct {
    uint8_t last;         /* the last byte loaded (A) */
    uint8_t offset;       /* Y */
    bool carry;           /* C: the last shift's carry */
    bool overflow;        /* V: the frame less 5 overflowed */
} HitboxRow;

/* The hitbox is loaded from the four-byte row of the object's frame less 5. */
HitboxRow SetHitboxFromFrameFive(uint8_t slot);

/* How EnterStateFiveOnPlayerState9 ended. */
typedef enum { FIVE_PLAYER_NOT_9, FIVE_NOT_STATE_3, FIVE_ENTERED } FiveEnd;
typedef struct {
    FiveEnd end;
    uint8_t value;        /* the state last compared */
} FiveResult;

/* An object in state 3 while the player (slot 0) is in state 9 enters state 5, facing right and with its frame list restarted. */
FiveResult EnterStateFiveOnPlayerState9(uint8_t slot);

/* What StopAndPickPhase left. */
typedef struct {
    uint8_t loaded;       /* the table byte (A) */
    uint8_t index;        /* Y */
    uint8_t new_phase;    /* the phase counter after INC */
} PhasePick;

/* The object's speeds are cleared and its pose is picked by a counter that wraps through eight table bytes. */
PhasePick StopAndPickPhase(uint8_t slot);

#endif
