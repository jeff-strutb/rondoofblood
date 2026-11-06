/*
 * The camera and the player: the player's speed moves the player on the screen
 * until it reaches the middle, $80, and from there moves the scroll, until the
 * level's end stops the scroll and the player moves on again. Position is on-screen
 * (the player is object 0), and the frame's scroll step comes out as ScrollStepX and
 * ScrollStepY. The original writes the two axes twice in the same shape; here Axis
 * holds what differs.
 *
 * The player overlay answers walls and floors at $A0xx, through PlayerWall and PlayerFloor.
 * A routine's body returns how it ended, as plain values. Its wrapper replays what those
 * steps leave in the registers and flags, in the assembly's order, through the *_exit_shim
 * functions, so that only the wrappers know them.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"

enum {
    SCREEN_MIDDLE = 0x80, PLAYER_MAX_X = 0xF0, PLAYER_MIN_X = 0x10,
    LEVEL_END_FROM_X = 0xC0, LEVEL_END_TO_X = 0xEC, LEVEL_END_TIMER = 0x20,
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
uint8_t PlayerDistanceSum(uint8_t object, bool carry_in)
{
    static const struct { uint16_t field; bool subtract; } terms[] = {
        { OBJ_3408 + 0x10, false }, { OBJ_3460 + 0x20, false }, { OBJ_Y_LO + 0x11, false },
        { OBJ_X_FRAC + 0x12, true }, { OBJ_Y_LO + 0x13, true }, { OBJ_X_LO + 0x14, false },
        { OBJ_2750 + 0x15, false }, { OBJ_2750 + 0x16, true }, { OBJ_X_FRAC + 0x17, true },
        { OBJ_Y_LO + 0x30, false }, { OBJ_2750 + 0x31, false }, { OBJ_X_LO + 0x32, true },
        { OBJ_Y_LO + 0x33, true }, { OBJ_FIELDS_2EE0, false }
    };
    PlayerDistance distance = DistanceXToPlayer(object);
    bool carry = distance.far ? carry_in : distance.object_further;
    unsigned sum = distance.distance;

    for (unsigned i = 0; i < sizeof terms / sizeof terms[0]; i++) {
        unsigned term = rd8(terms[i].field);

        sum = terms[i].subtract ? sum - term - (carry ? 0u : 1u) : sum + term + (carry ? 1u : 0u);
        carry = terms[i].subtract ? sum <= 0xFF : sum > 0xFF;
        sum &= 0xFF;
    }
    return (uint8_t)(sum - rd8(OBJ_2F90));
}

/* A, N, Z, C and V are the last subtract's, worked back from its result. */
void abi_PlayerDistanceSum(void)
{
    uint8_t result = PlayerDistanceSum(cpu.x, (cpu.p & P_C) != 0);
    uint8_t term = rd8(OBJ_2F90);
    uint8_t before = (uint8_t)(result + term);

    cpu.a = result;
    abi_nz(result);
    abi_flag(P_C, before >= term);
    abi_flag(P_V, ((before ^ term) & (before ^ result) & 0x80) != 0);
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

/* The two words of an 8.8 add or subtract, kept for the wrapper's flags. */
typedef struct {
    uint16_t a;
    uint16_t b;
} WordPair;

/* A the high byte of the sum, with N, Z, C and V from its add. */
static Alu word_add_exit_shim(WordPair words)
{
    Alu exit = { 0 };
    uint16_t sum = (uint16_t)(words.a + words.b);

    exit.a = (uint8_t)(sum >> 8);
    exit.nz = exit.a;
    exit.c = sum < words.a;
    exit.v = add_overflows(words.a, words.b);
    exit.v_set = true;
    return exit;
}

/* A the high byte of the difference, with N, Z, C and V from its subtract. */
static Alu word_subtract_exit_shim(WordPair words)
{
    Alu exit = { 0 };

    exit.a = (uint8_t)((uint16_t)(words.a - words.b) >> 8);
    exit.nz = exit.a;
    exit.c = words.a >= words.b;
    exit.v = subtract_overflows(words.a, words.b);
    exit.v_set = true;
    return exit;
}

/* How a move left or up ended, and the values it worked with. */
typedef enum {
    BACK_AT_EDGE, BACK_AT_TOP, BACK_PLAYER, BACK_AT_START, BACK_STEP_FAR, BACK_STEP_NEAR, BACK_BOTH
} BackEnd;

typedef struct {
    BackEnd how;
    uint16_t speed, from, next, step, position, added;
    bool at_middle, carried;
    uint8_t page;
} BackMove;

/* How a move down ended, and the values it worked with. */
typedef enum { DOWN_PLAYER, DOWN_AT_LIMIT, DOWN_STEP, DOWN_BOTH } DownEnd;

typedef struct {
    DownEnd how;
    uint16_t next, step;
    uint8_t high, next_page, page;
    bool next_carried, carried;
} DownMove;

BackMove MovePlayerLeft(void);
BackMove MovePlayerUp(void);
DownMove MovePlayerDown(void);

/* How MovePlayerYIfMoving ended. */
typedef enum { Y_NOT_MOVING, Y_UP, Y_DOWN } YEnd;

typedef struct {
    YEnd how;
    AxisMove move;
    FloorProbe floor;
} YMove;

/* What a wall pushed the player back to. */
typedef enum { WALL_NOT_THERE, WALL_STOPPED_ONLY, WALL_PAST_EDGE, WALL_PUSHED_BACK } WallPushEnd;

typedef struct {
    WallPushEnd how;
    bool leftward;
    WallProbe first;
    WallProbe second;
    uint8_t amount;
    uint8_t position;
} WallPush;

/* Which way the player's speed or push asked it to go. */
typedef enum { GOES_NOWHERE, GOES_LEFT, GOES_RIGHT } Direction;

typedef struct {
    bool moving;
    bool tested_right;
    uint8_t push;
    Direction way;
} Choice;

/* Moving unless there is a wall. */
typedef struct {
    WallPush wall;
    bool moved;
    RightMoveResult move;
} RightOrWall;

typedef struct {
    WallPush wall;
    bool moved;
    BackMove move;
} LeftOrWall;

/* PlayerXOnScreen: where the move left the player. */
typedef enum { ON_SCREEN_STAYED, ON_SCREEN_WALLED, ON_SCREEN_MOVED, ON_SCREEN_RIGHT_EDGE, ON_SCREEN_LEFT_EDGE } OnScreenEnd;

typedef struct {
    Choice choice;
    WallPush wall;
    OnScreenEnd how;
    WordPair move;
} OnScreen;

typedef struct {
    YMove y;
    OnScreen x;
    bool copied_push;
} CarriedMode;

typedef struct {
    YMove y;
    Choice choice;
    RightOrWall right;
    LeftOrWall left;
} FollowMode;

/* CameraModeLeftWall's leftward path. */
typedef enum {
    LEFT_WALL_NOT_LEFT, LEFT_WALL_AT_EDGE, LEFT_WALL_MOVED, LEFT_WALL_STOPPED, LEFT_WALL_PAST_RIGHT,
    LEFT_WALL_PUSHED
} LeftWallEnd;

typedef struct {
    YMove y;
    Choice choice;
    RightOrWall right;
    WallProbe wall;
    LeftWallEnd how;
    WordPair next;
    uint8_t depth;
    uint8_t push;
    uint8_t position;
} LeftWallMode;

/* CameraModeVertical's ends. */
typedef enum { VERTICAL_UP, VERTICAL_FLOOR_FIRST, VERTICAL_DOWN } VerticalEnd;

typedef struct {
    OnScreen x;
    VerticalEnd how;
    BackMove up;
    FloorProbe floor_before;
    DownMove down;
    FloorProbe floor_after;
} VerticalMode;

CarriedMode CameraModeCarried(void);
LeftWallMode CameraModeLeftWall(void);
FollowMode CameraModeFollow(void);
VerticalMode CameraModeVertical(void);
OnScreen PlayerXOnScreen(void);
WallPush PlayerWallRight(void);
WallPush PlayerWallLeft(void);
RightOrWall MovePlayerRightOrWall(void);
LeftOrWall MovePlayerLeftOrWall(void);
void PlayerXFromNextAlt(void);
WordPair PlayerNextX(void);
static void RunCameraMode(void);
static void x_on_screen_exit_shim(OnScreen x);
static void right_or_wall_exit_shim(RightOrWall result);
static void left_or_wall_exit_shim(LeftOrWall result);

static bool add8_overflows(uint8_t a, uint8_t b)
{
    return (~(a ^ b) & (a ^ (uint8_t)(a + b)) & 0x80) != 0;
}

/* By the speed's sign, or at no speed by bits 2 and 1 of what pushes the player. */
static Choice choose_direction(void)
{
    Choice choice = { 0 };

    choice.moving = (rd8(OBJ_VX_HI) | rd8(OBJ_VX_LO)) != 0;
    choice.push = rd8(zPlayerPush);
    if (choice.moving) {
        choice.way = (rd8(OBJ_VX_HI) & 0x80) ? GOES_LEFT : GOES_RIGHT;
    } else if (choice.push & PUSH_LEFT) {
        choice.way = GOES_LEFT;
    } else {
        choice.tested_right = true;
        choice.way = (choice.push & PUSH_RIGHT) ? GOES_RIGHT : GOES_NOWHERE;
    }
    return choice;
}

/* A speed's load, or the TST that decided: Z from its AND, N and V from the byte's top bits. */
static void choice_exit_shim(Choice choice)
{
    uint8_t mask = choice.tested_right ? PUSH_RIGHT : PUSH_LEFT;

    if (choice.moving) {
        cpu.a = rd8(OBJ_VX_HI);
        abi_nz(cpu.a);
        return;
    }
    cpu.a = 0;
    abi_nz(choice.push & mask);
    abi_flag(P_N, (choice.push & 0x80) != 0);
    abi_flag(P_V, (choice.push & 0x40) != 0);
}

/* For the wrappers: A, N and Z from a value, with C and V given. */
static void move_exit_shim(uint8_t a, bool carry, bool overflow)
{
    cpu.a = a;
    abi_nz(a);
    abi_flag(P_C, carry);
    abi_flag(P_V, overflow);
}

/* For the wrappers: A and the flags at each end of a move left or up. */
static void back_exit_shim(BackMove move)
{
    uint8_t next_high = (uint8_t)(move.next >> 8);
    uint8_t step_high = (uint8_t)(move.step >> 8);
    uint8_t page_sum = (uint8_t)(0xFF + move.page + (move.carried ? 1 : 0));
    uint16_t sum = (uint16_t)(move.position + move.added);

    switch (move.how) {
    case BACK_AT_EDGE:
        move_exit_shim(PLAYER_MIN_X, next_high >= PLAYER_MIN_X, add_overflows(move.speed, move.from));
        break;
    case BACK_AT_TOP:
        move_exit_shim(next_high, move.next < move.speed, add_overflows(move.speed, move.from));
        break;
    case BACK_PLAYER:
        move_exit_shim(next_high, true, add_overflows(move.speed, move.from));
        break;
    case BACK_AT_START:
        move_exit_shim(next_high, true, subtract_overflows(move.next, 0x8000));
        break;
    case BACK_STEP_FAR:
        move_exit_shim(step_high, move.at_middle, subtract_overflows(move.next, 0x8000));
        break;
    case BACK_STEP_NEAR:
        move_exit_shim(step_high, true, (move.page & 0x80) != 0 && (page_sum & 0x80) == 0);
        break;
    case BACK_BOTH:
        move_exit_shim((uint8_t)(sum >> 8), sum < move.position, add_overflows(move.position, move.added));
        break;
    }
}

/* For the wrappers: A and the flags at each end of a move down. */
static void down_exit_shim(DownMove move)
{
    bool next_overflow = move.high == 0x7F && move.next_carried;

    switch (move.how) {
    case DOWN_PLAYER:
        move_exit_shim(move.next_page, false, next_overflow);
        break;
    case DOWN_AT_LIMIT:
        move_exit_shim(move.next_page, true, next_overflow);
        break;
    case DOWN_STEP:
        move_exit_shim((uint8_t)(move.step >> 8), false, move.page == 0x7F && move.carried);
        break;
    case DOWN_BOTH:
        move_exit_shim((uint8_t)((uint16_t)(move.next - move.step) >> 8), move.next >= move.step,
                       subtract_overflows(move.next, move.step));
        break;
    }
}

/*
 * @routine 0047:59F MovePlayerX
 * @run 6A:659F
 * WHAT IT DOES: Moves the player by its X speed.
 * HOW: Adds the 8.8 speed to position low and fraction, with no carry into the high
 * byte: the player stays on the screen.
 * Entry: nothing. Exit: A position low, N, Z, C and V from its add. X and Y kept.
 */
WordPair MovePlayerX(void)
{
    WordPair words = { read16(OBJ_VX_LO, OBJ_VX_HI), read16(OBJ_X_FRAC, OBJ_X_LO) };

    write16(OBJ_X_FRAC, OBJ_X_LO, (uint16_t)(words.a + words.b));
    return words;
}

void abi_MovePlayerX(void)
{
    abi_alu(word_add_exit_shim(MovePlayerX()));
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
AxisMove MovePlayerY(void)
{
    AxisMove move;
    uint32_t after;

    move.velocity = (int16_t)read16(OBJ_VY_LO, OBJ_VY_HI);
    move.before = read16(OBJ_Y_FRAC, OBJ_Y_LO) | ((uint32_t)rd8(OBJ_Y_HI) << 16);
    after = move.before + (uint32_t)(int32_t)move.velocity;
    write16(OBJ_Y_FRAC, OBJ_Y_LO, (uint16_t)after);
    wr8(OBJ_Y_HI, (uint8_t)(after >> 16));
    if ((after & 0x800000) && rd8(PlayerMayLeaveTop) == 0)
        PlayerAtTop();
    return move;
}

/* C and V from the high byte's add; A that byte, or PlayerMayLeaveTop if it was read. */
static Alu move_y_exit_shim(AxisMove move)
{
    uint32_t added = (uint32_t)(int32_t)move.velocity & 0xFFFFFF;
    uint32_t sum = move.before + added;
    uint8_t sign = (uint8_t)(added >> 16);
    uint8_t high = (uint8_t)(move.before >> 16);
    uint8_t result = (uint8_t)(sum >> 16);
    Alu exit = { 0 };

    exit.a = (result & 0x80) ? rd8(PlayerMayLeaveTop) : result;
    exit.nz = exit.a;
    exit.c = sum > 0xFFFFFF;
    exit.v = (~(sign ^ high) & (sign ^ result) & 0x80) != 0;
    exit.v_set = true;
    return exit;
}

void abi_MovePlayerY(void)
{
    abi_alu(move_y_exit_shim(MovePlayerY()));
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
YMove MovePlayerYIfMoving(void)
{
    YMove move = { Y_NOT_MOVING };

    if ((rd8(OBJ_VY_LO) | rd8(OBJ_VY_HI)) == 0)
        return move;
    move.move = MovePlayerY();
    if (rd8(OBJ_VY_HI) & 0x80) {
        move.how = Y_UP;
        return move;
    }
    move.how = Y_DOWN;
    move.floor = PlayerFloor();
    return move;
}

/* A and the flags as the steps left them, in the order the assembly made them. */
static void y_move_exit_shim(YMove move)
{
    if (move.how == Y_NOT_MOVING) {
        cpu.a = 0;
        abi_nz(0);
        return;
    }
    abi_alu(move_y_exit_shim(move.move));
    cpu.a = rd8(OBJ_VY_HI);
    abi_nz(cpu.a);
    if (move.how == Y_DOWN)
        floor_exit_shim(move.floor);
}

void abi_MovePlayerYIfMoving(void)
{
    y_move_exit_shim(MovePlayerYIfMoving());
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
ColumnsEnd PlayerAndCameraFrame(void)
{
    uint16_t speed_x = read16(OBJ_VX_LO, OBJ_VX_HI);
    uint16_t speed_y = read16(OBJ_VY_LO, OBJ_VY_HI);
    ColumnsEnd moved;

    if (rd8(zPlayerPush) & PUSH_ADDS_X)
        write16(OBJ_VX_LO, OBJ_VX_HI, (uint16_t)(speed_x + read16(zPushXLo, zPushXHi)));
    if (rd8(zPlayerPush) & PUSH_ADDS_Y) {
        uint8_t fraction = rd8((uint16_t)(OBJ_Y_FRAC + rd8(PlayerAnchorObject)));

        write16(OBJ_VY_LO, OBJ_VY_HI, (uint16_t)(speed_y + read16(zPushYLo, zPushYHi)));
        wr8(ScrollYFrac, fraction);
        wr8(OBJ_Y_FRAC, fraction);
    }
    if (rd8(zPlayerPush) & PUSH_LIFTS_SCROLL) {
        uint8_t lift = rd8(zPushYHi);

        write16(ScrollStepYLo, ScrollStepYHi, (uint16_t)(lift << 8));
        wr8(OBJ_Y_LO, (uint8_t)(rd8(OBJ_Y_LO) - lift));
    } else {
        RunCameraMode();
    }
    write16(OBJ_VY_LO, OBJ_VY_HI, speed_y);
    write16(OBJ_VX_LO, OBJ_VX_HI, speed_x);
    moved = PlayerMoved();
    for (unsigned i = 0; i < PLAYER_PUSH_BYTES; i++)
        wr8((uint16_t)(zPlayerPush + i), 0);
    return moved;
}

/* A, Y and V from the overlay's step; the loop that clears the push leaves X, Z and C. */
void abi_PlayerAndCameraFrame(void)
{
    columns_exit_shim(PlayerAndCameraFrame());
    cpu.x = PLAYER_PUSH_BYTES;
    abi_nz(0);
    abi_flag(P_C, true);
}

/* One call to a camera mode with no C yet, with what the table's jump leaves. */
static void camera_mode_original_shim(uint16_t handler, uint8_t mode)
{
    cpu.a = (uint8_t)(mode << 1);
    cpu.x = cpu.a;
    abi_nz(cpu.a);
    abi_flag(P_C, (mode & 0x80) != 0);
    call_original(handler);
}

/*
 * The camera mode's handler, from a table of 30 words by CameraMode. RTS from the
 * handler comes back to the caller of RunCameraMode.
 */
static void RunCameraMode(void)
{
    uint8_t mode = rd8(CameraMode);
    uint16_t entry = (uint16_t)(CameraModeHandlers + (uint8_t)(mode << 1));
    uint16_t handler = read16(entry, (uint16_t)(entry + 1));

    switch (handler) {
    case 0x66C5:
        CameraModeCarried();
        break;
    case 0x6722:
        CameraModeLeftWall();
        break;
    case 0x678E:
        CameraModeFollow();
        break;
    case 0x6A0F:
        CameraModeVertical();
        break;
    default:
        camera_mode_original_shim(handler, mode);
        break;
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
CarriedMode CameraModeCarried(void)
{
    CarriedMode mode;

    write16(ScrollStepXLo, ScrollStepXHi, 0);
    write16(ScrollStepYLo, ScrollStepYHi, 0);
    mode.y = MovePlayerYIfMoving();
    mode.x = PlayerXOnScreen();
    mode.copied_push = !(rd8(zPlayerPush) & PUSH_ADDS_X);
    if (mode.copied_push) {
        write16(ScrollStepXLo, ScrollStepXHi, read16(zPushXLo, zPushXHi));
        write16(ScrollStepYLo, ScrollStepYHi, read16(zPushYLo, zPushYHi));
    }
    return mode;
}

void abi_CameraModeCarried(void)
{
    CarriedMode mode = CameraModeCarried();

    y_move_exit_shim(mode.y);
    x_on_screen_exit_shim(mode.x);
    if (mode.copied_push) {
        cpu.a = rd8(zPushYHi);
        abi_nz(cpu.a);
    }
}

/*
 * @routine 0047:6EF PlayerNextX
 * @run 6A:66EF
 * WHAT IT DOES: Works out where the player's speed would put it along X, without moving
 * it.
 * HOW: The 8.8 sum, left in zTemp04-05.
 * Entry: nothing. Exit: A zTemp05, N, Z, C, V from its add. X and Y kept.
 */
WordPair PlayerNextX(void)
{
    WordPair words = { read16(OBJ_VX_LO, OBJ_VX_HI), read16(OBJ_X_FRAC, OBJ_X_LO) };

    write16(zTemp04, zTemp05, (uint16_t)(words.a + words.b));
    return words;
}

void abi_PlayerNextX(void)
{
    abi_alu(word_add_exit_shim(PlayerNextX()));
}

/*
 * @routine 0047:701 CheckPlayerAtLevelEnd
 * @run 6A:6701
 * WHAT IT DOES: When the scroll is at the level's end and the player has walked into
 * the last stretch of the screen, $C0 to $EB, sets the player's two exit fields.
 * HOW: Compares the scroll's high byte with its limit, and the position with the range.
 * Entry: nothing. Exit: A and flags as the last compare or load left them. X, Y, V kept.
 */
void CheckPlayerAtLevelEnd(void)
{
    uint8_t x = rd8(OBJ_X_LO);

    if (rd8(ScrollXHiScreen) == rd8(ScrollXMaxHi) && x >= LEVEL_END_FROM_X && x < LEVEL_END_TO_X) {
        wr8(OBJ_3510, 0xFF);
        wr8(OBJ_3568, LEVEL_END_TIMER);
    }
}

/* A compare leaves A, N, Z and C; V stands. */
static void compare_exit_shim(uint8_t a, uint8_t b)
{
    cpu.a = a;
    abi_nz((uint8_t)(a - b));
    abi_flag(P_C, a >= b);
}

/* A and the flags from the compare that decided, or the last load. V stands. */
static void level_end_exit_shim(void)
{
    uint8_t scroll = rd8(ScrollXHiScreen);
    uint8_t limit = rd8(ScrollXMaxHi);
    uint8_t x = rd8(OBJ_X_LO);

    if (scroll != limit) {
        compare_exit_shim(scroll, limit);
    } else if (x < LEVEL_END_FROM_X) {
        compare_exit_shim(x, LEVEL_END_FROM_X);
    } else {
        compare_exit_shim(x, LEVEL_END_TO_X);
        if (x < LEVEL_END_TO_X) {
            cpu.a = LEVEL_END_TIMER;
            abi_nz(LEVEL_END_TIMER);
        }
    }
}

void abi_CheckPlayerAtLevelEnd(void)
{
    CheckPlayerAtLevelEnd();
    level_end_exit_shim();
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
LeftWallMode CameraModeLeftWall(void)
{
    LeftWallMode mode = { 0 };

    CheckPlayerAtLevelEnd();
    mode.y = MovePlayerYIfMoving();
    write16(ScrollStepXLo, ScrollStepXHi, 0);
    mode.choice = choose_direction();
    if (mode.choice.way == GOES_NOWHERE)
        return mode;
    if (mode.choice.way == GOES_RIGHT) {
        mode.right = MovePlayerRightOrWall();
        return mode;
    }
    mode.wall = PlayerWall(1);
    if (!wall_found(mode.wall)) {
        mode.next = PlayerNextX();
        if (rd8(zTemp05) <= PLAYER_MIN_X) {
            PlayerAtLeftEdge();
            mode.how = LEFT_WALL_AT_EDGE;
        } else {
            PlayerXFromNextAlt();
            mode.how = LEFT_WALL_MOVED;
        }
        return mode;
    }
    if (rd8(OBJ_27A8) == PLAYER_STATE_NO_WALL_PUSH) {
        if (!(rd8(OBJ_VY_HI) & 0x80))
            StopPlayerX();
        mode.how = LEFT_WALL_STOPPED;
        return mode;
    }
    mode.depth = rd8(zTemp00);
    mode.push = rd8((uint16_t)(WallLeftPush + mode.depth));
    mode.position = rd8(OBJ_X_LO);
    if ((uint8_t)(mode.push + mode.position) >= PLAYER_MAX_X) {
        PlayerAtRightEdge();
        mode.how = LEFT_WALL_PAST_RIGHT;
        return mode;
    }
    wr8(OBJ_X_LO, (uint8_t)(mode.push + mode.position));
    wr8(OBJ_X_FRAC, 0);
    StopPlayerX();
    mode.how = LEFT_WALL_PUSHED;
    return mode;
}

void abi_CameraModeLeftWall(void)
{
    LeftWallMode mode = CameraModeLeftWall();
    uint8_t sum = (uint8_t)(mode.push + mode.position);

    level_end_exit_shim();
    y_move_exit_shim(mode.y);
    choice_exit_shim(mode.choice);
    if (mode.choice.way == GOES_NOWHERE)
        return;
    if (mode.choice.way == GOES_RIGHT) {
        right_or_wall_exit_shim(mode.right);
        return;
    }
    cpu.y = 1;
    wall_exit_shim(mode.wall);
    switch (mode.how) {
    case LEFT_WALL_NOT_LEFT:
        break;
    case LEFT_WALL_AT_EDGE:
    case LEFT_WALL_MOVED:
        abi_alu(word_add_exit_shim(mode.next));
        compare_exit_shim(rd8(zTemp05), PLAYER_MIN_X);
        cpu.a = mode.how == LEFT_WALL_AT_EDGE ? PLAYER_MIN_X : rd8(zTemp05);
        abi_nz(cpu.a);
        break;
    case LEFT_WALL_STOPPED:
        abi_flag(P_C, true);
        cpu.a = rd8(OBJ_VY_HI);
        abi_nz(cpu.a);
        break;
    case LEFT_WALL_PAST_RIGHT:
    case LEFT_WALL_PUSHED:
        cpu.x = mode.depth;
        abi_flag(P_V, add8_overflows(mode.push, mode.position));
        compare_exit_shim(sum, PLAYER_MAX_X);
        if (mode.how == LEFT_WALL_PAST_RIGHT) {
            cpu.a = PLAYER_MAX_X;
            abi_nz(PLAYER_MAX_X);
        }
        break;
    }
}

/*
 * @routine 0047:78E CameraModeFollow
 * @run 6A:678E
 * WHAT IT DOES: The usual camera mode: moves the player down or up, then left or right
 * with the scroll following.
 * HOW: Follows the speed's sign, or bits 2 and 1 of zPlayerPush at no speed.
 * Entry: cpu flags current. Exit: A and flags as the path taken left them.
 */
/* FollowMode holds how each step ended, as plain values: no flags pass through this body. */
FollowMode CameraModeFollow(void)
{
    FollowMode mode = { 0 };

    mode.y = MovePlayerYIfMoving();
    write16(ScrollStepXLo, ScrollStepXHi, 0);
    mode.choice = choose_direction();
    if (mode.choice.way == GOES_LEFT)
        mode.left = MovePlayerLeftOrWall();
    else if (mode.choice.way == GOES_RIGHT)
        mode.right = MovePlayerRightOrWall();
    return mode;
}

void abi_CameraModeFollow(void)
{
    FollowMode mode = CameraModeFollow();

    y_move_exit_shim(mode.y);
    choice_exit_shim(mode.choice);
    if (mode.choice.way == GOES_LEFT)
        left_or_wall_exit_shim(mode.left);
    else if (mode.choice.way == GOES_RIGHT)
        right_or_wall_exit_shim(mode.right);
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
/* After a wall: stop only, or push back unless that passes the edge or meets another wall. */
static void push_from_wall(WallPush *push, uint8_t amount, uint8_t other_side)
{
    uint8_t moved;

    if (rd8(OBJ_27A8) == PLAYER_STATE_NO_WALL_PUSH) {
        if (!(rd8(OBJ_VY_HI) & 0x80))
            StopPlayerX();
        push->how = WALL_STOPPED_ONLY;
        return;
    }
    push->amount = amount;
    push->position = rd8(OBJ_X_LO);
    moved = (uint8_t)(amount + push->position);
    if (push->leftward ? moved >= PLAYER_MAX_X : moved < PLAYER_MIN_X) {
        if (push->leftward)
            PlayerAtRightEdge();
        else
            PlayerAtLeftEdge();
        push->how = WALL_PAST_EDGE;
        return;
    }
    wr8(OBJ_X_LO, moved);
    wr8(OBJ_X_FRAC, 0);
    StopPlayerX();
    wr8(zPlayerPush, 0);
    push->second = PlayerWall(other_side);
    if (wall_answer(push->second) != 0)
        wr8(OBJ_X_LO, push->position);
    push->how = WALL_PUSHED_BACK;
}

/* Registers and flags as the steps of a wall push left them, in the assembly's order. */
static void wall_push_exit_shim(WallPush push)
{
    cpu.y = push.leftward ? 1 : 0;
    wall_exit_shim(push.first);
    if (push.how == WALL_NOT_THERE)
        return;
    switch (push.how) {
    case WALL_STOPPED_ONLY:
        abi_flag(P_C, true);
        break;
    case WALL_PAST_EDGE:
        abi_flag(P_C, push.leftward);
        abi_flag(P_V, add8_overflows(push.amount, push.position));
        if (push.leftward)
            cpu.x = rd8(zTemp00);
        break;
    default:
        cpu.y = push.leftward ? 0 : 1;
        wall_exit_shim(push.second);
        cpu.x = push.position;
        break;
    }
    cpu.a = 1;
    abi_nz(1);
}

WallPush PlayerWallRight(void)
{
    WallPush push = { WALL_NOT_THERE };

    push.first = PlayerWall(0);
    if (wall_found(push.first))
        push_from_wall(&push, (uint8_t)(0u - rd8(zTemp00)), 1);
    return push;
}

void abi_PlayerWallRight(void)
{
    wall_push_exit_shim(PlayerWallRight());
}

/*
 * @routine 0048:003 MovePlayerRightOrWall
 * @run 6A:6803
 * WHAT IT DOES: Moves the player right, unless there is a wall.
 * HOW: PlayerWallRight, then MovePlayerRight.
 * Entry: cpu flags current. Exit: as the one that ran last.
 */
RightOrWall MovePlayerRightOrWall(void)
{
    RightOrWall result = { 0 };

    result.wall = PlayerWallRight();
    if (result.wall.how == WALL_NOT_THERE) {
        result.moved = true;
        result.move = MovePlayerRight();
    }
    return result;
}

static void right_or_wall_exit_shim(RightOrWall result)
{
    wall_push_exit_shim(result.wall);
    if (result.moved)
        abi_alu(right_move_exit_shim(result.move));
}

void abi_MovePlayerRightOrWall(void)
{
    right_or_wall_exit_shim(MovePlayerRightOrWall());
}

/* The position worked out in zTemp04-05 becomes the player's. */
static void position_from_next(const Axis *axis)
{
    wr8(axis->pos_frac, rd8(zTemp04));
    wr8(axis->pos_lo, rd8(zTemp05));
}

/*
 * @routine 0048:00C PlayerXFromNext
 * @run 6A:680C
 * WHAT IT DOES: Accepts the position PlayerNextX worked out.
 * HOW: Copies zTemp04-05 to the position.
 * Entry: zTemp04-05. Exit: A zTemp05, N and Z from it. X, Y, C, V kept.
 */
void PlayerXFromNext(void)
{
    position_from_next(&axis_x);
}

void abi_PlayerXFromNext(void)
{
    PlayerXFromNext();
    cpu.a = rd8(zTemp05);
    abi_nz(cpu.a);
}

/* A scroll position against a limit: the high byte given, the low in zTemp03. */
static bool scroll_reaches_limit(uint8_t high, uint16_t limit_hi, uint16_t limit_lo)
{
    return (uint16_t)((high << 8) | rd8(zTemp03)) >= read16(limit_lo, limit_hi);
}

/* A and N, Z, C from the compare that decided: the high bytes, or on equal the low. */
static void limit_exit_shim(uint8_t high, uint16_t limit_hi, uint16_t limit_lo)
{
    uint8_t a = high;
    uint8_t b = rd8(limit_hi);

    if (a == b) {
        a = rd8(zTemp03);
        b = rd8(limit_lo);
    }
    cpu.a = a;
    abi_nz((uint8_t)(a - b));
    abi_flag(P_C, a >= b);
}

/* A and N, Z from the compare with the middle, or at the middle from the fraction read. */
static void middle_exit_shim(uint8_t high, uint16_t frac)
{
    if (high == SCREEN_MIDDLE) {
        cpu.a = rd8(frac);
        abi_nz(cpu.a);
    } else {
        cpu.a = high;
        abi_nz((uint8_t)(high - SCREEN_MIDDLE));
    }
    abi_flag(P_C, high >= SCREEN_MIDDLE);
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
bool NextPastMiddle(void)
{
    return rd8(zTemp05) >= SCREEN_MIDDLE;
}

void abi_NextPastMiddle(void)
{
    NextPastMiddle();
    middle_exit_shim(rd8(zTemp05), OBJ_X_FRAC);
}

/*
 * @routine 0048:0C2 NextFractionSet
 * @run 6A:68C2
 * WHAT IT DOES: Sets C if the next position's fraction, zTemp04, is not zero. Nothing in
 * the kernel reaches it: left over from an earlier NextPastMiddle.
 * HOW: A read and a branch.
 * Entry: zTemp04. Exit: A zTemp04, N and Z from it; C set if not zero, else kept.
 */
bool NextFractionSet(void)
{
    return rd8(zTemp04) != 0;
}

void abi_NextFractionSet(void)
{
    if (NextFractionSet())
        abi_flag(P_C, true);
    cpu.a = rd8(zTemp04);
    abi_nz(cpu.a);
}

/*
 * @routine 0048:0C9 PlayerXPastMiddle
 * @run 6A:68C9
 * WHAT IT DOES: Says whether the player is at the screen's middle or past it, along X.
 * HOW: As NextPastMiddle, on the position.
 * Entry: nothing. Exit: as NextPastMiddle.
 */
bool PlayerXPastMiddle(void)
{
    return rd8(OBJ_X_LO) >= SCREEN_MIDDLE;
}

void abi_PlayerXPastMiddle(void)
{
    PlayerXPastMiddle();
    middle_exit_shim(rd8(OBJ_X_LO), OBJ_X_FRAC);
}

/*
 * @routine 0048:0DC NextPastMiddleAmount
 * @run 6A:68DC
 * WHAT IT DOES: Gives how far the next position lies past the middle.
 * HOW: Subtracts $8000 from zTemp04-05, into zTemp00-01.
 * Entry: zTemp04-05. Exit: A zTemp01, N, Z, C, V from its subtract. X and Y kept.
 */
WordPair NextPastMiddleAmount(void)
{
    WordPair words = { read16(zTemp04, zTemp05), (uint16_t)(SCREEN_MIDDLE << 8) };

    write16(zTemp00, zTemp01, (uint16_t)(words.a - words.b));
    return words;
}

void abi_NextPastMiddleAmount(void)
{
    abi_alu(word_subtract_exit_shim(NextPastMiddleAmount()));
}

/*
 * @routine 0048:0EA CompareScrollWithRightLimit
 * @run 6A:68EA
 * WHAT IT DOES: Compares a scroll position with the level's right limit.
 * HOW: High bytes, then on equal low bytes.
 * Entry: A high, zTemp03 low. Exit: A and N, Z, C from the last compare. X, Y, V kept.
 */
bool CompareScrollWithRightLimit(uint8_t high)
{
    return scroll_reaches_limit(high, ScrollXMaxHi, ScrollXMaxLo);
}

void abi_CompareScrollWithRightLimit(void)
{
    CompareScrollWithRightLimit(cpu.a);
    limit_exit_shim(cpu.a, ScrollXMaxHi, ScrollXMaxLo);
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
WallPush PlayerWallLeft(void)
{
    WallPush push = { WALL_NOT_THERE, true };

    push.first = PlayerWall(1);
    if (wall_found(push.first))
        push_from_wall(&push, rd8((uint16_t)(WallLeftPush + rd8(zTemp00))), 0);
    return push;
}

void abi_PlayerWallLeft(void)
{
    wall_push_exit_shim(PlayerWallLeft());
}

/*
 * @routine 0048:146 MovePlayerLeftOrWall
 * @run 6A:6946
 * WHAT IT DOES: Moves the player left, unless there is a wall.
 * HOW: PlayerWallLeft, then MovePlayerLeft.
 * Entry: cpu flags current. Exit: as the one that ran last.
 */
LeftOrWall MovePlayerLeftOrWall(void)
{
    LeftOrWall result = { 0 };

    result.wall = PlayerWallLeft();
    if (result.wall.how == WALL_NOT_THERE) {
        result.moved = true;
        result.move = MovePlayerLeft();
    }
    return result;
}

static void left_or_wall_exit_shim(LeftOrWall result)
{
    wall_push_exit_shim(result.wall);
    if (result.moved)
        back_exit_shim(result.move);
}

void abi_MovePlayerLeftOrWall(void)
{
    left_or_wall_exit_shim(MovePlayerLeftOrWall());
}

/*
 * @routine 0048:14F PlayerXFromNextAlt
 * @run 6A:694F
 * WHAT IT DOES: The same as PlayerXFromNext, written again for the leftward moves.
 * HOW: Copies zTemp04-05 to the position.
 * Entry: zTemp04-05. Exit: A zTemp05, N and Z from it. X, Y, C, V kept.
 */
void PlayerXFromNextAlt(void)
{
    position_from_next(&axis_x);
}

void abi_PlayerXFromNextAlt(void)
{
    PlayerXFromNextAlt();
    cpu.a = rd8(zTemp05);
    abi_nz(cpu.a);
}

/* The scroll takes the step: at the middle the player sits there, and the step is the amount. */
static void step_scroll_back(const Axis *axis, BackMove *move)
{
    if (move->at_middle) {
        write16(axis->pos_frac, axis->pos_lo, (uint16_t)(SCREEN_MIDDLE << 8));
        move->step = read16(zTemp00, zTemp01);
    } else {
        move->step = read16(axis->vel_lo, axis->vel_hi);
    }
    write16(axis->step_lo, axis->step_hi, move->step);
}

/*
 * The scroll takes a backward move. Past the middle the player moves; before it the
 * scroll takes the move, down to its start, and then the player the rest. A scroll
 * already at its start leaves it all to the player.
 */
static BackMove scroll_back(const Axis *axis, BackMove move)
{
    uint8_t lowest = rd8(axis->min_hi);
    uint16_t scroll = read16(axis->scroll_frac, axis->scroll_lo);
    uint16_t taken;
    uint16_t sum;

    write16(zTemp00, zTemp01, (uint16_t)(move.next - 0x8000));
    move.page = rd8(axis->scroll_hi);
    move.at_middle = rd8(axis->pos_lo) >= SCREEN_MIDDLE;
    if (move.page != lowest) {
        step_scroll_back(axis, &move);
        move.how = BACK_STEP_FAR;
        return move;
    }
    if (scroll == 0) {
        position_from_next(axis);
        move.how = BACK_AT_START;
        return move;
    }
    taken = move.at_middle ? read16(zTemp00, zTemp01) : read16(axis->vel_lo, axis->vel_hi);
    sum = (uint16_t)(scroll + taken);
    move.carried = sum < scroll;
    write16(zTemp02, zTemp03, sum);
    if ((uint8_t)(0xFF + move.page + (move.carried ? 1 : 0)) == lowest) {
        step_scroll_back(axis, &move);
        move.how = BACK_STEP_NEAR;
        return move;
    }
    write16(axis->step_lo, axis->step_hi, (uint16_t)(0u - scroll));
    move.position = read16(axis->pos_frac, axis->pos_lo);
    move.added = read16(axis->back_add_lo, axis->back_add_hi);
    write16(axis->pos_frac, axis->pos_lo, (uint16_t)(move.position + move.added));
    move.how = BACK_BOTH;
    return move;
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
BackMove MovePlayerLeft(void)
{
    BackMove move = { 0 };
    uint8_t high;

    move.speed = read16(OBJ_VX_LO, OBJ_VX_HI);
    move.from = read16(OBJ_X_FRAC, OBJ_X_LO);
    move.next = (uint16_t)(move.speed + move.from);
    high = (uint8_t)(move.next >> 8);
    write16(zTemp04, zTemp05, move.next);
    if (high <= PLAYER_MIN_X) {
        PlayerAtLeftEdge();
        move.how = BACK_AT_EDGE;
        return move;
    }
    if (high >= SCREEN_MIDDLE) {
        position_from_next(&axis_x);
        move.how = BACK_PLAYER;
        return move;
    }
    return scroll_back(&axis_x, move);
}

void abi_MovePlayerLeft(void)
{
    back_exit_shim(MovePlayerLeft());
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
VerticalMode CameraModeVertical(void)
{
    VerticalMode mode = { 0 };

    mode.x = PlayerXOnScreen();
    write16(ScrollStepYLo, ScrollStepYHi, 0);
    if ((rd8(OBJ_VY_HI) | rd8(OBJ_VY_LO)) == 0)
        wr8(ScrollYFrac, 0);
    if (rd8(OBJ_VY_HI) & 0x80) {
        mode.how = VERTICAL_UP;
        mode.up = MovePlayerUp();
        return mode;
    }
    mode.floor_before = PlayerFloor();
    if (floor_answer(mode.floor_before) != 0) {
        mode.how = VERTICAL_FLOOR_FIRST;
        return mode;
    }
    mode.how = VERTICAL_DOWN;
    mode.down = MovePlayerDown();
    wr8(zPlayerPush, 0);
    mode.floor_after = PlayerFloor();
    return mode;
}

void abi_CameraModeVertical(void)
{
    VerticalMode mode = CameraModeVertical();

    x_on_screen_exit_shim(mode.x);
    cpu.a = rd8(OBJ_VY_HI);
    abi_nz(cpu.a);
    if (mode.how == VERTICAL_UP) {
        back_exit_shim(mode.up);
        return;
    }
    floor_exit_shim(mode.floor_before);
    if (mode.how == VERTICAL_DOWN) {
        down_exit_shim(mode.down);
        floor_exit_shim(mode.floor_after);
    }
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
    abi_MovePlayerYIfMoving();
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
OnScreen PlayerXOnScreen(void)
{
    OnScreen screen = { 0 };
    uint8_t x;

    screen.choice = choose_direction();
    if (screen.choice.way == GOES_NOWHERE) {
        screen.how = ON_SCREEN_STAYED;
        return screen;
    }
    screen.wall = screen.choice.way == GOES_LEFT ? PlayerWallLeft() : PlayerWallRight();
    if (screen.wall.how != WALL_NOT_THERE) {
        screen.how = ON_SCREEN_WALLED;
        return screen;
    }
    screen.move = MovePlayerX();
    x = rd8(OBJ_X_LO);
    if (x >= PLAYER_MAX_X) {
        PlayerAtRightEdge();
        screen.how = ON_SCREEN_RIGHT_EDGE;
    } else if (x < PLAYER_MIN_X) {
        PlayerAtLeftEdge();
        screen.how = ON_SCREEN_LEFT_EDGE;
    } else {
        screen.how = ON_SCREEN_MOVED;
    }
    return screen;
}

static void x_on_screen_exit_shim(OnScreen screen)
{
    uint8_t x = (uint8_t)((uint16_t)(screen.move.a + screen.move.b) >> 8);

    choice_exit_shim(screen.choice);
    if (screen.how == ON_SCREEN_STAYED)
        return;
    wall_push_exit_shim(screen.wall);
    if (screen.how == ON_SCREEN_WALLED)
        return;
    abi_alu(word_add_exit_shim(screen.move));
    compare_exit_shim(x, PLAYER_MAX_X);
    if (screen.how == ON_SCREEN_RIGHT_EDGE) {
        cpu.a = PLAYER_MAX_X;
        abi_nz(PLAYER_MAX_X);
        return;
    }
    compare_exit_shim(x, PLAYER_MIN_X);
    if (screen.how == ON_SCREEN_LEFT_EDGE) {
        cpu.a = PLAYER_MIN_X;
        abi_nz(PLAYER_MIN_X);
    }
}

void abi_PlayerXOnScreen(void)
{
    x_on_screen_exit_shim(PlayerXOnScreen());
}

/*
 * @routine 0048:288 PlayerYFromNextWord
 * @run 6A:6A88
 * WHAT IT DOES: Accepts the Y position worked out in zTemp04-06, with its high byte.
 * HOW: Copies it to the position.
 * Entry: zTemp04-06. Exit: A the high byte, N and Z from it. X, Y, C, V kept.
 */
void PlayerYFromNextWord(void)
{
    position_from_next(&axis_y);
    wr8(OBJ_Y_HI, rd8(zTemp06));
}

void abi_PlayerYFromNextWord(void)
{
    PlayerYFromNextWord();
    cpu.a = rd8(zTemp06);
    abi_nz(cpu.a);
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
/* zTemp00-06 hold the sums here because the routine leaves them: they are RAM it must end with. */
DownMove MovePlayerDown(void)
{
    DownMove move = { 0 };
    uint16_t speed = read16(OBJ_VY_LO, OBJ_VY_HI);
    uint16_t from = read16(OBJ_Y_FRAC, OBJ_Y_LO);
    uint16_t limit = read16(ScrollYMaxLo, ScrollYMaxHi);
    uint16_t scroll = read16(ScrollYFrac, ScrollYLo);
    bool at_middle_already = rd8(OBJ_Y_LO) >= SCREEN_MIDDLE;
    uint16_t taken;
    uint16_t sum;

    write16(ScrollStepYLo, ScrollStepYHi, 0);
    move.next = (uint16_t)(speed + from);
    move.next_carried = move.next < speed;
    move.high = rd8(OBJ_Y_HI);
    move.next_page = (uint8_t)(move.high + (move.next_carried ? 1 : 0));
    write16(zTemp04, zTemp05, move.next);
    wr8(zTemp06, move.next_page);
    if ((move.next >> 8) < SCREEN_MIDDLE) {
        PlayerYFromNextWord();
        move.how = DOWN_PLAYER;
        return move;
    }
    wr8(zTemp03, (uint8_t)(scroll >> 8));
    move.page = rd8(ScrollYHiScreen);
    if ((uint16_t)((move.page << 8) | (scroll >> 8)) == limit) {
        PlayerYFromNextWord();
        move.how = DOWN_AT_LIMIT;
        return move;
    }
    taken = at_middle_already ? speed : (uint16_t)(move.next - (SCREEN_MIDDLE << 8));
    write16(zTemp00, zTemp01, taken);
    sum = (uint16_t)(scroll + taken);
    move.carried = sum < scroll;
    write16(zTemp02, zTemp03, sum);
    if ((uint16_t)(((uint8_t)(move.page + (move.carried ? 1 : 0)) << 8) | (sum >> 8)) < limit) {
        if (!at_middle_already)
            write16(OBJ_Y_FRAC, OBJ_Y_LO, (uint16_t)(SCREEN_MIDDLE << 8));
        move.step = taken;
        write16(ScrollStepYLo, ScrollStepYHi, taken);
        move.how = DOWN_STEP;
        return move;
    }
    move.step = (uint16_t)(((limit & 0xFF) << 8) - scroll);
    write16(ScrollStepYLo, ScrollStepYHi, move.step);
    write16(OBJ_Y_FRAC, OBJ_Y_LO, (uint16_t)(move.next - move.step));
    move.how = DOWN_BOTH;
    return move;
}

void abi_MovePlayerDown(void)
{
    down_exit_shim(MovePlayerDown());
}

/*
 * @routine 0048:33D PlayerYPastMiddle
 * @run 6A:6B3D
 * WHAT IT DOES: Says whether the player is at the screen's middle or past it, along Y.
 * HOW: As PlayerXPastMiddle.
 * Entry: nothing. Exit: as NextPastMiddle.
 */
bool PlayerYPastMiddle(void)
{
    return rd8(OBJ_Y_LO) >= SCREEN_MIDDLE;
}

void abi_PlayerYPastMiddle(void)
{
    PlayerYPastMiddle();
    middle_exit_shim(rd8(OBJ_Y_LO), OBJ_Y_FRAC);
}

/*
 * @routine 0048:350 CompareScrollWithBottomLimit
 * @run 6A:6B50
 * WHAT IT DOES: Compares a vertical scroll position with the section's bottom limit.
 * HOW: High bytes, then on equal low bytes.
 * Entry: A high, zTemp03 low. Exit: A and N, Z, C from the last compare. X, Y, V kept.
 */
bool CompareScrollWithBottomLimit(uint8_t high)
{
    return scroll_reaches_limit(high, ScrollYMaxHi, ScrollYMaxLo);
}

void abi_CompareScrollWithBottomLimit(void)
{
    CompareScrollWithBottomLimit(cpu.a);
    limit_exit_shim(cpu.a, ScrollYMaxHi, ScrollYMaxLo);
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
void PlayerYFromNext(void)
{
    position_from_next(&axis_y);
}

void abi_PlayerYFromNext(void)
{
    PlayerYFromNext();
    cpu.a = rd8(zTemp05);
    abi_nz(cpu.a);
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
BackMove MovePlayerUp(void)
{
    BackMove move = { 0 };
    uint8_t high;

    move.speed = read16(OBJ_VY_LO, OBJ_VY_HI);
    move.from = read16(OBJ_Y_FRAC, OBJ_Y_LO);
    move.next = (uint16_t)(move.speed + move.from);
    high = (uint8_t)(move.next >> 8);
    write16(zTemp04, zTemp05, move.next);
    if (high == 0 || move.next >= move.speed) {
        PlayerAtTop();
        move.how = BACK_AT_TOP;
        return move;
    }
    if (high >= SCREEN_MIDDLE) {
        position_from_next(&axis_y);
        move.how = BACK_PLAYER;
        return move;
    }
    return scroll_back(&axis_y, move);
}

void abi_MovePlayerUp(void)
{
    back_exit_shim(MovePlayerUp());
}
