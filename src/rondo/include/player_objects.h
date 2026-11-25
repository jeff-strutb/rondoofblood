/*
 * Lot L2: what the player's routines declare for each other and take from the files of the earlier filing
 * (richter_state.c, player_terrain.c) that no other header declares. The defining file is named at each.
 */
#ifndef RONDO_PLAYER_OBJECTS_H
#define RONDO_PLAYER_OBJECTS_H

#include "machine.h"
#include "routines.h"
#include "enemy_objects.h"

/* object/richter_state.c */
AnimationStep RichterStand(void);
/* object/player_terrain.c: the registers a read under his feet leaves */
void under_feet_exit_shim(ColumnRead one);
TileRead RichterReadOnScreen(uint8_t row, bool carry);

/* A body's exit state, and a TST after it: N and V are the memory's bits 7 and 6, Z from the mask. */
typedef struct {
    Ended end;
    bool tst;
    uint8_t memory;
    bool zero;
} EndedTst;

/* A body's exit state with the X it leaves. */
typedef struct {
    Ended end;
    uint8_t x;
} EndedX;

static inline EndedTst ended_tst_none(Ended end)
{
    EndedTst r = { end, false, 0, false };

    return r;
}

static inline EndedTst ended_tst(Ended before, uint8_t memory, uint8_t mask)
{
    EndedTst r = { before, true, memory, (memory & mask) == 0 };

    return r;
}

static inline void abi_ended_tst(EndedTst r)
{
    abi_ended(r.end);
    if (r.tst) {
        abi_flag(P_N, (r.memory & 0x80) != 0);
        abi_flag(P_V, (r.memory & 0x40) != 0);
        abi_flag(P_Z, r.zero);
    }
}

#endif
