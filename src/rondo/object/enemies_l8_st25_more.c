/* The rest of lot L8's overlay at sectors 0F6C and 0F6D: the tile read vector, the side probe and a walking object's step. The
 * converted bodies return the arm they ended on and the values behind it; their exit shims build A, Y and the flags. */
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
 * Entry: X object. Exit: A the attribute's bit 1, N and Z from it, C that bit, Y and V as the tile read left them (l8_side_bit1_exit_shim).
 */
L8SideBitArm ProbeTileSideBit1(uint8_t slot)
{
    L8SideBitArm r;
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
    r.read = StageTileReadVector9991();
    jsr_pop();
    (void)c;
    return r;
}

/* What ProbeTileSideBit1 leaves: the tile read's exit, then A its bit 1 and C that bit; X as the read left it. */
SideReadEnd l8_side_bit1_exit_shim(L8SideBitArm r)
{
    SideReadEnd read = r.read;
    uint8_t a = (uint8_t)(read.e.a & 0x02);

    read.e = then(read.e, ended_with_carry(ended_load(a), a != 0));
    return read;
}

void abi_ProbeTileSideBit1(void)
{
    SideReadEnd r = l8_side_bit1_exit_shim(ProbeTileSideBit1(cpu.x));

    abi_ended(r.e);
    if (r.x_changed) {
        cpu.x = r.x;
    }
}

/* ProbeTileSideBit1: the tile read's exit and the X it left. */
typedef struct {
    SideReadEnd read;
} L8SideBitArm;

/* WalkAndTurnAtEdge: the two answers, the step, the move, the distance and whether it was near, the player's Y, what the other code left, the probe, the facing and the reversed speed. */
enum { ST25M_WALK_LEAPT, ST25M_WALK_HURT, ST25M_WALK_OTHER, ST25M_WALK_NO_EDGE, ST25M_WALK_TURNED };
typedef struct {
    int arm;
    L8LeapYesArm leap;
    L8HurtOnceArm hurt;
    L8NumberStep step;
    AxisMove move;
    PlayerDistance distance;
    bool near;
    uint8_t height, facing;
    Ended other;
    L8SideBitArm probe;
    Sum negated;
} L8WalkArm;

/* DroppingPieceStep: the state, the record, the kind, its speed and index and the last speed byte of state 0; the timer of the later
 * states; in state 1 the step, the flashing piece, the moves, the Y speed, the tile read and its bit, the lift, the slot search, the
 * start and its feet, and the new state. */
enum { ST25M_DROP_LOADED, ST25M_DROP_COUNTING, ST25M_DROP_RISING, ST25M_DROP_OFF_FLOOR, ST25M_DROP_LANDED };
typedef struct {
    int arm;
    uint8_t state, kind, speed, index, last, timer, bit, placed, count;
    L8Record record;
    AxisMove move1, move2;
    L8NumberStep step;
    L8FlashPieceArm flash;
    SideReadEnd read;
    Sum lift, feet;
    L8Slot found;
} L8DropPieceArm;

static Ended l8_walk_so_far_shim(L8WalkArm r);

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
 * Entry: X object, Y kept unless a step sets it. Exit: as the last instruction that ran (l8_walk_exit_shim). The arms that end at the first two answers
 * and the near arm ran in no script.
 */
L8WalkArm WalkAndTurnAtEdge(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry;
    L8WalkArm r = { 0 };

    jsr_push(0xBCA7);
    r.leap = StartLeapWhenHurtAnswerYes(slot, y);
    jsr_pop();
    if (r.leap.yes) {
        r.arm = ST25M_WALK_LEAPT;
        return r;
    }
    jsr_push(0xBCAD);
    r.hurt = HurtOnceThenAnswerYes(slot, y);
    jsr_pop();
    if (r.hurt.yes) {
        r.arm = ST25M_WALK_HURT;
        return r;
    }
    jsr_push(0xBCB3);
    r.step = StepListByNumberTableC585(slot, y);
    jsr_pop();
    y = r.step.step != FRAME_HELD ? r.step.y : r.step.index;
    jsr_push(0xBCB6);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    jsr_push(0xBCB9);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    r.near = r.distance.distance < 0x40;
    if (r.near) {
        r.height = RD8(OBJ->y_lo[0]);
        if (r.height != 0x9A) {
            r.other = walk_other_shim(l8_walk_so_far_shim(r), y);
            r.arm = ST25M_WALK_OTHER;
            return r;
        }
    }
    jsr_push(0xBCCA);
    r.probe = ProbeTileSideBit1(slot);
    jsr_pop();
    if (r.probe.read.x_changed) {
        slot = r.probe.read.x;
    }
    if ((r.probe.read.e.a & 0x02) == 0) {
        r.arm = ST25M_WALK_NO_EDGE;
        return r;
    }
    jsr_push(0xBCCF);
    r.facing = ToggleFacing(slot);
    jsr_pop();
    r.negated = NegateVelocityX(slot);
    r.arm = ST25M_WALK_TURNED;
    return r;
}

/* What WalkAndTurnAtEdge leaves in A, Y and the flags, from the arm it ended on (the near arm: what the original code left). */
static Ended l8_walk_exit_shim(L8WalkArm r)
{
    Ended e;

    if (r.arm == ST25M_WALK_LEAPT) {
        return l8_leap_yes_exit_shim(r.leap);
    }
    if (r.arm == ST25M_WALK_HURT) {
        return then(l8_leap_yes_exit_shim(r.leap), l8_hurt_once_exit_shim(r.hurt));
    }
    if (r.arm == ST25M_WALK_OTHER) {
        return r.other;
    }
    e = then(l8_walk_so_far_shim(r), l8_side_bit1_exit_shim(r.probe).e);
    if (r.arm == ST25M_WALK_NO_EDGE) {
        return e;
    }
    e = then(e, l8_toggle_facing_exit_shim(r.facing));
    return then(e, ended_of_sum(r.negated));
}

void abi_WalkAndTurnAtEdge(void)
{
    abi_ended(l8_walk_exit_shim(WalkAndTurnAtEdge(cpu.x, cpu.y)));
}

/* What WalkAndTurnAtEdge's instructions have left by the distance test (and the height test when near): the registers the other
 * code at BCD3 is entered with, and the start of the later arms' exits. */
static Ended l8_walk_so_far_shim(L8WalkArm r)
{
    Ended e = then(l8_leap_yes_exit_shim(r.leap), l8_hurt_once_exit_shim(r.hurt));

    e = then(e, l8_number_step_c585_exit_shim(r.step));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_of_distance(r.distance));
    e = then(e, ended_compare(r.distance.distance, 0x40));
    if (r.near) {
        e = then(e, ended_compare(r.height, 0x9A));
    }
    return e;
}


enum {
    PIECE_LIFE = 0xB919, PIECE_FACING = 0xB91D, PIECE_SPEED = 0xB921, PIECE_VX = 0xB925, PIECE_VY = 0xB92D,
    PIECE_FALL_STEP = 0x10, PIECE_FEET = 0x08, PIECE_TYPE = 0x9B
};

/*
 * @routine 0F6C:024 DroppingPieceStep
 * @run 70:B824
 * WHAT IT DOES: A piece that falls and lands: its first frame loads its record and takes its animation, facing and speeds from
 * tables by the record's kind; in state 1 each frame steps its list, may start a flashing piece, moves, falls faster (Y speed +$10)
 * and, once falling, reads the tile $08 below it: when that tile has bit 0 the piece is set back out of the floor, may start a
 * type $9B object in a free slot of $30 to $4F, and goes to state 2 with its timer 6; other states count the timer down.
 * HOW: LoadObjectRecord, StepListByNumberTable, SpawnFlashingPieceWhenRising, ApplyVelocityX and Y, the tile read vector at $8006,
 * FindFreeSlotFrom30 and StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept unless a step sets it. Exit: as the last instruction that ran (l8_drop_piece_exit_shim).
 */
L8DropPieceArm DroppingPieceStep(uint8_t slot, uint8_t y_entry)
{
    Sum sum;
    Sum row;
    uint8_t a;
    uint8_t m;
    uint8_t y = y_entry;
    L8DropPieceArm r = { 0 };

    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        jsr_push(0xB82C);
        r.record = l8_record(slot);
        jsr_pop();
        r.kind = RD8(OBJ->next_kind[slot]);
        a = rd8((uint16_t)(PIECE_LIFE + r.kind));
        WR8(OBJ->life_timer[slot], a);
        a = rd8((uint16_t)(PIECE_FACING + r.kind));
        WR8(OBJ->facing_left[slot], a);
        r.speed = rd8((uint16_t)(PIECE_SPEED + r.kind));
        WR8(OBJ->vx_hi[slot], r.speed);
        jsr_push(0xB844);
        r.move1 = ApplyVelocityX(slot);
        jsr_pop();
        r.index = (uint8_t)(r.kind << 1);
        a = rd8((uint16_t)(PIECE_VX + r.index));
        WR8(OBJ->vx_lo[slot], a);
        a = rd8((uint16_t)(PIECE_VX + 1 + r.index));
        WR8(OBJ->vx_hi[slot], a);
        a = rd8((uint16_t)(PIECE_VY + r.index));
        WR8(OBJ->vy_lo[slot], a);
        r.last = rd8((uint16_t)(PIECE_VY + 1 + r.index));
        WR8(OBJ->vy_hi[slot], r.last);
        r.arm = ST25M_DROP_LOADED;
        return r;
    }
    if ((uint8_t)(r.state - 1) != 0) {
        r.timer = CountDownThenClearOnTimer(slot);
        r.arm = ST25M_DROP_COUNTING;
        return r;
    }
    jsr_push(0xB869);
    r.step = StepListByNumberTable(slot, y);
    jsr_pop();
    jsr_push(0xB86C);
    r.flash = SpawnFlashingPieceWhenRising(slot);
    jsr_pop();
    jsr_push(0xB86F);
    r.move1 = ApplyVelocityX(slot);
    jsr_pop();
    jsr_push(0xB872);
    r.move2 = ApplyVelocityY(slot);
    jsr_pop();
    a = RD8(OBJ->vy_lo[slot]);
    sum = add_bytes(a, PIECE_FALL_STEP, false);
    WR8(OBJ->vy_lo[slot], sum.value);
    a = RD8(OBJ->vy_hi[slot]);
    sum = add_bytes(a, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    r.last = RD8(OBJ->vy_hi[slot]);
    if ((r.last & 0x80) != 0) {
        r.arm = ST25M_DROP_RISING;
        return r;
    }
    a = RD8(OBJ->x_lo[slot]);
    WR8(ZP->temp00, a);
    a = RD8(OBJ->x_hi[slot]);
    WR8(ZP->temp01, a);
    a = RD8(OBJ->y_lo[slot]);
    sum = add_bytes(a, PIECE_FEET, false);
    WR8(ZP->temp02, sum.value);
    a = RD8(OBJ->y_hi[slot]);
    row = add_bytes(a, 0, sum.carry);
    WR8(ZP->temp03, row.value);
    jsr_push(0xB8A4);
    r.read = StageTileReadVector9991();
    jsr_pop();
    if (r.read.x_changed) {
        slot = r.read.x;
    }
    r.bit = (uint8_t)(r.read.e.a & 0x01);
    if (r.bit == 0) {
        r.arm = ST25M_DROP_OFF_FLOOR;
        return r;
    }
    a = RD8(OBJ->y_lo[slot]);
    m = RD8(ZP->temp02);
    r.lift = sub_bytes(a, m, true);
    WR8(OBJ->y_lo[slot], r.lift.value);
    jsr_push(0xB8B4);
    r.found = l8_free_slot30();
    jsr_pop();
    if (r.found.found) {
        jsr_push(0xB8BB);
        r.placed = StartObjectAtPlace(PIECE_TYPE, slot, r.found.slot);
        jsr_pop();
        a = RD8(OBJ->y_lo[r.found.slot]);
        r.feet = add_bytes(a, PIECE_FEET, false);
        WR8(OBJ->y_lo[r.found.slot], r.feet.value);
    }
    WR8(OBJ->f3510[slot], 6);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->flash_flags[slot], PIECE_FALL_STEP);
    r.count = INC8(OBJ->state[slot]);
    r.arm = ST25M_DROP_LANDED;
    return r;
}

/* What DroppingPieceStep leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_drop_piece_exit_shim(L8DropPieceArm r)
{
    Ended e;

    if (r.arm == ST25M_DROP_LOADED) {
        e = l8_record_ended(r.record);
        e = then(e, ended_with_y(ended_nz(r.kind), r.kind));
        e = then(e, ended_load(r.speed));
        e = then(e, ended_of_axis_move(r.move1));
        return then(e, ended_with_y(ended_with_carry(ended_load(r.last), (r.kind & 0x80) != 0), r.index));
    }
    if (r.arm == ST25M_DROP_COUNTING) {
        return then(ended_load((uint8_t)(r.state - 1)), l8_clear_on_timer_exit_shim(r.timer));
    }
    e = ended_load((uint8_t)(r.state - 1));
    e = then(e, l8_number_step_exit_shim(r.step));
    e = then(e, l8_flash_piece_exit_shim(r.flash));
    e = then(e, ended_of_axis_move(r.move1));
    e = then(e, ended_of_axis_move(r.move2));
    e = then(e, ended_load(r.last));
    if (r.arm == ST25M_DROP_RISING) {
        return e;
    }
    e = then(e, r.read.e);
    e = then(e, ended_load(r.bit));
    if (r.arm == ST25M_DROP_OFF_FLOOR) {
        return e;
    }
    e = then(e, ended_of_sum(r.lift));
    e = then(e, l8_slot_ended(r.found));
    if (r.found.found) {
        e = then(e, ended_load(r.placed));
        e = then(e, ended_of_sum(r.feet));
    }
    return then(e, ended_nz(r.count));
}

void abi_DroppingPieceStep(void)
{
    abi_ended(l8_drop_piece_exit_shim(DroppingPieceStep(cpu.x, cpu.y)));
}
