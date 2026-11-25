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
/* object/player_tile_probe.c */
TileRead ProbeTileRead9991(void);

/* A body's exit state, and a TST after it: N and V are the memory's bits 7 and 6, Z from the mask. */
typedef struct {
    Ended end;
    bool tst;
    uint8_t memory;
    bool zero;
    bool v_only;     /* a load after the TST set N and Z: only V stands from it */
} EndedTst;

/* A body's exit state with the X it leaves. */
typedef struct {
    Ended end;
    uint8_t x;
} EndedX;

static inline EndedTst ended_tst_none(Ended end)
{
    EndedTst r = { end, false, 0, false, false };

    return r;
}

static inline EndedTst ended_tst(Ended before, uint8_t memory, uint8_t mask)
{
    EndedTst r = { before, true, memory, (memory & mask) == 0, false };

    return r;
}

static inline void abi_ended_tst(EndedTst r)
{
    abi_ended(r.end);
    if (r.tst) {
        if (!r.v_only) {
            abi_flag(P_N, (r.memory & 0x80) != 0);
            abi_flag(P_Z, r.zero);
        }
        abi_flag(P_V, (r.memory & 0x40) != 0);
    }
}

/* object/player_leaves.c: the tile handlers of the stage overlays that hold their own tile read at $9991 */
Ended TileNudgeAt9C82(void);
Ended TileLift7At9C87(void);
Ended TileLift6At9C91(void);
Ended TileLift5At9C9B(void);
Ended TileLift4At9CA5(void);
Ended TileLift3At9CAF(void);
Ended TileLift2At9CB9(void);
Ended TileLift1At9CC3(void);
Ended TileHalfStepAAt9A93(uint8_t tile);
Ended TileHalfStepBAt9AA7(uint8_t tile);

/* object/player_input.c */
typedef struct { Ended end; bool x_set; uint8_t x; } TurnWindow;
Ended RichterFaceByPad(void);
TurnWindow RichterCheckTurnWindow(void);
EndedX RichterCheckItemHearts(void);

#endif
