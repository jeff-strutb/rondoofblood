/* The stage overlay's starting objects: rows of its spawn tables put in the object slots. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum {
    FIRST_PAIR_RETURN_1 = 0x8B57, FIRST_PAIR_RETURN_2 = 0x8B5B, SECOND_PAIR_RETURN_1 = 0x8FA1,
    SECOND_PAIR_RETURN_2 = 0x8FA5, FLAG_MARK = 0xC0, LIST_FOR_PAIR = 0x65, LIST_CHANNEL = 2,
    PAIR_FLAG_BIT5 = 0x20, SECOND_PAIR_SKIP_BIT = 0x01, ANCHOR_X = 0x54, ANCHOR_Y = 0x55, SPEED_ONE = 0x04,
    HOLD_TIMER = 0x20, PLAYER_MODE_TWO = 2
};

typedef SpawnedObject (*SpawnRow)(uint8_t row);

/*
 * Spawns rows 0 and 1, clears the stage's two state bytes at $3616 and $3617 to 0 and $C0,
 * clears the pair flag (bit 5 of zField8E) and starts list channel 2 with the list $65.
 */
static StageSpawnEnd spawn_first_pair(SpawnRow spawn)
{
    StageSpawnEnd end;
    SpawnedObject second;
    uint8_t flags;

    jsr_push(FIRST_PAIR_RETURN_1);
    (void)spawn(0);
    jsr_pop();
    jsr_push(FIRST_PAIR_RETURN_2);
    second = spawn(1);
    jsr_pop();
    WR8(STAGEVARS->state_3616[0], 0);
    WR8(STAGEVARS->state_3616[1], FLAG_MARK);
    flags = RD8(ZP->field8e);
    WR8(ZP->field8e, (uint8_t)(flags & ~PAIR_FLAG_BIT5));
    end.a = StartListChannelKind2(LIST_FOR_PAIR, LIST_CHANNEL);
    end.x = LIST_CHANNEL;
    end.y = second.slot;
    end.ran = true;
    return end;
}

/*
 * Unless zField8E bit 0 is set, spawns rows 3 and 4, then backs the two anchor objects' high
 * position bytes off by one, gives them speed 4 and a hold of $20, and sets the player byte to 2.
 */
static StageSpawnEnd spawn_second_pair(SpawnRow spawn)
{
    StageSpawnEnd end = { 0, 0, 0, false };
    SpawnedObject second;

    if ((RD8(ZP->field8e) & SECOND_PAIR_SKIP_BIT) != 0) {
        return end;
    }
    jsr_push(SECOND_PAIR_RETURN_1);
    (void)spawn(3);
    jsr_pop();
    jsr_push(SECOND_PAIR_RETURN_2);
    second = spawn(4);
    jsr_pop();
    DEC8(OBJ->x_hi[ANCHOR_X]);
    DEC8(OBJ->y_hi[ANCHOR_Y]);
    WR8(OBJ->vx_hi[ANCHOR_X], SPEED_ONE);
    WR8(OBJ->vy_hi[ANCHOR_Y], SPEED_ONE);
    WR8(OBJ->f3510[ANCHOR_X], HOLD_TIMER);
    WR8(OBJ->f3510[ANCHOR_Y], HOLD_TIMER);
    wr8(PlayerByte3BD8, PLAYER_MODE_TWO);
    WR8(OBJ->angle_lo[ANCHOR_X], 0);
    end.a = PLAYER_MODE_TWO;
    end.x = 4;
    end.y = second.slot;
    end.ran = true;
    return end;
}

/* What the pair leaves: A, X and Y (the second spawn's slot); N and Z from A. C and V are as the spawn left them. */
static void pair_exit_shim(StageSpawnEnd end)
{
    if (!end.ran) {
        return;
    }
    cpu.a = end.a;
    cpu.x = end.x;
    cpu.y = end.y;
    abi_nz(end.a);
}

/*
 * @routine 024E:353 SpawnStageObjectsRows01
 * @run 6B:8B53
 * WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state
 * bytes (one to $C0), clears bit 5 of zField8E and starts list channel 2 with list $65.
 * HOW: SpawnStageObject for row 0 and row 1, then the stores, then StartListChannelKind2.
 * Eleven stage overlays hold this code, the ones whose spawn code is at $93B0.
 * WAITS: no.
 * Entry: nothing. Exit: A 2, X 2, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
 */
StageSpawnEnd SpawnStageObjectsRows01(void)
{
    return spawn_first_pair(SpawnStageObject);
}

void abi_SpawnStageObjectsRows01(void)
{
    pair_exit_shim(SpawnStageObjectsRows01());
}

/*
 * @routine 05BE:353 SpawnStageObjectsRows01Long
 * @run 6B:8B53
 * WHAT IT DOES: SpawnStageObjectsRows01 for the other stage group, whose spawn code is at $96C2.
 * HOW: The same code, calling SpawnStageObjectLong.
 * WAITS: no.
 * Entry, exit: as SpawnStageObjectsRows01.
 */
StageSpawnEnd SpawnStageObjectsRows01Long(void)
{
    return spawn_first_pair(SpawnStageObjectLong);
}

void abi_SpawnStageObjectsRows01Long(void)
{
    pair_exit_shim(SpawnStageObjectsRows01Long());
}

/*
 * @routine 024E:798 SpawnStageObjectsRows34
 * @run 6B:8F98
 * WHAT IT DOES: Unless zField8E bit 0 is set, puts the stage's spawn rows 3 and 4 in their
 * slots and sets the two objects up: their high position bytes back one, speed 4, hold $20,
 * the player byte 2, and the first one's angle cleared.
 * HOW: SpawnStageObject for row 3 and row 4, then the stores.
 * Eleven stage overlays hold this code, the ones whose spawn code is at $93B0.
 * WAITS: no.
 * Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0, C and V as the spawn left
 * them; with bit 0 set everything is kept.
 */
StageSpawnEnd SpawnStageObjectsRows34(void)
{
    return spawn_second_pair(SpawnStageObject);
}

void abi_SpawnStageObjectsRows34(void)
{
    pair_exit_shim(SpawnStageObjectsRows34());
}

/*
 * @routine 05BE:798 SpawnStageObjectsRows34Long
 * @run 6B:8F98
 * WHAT IT DOES: SpawnStageObjectsRows34 for the other stage group, whose spawn code is at $96C2.
 * HOW: The same code, calling SpawnStageObjectLong.
 * WAITS: no.
 * Entry, exit: as SpawnStageObjectsRows34.
 */
StageSpawnEnd SpawnStageObjectsRows34Long(void)
{
    return spawn_second_pair(SpawnStageObjectLong);
}

void abi_SpawnStageObjectsRows34Long(void)
{
    pair_exit_shim(SpawnStageObjectsRows34Long());
}
