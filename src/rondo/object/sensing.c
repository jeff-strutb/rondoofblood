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
    lookup_exit_shim(StageLookup(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0));
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
    lookup_exit_shim(StageLookupAlt(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0));
}

/* zProduct0 is the probe's answer: bits are ORed into it, read and written back. */
static void add_to_answer(uint8_t bits)
{
    uint8_t answer = RD8(ZP->product.lo);

    WR8(ZP->product.lo, (uint8_t)(answer | bits));
}

/* The probe's start: the answer cleared and the request kept in zTemp12. */
static Probe probe_start(uint8_t request, uint8_t object, uint8_t column)
{
    Probe p = { object, column };

    WR8(ZP->product.lo, 0);
    WR8(ZP->temp12, request);
    return p;
}

/* The parts of a probe that ran later are put over the ones that ran first. */
static Probe probe_join(Probe head, Probe tail)
{
    head.column = tail.column;
    head.answer = tail.answer;
    if (tail.scanned) {
        head.scanned = true;
        head.scanned_to = tail.scanned_to;
    }
    if (tail.lifted) {
        head.lifted = true;
        head.lift_before = tail.lift_before;
        head.lift_amount = tail.lift_amount;
    }
    if (tail.wall_asked) {
        head.wall_asked = true;
        head.wall = tail.wall;
        head.pushed = tail.pushed;
        head.push_before = tail.push_before;
        head.push_amount = tail.push_amount;
    }
    return head;
}

/* Deadly ground throws the object up at a fixed speed, with no X speed. */
static void probe_deadly_hit(uint8_t object)
{
    WR8(OBJ->vx_hi[object], 0);
    WR8(OBJ->vx_lo[object], 0);
    WR8(OBJ->vy_hi[object], 0);
    WR8(OBJ->vy_lo[object], DEADLY_THROW);
    add_to_answer(PROBE_SPIKES_ANSWER);
}

/* The C a probe leaves, worked out as probe_exit_shim sets it, from the C it was entered with. */
static bool lookup_carry(LookupEnd end, bool before);
static bool probe_carry_out(Probe p, bool carry);

/*
 * @routine 0047:225 ProbeWalls
 * @run 6A:6225
 * WHAT IT DOES: The wall half of a probe, and its end: when the request asks about walls, looks
 * the terrain up a row lower, reads the two columns on the side the object is moving to and, if
 * one is solid, sets the answer's wall bit and, when asked, pushes the object out of the wall.
 * HOW: The request is in zTemp12, the probe height in zProduct1; the answer in zProduct0 is
 * the result.
 * WAITS: no.
 * Entry: X object, Y column, C in (kept to the lookup), D clear. Exit: A the answer, N and Z
 * from it; Y, C and V as the path left them.
 */
Probe ProbeWalls(uint8_t object, uint8_t column, bool carry_in)
{
    Probe p = { object, column };
    uint8_t request = RD8(ZP->temp12);
    uint8_t height, speed, y, bits, depth, amount, low, high;
    Sum sum;

    if ((request & PROBE_WALL) != 0) {
        p.wall_asked = true;
        height = RD8(ZP->product.mid);
        jsr_push(0x6231);
        p.wall = StageLookup((uint8_t)(height + 1), p.object, p.column, carry_in);
        jsr_pop();
        speed = RD8(OBJ->vx_hi[p.object]);
        y = (speed & 0x80) != 0 ? 0 : COLUMNS_ON_LEFT;
        bits = rd8((uint16_t)(TileHit + y));
        if ((bits & TILE_SOLID_SIDE) == 0) {
            y++;
            bits = rd8((uint16_t)(TileHit + y));
            if ((bits & TILE_SOLID_SIDE) == 0) {
                p.column = y;
                p.answer = RD8(ZP->product.lo);
                return p;
            }
        }
        p.column = y;
        add_to_answer(PROBE_WALL);
        request = RD8(ZP->temp12);
        if ((request & PROBE_WALL_PUSH) != 0) {
            if (y < COLUMNS_ON_LEFT) {
                depth = rd8((uint16_t)(TileDepthX + y));
                low = RD8(OBJ->x_lo[p.object]);
                amount = rd8((uint16_t)(WallPushTable + depth));
                sum = add_bytes(low, amount, false);
                WR8(OBJ->x_lo[p.object], sum.value);
                p.column = depth;
                p.pushed = PUSHED_RIGHT;
                p.push_before = low;
                p.push_amount = amount;
                if (sum.carry) {
                    INC8(OBJ->x_hi[p.object]);
                }
            } else {
                low = RD8(OBJ->x_lo[p.object]);
                depth = rd8((uint16_t)(TileDepthX + y));
                WR8(OBJ->x_lo[p.object], (uint8_t)(low - depth));
                high = RD8(OBJ->x_hi[p.object]);
                WR8(OBJ->x_hi[p.object], (uint8_t)(high - (low < depth ? 1 : 0)));
                p.pushed = PUSHED_LEFT;
                p.push_before = (uint16_t)(low | (high << 8));
                p.push_amount = depth;
            }
        }
    }
    p.answer = RD8(ZP->product.lo);
    return p;
}

void abi_ProbeWalls(void)
{
    probe_exit_shim(ProbeWalls(cpu.x, cpu.y, (cpu.p & P_C) != 0));
}

/*
 * @routine 0047:1FB ProbeFloorHit
 * @run 6A:61FB
 * WHAT IT DOES: Standing on a floor in the column in Y: sets the answer's floor bit and, when the
 * request asks, lifts the object onto it by the column's depth (a signed byte). Then the wall
 * half.
 * HOW: The depth is read twice, as the assembly does: once for its sign byte (kept in zTemp00),
 * once for the subtract; the object's Y goes down by it, the sign byte as the high part.
 * WAITS: no.
 * Entry: X object, Y floor column, C in, D clear. Exit: as ProbeWalls.
 */
Probe ProbeFloorHit(uint8_t object, uint8_t y, bool carry_in)
{
    Probe p = { object, y };
    uint8_t request, depth, sign, low, high;
    bool carry;

    add_to_answer(PROBE_FLOOR);
    request = RD8(ZP->temp12);
    if ((request & PROBE_FLOOR_PUSH) != 0) {
        depth = rd8((uint16_t)(TileDepthY + y));
        sign = (uint8_t)((uint8_t)(((depth >> 7) & 1) ^ 0xFF) + 1);
        WR8(ZP->temp00, sign);
        low = RD8(OBJ->y_lo[object]);
        depth = rd8((uint16_t)(TileDepthY + y));
        WR8(OBJ->y_lo[object], (uint8_t)(low - depth));
        high = RD8(OBJ->y_hi[object]);
        sign = RD8(ZP->temp00);
        WR8(OBJ->y_hi[object], (uint8_t)(high - sign - (low < depth ? 1 : 0)));
        p.lifted = true;
        p.lift_before = (uint16_t)(low | (high << 8));
        p.lift_amount = (uint16_t)(depth | (sign << 8));
    }
    carry = probe_carry_out(p, carry_in);
    return probe_join(p, ProbeWalls(object, p.column, carry));
}

void abi_ProbeFloorHit(void)
{
    probe_exit_shim(ProbeFloorHit(cpu.x, cpu.y, (cpu.p & P_C) != 0));
}

/*
 * @routine 0047:1C9 ProbeFloorRead
 * @run 6A:61C9
 * WHAT IT DOES: After a floor lookup: reads the three columns it filled; deadly ground in any of
 * them throws the object up at a fixed speed and sets the answer's spikes bit; otherwise the first
 * column with a solid top is the floor (ProbeFloorHit). Then the wall half.
 * HOW: The three attributes ORed are tested for the deadly bit; a loop looks for $21 in them.
 * WAITS: no.
 * Entry: X object, C in (what the lookup left), D clear. Exit: as ProbeWalls.
 */
Probe ProbeFloorRead(uint8_t object, uint8_t column, bool carry_in)
{
    Probe p = { object, column };
    uint8_t all, y, bits;

    all = rd8(TileHit);
    all = (uint8_t)(all | rd8(TileHit + 1));
    all = (uint8_t)(all | rd8(TileHit + 2));
    if ((all & TILE_DEADLY) != 0) {
        probe_deadly_hit(object);
        return probe_join(p, ProbeWalls(object, p.column, carry_in));
    }
    for (y = 0; y < FLOOR_COLUMNS; y++) {
        bits = rd8((uint16_t)(TileHit + y));
        if ((bits & TILE_SOLID_TOP) != 0) {
            p.column = y;
            p.scanned = true;
            p.scanned_to = y;
            return probe_join(p, ProbeFloorHit(object, y, y > 0 ? false : carry_in));
        }
    }
    p.column = FLOOR_COLUMNS;
    p.scanned = true;
    p.scanned_to = FLOOR_COLUMNS;
    return probe_join(p, ProbeWalls(object, p.column, true));
}

void abi_ProbeFloorRead(void)
{
    probe_exit_shim(ProbeFloorRead(cpu.x, cpu.y, (cpu.p & P_C) != 0));
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
    if (p.floor_asked)
        lookup_exit_shim(p.floor);
    if (p.scanned && p.scanned_to > 0)
        abi_flag(P_C, p.scanned_to >= FLOOR_COLUMNS);
    if (p.lifted)
        word_subtract_shim(p.lift_before, p.lift_amount);
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
 * HOW: The overlay's lookup fills TileHit and the depths; ProbeFloorRead reads the floor, the
 * wall half is ProbeWalls.
 * WAITS: no.
 * Entry: A request, X object, Y column, zProduct1 probe height, C in (it reaches the overlay's
 * lookup unchanged), D clear. Exit: A answer, N and Z from it; Y, C and V as the path left them.
 * Clobbers zTemp00, zTemp12, zProduct0.
 */
Probe ProbeTerrain(uint8_t request, uint8_t object, uint8_t column, bool carry_in)
{
    Probe p = probe_start(request, object, column);
    uint8_t height;

    if ((request & PROBE_FLOOR) == 0) {
        return probe_join(p, ProbeWalls(object, column, carry_in));
    }
    p.floor_asked = true;
    height = RD8(ZP->product.mid);
    jsr_push(0x61C9);
    p.floor = StageLookup(height, object, column, carry_in);
    jsr_pop();
    p.column = p.floor.column_after;
    return probe_join(p, ProbeFloorRead(object, p.column, lookup_carry(p.floor, carry_in)));
}

void abi_ProbeTerrain(void)
{
    probe_exit_shim(ProbeTerrain(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0));
}

/*
 * @routine 0047:282 ProbeTerrainAlt
 * @run 6A:6282
 * WHAT IT DOES: ProbeTerrain, using the overlay's second lookup for the floor.
 * HOW: As ProbeTerrain; it jumps into ProbeFloorRead after the lookup.
 * WAITS: no.
 * Entry and exit: as ProbeTerrain.
 */
Probe ProbeTerrainAlt(uint8_t request, uint8_t object, uint8_t column, bool carry_in)
{
    Probe p = probe_start(request, object, column);
    uint8_t height;

    if ((request & PROBE_FLOOR) == 0) {
        return probe_join(p, ProbeWalls(object, column, carry_in));
    }
    p.floor_asked = true;
    height = RD8(ZP->product.mid);
    jsr_push(0x6292);
    p.floor = StageLookupAlt(height, object, column, carry_in);
    jsr_pop();
    p.column = p.floor.column_after;
    return probe_join(p, ProbeFloorRead(object, p.column, lookup_carry(p.floor, carry_in)));
}

void abi_ProbeTerrainAlt(void)
{
    probe_exit_shim(ProbeTerrainAlt(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0));
}

/*
 * @routine 0047:295 ProbeTerrainFrontColumn
 * @run 6A:6295
 * WHAT IT DOES: ProbeTerrainAlt looking at the first column only.
 * HOW: The overlay's second lookup, then one column of TileHit: deadly throws;
 * any floor bit is floor (ProbeFloorHit with column 0).
 * WAITS: no.
 * Entry and exit: as ProbeTerrain.
 */
Probe ProbeTerrainFrontColumn(uint8_t request, uint8_t object, uint8_t column, bool carry_in)
{
    Probe p = probe_start(request, object, column);
    uint8_t height, bits;
    bool carry;

    if ((request & PROBE_FLOOR) == 0) {
        return probe_join(p, ProbeWalls(object, column, carry_in));
    }
    p.floor_asked = true;
    height = RD8(ZP->product.mid);
    jsr_push(0x62A5);
    p.floor = StageLookupAlt(height, object, column, carry_in);
    jsr_pop();
    p.column = p.floor.column_after;
    carry = lookup_carry(p.floor, carry_in);
    bits = rd8(TileHit);
    if ((bits & TILE_DEADLY) != 0) {
        probe_deadly_hit(object);
        return probe_join(p, ProbeWalls(object, p.column, carry));
    }
    p.column = 0;
    bits = rd8(TileHit);
    if ((bits & TILE_FLOOR_ANY) != 0) {
        return probe_join(p, ProbeFloorHit(object, 0, carry));
    }
    return probe_join(p, ProbeWalls(object, 0, carry));
}

void abi_ProbeTerrainFrontColumn(void)
{
    probe_exit_shim(ProbeTerrainFrontColumn(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0));
}

/* The carry a lookup leaves: set when the typed lookup ran, kept when another overlay's code answered. */
static bool lookup_carry(LookupEnd end, bool before)
{
    return end.by_original ? before : true;
}

/* The C a probe leaves, worked out as probe_exit_shim sets it, from the C it was entered with. */
static bool probe_carry_out(Probe p, bool carry)
{
    if (p.floor_asked) {
        carry = lookup_carry(p.floor, carry);
    }
    if (p.scanned && p.scanned_to > 0) {
        carry = p.scanned_to >= FLOOR_COLUMNS;
    }
    if (p.lifted) {
        carry = p.lift_before >= p.lift_amount;
    }
    if (p.wall_asked) {
        carry = lookup_carry(p.wall, carry);
        if (p.pushed == PUSHED_RIGHT) {
            carry = (uint8_t)p.push_before + (uint8_t)p.push_amount > 0xFF;
        } else if (p.pushed == PUSHED_LEFT) {
            carry = p.push_before >= p.push_amount;
        }
    }
    return carry;
}

/* The C the gravity step left (kept at the limit it is the compare's: set). */
static bool gravity_carry(GravityResult g)
{
    return g.gained ? g.carried : true;
}

/* ApplyVelocity, the work bytes cleared, ApplyGravity: the start every mover but NoMove has. */
static MoverStart mover_start(uint8_t object, uint16_t ret_velocity, uint16_t ret_gravity)
{
    MoverStart start = { true };

    jsr_push(ret_velocity);
    start.move = ApplyVelocity(object);
    jsr_pop();
    WR8(LEVEL->stream_count.hi, 0);
    WR8(LEVEL->stream_count.lo, 0);
    jsr_push(ret_gravity);
    start.gravity = ApplyGravity(object);
    jsr_pop();
    return start;
}

/* A the hits, and C says the bit asked about is among them. X and Y as the last probe left them. */
void mover_exit_shim(Mover mover, uint8_t carry_bit)
{
    if (mover.start.moved) {
        axis_move_flags_shim(mover.start.move);
        gravity_exit_shim(mover.start.gravity);
    }
    if (mover.floor_probed) {
        probe_exit_shim(mover.floor);
    }
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
 * Entry: A probe height, X object, Y column, C in (kept to the lookups), D clear. Exit: A bit 0
 * landed, bit 1 walled; C 1 if walled; N and Z from A. Y and V as the probes left them. X kept.
 */
Mover MoveAndLand(uint8_t height, uint8_t object, uint8_t column, bool carry_in)
{
    Mover mover = { object };
    uint8_t speed, hits;
    bool carry;

    WR8(ZP->product.mid, height);
    mover.start = mover_start(object, 0x6332, 0x633B);
    carry = gravity_carry(mover.start.gravity);
    speed = RD8(OBJ->vy_hi[object]);
    if ((speed & 0x80) == 0) {
        mover.floor_probed = true;
        jsr_push(0x6345);
        mover.floor = ProbeTerrain(REQUEST_FLOOR_LIFT, object, column, carry);
        jsr_pop();
        column = mover.floor.column;
        carry = probe_carry_out(mover.floor, carry);
        if ((mover.floor.answer & PROBE_FLOOR) != 0) {
            INC8(LEVEL->stream_count.lo);
            WR8(OBJ->vy_hi[object], 0);
            WR8(OBJ->vy_lo[object], 0);
        }
    }
    jsr_push(0x6357);
    mover.wall = ProbeTerrain(REQUEST_WALL_PUSH, object, column, carry);
    jsr_pop();
    hits = RD8(LEVEL->stream_count.lo);
    mover.hits = (mover.wall.answer & PROBE_WALL) != 0 ? (uint8_t)(hits | PROBE_WALL) : hits;
    (void)carry_in;
    return mover;
}

void abi_MoveAndLand(void)
{
    mover_exit_shim(MoveAndLand(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0), PROBE_WALL);
}

/* The end the three stop movers share, from $6382: the wall probe, then stopping if landed. */
static void stop_tail(Mover *mover, uint8_t column, bool carry)
{
    uint8_t object = mover->object, landed, walled;

    jsr_push(0x6387);
    mover->wall = ProbeTerrain(REQUEST_WALL_PUSH, object, column, carry);
    jsr_pop();
    walled = (uint8_t)(mover->wall.answer & PROBE_WALL);
    WR8(LEVEL->stream_count.lo, walled);
    landed = RD8(LEVEL->stream_count.hi);
    if ((landed & PROBE_FLOOR) != 0) {
        WR8(OBJ->vx_hi[object], 0);
        WR8(OBJ->vx_lo[object], 0);
        WR8(OBJ->vy_hi[object], 0);
        WR8(OBJ->vy_lo[object], 0);
        walled = RD8(LEVEL->stream_count.lo);
        mover->hits = (uint8_t)(walled | PROBE_FLOOR);
        return;
    }
    mover->hits = RD8(LEVEL->stream_count.lo);
}

/* From the rising test on: the floor probe when not rising (by the one given), then the end. */
static void stop_from_rising_test(Mover *mover, Probe (*probe)(uint8_t, uint8_t, uint8_t, bool), uint16_t ret,
                                  uint8_t column, bool carry)
{
    uint8_t speed = RD8(OBJ->vy_hi[mover->object]);

    if ((speed & 0x80) == 0) {
        mover->floor_probed = true;
        jsr_push(ret);
        mover->floor = probe(REQUEST_FLOOR_LIFT, mover->object, column, carry);
        jsr_pop();
        column = mover->floor.column;
        carry = probe_carry_out(mover->floor, carry);
        WR8(LEVEL->stream_count.hi, mover->floor.answer);
    }
    stop_tail(mover, column, carry);
}

/*
 * @routine 0047:375 StopOnLandingNoMove
 * @run 6A:6375
 * WHAT IT DOES: The landing half of StopOnLanding, for a caller that has moved
 * the object itself.
 * HOW: Floor probe with the overlay's second lookup when not rising, then the
 * wall probe and the stop.
 * WAITS: no.
 * Entry: X object, Y column, zProduct1 probe height, WorkByteA clear, C in. Exit: A bit 0
 * landed, bit 1 walled; C 1 if landed; N and Z from A. Y and V as the probes left
 * them.
 */
Mover StopOnLandingNoMove(uint8_t object, uint8_t column, bool carry_in)
{
    Mover mover = { object };

    stop_from_rising_test(&mover, ProbeTerrainAlt, 0x637F, column, carry_in);
    return mover;
}

void abi_StopOnLandingNoMove(void)
{
    mover_exit_shim(StopOnLandingNoMove(cpu.x, cpu.y, (cpu.p & P_C) != 0), PROBE_FLOOR);
}

/*
 * @routine 0047:367 StopOnLanding
 * @run 6A:6367
 * WHAT IT DOES: One frame of a thrown object that stops dead where it lands: moves
 * it, applies gravity, and on landing zeroes all its speed.
 * HOW: The start of MoveAndLand, then runs into StopOnLandingNoMove.
 * WAITS: no.
 * Entry: A probe height, X object, Y column, C in, D clear. Exit: as StopOnLandingNoMove.
 */
Mover StopOnLanding(uint8_t height, uint8_t object, uint8_t column, bool carry_in)
{
    Mover mover = { object };

    (void)carry_in;
    WR8(ZP->product.mid, height);
    mover.start = mover_start(object, 0x636C, 0x6375);
    stop_from_rising_test(&mover, ProbeTerrainAlt, 0x637F, column, gravity_carry(mover.start.gravity));
    return mover;
}

void abi_StopOnLanding(void)
{
    mover_exit_shim(StopOnLanding(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0), PROBE_FLOOR);
}

/*
 * @routine 0047:3AB StopOnLandingFrontColumn
 * @run 6A:63AB
 * WHAT IT DOES: StopOnLanding, reading only the first column of floor.
 * HOW: The same start, ProbeTerrainFrontColumn for the floor, the same end.
 * WAITS: no.
 * Entry: A probe height, X object, Y column, C in, D clear. Exit: as StopOnLandingNoMove.
 */
Mover StopOnLandingFrontColumn(uint8_t height, uint8_t object, uint8_t column, bool carry_in)
{
    Mover mover = { object };

    (void)carry_in;
    WR8(ZP->product.mid, height);
    mover.start = mover_start(object, 0x63B0, 0x63B9);
    stop_from_rising_test(&mover, ProbeTerrainFrontColumn, 0x63C3, column, gravity_carry(mover.start.gravity));
    return mover;
}

void abi_StopOnLandingFrontColumn(void)
{
    mover_exit_shim(StopOnLandingFrontColumn(cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0), PROBE_FLOOR);
}
