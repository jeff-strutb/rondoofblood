/*
 * What an object can tell about the player and about the ground under it.
 *
 * A routine's body returns how it ended, as plain values. Its wrapper replays what those steps
 * leave in the registers and flags, in the assembly's order, through the *_exit_shim functions.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"

/* Bits of a terrain probe's request and of its answer. */
enum {
    PROBE_FLOOR = 0x01, PROBE_WALL = 0x02, PROBE_FLOOR_PUSH = 0x08,
    PROBE_SPIKES_ANSWER = 0x10, PROBE_WALL_PUSH = 0x20
};
/* Tile attributes the overlay's lookups leave in TileHit. */
enum { TILE_SOLID_TOP = 0x21, TILE_SOLID_SIDE = 0x02, TILE_FLOOR_ANY = 0x01, TILE_DEADLY = 0x10 };
enum {
    GRAVITY = 0x38, FALL_LIMIT = 7, FLOOR_COLUMNS = 3, COLUMNS_ON_LEFT = 2, DEADLY_THROW = 0x40,
    REQUEST_FLOOR_LIFT = 0x09, REQUEST_WALL_PUSH = 0x22
};

/* |object - player| on one axis as a byte, or $FF if not in the first screen. */
static PlayerDistance axis_distance(uint8_t object, uint16_t high_column, uint16_t low_column)
{
    PlayerDistance d = { 0xFF, true, false, 0, 0 };
    uint8_t high = RD8_IDX(high_column, object);
    uint8_t player_high = RD8_IDX(high_column, 0);

    if ((high | player_high) == 0) {
        d.mine = RD8_IDX(low_column, object);
        d.player = RD8_IDX(low_column, 0);
        d.far = false;
        d.object_further = d.mine >= d.player;
        d.distance = d.object_further ? (uint8_t)(d.mine - d.player) : (uint8_t)(d.player - d.mine);
    }
    return d;
}

/* A and N and Z from the distance; C and V from the subtract, if one was made. */
void distance_exit_shim(PlayerDistance d)
{
    cpu.a = d.distance;
    abi_nz(d.distance);
    if (!d.far) {
        uint8_t difference = (uint8_t)(d.mine - d.player);

        abi_flag(P_C, d.object_further);
        abi_flag(P_V, ((d.mine ^ d.player) & (d.mine ^ difference) & 0x80) != 0);
    }
}

/*
 * @routine 0047:2CB DistanceXToPlayer
 * @run 6A:62CB
 * WHAT IT DOES: Gives how far an object is from the player along X, in pixels,
 * as a byte: $FF when either lies past the first 256 pixels.
 * HOW: The high bytes ORed must be zero; then the absolute low difference.
 * WAITS: no.
 * Entry: X object. Exit: A distance, N and Z from it, C 1 if the object is the
 * further along, V from the subtract; when $FF, N 1, Z 0, C and V kept. X and Y
 * kept.
 */
PlayerDistance DistanceXToPlayer(uint8_t object)
{
    return axis_distance(object, COL(OBJ->x_hi), COL(OBJ->x_lo));
}

void abi_DistanceXToPlayer(void)
{
    distance_exit_shim(DistanceXToPlayer(cpu.x));
}

/*
 * @routine 0047:2E3 DistanceYToPlayer
 * @run 6A:62E3
 * WHAT IT DOES: Gives how far an object is from the player along Y, as
 * DistanceXToPlayer does along X.
 * HOW: The same, on the Y position.
 * WAITS: no.
 * Entry: X object. Exit: as DistanceXToPlayer.
 */
PlayerDistance DistanceYToPlayer(uint8_t object)
{
    return axis_distance(object, COL(OBJ->y_hi), COL(OBJ->y_lo));
}

void abi_DistanceYToPlayer(void)
{
    distance_exit_shim(DistanceYToPlayer(cpu.x));
}

/*
 * @routine 0047:2FB SideOfPlayer
 * @run 6A:62FB
 * WHAT IT DOES: Says which side of the player an object is on: bit 0 set when it
 * is left of the player, bit 1 when above.
 * HOW: On each axis, a high byte of the object nonzero decides by its sign
 * (negative is before the player); zero compares the low byte with the player's.
 * The player's high byte is not looked at. The bit for X goes to zTemp00, which is read
 * back at the end.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the bits, N and Z from it, C from the last compare
 * made (Y's when the Y high byte is zero, else X's when that is zero), else kept. zTemp00
 * bit 0. X, Y and V kept.
 */
SideResult SideOfPlayer(uint8_t object)
{
    SideResult result = { 0, false, false };
    uint8_t high, mine, player;

    high = RD8(OBJ->x_hi[object]);
    if (high == 0) {
        mine = RD8(OBJ->x_lo[object]);
        player = RD8(OBJ->x_lo[0]);
        result.carry_set = true;
        result.carry = mine >= player;
        WR8(ZP->temp00, mine < player ? 1 : 0);
    } else {
        WR8(ZP->temp00, (high & 0x80) != 0 ? 1 : 0);
    }
    high = RD8(OBJ->y_hi[object]);
    if (high == 0) {
        mine = RD8(OBJ->y_lo[object]);
        player = RD8(OBJ->y_lo[0]);
        result.carry_set = true;
        result.carry = mine >= player;
        if (mine < player) {
            result.sides = (uint8_t)(RD8(ZP->temp00) | 2);
            return result;
        }
    } else if ((high & 0x80) != 0) {
        result.sides = (uint8_t)(RD8(ZP->temp00) | 2);
        return result;
    }
    result.sides = RD8(ZP->temp00);
    return result;
}

void side_of_player_exit_shim(SideResult result)
{
    cpu.a = result.sides;
    abi_nz(result.sides);
    if (result.carry_set) {
        abi_flag(P_C, result.carry);
    }
}

void abi_SideOfPlayer(void)
{
    side_of_player_exit_shim(SideOfPlayer(cpu.x));
}

/*
 * @routine 0047:3F6 ApplyGravityUnlimited
 * @run 6A:63F6
 * WHAT IT DOES: Adds $38/256 pixel a frame to an object's Y speed, whatever it is.
 * HOW: An 8.8 add carried into the high byte by an increment.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the new low byte, C and V from its add, N and Z from it
 * or, on a carry, from the new high byte. X and Y kept.
 */
GravityResult ApplyGravityUnlimited(uint8_t object)
{
    GravityResult result = { false, 0, 0, false, false, false };
    uint8_t low = RD8(OBJ->vy_lo[object]);
    Sum sum = add_bytes(low, GRAVITY, false);

    WR8(OBJ->vy_lo[object], sum.value);
    result.gained = true;
    result.low = sum.value;
    result.carried = sum.carry;
    result.overflow = sum.overflow;
    if (sum.carry) {
        result.high = INC8(OBJ->vy_hi[object]);
    }
    return result;
}

/*
 * @routine 0047:3ED ApplyGravity
 * @run 6A:63ED
 * WHAT IT DOES: Pulls an object down: adds $38/256 pixel a frame to its Y speed,
 * until the speed reaches 7 pixels a frame.
 * HOW: Rising speeds always gain; falling speeds of 7 or more do not. Runs into
 * ApplyGravityUnlimited.
 * WAITS: no.
 * Entry: X object, D clear. Exit, at the limit: A the speed high, N and Z of A - 7, C 1.
 * Exit, gaining: as ApplyGravityUnlimited. X and Y kept.
 */
GravityResult ApplyGravity(uint8_t object)
{
    uint8_t high = RD8(OBJ->vy_hi[object]);
    GravityResult result = { false, 0, 0, false, false, false };

    if ((high & 0x80) == 0 && high >= FALL_LIMIT) {
        result.high = high;
        return result;
    }
    return ApplyGravityUnlimited(object);
}

/* What the add, or at the limit the compare, leaves in A and the flags. V stands at the limit. */
void gravity_exit_shim(GravityResult result)
{
    if (result.gained) {
        cpu.a = result.low;
        abi_nz(result.carried ? result.high : result.low);
        abi_flag(P_C, result.carried);
        abi_flag(P_V, result.overflow);
    } else {
        cpu.a = result.high;
        abi_nz((uint8_t)(result.high - FALL_LIMIT));
        abi_flag(P_C, true);
    }
}

void abi_ApplyGravityUnlimited(void)
{
    gravity_exit_shim(ApplyGravityUnlimited(cpu.x));
}

void abi_ApplyGravity(void)
{
    gravity_exit_shim(ApplyGravity(cpu.x));
}

/*
 * @routine 0047:2BD LookUpTerrainBelow
 * @run 6A:62BD
 * WHAT IT DOES: Reads the terrain around an object through the stage overlay's
 * first terrain lookup, which leaves its answer in TileHit.
 * HOW: Jumps to the overlay's vector.
 * WAITS: no.
 * Entry: A probe height, X object. Exit: as the overlay leaves it.
 */
void abi_LookUpTerrainBelow(void)
{
    lookup_exit_shim(StageLookup(cpu.a, cpu.x, cpu.y));
}

/*
 * @routine 0047:2C0 LookUpTerrainBelowAlt
 * @run 6A:62C0
 * WHAT IT DOES: Reads the terrain around an object through the stage overlay's
 * second terrain lookup, which leaves its answer in TileHit.
 * HOW: Jumps to the overlay's vector.
 * WAITS: no.
 * Entry: A probe height, X object. Exit: as the overlay leaves it.
 */
void abi_LookUpTerrainBelowAlt(void)
{
    lookup_exit_shim(StageLookupAlt(cpu.a, cpu.x, cpu.y));
}

static void add_to_answer(uint8_t bits)
{
    wr8(zProduct0, (uint8_t)(rd8(zProduct0) | bits));
}

/* Is a wall bit set in the side the object is moving toward? Leaves the column on it. */
static bool wall_on_moving_side(Probe *p)
{
    p->column = (rd8((uint16_t)(OBJ_VX_HI + p->object)) & 0x80) ? 0 : COLUMNS_ON_LEFT;
    if (rd8((uint16_t)(TileHit + p->column)) & TILE_SOLID_SIDE)
        return true;
    p->column++;
    return (rd8((uint16_t)(TileHit + p->column)) & TILE_SOLID_SIDE) != 0;
}

static void push_out_of_wall(Probe *p)
{
    uint16_t lo = (uint16_t)(OBJ_X_LO + p->object);
    uint16_t hi = (uint16_t)(OBJ_X_HI + p->object);

    if (p->column < COLUMNS_ON_LEFT) {
        uint8_t depth = rd8((uint16_t)(TileDepthX + p->column));
        uint8_t push = rd8((uint16_t)(WallPushTable + depth));
        uint8_t low = rd8(lo);

        p->column = depth;
        p->pushed = PUSHED_RIGHT;
        p->push_before = low;
        p->push_amount = push;
        wr8(lo, (uint8_t)(low + push));
        if (low + push > 0xFF)
            wr8(hi, (uint8_t)(rd8(hi) + 1));
    } else {
        uint16_t x = (uint16_t)(rd8(lo) | (rd8(hi) << 8));
        uint16_t depth = rd8((uint16_t)(TileDepthX + p->column));
        uint16_t pushed = (uint16_t)(x - depth);

        p->pushed = PUSHED_LEFT;
        p->push_before = x;
        p->push_amount = depth;
        wr8(lo, (uint8_t)pushed);
        wr8(hi, (uint8_t)(pushed >> 8));
    }
}

/* The wall half of a probe, and its end: every probe finishes here. */
static void probe_walls(Probe *p)
{
    if (rd8(zTemp12) & PROBE_WALL) {
        p->wall_asked = true;
        p->wall = StageLookup((uint8_t)(rd8(zProduct1) + 1), p->object, p->column);
        p->column = p->wall.column_after;
        if (wall_on_moving_side(p)) {
            add_to_answer(PROBE_WALL);
            if (rd8(zTemp12) & PROBE_WALL_PUSH)
                push_out_of_wall(p);
        }
    }
    p->answer = rd8(zProduct0);
}

/* Standing on a floor: says so, and on request lifts the object onto it. */
static void probe_floor_hit(Probe *p)
{
    add_to_answer(PROBE_FLOOR);
    if (rd8(zTemp12) & PROBE_FLOOR_PUSH) {
        uint16_t lo = (uint16_t)(OBJ_Y_LO + p->object);
        uint16_t hi = (uint16_t)(OBJ_Y_HI + p->object);
        uint16_t depth = (uint16_t)(int16_t)(int8_t)rd8((uint16_t)(TileDepthY + p->column));
        uint16_t y = (uint16_t)(rd8(lo) | (rd8(hi) << 8));
        uint16_t lifted = (uint16_t)(y - depth);

        p->lifted = true;
        p->lift_before = y;
        p->lift_amount = depth;
        wr8(zTemp00, (uint8_t)(depth >> 8));
        wr8(lo, (uint8_t)lifted);
        wr8(hi, (uint8_t)(lifted >> 8));
    }
}

/* Deadly ground throws the object up at a fixed speed, with no X speed. */
static void probe_deadly_hit(const Probe *p)
{
    wr8((uint16_t)(OBJ_VX_HI + p->object), 0);
    wr8((uint16_t)(OBJ_VX_LO + p->object), 0);
    wr8((uint16_t)(OBJ_VY_HI + p->object), 0);
    wr8((uint16_t)(OBJ_VY_LO + p->object), DEADLY_THROW);
    add_to_answer(PROBE_SPIKES_ANSWER);
}

/* Reads the three columns the lookup filled and acts on what is under them. */
static void probe_read_floor(Probe *p)
{
    uint8_t all = (uint8_t)(rd8(TileHit) | rd8(TileHit + 1) | rd8(TileHit + 2));

    if (all & TILE_DEADLY) {
        probe_deadly_hit(p);
        return;
    }
    for (p->column = 0; p->column < FLOOR_COLUMNS; p->column++)
        if (rd8((uint16_t)(TileHit + p->column)) & TILE_SOLID_TOP)
            break;
    p->scanned = true;
    p->scanned_to = p->column;
    if (p->column < FLOOR_COLUMNS)
        probe_floor_hit(p);
}

static Probe probe_start(uint8_t request, uint8_t object, uint8_t column)
{
    Probe p = { object, column };

    wr8(zProduct0, 0);
    wr8(zTemp12, request);
    return p;
}

/* C and V from a word's subtract, as the lifts and the left push leave them. */
static void word_subtract_shim(uint16_t before, uint16_t amount)
{
    uint16_t after = (uint16_t)(before - amount);

    abi_flag(P_C, before >= amount);
    abi_flag(P_V, ((before ^ amount) & (before ^ after) & 0x8000) != 0);
}

/* Registers and flags as a probe's steps leave them, in the assembly's order. */
void probe_exit_shim(Probe p)
{
    if (p.floor_asked) {
        lookup_exit_shim(p.floor);
        if (p.scanned && p.scanned_to > 0)
            abi_flag(P_C, p.scanned_to >= FLOOR_COLUMNS);
        if (p.lifted)
            word_subtract_shim(p.lift_before, p.lift_amount);
    }
    if (p.wall_asked) {
        lookup_exit_shim(p.wall);
        if (p.pushed == PUSHED_RIGHT) {
            uint8_t before = (uint8_t)p.push_before;
            uint8_t amount = (uint8_t)p.push_amount;

            abi_flag(P_C, before + amount > 0xFF);
            abi_flag(P_V, (~(before ^ amount) & (before ^ (uint8_t)(before + amount)) & 0x80) != 0);
        } else if (p.pushed == PUSHED_LEFT) {
            word_subtract_shim(p.push_before, p.push_amount);
        }
    }
    cpu.a = p.answer;
    abi_nz(p.answer);
    cpu.x = p.object;
    cpu.y = p.column;
}

/*
 * @routine 0047:1BC ProbeTerrain
 * @run 6A:61BC
 * WHAT IT DOES: Asks what terrain an object touches, and on request acts on it.
 * Bit 0 asks about the floor, bit 3 lifts the object out of it; bit 1 asks about
 * walls, bit 5 pushes the object out. The answer has bit 0 for floor, bit 1 for
 * wall, bit 4 for deadly ground, which also throws the object.
 * HOW: The overlay's lookup fills TileHit and the depths; the floor half reads
 * three columns, the wall half the two on the side the object is moving to.
 * WAITS: no.
 * Entry: A request, X object, zProduct1 probe height. Exit: A answer, N and Z from
 * it; Y, C and V as the path left them. Clobbers zTemp00, zTemp12, zProduct0.
 */
Probe ProbeTerrain(uint8_t request, uint8_t object, uint8_t column)
{
    Probe p = probe_start(request, object, column);

    if (request & PROBE_FLOOR) {
        p.floor_asked = true;
        p.floor = StageLookup(rd8(zProduct1), p.object, p.column);
        p.column = p.floor.column_after;
        probe_read_floor(&p);
    }
    probe_walls(&p);
    return p;
}

void abi_ProbeTerrain(void)
{
    probe_exit_shim(ProbeTerrain(cpu.a, cpu.x, cpu.y));
}

/*
 * @routine 0047:282 ProbeTerrainAlt
 * @run 6A:6282
 * WHAT IT DOES: ProbeTerrain, using the overlay's second lookup for the floor.
 * HOW: As ProbeTerrain.
 * WAITS: no.
 * Entry and exit: as ProbeTerrain.
 */
Probe ProbeTerrainAlt(uint8_t request, uint8_t object, uint8_t column)
{
    Probe p = probe_start(request, object, column);

    if (request & PROBE_FLOOR) {
        p.floor_asked = true;
        p.floor = StageLookupAlt(rd8(zProduct1), p.object, p.column);
        p.column = p.floor.column_after;
        probe_read_floor(&p);
    }
    probe_walls(&p);
    return p;
}

void abi_ProbeTerrainAlt(void)
{
    probe_exit_shim(ProbeTerrainAlt(cpu.a, cpu.x, cpu.y));
}

/*
 * @routine 0047:295 ProbeTerrainFrontColumn
 * @run 6A:6295
 * WHAT IT DOES: ProbeTerrainAlt looking at the first column only.
 * HOW: The overlay's second lookup, then one column of TileHit: deadly throws;
 * any floor bit is floor.
 * WAITS: no.
 * Entry and exit: as ProbeTerrain.
 */
Probe ProbeTerrainFrontColumn(uint8_t request, uint8_t object, uint8_t column)
{
    Probe p = probe_start(request, object, column);

    if (request & PROBE_FLOOR) {
        p.floor_asked = true;
        p.floor = StageLookupAlt(rd8(zProduct1), p.object, p.column);
        p.column = p.floor.column_after;
        if (rd8(TileHit) & TILE_DEADLY) {
            probe_deadly_hit(&p);
        } else {
            p.column = 0;
            if (rd8(TileHit) & TILE_FLOOR_ANY)
                probe_floor_hit(&p);
        }
    }
    probe_walls(&p);
    return p;
}

void abi_ProbeTerrainFrontColumn(void)
{
    probe_exit_shim(ProbeTerrainFrontColumn(cpu.a, cpu.x, cpu.y));
}

/* How a mover's start went: the move and whether gravity gained. */
typedef struct {
    bool moved;
    AxisMove move;
    GravityResult gravity;
} MoverStart;

/* How a mover went, for what the wrapper replays. */
typedef struct {
    uint8_t object;
    MoverStart start;
    bool floor_probed;
    Probe floor;
    Probe wall;
    uint8_t hits;
} Mover;

static MoverStart mover_start(uint8_t height, uint8_t object)
{
    MoverStart start = { true };

    wr8(zProduct1, height);
    start.move = ApplyVelocity(object);
    wr8(WorkByteA, 0);
    wr8(WorkByteB, 0);
    start.gravity = ApplyGravity(object);
    return start;
}

/* A the hits, and C says the bit asked about is among them. X and Y as the last probe left them. */
static void mover_exit_shim(Mover mover, uint8_t carry_bit)
{
    if (mover.start.moved) {
        axis_move_flags_shim(mover.start.move);
        gravity_exit_shim(mover.start.gravity);
    }
    if (mover.floor_probed)
        probe_exit_shim(mover.floor);
    probe_exit_shim(mover.wall);
    cpu.a = mover.hits;
    abi_nz(mover.hits);
    abi_flag(P_C, (mover.hits & carry_bit) != 0);
}

/*
 * @routine 0047:32D MoveAndLand
 * @run 6A:632D
 * WHAT IT DOES: One frame of a falling, walking object: moves it, applies
 * gravity, lands it on floor (stopping its fall) and pushes it out of walls.
 * HOW: ApplyVelocity, ApplyGravity; a floor probe only when not rising, then a
 * wall probe.
 * WAITS: no.
 * Entry: A probe height, X object. Exit: A bit 0 landed, bit 1 walled; C 1 if
 * walled; N and Z from A. Y and V as the probes left them. X kept.
 */
Mover MoveAndLand(uint8_t height, uint8_t object, uint8_t column)
{
    Mover mover = { object };

    mover.start = mover_start(height, object);
    if (!(rd8((uint16_t)(OBJ_VY_HI + object)) & 0x80)) {
        mover.floor_probed = true;
        mover.floor = ProbeTerrain(REQUEST_FLOOR_LIFT, object, column);
        column = mover.floor.column;
        if (mover.floor.answer & PROBE_FLOOR) {
            wr8(WorkByteB, (uint8_t)(rd8(WorkByteB) + 1));
            wr8((uint16_t)(OBJ_VY_HI + object), 0);
            wr8((uint16_t)(OBJ_VY_LO + object), 0);
        }
    }
    mover.wall = ProbeTerrain(REQUEST_WALL_PUSH, object, column);
    mover.hits = rd8(WorkByteB);
    if (mover.wall.answer & PROBE_WALL)
        mover.hits |= PROBE_WALL;
    return mover;
}

void abi_MoveAndLand(void)
{
    mover_exit_shim(MoveAndLand(cpu.a, cpu.x, cpu.y), PROBE_WALL);
}

/* The end of the movers below: the wall probe and stopping on landing. */
static void mover_finish(Mover *mover, uint8_t column)
{
    mover->wall = ProbeTerrain(REQUEST_WALL_PUSH, mover->object, column);
    mover->hits = mover->wall.answer & PROBE_WALL;
    wr8(WorkByteB, mover->hits);
    if (rd8(WorkByteA) & PROBE_FLOOR) {
        wr8((uint16_t)(OBJ_VX_HI + mover->object), 0);
        wr8((uint16_t)(OBJ_VX_LO + mover->object), 0);
        wr8((uint16_t)(OBJ_VY_HI + mover->object), 0);
        wr8((uint16_t)(OBJ_VY_LO + mover->object), 0);
        mover->hits |= PROBE_FLOOR;
    }
}

/* The floor probe when not rising, by the one given, then the end. */
static void land_with(Mover *mover, Probe (*probe)(uint8_t, uint8_t, uint8_t), uint8_t column)
{
    if (!(rd8((uint16_t)(OBJ_VY_HI + mover->object)) & 0x80)) {
        mover->floor_probed = true;
        mover->floor = probe(REQUEST_FLOOR_LIFT, mover->object, column);
        column = mover->floor.column;
        wr8(WorkByteA, mover->floor.answer);
    }
    mover_finish(mover, column);
}

/*
 * @routine 0047:375 StopOnLandingNoMove
 * @run 6A:6375
 * WHAT IT DOES: The landing half of StopOnLanding, for a caller that has moved
 * the object itself.
 * HOW: Floor probe with the overlay's second lookup when not rising, then the
 * wall probe and the stop.
 * WAITS: no.
 * Entry: X object, zProduct1 probe height, WorkByteA clear. Exit: A bit 0
 * landed, bit 1 walled; C 1 if landed; N and Z from A. Y and V as the probes left
 * them.
 */
Mover StopOnLandingNoMove(uint8_t object, uint8_t column)
{
    Mover mover = { object };

    land_with(&mover, ProbeTerrainAlt, column);
    return mover;
}

void abi_StopOnLandingNoMove(void)
{
    mover_exit_shim(StopOnLandingNoMove(cpu.x, cpu.y), PROBE_FLOOR);
}

/*
 * @routine 0047:367 StopOnLanding
 * @run 6A:6367
 * WHAT IT DOES: One frame of a thrown object that stops dead where it lands: moves
 * it, applies gravity, and on landing zeroes all its speed.
 * HOW: The start of MoveAndLand, then runs into StopOnLandingNoMove.
 * WAITS: no.
 * Entry: A probe height, X object. Exit: as StopOnLandingNoMove.
 */
Mover StopOnLanding(uint8_t height, uint8_t object, uint8_t column)
{
    Mover mover = { object };

    mover.start = mover_start(height, object);
    land_with(&mover, ProbeTerrainAlt, column);
    return mover;
}

void abi_StopOnLanding(void)
{
    mover_exit_shim(StopOnLanding(cpu.a, cpu.x, cpu.y), PROBE_FLOOR);
}

/*
 * @routine 0047:3AB StopOnLandingFrontColumn
 * @run 6A:63AB
 * WHAT IT DOES: StopOnLanding, reading only the first column of floor.
 * HOW: The same start, ProbeTerrainFrontColumn for the floor, the same end.
 * WAITS: no.
 * Entry: A probe height, X object. Exit: as StopOnLandingNoMove.
 */
Mover StopOnLandingFrontColumn(uint8_t height, uint8_t object, uint8_t column)
{
    Mover mover = { object };

    mover.start = mover_start(height, object);
    land_with(&mover, ProbeTerrainFrontColumn, column);
    return mover;
}

void abi_StopOnLandingFrontColumn(void)
{
    mover_exit_shim(StopOnLandingFrontColumn(cpu.a, cpu.x, cpu.y), PROBE_FLOOR);
}
