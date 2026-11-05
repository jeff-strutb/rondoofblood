/* What an object can tell about the player and about the ground under it. */
#include "machine.h"
#include "ram.h"
#include "routines.h"

/* Bits of a terrain probe's request and of its answer. */
enum {
    PROBE_FLOOR = 0x01, PROBE_WALL = 0x02, PROBE_FLOOR_PUSH = 0x08,
    PROBE_SPIKES_ANSWER = 0x10, PROBE_WALL_PUSH = 0x20
};
/* Tile attributes the overlay's lookups leave in TileHit. */
enum { TILE_SOLID_TOP = 0x21, TILE_SOLID_SIDE = 0x02, TILE_FLOOR_ANY = 0x01, TILE_DEADLY = 0x10 };

/* |object - player| on one axis as a byte, or $FF if not in the first screen. */
static Alu axis_distance(uint8_t object, uint16_t high_field, uint16_t low_field)
{
    uint8_t a = rd8((uint16_t)(high_field + object));
    uint8_t b = rd8(high_field);
    Alu r = { 0 };

    a |= b;
    if (a != 0) {
        r.a = 0xFF;
        r.nz = 0xFF;
        r.c_kept = true;
        return r;
    }
    a = rd8((uint16_t)(low_field + object));
    b = rd8(low_field);
    r = sbc8(a, b, true);
    if (!r.c) {
        r.a = (uint8_t)(~r.a + 1);
        r.nz = r.a;
    }
    return r;
}

/*
 * @routine 0047:2CB DistanceXToPlayer
 * @run 6A:62CB
 * WHAT IT DOES: Gives how far an object is from the player along X, in pixels,
 * as a byte: $FF when either lies past the first 256 pixels.
 * HOW: The high bytes ORed must be zero; then the absolute low difference.
 * Entry: X object. Exit: A distance, N and Z from it, C 1 if the object is the
 * further along, V from the subtract; when $FF, N 1, Z 0, C and V kept. X and Y
 * kept.
 */
Alu DistanceXToPlayer(uint8_t object)
{
    return axis_distance(object, OBJ_X_HI, OBJ_X_LO);
}

void abi_DistanceXToPlayer(void)
{
    abi_alu(DistanceXToPlayer(cpu.x));
}

/*
 * @routine 0047:2E3 DistanceYToPlayer
 * @run 6A:62E3
 * WHAT IT DOES: Gives how far an object is from the player along Y, as
 * DistanceXToPlayer does along X.
 * HOW: The same, on the Y position.
 * Entry: X object. Exit: as DistanceXToPlayer.
 */
Alu DistanceYToPlayer(uint8_t object)
{
    return axis_distance(object, OBJ_Y_HI, OBJ_Y_LO);
}

void abi_DistanceYToPlayer(void)
{
    abi_alu(DistanceYToPlayer(cpu.x));
}

/*
 * @routine 0047:2FB SideOfPlayer
 * @run 6A:62FB
 * WHAT IT DOES: Says which side of the player an object is on: bit 0 set when it
 * is left of the player, bit 1 when above.
 * HOW: On each axis, a high byte of the object nonzero decides by its sign
 * (negative is before the player); zero compares the low byte with the player's.
 * The player's high byte is not looked at.
 * Entry: X object. Exit: A the bits, N and Z from it, C from the last compare
 * made, else kept. zTemp00 bit 0. X, Y and V kept.
 */
Alu SideOfPlayer(uint8_t object)
{
    uint8_t high = rd8((uint16_t)(OBJ_X_HI + object));
    bool before;
    Alu r = { 0 };

    r.c_kept = true;
    if (high != 0) {
        before = (high & 0x80) != 0;
    } else {
        uint8_t mine = rd8((uint16_t)(OBJ_X_LO + object));
        uint8_t player = rd8(OBJ_X_LO);

        r.c = mine >= player;
        r.c_kept = false;
        before = !r.c;
    }
    wr8(zTemp00, before ? 1 : 0);
    high = rd8((uint16_t)(OBJ_Y_HI + object));
    if (high != 0) {
        before = (high & 0x80) != 0;
    } else {
        uint8_t mine = rd8((uint16_t)(OBJ_Y_LO + object));
        uint8_t player = rd8(OBJ_Y_LO);

        r.c = mine >= player;
        r.c_kept = false;
        before = !r.c;
    }
    r.a = rd8(zTemp00);
    if (before)
        r.a |= 0x02;
    r.nz = r.a;
    return r;
}

void abi_SideOfPlayer(void)
{
    abi_alu(SideOfPlayer(cpu.x));
}

/*
 * @routine 0047:3F6 ApplyGravityUnlimited
 * @run 6A:63F6
 * WHAT IT DOES: Adds $38/256 pixel a frame to an object's Y speed, whatever it is.
 * HOW: An 8.8 add carried into the high byte by an increment.
 * Entry: X object. Exit: A the new low byte, C and V from its add, N and Z from it
 * or, on a carry, from the new high byte. X and Y kept.
 */
Alu ApplyGravityUnlimited(uint8_t object)
{
    uint8_t low = rd8((uint16_t)(OBJ_VY_LO + object));
    Alu r = adc8(low, 0x38, false);

    wr8((uint16_t)(OBJ_VY_LO + object), r.a);
    if (r.c) {
        uint8_t high = (uint8_t)(rd8((uint16_t)(OBJ_VY_HI + object)) + 1);

        wr8((uint16_t)(OBJ_VY_HI + object), high);
        r.nz = high;
    }
    return r;
}

void abi_ApplyGravityUnlimited(void)
{
    abi_alu(ApplyGravityUnlimited(cpu.x));
}

/*
 * @routine 0047:3ED ApplyGravity
 * @run 6A:63ED
 * WHAT IT DOES: Pulls an object down: adds $38/256 pixel a frame to its Y speed,
 * until the speed reaches 7 pixels a frame.
 * HOW: Rising speeds always gain; falling speeds of 7 or more do not. Runs into
 * ApplyGravityUnlimited.
 * Entry: X object. Exit, at the limit: A the speed high, N and Z of A - 7, C 1.
 * Exit, gaining: as ApplyGravityUnlimited. X and Y kept.
 */
Alu ApplyGravity(uint8_t object)
{
    uint8_t high = rd8((uint16_t)(OBJ_VY_HI + object));
    Alu r = { 0 };

    if (!(high & 0x80) && high >= 7) {
        r.a = high;
        r.nz = (uint8_t)(high - 7);
        r.c = true;
        return r;
    }
    return ApplyGravityUnlimited(object);
}

void abi_ApplyGravity(void)
{
    abi_alu(ApplyGravity(cpu.x));
}

/*
 * @routine 0047:2BD LookUpTerrainBelow
 * @run 6A:62BD
 * WHAT IT DOES: Reads the terrain around an object through the stage overlay's
 * first terrain lookup, which leaves its answer in TileHit.
 * HOW: Jumps to the overlay's vector.
 * Entry: A probe height, X object. Exit: as the overlay leaves it.
 */
void abi_LookUpTerrainBelow(void)
{
    call_original(OverlayTerrainLookup);
}

/*
 * @routine 0047:2C0 LookUpTerrainBelowAlt
 * @run 6A:62C0
 * WHAT IT DOES: Reads the terrain around an object through the stage overlay's
 * second terrain lookup, which leaves its answer in TileHit.
 * HOW: Jumps to the overlay's vector.
 * Entry: A probe height, X object. Exit: as the overlay leaves it.
 */
void abi_LookUpTerrainBelowAlt(void)
{
    call_original(OverlayTerrainLookupAlt);
}

/*
 * A probe's state along its paths: what the assembly keeps in Y and the flags.
 * The overlay's lookups may leave C and V alone, so what came before them must
 * be in cpu when they are called.
 */
typedef struct {
    uint8_t object;
    uint8_t y;
    Alu last;
} Probe;

static void run_lookup(Probe *p, uint16_t vector, uint8_t height)
{
    abi_alu_flags(&p->last);
    cpu.a = height;
    abi_nz(height);
    cpu.x = p->object;
    cpu.y = p->y;
    call_original(vector);
    p->object = cpu.x;
    p->y = cpu.y;
}

static void or_answer(uint8_t bits)
{
    uint8_t answer = rd8(zProduct0);

    wr8(zProduct0, (uint8_t)(answer | bits));
}

/* Is a wall bit set in the side the object is moving toward? Leaves Y on it. */
static bool wall_on_moving_side(Probe *p)
{
    uint8_t byte;

    p->y = 0;
    byte = rd8((uint16_t)(OBJ_VX_HI + p->object));
    if (!(byte & 0x80))
        p->y = 2;
    byte = rd8((uint16_t)(TileHit + p->y));
    if (byte & TILE_SOLID_SIDE)
        return true;
    p->y++;
    byte = rd8((uint16_t)(TileHit + p->y));
    return (byte & TILE_SOLID_SIDE) != 0;
}

static void push_out_of_wall(Probe *p)
{
    uint8_t low;
    uint8_t other;

    if (p->y < 2) {
        p->y = rd8((uint16_t)(TileDepthX + p->y));
        low = rd8((uint16_t)(OBJ_X_LO + p->object));
        other = rd8((uint16_t)(WallPushTable + p->y));
        p->last = adc8(low, other, false);
        wr8((uint16_t)(OBJ_X_LO + p->object), p->last.a);
        if (p->last.c) {
            low = (uint8_t)(rd8((uint16_t)(OBJ_X_HI + p->object)) + 1);
            wr8((uint16_t)(OBJ_X_HI + p->object), low);
        }
    } else {
        low = rd8((uint16_t)(OBJ_X_LO + p->object));
        other = rd8((uint16_t)(TileDepthX + p->y));
        p->last = sbc8(low, other, true);
        wr8((uint16_t)(OBJ_X_LO + p->object), p->last.a);
        low = rd8((uint16_t)(OBJ_X_HI + p->object));
        p->last = sbc8(low, 0, p->last.c);
        wr8((uint16_t)(OBJ_X_HI + p->object), p->last.a);
    }
}

/* The wall half of a probe, and its end: every probe finishes here. */
static void probe_walls(Probe *p)
{
    uint8_t request = rd8(zTemp12);

    if (request & PROBE_WALL) {
        uint8_t height = rd8(zProduct1);

        run_lookup(p, OverlayTerrainLookup, (uint8_t)(height + 1));
        if (wall_on_moving_side(p)) {
            or_answer(PROBE_WALL);
            request = rd8(zTemp12);
            if (request & PROBE_WALL_PUSH)
                push_out_of_wall(p);
        }
    }
    p->last.a = rd8(zProduct0);
    p->last.nz = p->last.a;
    abi_alu_flags(&p->last);
}

/* Standing on a floor: says so, and on request lifts the object onto it. */
static void probe_floor_hit(Probe *p)
{
    uint8_t request;

    or_answer(PROBE_FLOOR);
    request = rd8(zTemp12);
    if (request & PROBE_FLOOR_PUSH) {
        uint8_t depth = rd8((uint16_t)(TileDepthY + p->y));
        uint8_t sign = (depth & 0x80) ? 0xFF : 0x00;
        uint8_t byte;

        wr8(zTemp00, sign);
        byte = rd8((uint16_t)(OBJ_Y_LO + p->object));
        depth = rd8((uint16_t)(TileDepthY + p->y));
        p->last = sbc8(byte, depth, true);
        wr8((uint16_t)(OBJ_Y_LO + p->object), p->last.a);
        byte = rd8((uint16_t)(OBJ_Y_HI + p->object));
        sign = rd8(zTemp00);
        p->last = sbc8(byte, sign, p->last.c);
        wr8((uint16_t)(OBJ_Y_HI + p->object), p->last.a);
    }
}

/* Deadly ground throws the object up at a fixed speed, with no X speed. */
static void probe_deadly_hit(Probe *p)
{
    wr8((uint16_t)(OBJ_VX_HI + p->object), 0);
    wr8((uint16_t)(OBJ_VX_LO + p->object), 0);
    wr8((uint16_t)(OBJ_VY_HI + p->object), 0);
    wr8((uint16_t)(OBJ_VY_LO + p->object), 0x40);
    or_answer(PROBE_SPIKES_ANSWER);
}

/* Reads the three columns the lookup filled and acts on what is under them. */
static void probe_read_floor(Probe *p)
{
    uint8_t all = rd8(TileHit);
    uint8_t more = rd8(TileHit + 1);

    all |= more;
    more = rd8(TileHit + 2);
    all |= more;
    if (all & TILE_DEADLY) {
        probe_deadly_hit(p);
        return;
    }
    p->y = 0;
    do {
        uint8_t hit = rd8((uint16_t)(TileHit + p->y));

        if (hit & TILE_SOLID_TOP) {
            probe_floor_hit(p);
            return;
        }
        p->y++;
    } while (p->y < 3);
    p->last.c = true;
    p->last.c_kept = false;
}

static Probe probe_start(uint8_t request, uint8_t object, uint8_t y)
{
    Probe p = { object, y, { 0 } };

    wr8(zProduct0, 0);
    wr8(zTemp12, request);
    p.last.c_kept = true;
    return p;
}

static uint8_t probe_height(void)
{
    return rd8(zProduct1);
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
 * Entry: A request, X object, zProduct1 probe height. Exit: A answer, N and Z from
 * it; Y, C and V as the path left them. Clobbers zTemp00, zTemp12, zProduct0.
 */
Probe ProbeTerrain(uint8_t request, uint8_t object, uint8_t y)
{
    Probe p = probe_start(request, object, y);

    if (request & PROBE_FLOOR) {
        run_lookup(&p, OverlayTerrainLookup, probe_height());
        probe_read_floor(&p);
    }
    probe_walls(&p);
    return p;
}

static void abi_exit_probe(Probe p)
{
    abi_alu(p.last);
    cpu.x = p.object;
    cpu.y = p.y;
}

void abi_ProbeTerrain(void)
{
    abi_exit_probe(ProbeTerrain(cpu.a, cpu.x, cpu.y));
}

/*
 * @routine 0047:282 ProbeTerrainAlt
 * @run 6A:6282
 * WHAT IT DOES: ProbeTerrain, using the overlay's second lookup for the floor.
 * HOW: As ProbeTerrain.
 * Entry and exit: as ProbeTerrain.
 */
Probe ProbeTerrainAlt(uint8_t request, uint8_t object, uint8_t y)
{
    Probe p = probe_start(request, object, y);

    if (request & PROBE_FLOOR) {
        run_lookup(&p, OverlayTerrainLookupAlt, probe_height());
        probe_read_floor(&p);
    }
    probe_walls(&p);
    return p;
}

void abi_ProbeTerrainAlt(void)
{
    abi_exit_probe(ProbeTerrainAlt(cpu.a, cpu.x, cpu.y));
}

/*
 * @routine 0047:295 ProbeTerrainFrontColumn
 * @run 6A:6295
 * WHAT IT DOES: ProbeTerrainAlt looking at the first column only.
 * HOW: The overlay's second lookup, then one column of TileHit: deadly throws;
 * any floor bit is floor.
 * Entry and exit: as ProbeTerrain.
 */
Probe ProbeTerrainFrontColumn(uint8_t request, uint8_t object, uint8_t y)
{
    Probe p = probe_start(request, object, y);

    if (request & PROBE_FLOOR) {
        uint8_t hit;

        run_lookup(&p, OverlayTerrainLookupAlt, probe_height());
        hit = rd8(TileHit);
        if (hit & TILE_DEADLY) {
            probe_deadly_hit(&p);
        } else {
            p.y = 0;
            hit = rd8(TileHit);
            if (hit & TILE_FLOOR_ANY)
                probe_floor_hit(&p);
        }
    }
    probe_walls(&p);
    return p;
}

void abi_ProbeTerrainFrontColumn(void)
{
    abi_exit_probe(ProbeTerrainFrontColumn(cpu.a, cpu.x, cpu.y));
}

/* Standard requests the movers make. */
enum { REQUEST_FLOOR_LIFT = 0x09, REQUEST_WALL_PUSH = 0x22 };

/* A mover's result: what it returns in A with C for hit a wall or floor. */
static Alu mover_result(uint8_t value, bool carry, Probe p)
{
    Alu r = p.last;

    r.a = value;
    r.nz = value;
    r.c = carry;
    r.c_kept = false;
    return r;
}

static void mover_start(uint8_t height, uint8_t object)
{
    Alu step;

    wr8(zProduct1, height);
    step = ApplyVelocity(object);
    abi_alu_flags(&step);
    wr8(WorkByteA, 0);
    wr8(WorkByteB, 0);
    step = ApplyGravity(object);
    abi_alu_flags(&step);
}

/*
 * @routine 0047:32D MoveAndLand
 * @run 6A:632D
 * WHAT IT DOES: One frame of a falling, walking object: moves it, applies
 * gravity, lands it on floor (stopping its fall) and pushes it out of walls.
 * HOW: ApplyVelocity, ApplyGravity; a floor probe only when not rising, then a
 * wall probe.
 * Entry: A probe height, X object. Exit: A bit 0 landed, bit 1 walled; C 1 if
 * walled; N and Z from A. Y and V as the probes left them. X kept.
 */
Alu MoveAndLand(uint8_t height, uint8_t object, uint8_t y, uint8_t *exit_y)
{
    uint8_t byte;
    Probe p;

    mover_start(height, object);
    byte = rd8((uint16_t)(OBJ_VY_HI + object));
    if (!(byte & 0x80)) {
        p = ProbeTerrain(REQUEST_FLOOR_LIFT, object, y);
        object = p.object;
        y = p.y;
        if (p.last.a & PROBE_FLOOR) {
            byte = (uint8_t)(rd8(WorkByteB) + 1);
            wr8(WorkByteB, byte);
            wr8((uint16_t)(OBJ_VY_HI + object), 0);
            wr8((uint16_t)(OBJ_VY_LO + object), 0);
        }
    }
    p = ProbeTerrain(REQUEST_WALL_PUSH, object, y);
    *exit_y = p.y;
    byte = rd8(WorkByteB);
    if (p.last.a & PROBE_WALL)
        return mover_result((uint8_t)(byte | PROBE_WALL), true, p);
    return mover_result(byte, false, p);
}

void abi_MoveAndLand(void)
{
    uint8_t y;

    abi_alu(MoveAndLand(cpu.a, cpu.x, cpu.y, &y));
    cpu.y = y;
}

/* The end of the two movers below: the wall probe and stopping on landing. */
static Alu mover_finish(uint8_t object, uint8_t y, uint8_t *exit_y)
{
    Probe p = ProbeTerrain(REQUEST_WALL_PUSH, object, y);
    uint8_t byte;

    object = p.object;
    *exit_y = p.y;
    byte = p.last.a & PROBE_WALL;
    wr8(WorkByteB, byte);
    byte = rd8(WorkByteA);
    if (byte & PROBE_FLOOR) {
        wr8((uint16_t)(OBJ_VX_HI + object), 0);
        wr8((uint16_t)(OBJ_VX_LO + object), 0);
        wr8((uint16_t)(OBJ_VY_HI + object), 0);
        wr8((uint16_t)(OBJ_VY_LO + object), 0);
        byte = rd8(WorkByteB);
        return mover_result((uint8_t)(byte | PROBE_FLOOR), true, p);
    }
    byte = rd8(WorkByteB);
    return mover_result(byte, false, p);
}

/*
 * @routine 0047:375 StopOnLandingNoMove
 * @run 6A:6375
 * WHAT IT DOES: The landing half of StopOnLanding, for a caller that has moved
 * the object itself.
 * HOW: Floor probe with the overlay's second lookup when not rising, then the
 * wall probe and the stop.
 * Entry: X object, zProduct1 probe height, WorkByteA clear. Exit: A bit 0
 * landed, bit 1 walled; C 1 if landed; N and Z from A. Y and V as the probes left
 * them.
 */
Alu StopOnLandingNoMove(uint8_t object, uint8_t y, uint8_t *exit_y)
{
    uint8_t byte = rd8((uint16_t)(OBJ_VY_HI + object));

    if (!(byte & 0x80)) {
        Probe p = ProbeTerrainAlt(REQUEST_FLOOR_LIFT, object, y);

        object = p.object;
        y = p.y;
        wr8(WorkByteA, p.last.a);
    }
    return mover_finish(object, y, exit_y);
}

void abi_StopOnLandingNoMove(void)
{
    uint8_t y;

    abi_alu(StopOnLandingNoMove(cpu.x, cpu.y, &y));
    cpu.y = y;
}

/*
 * @routine 0047:367 StopOnLanding
 * @run 6A:6367
 * WHAT IT DOES: One frame of a thrown object that stops dead where it lands: moves
 * it, applies gravity, and on landing zeroes all its speed.
 * HOW: The start of MoveAndLand, then runs into StopOnLandingNoMove.
 * Entry: A probe height, X object. Exit: as StopOnLandingNoMove.
 */
Alu StopOnLanding(uint8_t height, uint8_t object, uint8_t y, uint8_t *exit_y)
{
    mover_start(height, object);
    return StopOnLandingNoMove(object, y, exit_y);
}

void abi_StopOnLanding(void)
{
    uint8_t y;

    abi_alu(StopOnLanding(cpu.a, cpu.x, cpu.y, &y));
    cpu.y = y;
}

/*
 * @routine 0047:3AB StopOnLandingFrontColumn
 * @run 6A:63AB
 * WHAT IT DOES: StopOnLanding, reading only the first column of floor.
 * HOW: The same start, ProbeTerrainFrontColumn for the floor, the same end.
 * Entry: A probe height, X object. Exit: as StopOnLandingNoMove.
 */
Alu StopOnLandingFrontColumn(uint8_t height, uint8_t object, uint8_t y, uint8_t *exit_y)
{
    uint8_t byte;

    mover_start(height, object);
    byte = rd8((uint16_t)(OBJ_VY_HI + object));
    if (!(byte & 0x80)) {
        Probe p = ProbeTerrainFrontColumn(REQUEST_FLOOR_LIFT, object, y);

        object = p.object;
        y = p.y;
        wr8(WorkByteA, p.last.a);
    }
    return mover_finish(object, y, exit_y);
}

void abi_StopOnLandingFrontColumn(void)
{
    uint8_t y;

    abi_alu(StopOnLandingFrontColumn(cpu.a, cpu.x, cpu.y, &y));
    cpu.y = y;
}
