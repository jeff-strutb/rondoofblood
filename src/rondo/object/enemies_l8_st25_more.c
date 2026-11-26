/* The rest of lot L8's overlay at sectors 0F6C and 0F6D: the tile read vector, the side probe and a walking object's step. */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "player_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"

void abi_ProbeTileRead9991(void);
Ended StepListByNumberTableC585(uint8_t slot, uint8_t y_entry);
Ended ToggleFacing(uint8_t slot);
Ended HurtOnceThenAnswerYes(uint8_t slot, uint8_t y_entry);
Ended StartLeapWhenHurtAnswerYes(uint8_t slot, uint8_t y_entry);

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/* What the machine's A, Y and flags hold, as the exit of a piece that has just put them there. */
static Ended ended_of_cpu_shim(void)
{
    uint8_t nz = (cpu.p & P_Z) != 0 ? 0 : ((cpu.p & P_N) != 0 ? 0x80 : 0x01);
    Ended e = ended_with_y(ended_with_overflow(ended_with_carry(ended_load(cpu.a), (cpu.p & P_C) != 0), (cpu.p & P_V) != 0), cpu.y);

    e.nz = nz;
    return e;
}

/* An exit and the X the code left, when original code ran and may have changed it. */
typedef struct {
    Ended e;
    bool x_changed;
    uint8_t x;
} SideReadEnd;

/* The exit of a tile read: A, Y and the flags as terrain_read_exit_shim puts them, and X when the read ran original code. */
static SideReadEnd ended_of_tile_read_shim(TileRead read)
{
    SideReadEnd out;

    terrain_read_exit_shim(read);
    out.e = ended_of_cpu_shim();
    out.x_changed = read.kind == TILE_BY_ORIGINAL;
    out.x = cpu.x;
    return out;
}

/*
 * @routine 05BD:006 StageTileReadVector9991
 * @run 6B:8006
 * WHAT IT DOES: Goes to the tile read at $9991 (ProbeTileRead9991).
 * HOW: One jump. The twelve overlays of the group whose tile read is at $9991 hold this vector.
 * WAITS: no.
 * Entry, exit: as ProbeTileRead9991.
 */
SideReadEnd StageTileReadVector9991(void)
{
    return ended_of_tile_read_shim(ProbeTileRead9991());
}

void abi_StageTileReadVector9991(void)
{
    abi_ProbeTileRead9991();
}

/* The bytes at $C357: the x offsets, two a side, for the point the side probe reads. */
enum { SIDE_OFFSETS = 0xC357, SIDE_PROBE_RETURN = 0xC34F, SIDE_FEET = 0x16 };

/*
 * @routine 0F6D:324 ProbeTileSideBit1
 * @run 71:C324
 * WHAT IT DOES: Says (C 1) whether the tile a little ahead of the object's feet has bit 1 in its attribute.
 * HOW: The side of the point comes from the sign of the X speed (two offsets at $C357), $16 below the object's Y.
 * The tile read is the vector at $8006.
 * WAITS: no.
 * Entry: X object. Exit: A the attribute's bit 1, N and Z from it, C that bit, Y and V as the tile read left them.
 */
SideReadEnd ProbeTileSideBit1(uint8_t slot)
{
    SideReadEnd read;
    Ended e;
    Sum low;
    Sum high;
    Sum row_low;
    Sum row_high;
    uint8_t a = RD8(OBJ->vx_hi[slot]);
    uint8_t y;
    uint8_t m;
    bool c = (a & 0x80) != 0;
    bool next;

    a = (uint8_t)(a << 1);
    next = (a & 0x80) != 0;
    a = (uint8_t)((a << 1) | (c ? 1 : 0));
    c = next;
    a = (uint8_t)(a & 0x01);
    y = (uint8_t)(a << 1);
    a = RD8(OBJ->x_lo[slot]);
    m = rd8((uint16_t)(SIDE_OFFSETS + y));
    low = add_bytes(a, m, false);
    WR8(ZP->temp00, low.value);
    a = RD8(OBJ->x_hi[slot]);
    m = rd8((uint16_t)(SIDE_OFFSETS + 1 + y));
    high = add_bytes(a, m, low.carry);
    WR8(ZP->temp01, high.value);
    a = RD8(OBJ->y_lo[slot]);
    row_low = add_bytes(a, SIDE_FEET, false);
    WR8(ZP->temp02, row_low.value);
    m = RD8(OBJ->y_hi[slot]);
    row_high = add_bytes(0, m, row_low.carry);
    WR8(ZP->temp03, row_high.value);
    jsr_push(SIDE_PROBE_RETURN);
    read = StageTileReadVector9991();
    jsr_pop();
    e = read.e;
    a = (uint8_t)(e.a & 0x02);
    read.e = then(e, ended_with_carry(ended_load(a), a != 0));
    return read;
}

void abi_ProbeTileSideBit1(void)
{
    SideReadEnd r = ProbeTileSideBit1(cpu.x);

    abi_ended(r.e);
    if (r.x_changed) {
        cpu.x = r.x;
    }
}

/* The code the walking step runs on at BCD3 when it is on the ground tile and near: it has no C. */
static Ended walk_other_shim(Ended e, uint8_t y)
{
    cpu.a = e.a;
    abi_nz(e.nz);
    abi_flag(P_C, e.c);
    abi_flag(P_V, e.v);
    cpu.y = y;
    call_original(0xBCD3);
    return ended_of_cpu_shim();
}

/*
 * @routine 0F6C:4A4 WalkAndTurnAtEdge
 * @run 70:BCA4
 * WHAT IT DOES: A walking object's step: the leap or the hurt answer ends it (C 1); otherwise it steps its animation, moves along X and,
 * when it is near the player and not at its usual height, goes on in other code; when the tile ahead of its feet is solid (bit 1) it
 * turns round.
 * HOW: StartLeapWhenHurtAnswerYes, HurtOnceThenAnswerYes, StepListByNumberTableC585, ApplyVelocityX, DistanceXToPlayer, ProbeTileSideBit1,
 * and for the turn ToggleFacing and a tail jump into NegateVelocityX.
 * WAITS: no.
 * Entry: X object, Y kept unless a step sets it. Exit: as the last instruction that ran. The arms that end at the first two answers
 * and the near arm ran in no script.
 */
Ended WalkAndTurnAtEdge(uint8_t slot, uint8_t y_entry)
{
    Ended e;
    PlayerDistance distance;
    AxisMove move;
    uint8_t y = y_entry;
    uint8_t height;
    SideReadEnd probe;

    jsr_push(0xBCA7);
    e = StartLeapWhenHurtAnswerYes(slot, y);
    jsr_pop();
    if (e.c) {
        return e;
    }
    y = ended_y_or(e, y);
    jsr_push(0xBCAD);
    e = then(e, HurtOnceThenAnswerYes(slot, y));
    jsr_pop();
    if (e.c) {
        return e;
    }
    y = ended_y_or(e, y);
    jsr_push(0xBCB3);
    e = then(e, StepListByNumberTableC585(slot, y));
    jsr_pop();
    y = ended_y_or(e, y);
    jsr_push(0xBCB6);
    move = ApplyVelocityX(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    jsr_push(0xBCB9);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_distance(distance));
    e = then(e, ended_compare(distance.distance, 0x40));
    if (distance.distance < 0x40) {
        height = RD8(OBJ->y_lo[0]);
        e = then(e, ended_compare(height, 0x9A));
        if (height != 0x9A) {
            return walk_other_shim(e, y);
        }
    }
    jsr_push(0xBCCA);
    probe = ProbeTileSideBit1(slot);
    jsr_pop();
    e = then(e, probe.e);
    if (probe.x_changed) {
        slot = probe.x;
    }
    if (!e.c) {
        return e;
    }
    jsr_push(0xBCCF);
    e = then(e, ToggleFacing(slot));
    jsr_pop();
    return then(e, ended_of_sum(NegateVelocityX(slot)));
}

void abi_WalkAndTurnAtEdge(void)
{
    abi_ended(WalkAndTurnAtEdge(cpu.x, cpu.y));
}
