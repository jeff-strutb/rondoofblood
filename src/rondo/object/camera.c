/*
 * The camera and the player: the player's speed moves the player on the screen
 * until it reaches the middle, $80, and from there moves the scroll, until the
 * level's end stops the scroll and the player moves on again. Position is on-screen
 * (the player is object 0), and the frame's scroll step comes out as ScrollStepX and
 * ScrollStepY. The original writes the two axes twice in the same shape; here Axis
 * holds what differs.
 *
 * The player overlay answers walls and floors at $A0xx. Routines that reach it keep
 * A and the flags in an Alu flushed to cpu by call_with, and write X and Y straight
 * to cpu where the assembly loads them, since that code sees them. A routine here
 * that starts from alu_from_cpu must be called with cpu's flags current.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"

enum {
    SCREEN_MIDDLE = 0x80, PLAYER_MAX_X = 0xF0, PLAYER_MIN_X = 0x10,
    PUSH_ADDS_X = 0x10, PUSH_ADDS_Y = 0x20, PUSH_LIFTS_SCROLL = 0x08,
    PUSH_LEFT = 0x04, PUSH_RIGHT = 0x02,
    PLAYER_STATE_NO_WALL_PUSH = 3,
    PLAYER_PUSH_BYTES = 7
};

/* One axis of the camera: the player's fields, the scroll's and the limits'. */
typedef struct {
    uint16_t vel_lo, vel_hi, pos_frac, pos_lo, pos_hi_forward;
    uint16_t scroll_frac, scroll_lo, scroll_hi;
    uint16_t step_lo, step_hi;
    uint16_t limit_hi, limit_lo, min_hi;
    uint16_t back_add_lo, back_add_hi;
} Axis;

static const Axis axis_x = {
    OBJ_VX_LO, OBJ_VX_HI, OBJ_X_FRAC, OBJ_X_LO, 0,
    ScrollXFrac, ScrollXLo, ScrollXHiScreen,
    ScrollStepXLo, ScrollStepXHi,
    ScrollXMaxHi, ScrollXMaxLo, ScrollXMinHi,
    zTemp02, zTemp03
};

static const Axis axis_y = {
    OBJ_VY_LO, OBJ_VY_HI, OBJ_Y_FRAC, OBJ_Y_LO, OBJ_Y_HI,
    ScrollYFrac, ScrollYLo, ScrollYHiScreen,
    ScrollStepYLo, ScrollStepYHi,
    ScrollYMaxHi, ScrollYMaxLo, ScrollYMinHi,
    ScrollStepYLo, ScrollStepYHi
};

/* How a move to the right ended, and the values it worked with. */
typedef enum {
    MOVED_TO_RIGHT_EDGE, MOVED_PLAYER, MOVED_PLAYER_AT_SCROLL_END, MOVED_SCROLL, MOVED_BOTH
} RightMove;

typedef struct {
    RightMove how;
    uint16_t speed;
    uint16_t from;
    uint16_t next;
    uint16_t scroll;
    uint16_t step;
    uint8_t page;
} RightMoveResult;

RightMoveResult MovePlayerRight(void);

static bool add_overflows(uint16_t a, uint16_t b)
{
    return (~(a ^ b) & (a ^ (uint16_t)(a + b)) & 0x8000) != 0;
}

static bool subtract_overflows(uint16_t a, uint16_t b)
{
    return ((a ^ b) & (a ^ (uint16_t)(a - b)) & 0x8000) != 0;
}

/* What the assembly leaves in A, C and V at the end of that move. */
static Alu right_move_exit_shim(RightMoveResult move)
{
    Alu exit = { 0 };
    uint8_t screen = (uint8_t)(move.next >> 8);

    exit.v_set = true;
    exit.v = add_overflows(move.speed, move.from);
    switch (move.how) {
    case MOVED_TO_RIGHT_EDGE:
        exit.a = PLAYER_MAX_X;
        exit.c = true;
        break;
    case MOVED_PLAYER:
        exit.a = screen;
        break;
    case MOVED_PLAYER_AT_SCROLL_END:
        exit.a = screen;
        exit.c = true;
        break;
    case MOVED_SCROLL:
        exit.a = (uint8_t)(move.step >> 8);
        exit.v = move.page == 0x7F && (uint32_t)move.step + move.scroll > 0xFFFF;
        break;
    case MOVED_BOTH:
        exit.a = (uint8_t)((uint16_t)(move.next - move.step) >> 8);
        exit.c = move.next >= move.step;
        exit.v = subtract_overflows(move.next, move.step);
        break;
    }
    exit.nz = exit.a;
    return exit;
}

/*
 * @routine 0047:557 PlayerDistanceSum
 * @run 6A:6557
 * WHAT IT DOES: Takes the distance to the player and adds and subtracts fifteen bytes
 * of the object fields to it. What the sum means is not known yet.
 * HOW: A carry chain through all of them, set once before the last.
 * Entry: X object. Exit: A the sum, N, Z, C and V from the last subtract. X and Y kept.
 */
Alu PlayerDistanceSum(uint8_t object, Alu entry)
{
    static const struct { uint16_t addr; bool subtract; } steps[] = {
        { 0x3418, false }, { 0x3480, false }, { 0x2CE1, false }, { 0x2C32, true },
        { 0x2CE3, true }, { 0x2BDC, false }, { 0x2765, false }, { 0x2766, true },
        { 0x2C37, true }, { 0x2D00, false }, { 0x2781, false }, { 0x2BFA, true },
        { 0x2D03, true }, { 0x2EE0, false }
    };
    Alu r = alu_then(entry, DistanceXToPlayer(object));
    uint8_t byte;

    for (unsigned i = 0; i < sizeof steps / sizeof steps[0]; i++) {
        byte = rd8(steps[i].addr);
        r = steps[i].subtract ? sbc8(r.a, byte, r.c) : adc8(r.a, byte, r.c);
    }
    byte = rd8(0x2F90);
    return sbc8(r.a, byte, true);
}

void abi_PlayerDistanceSum(void)
{
    abi_alu(PlayerDistanceSum(cpu.x, alu_from_cpu()));
}

/*
 * @routine 0047:589 SoundIfOnFirstScreen
 * @run 6A:6589
 * WHAT IT DOES: Plays a sound if the object is on the first screen across and down.
 * HOW: The high bytes of its position ORed must be zero, then SoundCommand.
 * Entry: A command, X object. Exit, not played: A kept, N and Z from it. Exit, played:
 * as SoundCommand.
 */
bool ObjectOnFirstScreen(uint8_t object)
{
    uint8_t x_high = rd8((uint16_t)(OBJ_X_HI + object));
    uint8_t y_high = rd8((uint16_t)(OBJ_Y_HI + object));

    return (x_high | y_high) == 0;
}

void abi_SoundIfOnFirstScreen(void)
{
    bool on_first = ObjectOnFirstScreen(cpu.x);

    abi_nz(cpu.a);
    if (on_first)
        abi_SoundCommand();
}

/*
 * @routine 0047:598 StopPlayerX
 * @run 6A:6598
 * WHAT IT DOES: Stops the player moving along X.
 * HOW: Zeroes the X speed.
 * Entry: nothing. Exit: all registers and flags kept.
 */
void StopPlayerX(void)
{
    wr8(OBJ_VX_LO, 0);
    wr8(OBJ_VX_HI, 0);
}

void abi_StopPlayerX(void)
{
    StopPlayerX();
}

/*
 * @routine 0047:59F MovePlayerX
 * @run 6A:659F
 * WHAT IT DOES: Moves the player by its X speed.
 * HOW: Adds the 8.8 speed to position low and fraction, with no carry into the high
 * byte: the player stays on the screen.
 * Entry: nothing. Exit: A position low, N, Z, C and V from its add. X and Y kept.
 */
Alu MovePlayerX(void)
{
    uint8_t a = rd8(OBJ_VX_LO);
    uint8_t b = rd8(OBJ_X_FRAC);
    Alu r = adc8(a, b, false);

    wr8(OBJ_X_FRAC, r.a);
    a = rd8(OBJ_VX_HI);
    b = rd8(OBJ_X_LO);
    r = adc8(a, b, r.c);
    wr8(OBJ_X_LO, r.a);
    return r;
}

void abi_MovePlayerX(void)
{
    abi_alu(MovePlayerX());
}

/*
 * @routine 0047:5B3 MovePlayerY
 * @run 6A:65B3
 * WHAT IT DOES: Moves the player by its Y speed, and stops it at the top of the map if
 * this section does not let it leave.
 * HOW: A 16.8 add with the speed's sign in the high byte. A negative result with
 * PlayerMayLeaveTop clear becomes zero.
 * Entry: nothing. Exit: A the position's high byte, or PlayerMayLeaveTop if it was read;
 * N and Z from A, C and V from the add. X and Y kept.
 */
Alu MovePlayerY(void)
{
    uint8_t a = rd8(OBJ_VY_LO);
    uint8_t b = rd8(OBJ_Y_FRAC);
    Alu r = adc8(a, b, false);

    wr8(OBJ_Y_FRAC, r.a);
    a = rd8(OBJ_VY_HI);
    b = rd8(OBJ_Y_LO);
    r = adc8(a, b, r.c);
    wr8(OBJ_Y_LO, r.a);
    a = rd8(OBJ_VY_HI);
    a = (a & 0x80) ? 0xFF : 0x00;
    b = rd8(OBJ_Y_HI);
    r = adc8(a, b, r.c);
    wr8(OBJ_Y_HI, r.a);
    if (r.a & 0x80) {
        uint8_t may_leave = rd8(PlayerMayLeaveTop);

        alu_load(&r, may_leave);
        if (may_leave == 0) {
            wr8(OBJ_Y_LO, 0);
            wr8(OBJ_Y_HI, 0);
            wr8(OBJ_Y_FRAC, 0);
        }
    }
    return r;
}

void abi_MovePlayerY(void)
{
    abi_alu(MovePlayerY());
}

/*
 * @routine 0047:5E7 ScrollStepXFromPlayer
 * @run 6A:65E7
 * WHAT IT DOES: Takes the player's X speed as the frame's scroll step.
 * HOW: Copies the word.
 * Entry: nothing. Exit: A the high byte, N and Z from it. X, Y, C, V kept.
 */
uint8_t ScrollStepXFromPlayer(void)
{
    uint8_t byte = rd8(OBJ_VX_LO);

    wr8(ScrollStepXLo, byte);
    byte = rd8(OBJ_VX_HI);
    wr8(ScrollStepXHi, byte);
    return byte;
}

void abi_ScrollStepXFromPlayer(void)
{
    cpu.a = ScrollStepXFromPlayer();
    abi_nz(cpu.a);
}

/*
 * @routine 0047:5F4 MovePlayerYIfMoving
 * @run 6A:65F4
 * WHAT IT DOES: Moves the player along Y if it has a Y speed, and when that speed is
 * downward asks the player overlay about the floor.
 * HOW: MovePlayerY, then the overlay's floor entry on the way down.
 * Entry: nothing. Exit, not moving: A zero, Z set. Exit, moving up: A the speed's high
 * byte, C and V from MovePlayerY. Exit, moving down: as the overlay leaves them.
 */
Alu MovePlayerYIfMoving(void)
{
    uint8_t low = rd8(OBJ_VY_LO);
    uint8_t high = rd8(OBJ_VY_HI);
    Alu r = alu_none();

    alu_load(&r, (uint8_t)(low | high));
    if (r.a == 0)
        return r;
    r = MovePlayerY();
    high = rd8(OBJ_VY_HI);
    alu_load(&r, high);
    if (high & 0x80)
        return r;
    return call_with(PlayerOvlFloor, r);
}

void abi_MovePlayerYIfMoving(void)
{
    abi_alu(MovePlayerYIfMoving());
}

/*
 * @routine 0047:608 PlayerAndCameraFrame
 * @run 6A:6608
 * WHAT IT DOES: One frame of the player's movement and the camera. Adds what is pushing
 * the player (a moving platform, say) to its speed for the frame, runs the section's
 * camera mode, puts the speed back, runs the overlay's step after moving and clears the
 * push.
 * HOW: The speed is kept on the stack. Bit 4 and bit 5 of zPlayerPush add the push to X
 * and Y speed; bit 5 also takes the Y fraction from PlayerAnchorObject. Bit 3 moves the
 * player down the screen against the push's Y and scrolls by it, in place of the camera
 * mode.
 * Entry: nothing. Exit: A and Y as the overlay leaves them, V too, X 7, N 0, Z 1, C 1.
 */
Alu PlayerAndCameraFrame(void)
{
    uint8_t vx_lo = rd8(OBJ_VX_LO);
    uint8_t vx_hi = rd8(OBJ_VX_HI);
    uint8_t vy_lo = rd8(OBJ_VY_LO);
    uint8_t vy_hi = rd8(OBJ_VY_HI);
    uint8_t a;
    uint8_t b;
    Alu r = alu_from_cpu();

    alu_load(&r, vy_hi);
    if (rd8(zPlayerPush) & PUSH_ADDS_X) {
        a = rd8(OBJ_VX_LO);
        b = rd8(zPushXLo);
        r = adc8(a, b, false);
        wr8(OBJ_VX_LO, r.a);
        a = rd8(OBJ_VX_HI);
        b = rd8(zPushXHi);
        r = adc8(a, b, r.c);
        wr8(OBJ_VX_HI, r.a);
    }
    if (rd8(zPlayerPush) & PUSH_ADDS_Y) {
        a = rd8(OBJ_VY_LO);
        b = rd8(zPushYLo);
        r = adc8(a, b, false);
        wr8(OBJ_VY_LO, r.a);
        a = rd8(OBJ_VY_HI);
        b = rd8(zPushYHi);
        r = adc8(a, b, r.c);
        wr8(OBJ_VY_HI, r.a);
        cpu.x = rd8(PlayerAnchorObject);
        a = rd8((uint16_t)(OBJ_Y_FRAC + cpu.x));
        alu_load(&r, a);
        wr8(ScrollYFrac, a);
        wr8(OBJ_Y_FRAC, a);
    }
    if (rd8(zPlayerPush) & PUSH_LIFTS_SCROLL) {
        wr8(ScrollStepYLo, 0);
        a = rd8(zPushYHi);
        wr8(ScrollStepYHi, a);
        a = (uint8_t)((a ^ 0xFF) + 1);
        b = rd8(OBJ_Y_LO);
        r = adc8(a, b, false);
        wr8(OBJ_Y_LO, r.a);
    } else {
        r = RunCameraMode(r);
    }
    wr8(OBJ_VY_HI, vy_hi);
    wr8(OBJ_VY_LO, vy_lo);
    wr8(OBJ_VX_HI, vx_hi);
    wr8(OBJ_VX_LO, vx_lo);
    alu_load(&r, vx_lo);
    r = call_with(PlayerOvlMoved, r);
    cpu.x = PLAYER_PUSH_BYTES;
    for (unsigned i = 0; i < PLAYER_PUSH_BYTES; i++)
        wr8((uint16_t)(zPlayerPush + i), 0);
    a = r.a;
    alu_compare(&r, PLAYER_PUSH_BYTES, PLAYER_PUSH_BYTES);
    r.a = a;
    return r;
}

void abi_PlayerAndCameraFrame(void)
{
    abi_alu(PlayerAndCameraFrame());
}

/*
 * The camera mode's handler, from a table of 30 words by CameraMode. RTS from the
 * handler comes back to the caller of RunCameraMode.
 */
Alu RunCameraMode(Alu r)
{
    uint8_t mode = rd8(CameraMode);
    uint8_t lo;
    uint8_t hi;
    uint16_t handler;

    r.a = (uint8_t)(mode << 1);
    alu_nz(&r, r.a);
    r.c = (mode & 0x80) != 0;
    r.c_kept = false;
    cpu.x = r.a;
    lo = rd8((uint16_t)(CameraModeHandlers + cpu.x));
    hi = rd8((uint16_t)(CameraModeHandlers + cpu.x + 1));
    handler = (uint16_t)(lo | (hi << 8));
    abi_alu(r);
    switch (handler) {
    case 0x66C5:
        return CameraModeCarried();
    case 0x6722:
        return CameraModeLeftWall();
    case 0x678E:
        return CameraModeFollow();
    case 0x6A0F:
        return CameraModeVertical();
    default:
        call_original(handler);
        return alu_from_cpu();
    }
}

/*
 * @routine 0047:6C5 CameraModeCarried
 * @run 6A:66C5
 * WHAT IT DOES: The camera mode where the scroll follows what carries the player, not
 * the player: moves the player on the screen, and takes the push as the scroll step
 * unless bit 4 of zPlayerPush says it was already added to the speed.
 * HOW: Zeroes both steps, MovePlayerYIfMoving, PlayerXOnScreen, copies the push.
 * Entry: cpu flags current. Exit: A and N, Z the last read; C and V as the moves left
 * them; X and Y as the overlay left them.
 */
Alu CameraModeCarried(void)
{
    Alu r;
    uint8_t byte;

    wr8(ScrollStepXLo, 0);
    wr8(ScrollStepXHi, 0);
    wr8(ScrollStepYLo, 0);
    wr8(ScrollStepYHi, 0);
    r = MovePlayerYIfMoving();
    abi_alu(r);
    r = PlayerXOnScreen();
    if (rd8(zPlayerPush) & PUSH_ADDS_X)
        return r;
    byte = rd8(zPushXLo);
    wr8(ScrollStepXLo, byte);
    byte = rd8(zPushXHi);
    wr8(ScrollStepXHi, byte);
    byte = rd8(zPushYLo);
    wr8(ScrollStepYLo, byte);
    byte = rd8(zPushYHi);
    wr8(ScrollStepYHi, byte);
    alu_load(&r, byte);
    return r;
}

void abi_CameraModeCarried(void)
{
    abi_alu(CameraModeCarried());
}

/*
 * @routine 0047:6EF PlayerNextX
 * @run 6A:66EF
 * WHAT IT DOES: Works out where the player's speed would put it along X, without moving
 * it.
 * HOW: The 8.8 sum, left in zTemp04-05.
 * Entry: nothing. Exit: A zTemp05, N, Z, C, V from its add. X and Y kept.
 */
Alu PlayerNextX(void)
{
    uint8_t a = rd8(OBJ_VX_LO);
    uint8_t b = rd8(OBJ_X_FRAC);
    Alu r = adc8(a, b, false);

    wr8(zTemp04, r.a);
    a = rd8(OBJ_VX_HI);
    b = rd8(OBJ_X_LO);
    r = adc8(a, b, r.c);
    wr8(zTemp05, r.a);
    return r;
}

void abi_PlayerNextX(void)
{
    abi_alu(PlayerNextX());
}

/*
 * @routine 0047:701 CheckPlayerAtLevelEnd
 * @run 6A:6701
 * WHAT IT DOES: When the scroll is at the level's end and the player has walked into
 * the last stretch of the screen, $C0 to $EB, sets the player's two exit fields.
 * HOW: Compares the scroll's high byte with its limit, and the position with the range.
 * Entry: nothing. Exit: A and flags as the last compare or load left them. X, Y, V kept.
 */
Alu CheckPlayerAtLevelEnd(void)
{
    uint8_t a = rd8(ScrollXHiScreen);
    uint8_t b = rd8(ScrollXMaxHi);
    Alu r = { 0 };

    alu_compare(&r, a, b);
    if (a != b)
        return r;
    a = rd8(OBJ_X_LO);
    alu_compare(&r, a, 0xC0);
    if (a < 0xC0)
        return r;
    a = rd8(OBJ_X_LO);
    alu_compare(&r, a, 0xEC);
    if (a >= 0xEC)
        return r;
    wr8(OBJ_3510, 0xFF);
    wr8(OBJ_3568, 0x20);
    alu_load(&r, 0x20);
    return r;
}

void abi_CheckPlayerAtLevelEnd(void)
{
    abi_alu(CheckPlayerAtLevelEnd());
}

/*
 * @routine 0047:722 CameraModeLeftWall
 * @run 6A:6722
 * WHAT IT DOES: The camera mode for a section the scroll does not leave to the left:
 * right as in CameraModeFollow, left moves the player on the screen alone. A wall to the
 * left pushes the player right by a table on how deep it went.
 * HOW: Follows the speed's sign, or bits 2 and 1 of zPlayerPush at no speed, then the
 * overlay's wall entry with Y 1.
 * Entry: cpu flags current. Exit: A and flags as the path taken left them; X and Y as
 * the overlay left them or X the depth.
 */
Alu CameraModeLeftWall(void)
{
    Alu r = CheckPlayerAtLevelEnd();
    uint8_t high;
    uint8_t low;
    uint8_t depth;
    uint8_t byte;
    bool left;

    r = alu_then(r, MovePlayerYIfMoving());
    wr8(ScrollStepXLo, 0);
    wr8(ScrollStepXHi, 0);
    high = rd8(OBJ_VX_HI);
    low = rd8(OBJ_VX_LO);
    alu_load(&r, (uint8_t)(high | low));
    if (r.a != 0) {
        high = rd8(OBJ_VX_HI);
        alu_load(&r, high);
        left = (high & 0x80) != 0;
    } else {
        alu_tst(&r, PUSH_LEFT, rd8(zPlayerPush));
        left = r.nz != 0;
        if (!left) {
            alu_tst(&r, PUSH_RIGHT, rd8(zPlayerPush));
            if (r.nz == 0)
                return r;
        }
    }
    if (!left) {
        abi_alu(r);
        return MovePlayerRightOrWall();
    }
    cpu.y = 1;
    alu_nz(&r, 1);
    r = call_with(PlayerOvlWall, r);
    if (r.nz == 0) {
        r = PlayerNextX();
        alu_compare(&r, r.a, PLAYER_MIN_X);
        if (r.a <= PLAYER_MIN_X) {
            PlayerAtLeftEdge();
            alu_load(&r, PLAYER_MIN_X);
            return r;
        }
        return PlayerXFromNextAlt(r);
    }
    byte = rd8(OBJ_27A8);
    alu_compare(&r, byte, PLAYER_STATE_NO_WALL_PUSH);
    if (byte == PLAYER_STATE_NO_WALL_PUSH) {
        byte = rd8(OBJ_VY_HI);
        alu_load(&r, byte);
        if (!(byte & 0x80))
            StopPlayerX();
        return r;
    }
    depth = rd8(zTemp00);
    cpu.x = depth;
    byte = rd8((uint16_t)(WallLeftPush + depth));
    low = rd8(OBJ_X_LO);
    r = adc8(byte, low, false);
    alu_compare(&r, r.a, PLAYER_MAX_X);
    if (r.a >= PLAYER_MAX_X) {
        PlayerAtRightEdge();
        alu_load(&r, PLAYER_MAX_X);
        return r;
    }
    wr8(OBJ_X_LO, r.a);
    wr8(OBJ_X_FRAC, 0);
    StopPlayerX();
    return r;
}

void abi_CameraModeLeftWall(void)
{
    abi_alu(CameraModeLeftWall());
}

/*
 * @routine 0047:78E CameraModeFollow
 * @run 6A:678E
 * WHAT IT DOES: The usual camera mode: moves the player down or up, then left or right
 * with the scroll following.
 * HOW: Follows the speed's sign, or bits 2 and 1 of zPlayerPush at no speed.
 * Entry: cpu flags current. Exit: A and flags as the path taken left them.
 */
Alu CameraModeFollow(void)
{
    Alu r = MovePlayerYIfMoving();
    uint8_t high;
    uint8_t low;

    wr8(ScrollStepXLo, 0);
    wr8(ScrollStepXHi, 0);
    high = rd8(OBJ_VX_HI);
    low = rd8(OBJ_VX_LO);
    alu_load(&r, (uint8_t)(high | low));
    if (r.a != 0) {
        high = rd8(OBJ_VX_HI);
        alu_load(&r, high);
        abi_alu(r);
        return (high & 0x80) ? MovePlayerLeftOrWall() : MovePlayerRightOrWall();
    }
    alu_tst(&r, PUSH_LEFT, rd8(zPlayerPush));
    if (r.nz != 0) {
        abi_alu(r);
        return MovePlayerLeftOrWall();
    }
    alu_tst(&r, PUSH_RIGHT, rd8(zPlayerPush));
    if (r.nz != 0) {
        abi_alu(r);
        return MovePlayerRightOrWall();
    }
    return r;
}

void abi_CameraModeFollow(void)
{
    abi_alu(CameraModeFollow());
}

/*
 * @routine 0047:7BB PlayerWallRight
 * @run 6A:67BB
 * WHAT IT DOES: Asks the overlay about a wall to the right, and pushes the player back
 * out of one: left by how deep it went, but not into another wall the overlay finds
 * there, and not past the left edge.
 * HOW: Y 0 on the wall entry; on a wall, zTemp00 is the depth. In player state 3 the
 * player only stops, when not rising. The second question is Y 1 with the position moved
 * and zPlayerPush clear; if there is a wall there too, the position goes back.
 * Entry: cpu flags current. Exit, no wall: A and flags as the overlay left them, Z set.
 * Exit, wall: A 1, N and Z from it.
 */
Alu PlayerWallRight(void)
{
    Alu r = alu_from_cpu();
    uint8_t byte;
    uint8_t position;

    cpu.y = 0;
    r = call_with(PlayerOvlWall, r);
    if (r.nz == 0)
        return r;
    byte = rd8(OBJ_27A8);
    alu_compare(&r, byte, PLAYER_STATE_NO_WALL_PUSH);
    if (byte == PLAYER_STATE_NO_WALL_PUSH) {
        byte = rd8(OBJ_VY_HI);
        alu_load(&r, byte);
        if (!(byte & 0x80))
            StopPlayerX();
    } else {
        byte = rd8(zTemp00);
        byte = (uint8_t)((byte ^ 0xFF) + 1);
        position = rd8(OBJ_X_LO);
        r = adc8(byte, position, false);
        alu_compare(&r, r.a, PLAYER_MIN_X);
        if (r.a < PLAYER_MIN_X) {
            PlayerAtLeftEdge();
        } else {
            position = rd8(OBJ_X_LO);
            cpu.x = position;
            wr8(OBJ_X_LO, r.a);
            wr8(OBJ_X_FRAC, 0);
            StopPlayerX();
            wr8(zPlayerPush, 0);
            cpu.y = 1;
            alu_nz(&r, 1);
            r = call_with(PlayerOvlWall, r);
            cpu.x = position;
            alu_load(&r, r.a);
            if (r.a != 0)
                wr8(OBJ_X_LO, position);
        }
    }
    alu_load(&r, 1);
    return r;
}

void abi_PlayerWallRight(void)
{
    abi_alu(PlayerWallRight());
}

/*
 * @routine 0048:003 MovePlayerRightOrWall
 * @run 6A:6803
 * WHAT IT DOES: Moves the player right, unless there is a wall.
 * HOW: PlayerWallRight, then MovePlayerRight.
 * Entry: cpu flags current. Exit: as the one that ran last.
 */
Alu MovePlayerRightOrWall(void)
{
    Alu r = PlayerWallRight();

    if (r.nz != 0)
        return r;
    return right_move_exit_shim(MovePlayerRight());
}

void abi_MovePlayerRightOrWall(void)
{
    abi_alu(MovePlayerRightOrWall());
}

/* The position worked out in zTemp04-05 becomes the player's. */
static Alu from_next(const Axis *axis, Alu r)
{
    uint8_t byte = rd8(zTemp04);

    wr8(axis->pos_frac, byte);
    byte = rd8(zTemp05);
    wr8(axis->pos_lo, byte);
    alu_load(&r, byte);
    return r;
}

/*
 * @routine 0048:00C PlayerXFromNext
 * @run 6A:680C
 * WHAT IT DOES: Accepts the position PlayerNextX worked out.
 * HOW: Copies zTemp04-05 to the position.
 * Entry: zTemp04-05. Exit: A zTemp05, N and Z from it. X, Y, C, V kept.
 */
void abi_PlayerXFromNext(void)
{
    abi_alu(from_next(&axis_x, alu_from_cpu()));
}

/* A scroll position against a limit: the high in A, the low in zTemp03. */
static Alu compare_with_limit(uint8_t high, uint16_t limit_hi, uint16_t limit_lo)
{
    uint8_t byte = rd8(limit_hi);
    Alu r = { 0 };

    alu_compare(&r, high, byte);
    if (high != byte)
        return r;
    high = rd8(zTemp03);
    byte = rd8(limit_lo);
    alu_compare(&r, high, byte);
    return r;
}

/* At the screen's middle or past it, by the position itself. */
static Alu position_past_middle(const Axis *axis)
{
    uint8_t byte = rd8(axis->pos_lo);
    Alu r = { 0 };

    alu_compare(&r, byte, SCREEN_MIDDLE);
    if (byte == SCREEN_MIDDLE) {
        byte = rd8(axis->pos_frac);
        alu_load(&r, byte);
    }
    return r;
}

/* What is left of the move goes to the scroll: the step is the amount past the middle. */
static Alu step_from_amount(const Axis *axis, Alu r)
{
    uint8_t byte = rd8(zTemp00);

    wr8(axis->step_lo, byte);
    byte = rd8(zTemp01);
    wr8(axis->step_hi, byte);
    alu_load(&r, byte);
    return r;
}

/*
 * The scroll takes a forward move. Before the limit it takes all of what is past the
 * middle; at the limit only what reaches it, and the player the rest.
 */
static Alu scroll_forward(const Axis *axis, Alu r)
{
    uint8_t a;
    uint8_t b;
    Alu sum;

    a = rd8(axis->scroll_lo);
    wr8(zTemp03, a);
    a = rd8(axis->scroll_hi);
    r = alu_then(r, compare_with_limit(a, axis->limit_hi, axis->limit_lo));
    if (r.nz == 0) {
        r = from_next(axis, r);
        if (axis->pos_hi_forward) {
            a = rd8(zTemp06);
            wr8(axis->pos_hi_forward, a);
            alu_load(&r, a);
        }
        return r;
    }
    r = alu_then(r, NextPastMiddleAmount());
    r = alu_then(r, position_past_middle(axis));
    if (r.c) {
        a = rd8(axis->vel_lo);
        wr8(zTemp00, a);
        b = rd8(axis->scroll_frac);
        sum = adc8(a, b, false);
        wr8(zTemp02, sum.a);
        a = rd8(axis->vel_hi);
        wr8(zTemp01, a);
        b = rd8(axis->scroll_lo);
        sum = adc8(a, b, sum.c);
        wr8(zTemp03, sum.a);
        b = rd8(axis->scroll_hi);
        sum = adc8(0, b, sum.c);
        sum = alu_then(sum, compare_with_limit(sum.a, axis->limit_hi, axis->limit_lo));
        if (!sum.c)
            return step_from_amount(axis, sum);
    } else {
        a = rd8(zTemp00);
        b = rd8(axis->scroll_frac);
        sum = adc8(a, b, false);
        wr8(zTemp02, sum.a);
        a = rd8(zTemp01);
        b = rd8(axis->scroll_lo);
        sum = adc8(a, b, sum.c);
        wr8(zTemp03, sum.a);
        b = rd8(axis->scroll_hi);
        sum = adc8(0, b, sum.c);
        sum = alu_then(sum, compare_with_limit(sum.a, axis->limit_hi, axis->limit_lo));
        if (!sum.c) {
            wr8(axis->pos_lo, SCREEN_MIDDLE);
            wr8(axis->pos_frac, 0);
            return step_from_amount(axis, sum);
        }
    }
    b = rd8(axis->scroll_frac);
    sum = sbc8(0, b, true);
    wr8(axis->step_lo, sum.a);
    a = rd8(axis->limit_lo);
    b = rd8(axis->scroll_lo);
    sum = sbc8(a, b, sum.c);
    wr8(axis->step_hi, sum.a);
    a = rd8(zTemp04);
    b = rd8(axis->step_lo);
    sum = sbc8(a, b, true);
    wr8(axis->pos_frac, sum.a);
    a = rd8(zTemp05);
    b = rd8(axis->step_hi);
    sum = sbc8(a, b, sum.c);
    wr8(axis->pos_lo, sum.a);
    return sum;
}

/* Words of plain RAM, by where their two bytes are. */
static uint16_t read16(uint16_t lo, uint16_t hi)
{
    uint8_t low = rd8(lo);

    return (uint16_t)(low | (rd8(hi) << 8));
}

static void write16(uint16_t lo, uint16_t hi, uint16_t value)
{
    wr8(lo, (uint8_t)value);
    wr8(hi, (uint8_t)(value >> 8));
}

static void set_player_x(uint16_t x)
{
    write16(OBJ_X_FRAC, OBJ_X_LO, x);
}

static void set_scroll_step_x(uint16_t step)
{
    write16(ScrollStepXLo, ScrollStepXHi, step);
}

/*
 * @routine 0048:017 MovePlayerRight
 * @run 6A:6817
 * WHAT IT DOES: Moves the player right, or the scroll under it. Before the middle, or
 * with the scroll at the level's end, the player moves; past the middle, the scroll
 * moves by what is left, and the player stays at the middle. At the level's end the
 * scroll takes only what reaches the limit, and the player the rest.
 * HOW: 8.8 positions on the screen against a 16.8 scroll in the level.
 * Entry: nothing. Exit: A the last value, N and Z from it, C and V as
 * right_move_exit_shim gives them. X and Y kept. Clobbers zTemp00-05.
 */
RightMoveResult MovePlayerRight(void)
{
    RightMoveResult move = { 0 };
    uint32_t position;
    uint32_t moved;
    uint16_t limit;

    move.speed = read16(OBJ_VX_LO, OBJ_VX_HI);
    move.from = read16(OBJ_X_FRAC, OBJ_X_LO);
    move.next = (uint16_t)(move.speed + move.from);
    set_scroll_step_x(0);
    write16(zTemp04, zTemp05, move.next);
    if ((move.next >> 8) >= PLAYER_MAX_X) {
        set_player_x((uint16_t)(PLAYER_MAX_X << 8));
        move.how = MOVED_TO_RIGHT_EDGE;
        return move;
    }
    if ((move.next >> 8) < SCREEN_MIDDLE) {
        set_player_x(move.next);
        move.how = MOVED_PLAYER;
        return move;
    }
    move.scroll = read16(ScrollXFrac, ScrollXLo);
    wr8(zTemp03, (uint8_t)(move.scroll >> 8));
    limit = read16(ScrollXMaxLo, ScrollXMaxHi);
    move.page = rd8(ScrollXHiScreen);
    position = ((uint32_t)move.page << 16) | move.scroll;
    if ((uint16_t)(position >> 8) == limit) {
        set_player_x(move.next);
        move.how = MOVED_PLAYER_AT_SCROLL_END;
        return move;
    }
    bool at_middle_already = (move.from >> 8) >= SCREEN_MIDDLE;

    move.step = at_middle_already ? move.speed : (uint16_t)(move.next - 0x8000);
    write16(zTemp00, zTemp01, move.step);
    moved = position + move.step;
    write16(zTemp02, zTemp03, (uint16_t)moved);
    if ((uint16_t)(moved >> 8) < limit) {
        if (!at_middle_already)
            set_player_x((uint16_t)(SCREEN_MIDDLE << 8));
        set_scroll_step_x(move.step);
        move.how = MOVED_SCROLL;
        return move;
    }
    move.step = (uint16_t)((limit << 8) - move.scroll);
    set_scroll_step_x(move.step);
    set_player_x((uint16_t)(move.next - move.step));
    move.how = MOVED_BOTH;
    return move;
}

void abi_MovePlayerRight(void)
{
    abi_alu(right_move_exit_shim(MovePlayerRight()));
}

/*
 * @routine 0048:0AF PlayerAtRightEdge
 * @run 6A:68AF
 * WHAT IT DOES: Stops the player at the right edge of the screen.
 * HOW: Position $F0, fraction zero.
 * Entry: nothing. Exit: A $F0, N 1, Z 0. X, Y, C, V kept.
 */
void PlayerAtRightEdge(void)
{
    wr8(OBJ_X_LO, PLAYER_MAX_X);
    wr8(OBJ_X_FRAC, 0);
}

void abi_PlayerAtRightEdge(void)
{
    PlayerAtRightEdge();
    cpu.a = PLAYER_MAX_X;
    abi_nz(PLAYER_MAX_X);
}

/*
 * @routine 0048:0B8 NextPastMiddle
 * @run 6A:68B8
 * WHAT IT DOES: Says whether the next position, in zTemp05, is at the screen's middle or
 * past it.
 * HOW: Compares with $80. At $80 it goes on to look at the player's X fraction, also for
 * a Y position, and the answer is C set either way.
 * Entry: zTemp05. Exit: C the answer; A and N, Z from the last read or compare. X, Y, V
 * kept.
 */
Alu NextPastMiddle(void)
{
    uint8_t byte = rd8(zTemp05);
    Alu r = { 0 };

    alu_compare(&r, byte, SCREEN_MIDDLE);
    if (byte == SCREEN_MIDDLE) {
        byte = rd8(OBJ_X_FRAC);
        alu_load(&r, byte);
    }
    return r;
}

void abi_NextPastMiddle(void)
{
    abi_alu(NextPastMiddle());
}

/*
 * @routine 0048:0C2 NextFractionSet
 * @run 6A:68C2
 * WHAT IT DOES: Sets C if the next position's fraction, zTemp04, is not zero. Nothing in
 * the kernel reaches it: left over from an earlier NextPastMiddle.
 * HOW: A read and a branch.
 * Entry: zTemp04. Exit: A zTemp04, N and Z from it; C set if not zero, else kept.
 */
Alu NextFractionSet(void)
{
    uint8_t byte = rd8(zTemp04);
    Alu r = alu_none();

    alu_load(&r, byte);
    if (byte != 0) {
        r.c = true;
        r.c_kept = false;
    }
    return r;
}

void abi_NextFractionSet(void)
{
    abi_alu(NextFractionSet());
}

/*
 * @routine 0048:0C9 PlayerXPastMiddle
 * @run 6A:68C9
 * WHAT IT DOES: Says whether the player is at the screen's middle or past it, along X.
 * HOW: As NextPastMiddle, on the position.
 * Entry: nothing. Exit: as NextPastMiddle.
 */
Alu PlayerXPastMiddle(void)
{
    return position_past_middle(&axis_x);
}

void abi_PlayerXPastMiddle(void)
{
    abi_alu(PlayerXPastMiddle());
}

/*
 * @routine 0048:0DC NextPastMiddleAmount
 * @run 6A:68DC
 * WHAT IT DOES: Gives how far the next position lies past the middle.
 * HOW: Subtracts $8000 from zTemp04-05, into zTemp00-01.
 * Entry: zTemp04-05. Exit: A zTemp01, N, Z, C, V from its subtract. X and Y kept.
 */
Alu NextPastMiddleAmount(void)
{
    uint8_t byte = rd8(zTemp04);
    Alu r = sbc8(byte, 0x00, true);

    wr8(zTemp00, r.a);
    byte = rd8(zTemp05);
    r = sbc8(byte, SCREEN_MIDDLE, r.c);
    wr8(zTemp01, r.a);
    return r;
}

void abi_NextPastMiddleAmount(void)
{
    abi_alu(NextPastMiddleAmount());
}

/*
 * @routine 0048:0EA CompareScrollWithRightLimit
 * @run 6A:68EA
 * WHAT IT DOES: Compares a scroll position with the level's right limit.
 * HOW: High bytes, then on equal low bytes.
 * Entry: A high, zTemp03 low. Exit: A and N, Z, C from the last compare. X, Y, V kept.
 */
Alu CompareScrollWithRightLimit(uint8_t high)
{
    return compare_with_limit(high, ScrollXMaxHi, ScrollXMaxLo);
}

void abi_CompareScrollWithRightLimit(void)
{
    abi_alu(CompareScrollWithRightLimit(cpu.a));
}

/*
 * @routine 0048:0F5 PlayerWallLeft
 * @run 6A:68F5
 * WHAT IT DOES: Asks the overlay about a wall to the left, and pushes the player back
 * out of one: right by a table on how deep it went, but not into another wall the
 * overlay finds there, and not past the right edge.
 * HOW: As PlayerWallRight, with Y 1 then Y 0.
 * Entry: cpu flags current. Exit: as PlayerWallRight.
 */
Alu PlayerWallLeft(void)
{
    Alu r = alu_from_cpu();
    uint8_t byte;
    uint8_t position;

    cpu.y = 1;
    alu_nz(&r, 1);
    r = call_with(PlayerOvlWall, r);
    if (r.nz == 0)
        return r;
    byte = rd8(OBJ_27A8);
    alu_compare(&r, byte, PLAYER_STATE_NO_WALL_PUSH);
    if (byte == PLAYER_STATE_NO_WALL_PUSH) {
        byte = rd8(OBJ_VY_HI);
        alu_load(&r, byte);
        if (!(byte & 0x80))
            StopPlayerX();
    } else {
        cpu.x = rd8(zTemp00);
        byte = rd8((uint16_t)(WallLeftPush + cpu.x));
        position = rd8(OBJ_X_LO);
        r = adc8(byte, position, false);
        alu_compare(&r, r.a, PLAYER_MAX_X);
        if (r.a >= PLAYER_MAX_X) {
            PlayerAtRightEdge();
        } else {
            position = rd8(OBJ_X_LO);
            cpu.x = position;
            wr8(OBJ_X_LO, r.a);
            wr8(OBJ_X_FRAC, 0);
            StopPlayerX();
            wr8(zPlayerPush, 0);
            cpu.y = 0;
            alu_nz(&r, 0);
            r = call_with(PlayerOvlWall, r);
            cpu.x = position;
            alu_load(&r, r.a);
            if (r.a != 0)
                wr8(OBJ_X_LO, position);
        }
    }
    alu_load(&r, 1);
    return r;
}

void abi_PlayerWallLeft(void)
{
    abi_alu(PlayerWallLeft());
}

/*
 * @routine 0048:146 MovePlayerLeftOrWall
 * @run 6A:6946
 * WHAT IT DOES: Moves the player left, unless there is a wall.
 * HOW: PlayerWallLeft, then MovePlayerLeft.
 * Entry: cpu flags current. Exit: as the one that ran last.
 */
Alu MovePlayerLeftOrWall(void)
{
    Alu r = PlayerWallLeft();

    if (r.nz != 0)
        return r;
    return MovePlayerLeft();
}

void abi_MovePlayerLeftOrWall(void)
{
    abi_alu(MovePlayerLeftOrWall());
}

/*
 * @routine 0048:14F PlayerXFromNextAlt
 * @run 6A:694F
 * WHAT IT DOES: The same as PlayerXFromNext, written again for the leftward moves.
 * HOW: Copies zTemp04-05 to the position.
 * Entry: zTemp04-05. Exit: A zTemp05, N and Z from it. X, Y, C, V kept.
 */
Alu PlayerXFromNextAlt(Alu r)
{
    return from_next(&axis_x, r);
}

void abi_PlayerXFromNextAlt(void)
{
    abi_alu(PlayerXFromNextAlt(alu_from_cpu()));
}

/*
 * The scroll takes a backward move. Past the middle the player moves; before it the
 * scroll takes the move, down to its start, and then the player the rest. A scroll
 * already at its start leaves it all to the player.
 */
static Alu scroll_back(const Axis *axis, Alu r)
{
    uint8_t a = rd8(axis->scroll_hi);
    uint8_t b = rd8(axis->min_hi);
    bool at_middle;
    bool to_step;
    Alu sum;

    alu_compare(&r, a, b);
    if (a != b) {
        r = alu_then(r, position_past_middle(axis));
        sum = r;
        at_middle = r.c;
        to_step = true;
    } else {
        a = rd8(axis->scroll_frac);
        b = rd8(axis->scroll_lo);
        alu_load(&r, (uint8_t)(a | b));
        if (r.a == 0)
            return from_next(axis, r);
        r = alu_then(r, position_past_middle(axis));
        at_middle = r.c;
        if (at_middle) {
            a = rd8(zTemp00);
            b = rd8(axis->scroll_frac);
        } else {
            a = rd8(axis->vel_lo);
            b = rd8(axis->scroll_frac);
        }
        sum = adc8(a, b, false);
        wr8(zTemp02, sum.a);
        a = rd8(at_middle ? zTemp01 : axis->vel_hi);
        b = rd8(axis->scroll_lo);
        sum = adc8(a, b, sum.c);
        wr8(zTemp03, sum.a);
        b = rd8(axis->scroll_hi);
        sum = adc8(0xFF, b, sum.c);
        b = rd8(axis->min_hi);
        alu_compare(&sum, sum.a, b);
        to_step = sum.a == b;
    }
    if (to_step) {
        if (at_middle) {
            wr8(axis->pos_lo, SCREEN_MIDDLE);
            wr8(axis->pos_frac, 0);
            return step_from_amount(axis, sum);
        }
        a = rd8(axis->vel_lo);
        wr8(axis->step_lo, a);
        a = rd8(axis->vel_hi);
        wr8(axis->step_hi, a);
        alu_load(&sum, a);
        return sum;
    }
    b = rd8(axis->scroll_frac);
    sum = sbc8(0, b, true);
    wr8(axis->step_lo, sum.a);
    b = rd8(axis->scroll_lo);
    sum = sbc8(0, b, sum.c);
    wr8(axis->step_hi, sum.a);
    a = rd8(axis->pos_frac);
    b = rd8(axis->back_add_lo);
    sum = adc8(a, b, false);
    wr8(axis->pos_frac, sum.a);
    a = rd8(axis->pos_lo);
    b = rd8(axis->back_add_hi);
    sum = adc8(a, b, sum.c);
    wr8(axis->pos_lo, sum.a);
    return sum;
}

/*
 * @routine 0048:15A MovePlayerLeft
 * @run 6A:695A
 * WHAT IT DOES: Moves the player left, or the scroll under it, as MovePlayerRight does
 * to the right, and stops the player at the left edge of the screen.
 * HOW: The next position, the edge, then the scroll's start in place of its limit.
 * Entry: nothing. Exit: A, flags as the last step left them. X and Y kept. Clobbers
 * zTemp00-05.
 */
Alu MovePlayerLeft(void)
{
    Alu r = PlayerNextX();

    alu_compare(&r, r.a, PLAYER_MIN_X);
    if (r.a <= PLAYER_MIN_X) {
        PlayerAtLeftEdge();
        alu_load(&r, PLAYER_MIN_X);
        return r;
    }
    r = alu_then(r, NextPastMiddle());
    if (r.c)
        return from_next(&axis_x, r);
    r = alu_then(r, NextPastMiddleAmount());
    return scroll_back(&axis_x, r);
}

void abi_MovePlayerLeft(void)
{
    abi_alu(MovePlayerLeft());
}

/*
 * @routine 0048:206 PlayerAtLeftEdge
 * @run 6A:6A06
 * WHAT IT DOES: Stops the player at the left edge of the screen.
 * HOW: Position $10, fraction zero.
 * Entry: nothing. Exit: A $10, N 0, Z 0. X, Y, C, V kept.
 */
void PlayerAtLeftEdge(void)
{
    wr8(OBJ_X_LO, PLAYER_MIN_X);
    wr8(OBJ_X_FRAC, 0);
}

void abi_PlayerAtLeftEdge(void)
{
    PlayerAtLeftEdge();
    cpu.a = PLAYER_MIN_X;
    abi_nz(PLAYER_MIN_X);
}

/*
 * @routine 0048:20F CameraModeVertical
 * @run 6A:6A0F
 * WHAT IT DOES: The camera mode for a section that scrolls up and down: the player moves
 * on the screen along X, and the scroll follows it along Y. Going down it asks the
 * overlay about the floor before and after moving.
 * HOW: PlayerXOnScreen; a Y speed of zero clears the Y scroll's fraction; then the Y
 * speed's sign. The first floor answer not zero means no move.
 * Entry: cpu flags current. Exit: A and flags as the path taken left them.
 */
Alu CameraModeVertical(void)
{
    Alu r = PlayerXOnScreen();
    uint8_t high;
    uint8_t low;

    wr8(ScrollStepYHi, 0);
    wr8(ScrollStepYLo, 0);
    high = rd8(OBJ_VY_HI);
    low = rd8(OBJ_VY_LO);
    alu_load(&r, (uint8_t)(high | low));
    if (r.a == 0)
        wr8(ScrollYFrac, 0);
    high = rd8(OBJ_VY_HI);
    alu_load(&r, high);
    if (high & 0x80)
        return MovePlayerUp();
    r = call_with(PlayerOvlFloor, r);
    if (r.nz != 0)
        return r;
    r = MovePlayerDown();
    wr8(zPlayerPush, 0);
    return call_with(PlayerOvlFloor, r);
}

void abi_CameraModeVertical(void)
{
    abi_alu(CameraModeVertical());
}

/*
 * @routine 0048:239 MovePlayerYIfMovingGoTo
 * @run 6A:6A39
 * WHAT IT DOES: Goes to MovePlayerYIfMoving. Nothing in the kernel reaches it.
 * HOW: One jump.
 * Entry, exit: as MovePlayerYIfMoving.
 */
void abi_MovePlayerYIfMovingGoTo(void)
{
    abi_alu(MovePlayerYIfMoving());
}

/*
 * @routine 0048:23C PlayerStopAtRightEdge
 * @run 6A:6A3C
 * WHAT IT DOES: Stops the player at the right edge, for PlayerXOnScreen.
 * HOW: Writes the position and zeroes the fraction.
 * Entry: nothing. Exit: A $F0, N 1, Z 0. X, Y, C, V kept.
 */
void abi_PlayerStopAtRightEdge(void)
{
    PlayerAtRightEdge();
    cpu.a = PLAYER_MAX_X;
    abi_nz(PLAYER_MAX_X);
}

/*
 * @routine 0048:245 PlayerStopAtLeftEdge
 * @run 6A:6A45
 * WHAT IT DOES: Stops the player at the left edge, for PlayerXOnScreen.
 * HOW: The same write as PlayerStopAtRightEdge, with $10.
 * Entry: nothing. Exit: A $10, N 0, Z 0. X, Y, C, V kept.
 */
void abi_PlayerStopAtLeftEdge(void)
{
    PlayerAtLeftEdge();
    cpu.a = PLAYER_MIN_X;
    abi_nz(PLAYER_MIN_X);
}

/*
 * @routine 0048:249 PlayerXOnScreen
 * @run 6A:6A49
 * WHAT IT DOES: Moves the player along X by its speed, on the screen alone, unless there
 * is a wall, and keeps it between $10 and $EF.
 * HOW: Follows the speed's sign, or bits 2 and 1 of zPlayerPush at no speed, through the
 * wall checks.
 * Entry: cpu flags current. Exit: A and flags as the path taken left them.
 */
Alu PlayerXOnScreen(void)
{
    uint8_t low = rd8(OBJ_VX_LO);
    uint8_t high = rd8(OBJ_VX_HI);
    Alu r = alu_from_cpu();
    bool left;

    alu_load(&r, (uint8_t)(low | high));
    if (r.a != 0) {
        high = rd8(OBJ_VX_HI);
        alu_load(&r, high);
        left = (high & 0x80) != 0;
    } else {
        alu_tst(&r, PUSH_LEFT, rd8(zPlayerPush));
        left = r.nz != 0;
        if (!left) {
            alu_tst(&r, PUSH_RIGHT, rd8(zPlayerPush));
            if (r.nz == 0)
                return r;
        }
    }
    abi_alu(r);
    r = left ? PlayerWallLeft() : PlayerWallRight();
    if (r.nz != 0)
        return r;
    r = MovePlayerX();
    alu_compare(&r, r.a, PLAYER_MAX_X);
    if (r.a >= PLAYER_MAX_X) {
        PlayerAtRightEdge();
        alu_load(&r, PLAYER_MAX_X);
        return r;
    }
    alu_compare(&r, r.a, PLAYER_MIN_X);
    if (r.a < PLAYER_MIN_X) {
        PlayerAtLeftEdge();
        alu_load(&r, PLAYER_MIN_X);
    }
    return r;
}

void abi_PlayerXOnScreen(void)
{
    abi_alu(PlayerXOnScreen());
}

/*
 * @routine 0048:288 PlayerYFromNextWord
 * @run 6A:6A88
 * WHAT IT DOES: Accepts the Y position worked out in zTemp04-06, with its high byte.
 * HOW: Copies it to the position.
 * Entry: zTemp04-06. Exit: A the high byte, N and Z from it. X, Y, C, V kept.
 */
void abi_PlayerYFromNextWord(void)
{
    Alu r = from_next(&axis_y, alu_from_cpu());
    uint8_t byte = rd8(zTemp06);

    wr8(OBJ_Y_HI, byte);
    alu_load(&r, byte);
    abi_alu(r);
}

/*
 * @routine 0048:298 MovePlayerDown
 * @run 6A:6A98
 * WHAT IT DOES: Moves the player down, or the scroll under it, as MovePlayerRight does
 * to the right.
 * HOW: The 16.8 next position in zTemp04-06, then as MovePlayerRight on the Y fields.
 * Entry: nothing. Exit: A, flags as the last step left them. X and Y kept. Clobbers
 * zTemp00-06.
 */
Alu MovePlayerDown(void)
{
    uint8_t a;
    uint8_t b;
    Alu r;

    wr8(ScrollStepYLo, 0);
    wr8(ScrollStepYHi, 0);
    a = rd8(OBJ_VY_LO);
    b = rd8(OBJ_Y_FRAC);
    r = adc8(a, b, false);
    wr8(zTemp04, r.a);
    a = rd8(OBJ_VY_HI);
    b = rd8(OBJ_Y_LO);
    r = adc8(a, b, r.c);
    wr8(zTemp05, r.a);
    b = rd8(OBJ_Y_HI);
    r = adc8(0, b, r.c);
    wr8(zTemp06, r.a);
    r = alu_then(r, NextPastMiddle());
    if (!r.c) {
        r = from_next(&axis_y, r);
        a = rd8(zTemp06);
        wr8(OBJ_Y_HI, a);
        alu_load(&r, a);
        return r;
    }
    return scroll_forward(&axis_y, r);
}

void abi_MovePlayerDown(void)
{
    abi_alu(MovePlayerDown());
}

/*
 * @routine 0048:33D PlayerYPastMiddle
 * @run 6A:6B3D
 * WHAT IT DOES: Says whether the player is at the screen's middle or past it, along Y.
 * HOW: As PlayerXPastMiddle.
 * Entry: nothing. Exit: as NextPastMiddle.
 */
Alu PlayerYPastMiddle(void)
{
    return position_past_middle(&axis_y);
}

void abi_PlayerYPastMiddle(void)
{
    abi_alu(PlayerYPastMiddle());
}

/*
 * @routine 0048:350 CompareScrollWithBottomLimit
 * @run 6A:6B50
 * WHAT IT DOES: Compares a vertical scroll position with the section's bottom limit.
 * HOW: High bytes, then on equal low bytes.
 * Entry: A high, zTemp03 low. Exit: A and N, Z, C from the last compare. X, Y, V kept.
 */
Alu CompareScrollWithBottomLimit(uint8_t high)
{
    return compare_with_limit(high, ScrollYMaxHi, ScrollYMaxLo);
}

void abi_CompareScrollWithBottomLimit(void)
{
    abi_alu(CompareScrollWithBottomLimit(cpu.a));
}

/*
 * @routine 0048:35B PlayerAtTop
 * @run 6A:6B5B
 * WHAT IT DOES: Puts the player at the top of the map.
 * HOW: Zeroes its Y position.
 * Entry: nothing. Exit: all registers and flags kept.
 */
void PlayerAtTop(void)
{
    wr8(OBJ_Y_FRAC, 0);
    wr8(OBJ_Y_LO, 0);
    wr8(OBJ_Y_HI, 0);
}

void abi_PlayerAtTop(void)
{
    PlayerAtTop();
}

/*
 * @routine 0048:365 PlayerYFromNext
 * @run 6A:6B65
 * WHAT IT DOES: Accepts the Y position worked out in zTemp04-05.
 * HOW: Copies it to the position.
 * Entry: zTemp04-05. Exit: A zTemp05, N and Z from it. X, Y, C, V kept.
 */
void abi_PlayerYFromNext(void)
{
    abi_alu(from_next(&axis_y, alu_from_cpu()));
}

/*
 * @routine 0048:370 MovePlayerUp
 * @run 6A:6B70
 * WHAT IT DOES: Moves the player up, or the scroll under it, as MovePlayerLeft does to
 * the left, and stops the player at the top of the map if it would pass it.
 * HOW: The next position, zero or a borrow meaning the top, then as MovePlayerLeft on the
 * Y fields.
 * Entry: nothing. Exit: A, flags as the last step left them. X and Y kept. Clobbers
 * zTemp00-05.
 */
Alu MovePlayerUp(void)
{
    uint8_t a = rd8(OBJ_VY_LO);
    uint8_t b = rd8(OBJ_Y_FRAC);
    Alu r = adc8(a, b, false);

    wr8(zTemp04, r.a);
    a = rd8(OBJ_VY_HI);
    b = rd8(OBJ_Y_LO);
    r = adc8(a, b, r.c);
    wr8(zTemp05, r.a);
    if (r.a == 0 || !r.c) {
        PlayerAtTop();
        return r;
    }
    r = alu_then(r, NextPastMiddle());
    if (r.c)
        return from_next(&axis_y, r);
    r = alu_then(r, NextPastMiddleAmount());
    return scroll_back(&axis_y, r);
}

void abi_MovePlayerUp(void)
{
    abi_alu(MovePlayerUp());
}
