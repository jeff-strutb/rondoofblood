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
#include "access.h"
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
 * WHAT IT DOES: Takes the distance to the player and adds and subtracts fourteen bytes
 * of fixed object slots to it, then takes off the player's f2f90. What the sum means is not
 * known yet.
 * HOW: A carry chain through all of them: it starts as DistanceXToPlayer's carry (the caller's
 * when the distance is $FF), and a SEC comes before the last subtract.
 * WAITS: no.
 * Entry: X object, C in (kept through a far distance), D clear. Exit: A the sum, N, Z, C and V
 * from the last subtract. X and Y kept.
 */
Sum PlayerDistanceSum(uint8_t object, bool carry_in)
{
    PlayerDistance distance;
    Sum sum;

    jsr_push(0x655A);
    distance = DistanceXToPlayer(object);
    jsr_pop();
    sum.value = distance.distance;
    sum.carry = distance.far ? carry_in : distance.object_further;
    sum.overflow = false;
    sum = add_bytes(sum.value, RD8(OBJ->anim_step[0x10]), sum.carry);
    sum = add_bytes(sum.value, RD8(OBJ->anim_timer[0x20]), sum.carry);
    sum = add_bytes(sum.value, RD8(OBJ->y_lo[0x11]), sum.carry);
    sum = add_bytes(sum.value, (uint8_t)~RD8(OBJ->x_frac[0x12]), sum.carry);
    sum = add_bytes(sum.value, (uint8_t)~RD8(OBJ->y_lo[0x13]), sum.carry);
    sum = add_bytes(sum.value, RD8(OBJ->x_lo[0x14]), sum.carry);
    sum = add_bytes(sum.value, RD8(OBJ->kind[0x15]), sum.carry);
    sum = add_bytes(sum.value, (uint8_t)~RD8(OBJ->kind[0x16]), sum.carry);
    sum = add_bytes(sum.value, (uint8_t)~RD8(OBJ->x_frac[0x17]), sum.carry);
    sum = add_bytes(sum.value, RD8(OBJ->y_lo[0x30]), sum.carry);
    sum = add_bytes(sum.value, RD8(OBJ->kind[0x31]), sum.carry);
    sum = add_bytes(sum.value, (uint8_t)~RD8(OBJ->x_lo[0x32]), sum.carry);
    sum = add_bytes(sum.value, (uint8_t)~RD8(OBJ->y_lo[0x33]), sum.carry);
    sum = add_bytes(sum.value, RD8(OBJ->fields_2ee0[0]), sum.carry);
    return add_bytes(sum.value, (uint8_t)~RD8(OBJ->hurt_timer[0]), true);
}

/* A the sum, N and Z from it, C and V from the last subtract. */
void distance_sum_exit_shim(Sum last)
{
    cpu.a = last.value;
    abi_nz(last.value);
    abi_flag(P_C, last.carry);
    abi_flag(P_V, last.overflow);
}

void abi_PlayerDistanceSum(void)
{
    distance_sum_exit_shim(PlayerDistanceSum(cpu.x, (cpu.p & P_C) != 0));
}

/*
 * @routine 0047:589 SoundIfOnFirstScreen
 * @run 6A:6589
 * WHAT IT DOES: Plays a sound if the object is on the first screen across and down.
 * HOW: Keeps the command on the stack, reads the object's two position high bytes and returns
 * if their OR is not zero; otherwise pulls the command and goes to SoundCommand.
 * WAITS: no.
 * Entry: A command, X object. Exit, not played: A the command, N and Z from it, C and V kept.
 * Exit, played: as SoundCommand with the command's flags.
 */
SoundOnScreen SoundIfOnFirstScreen(uint8_t command, uint8_t object, uint8_t kept_y)
{
    SoundOnScreen result = { 0 };
    uint8_t x_high, y_high;

    push8(command);
    x_high = RD8(OBJ->x_hi[object]);
    y_high = RD8(OBJ->y_hi[object]);
    result.command = pull8();
    if ((uint8_t)(x_high | y_high) != 0) {
        return result;
    }
    result.played = true;
    result.call = SoundCommand(result.command, object, kept_y);
    return result;
}

void sound_if_on_first_screen_exit_shim(SoundOnScreen result)
{
    if (!result.played) {
        cpu.a = result.command;
        abi_nz(result.command);
        return;
    }
    abi_nz(result.command);
    sound_command_exit_shim(result.command, result.call);
}

void abi_SoundIfOnFirstScreen(void)
{
    sound_if_on_first_screen_exit_shim(SoundIfOnFirstScreen(cpu.a, cpu.x, cpu.y));
}

/*
 * @routine 0047:598 StopPlayerX
 * @run 6A:6598
 * WHAT IT DOES: Stops the player moving along X.
 * HOW: Zeroes the X speed.
 * WAITS: no.
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
    uint8_t sign;
    uint8_t high;
    uint8_t result;
    uint8_t a;
    bool carry_in;
    bool carry;
} PlayerYMove;

typedef struct {
    YEnd how;
    uint8_t vy_hi;
    PlayerYMove move;
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
    uint8_t index;
} WallPush;

/* Which way the player's speed or push asked it to go. */
typedef enum { GOES_NOWHERE, GOES_LEFT, GOES_RIGHT } Direction;

typedef struct {
    bool moving;
    bool tested_right;
    uint8_t push;
    uint8_t speed_hi;
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
    uint8_t push_y_hi; /* the last byte the push copy read, which is A on leaving */
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

/* A after PlayerNextX: the high byte of the position's sum, which the assembly compares without reading zTemp05 again. */
static uint8_t next_high(WordPair next)
{
    return (uint8_t)((uint16_t)(next.a + next.b) >> 8);
}

/* What CheckPlayerAtLevelEnd leaves: A, the value whose N and Z the last compare or load set, and the carry. */
typedef struct {
    uint8_t a, flags_from;
    bool c;
} LevelEnd;

typedef struct {
    LevelEnd level;
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
    uint8_t speed_hi;
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
uint8_t PlayerXFromNextAlt(void);
WordPair PlayerNextX(void);
void CameraModeDispatch(void);
void abi_CameraModeDispatch(void);
static void x_on_screen_exit_shim(OnScreen x);
static void right_or_wall_exit_shim(RightOrWall result);
static void left_or_wall_exit_shim(LeftOrWall result);

static bool add8_overflows(uint8_t a, uint8_t b)
{
    return (~(a ^ b) & (a ^ (uint8_t)(a + b)) & 0x80) != 0;
}

/* By the speed's sign, or at no speed by bits 2 and 1 of what pushes the player. */
static Choice choose_direction(bool lo_first)
{
    Choice choice = { 0 };
    uint8_t first;
    uint8_t second;

    first = rd8(lo_first ? OBJ_VX_LO : OBJ_VX_HI);
    second = rd8(lo_first ? OBJ_VX_HI : OBJ_VX_LO);
    choice.moving = (first | second) != 0;
    if (choice.moving) {
        choice.speed_hi = rd8(OBJ_VX_HI);
        choice.way = (choice.speed_hi & 0x80) ? GOES_LEFT : GOES_RIGHT;
        return choice;
    }
    choice.push = rd8(zPlayerPush);
    if (choice.push & PUSH_LEFT) {
        choice.way = GOES_LEFT;
        return choice;
    }
    choice.push = rd8(zPlayerPush);
    choice.tested_right = true;
    choice.way = (choice.push & PUSH_RIGHT) ? GOES_RIGHT : GOES_NOWHERE;
    return choice;
}

/* A speed's load, or the TST that decided: Z from its AND, N and V from the byte's top bits. */
static void choice_exit_shim(Choice choice)
{
    uint8_t mask = choice.tested_right ? PUSH_RIGHT : PUSH_LEFT;

    if (choice.moving) {
        cpu.a = choice.speed_hi;
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

/* The player's X speed added to its X position, low byte then high with the carry, written to
 * a word: the reads and writes in the assembly's order. */
static WordPair add_speed_to_x(uint16_t dest_lo, uint16_t dest_hi)
{
    WordPair words;
    uint8_t vlo = rd8(OBJ_VX_LO);
    uint8_t flo = rd8(OBJ_X_FRAC);
    unsigned low = (unsigned)vlo + flo;
    uint8_t vhi;
    uint8_t xhi;

    wr8(dest_lo, (uint8_t)low);
    vhi = rd8(OBJ_VX_HI);
    xhi = rd8(OBJ_X_LO);
    wr8(dest_hi, (uint8_t)(vhi + xhi + (low >> 8)));
    words.a = (uint16_t)((vhi << 8) | vlo);
    words.b = (uint16_t)((xhi << 8) | flo);
    return words;
}

/*
 * @routine 0047:59F MovePlayerX
 * @run 6A:659F
 * WHAT IT DOES: Moves the player by its X speed.
 * HOW: Adds the 8.8 speed to position low and fraction, with no carry into the high
 * byte: the player stays on the screen.
 * WAITS: no.
 * Entry: nothing. Exit: A position low, N, Z, C and V from its add. X and Y kept.
 */
WordPair MovePlayerX(void)
{
    return add_speed_to_x(OBJ_X_FRAC, OBJ_X_LO);
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
 * WAITS: no.
 * Entry: nothing. Exit: A the position's high byte, or PlayerMayLeaveTop if it was read;
 * N and Z from A, C and V from the add. X and Y kept.
 */
PlayerYMove MovePlayerY(void)
{
    PlayerYMove result;
    uint8_t vel_lo = rd8(OBJ_VY_LO);
    uint8_t frac = rd8(OBJ_Y_FRAC);
    uint8_t vel_hi;
    uint8_t lo;
    uint8_t sign;
    uint8_t hi;
    unsigned sum;

    sum = (unsigned)vel_lo + frac;
    wr8(OBJ_Y_FRAC, (uint8_t)sum);
    vel_hi = rd8(OBJ_VY_HI);
    lo = rd8(OBJ_Y_LO);
    sum = (unsigned)vel_hi + lo + (sum >> 8);
    wr8(OBJ_Y_LO, (uint8_t)sum);
    sign = (rd8(OBJ_VY_HI) & 0x80) ? 0xFF : 0x00;
    hi = rd8(OBJ_Y_HI);
    result.carry_in = (sum >> 8) != 0;
    result.sign = sign;
    result.high = hi;
    sum = (unsigned)sign + hi + result.carry_in;
    result.result = (uint8_t)sum;
    result.carry = sum > 0xFF;
    wr8(OBJ_Y_HI, result.result);
    result.a = result.result;
    if (result.result & 0x80) {
        result.a = rd8(PlayerMayLeaveTop);
        if (result.a == 0) {
            wr8(OBJ_Y_LO, 0);
            wr8(OBJ_Y_HI, 0);
            wr8(OBJ_Y_FRAC, 0);
        }
    }
    return result;
}

/* C and V from the high byte's add; A that byte, or PlayerMayLeaveTop if it was read. */
static Alu move_y_exit_shim(PlayerYMove move)
{
    Alu exit = { 0 };

    exit.a = move.a;
    exit.nz = exit.a;
    exit.c = move.carry;
    exit.v = (~(move.sign ^ move.high) & (move.sign ^ move.result) & 0x80) != 0;
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
 * WAITS: no.
 * Entry: nothing. Exit, not moving: A zero, Z set. Exit, moving up: A the speed's high
 * byte, C and V from MovePlayerY. Exit, moving down: as the overlay leaves them.
 */
YMove MovePlayerYIfMoving(void)
{
    YMove move = { Y_NOT_MOVING };
    uint8_t vy_lo = rd8(OBJ_VY_LO);
    uint8_t vy_hi = rd8(OBJ_VY_HI);

    if ((vy_lo | vy_hi) == 0)
        return move;
    jsr_push(0x65FF);
    move.move = MovePlayerY();
    jsr_pop();
    move.vy_hi = rd8(OBJ_VY_HI);
    if (move.vy_hi & 0x80) {
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
    cpu.a = move.vy_hi;
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
 * WAITS: no.
 * Entry: nothing. Exit: A and Y as the overlay leaves them, V too, X 7, N 0, Z 1, C 1.
 */
ColumnsEnd PlayerAndCameraFrame(void)
{
    ColumnsEnd moved;
    uint8_t low;
    uint8_t high;
    uint8_t push_low;
    uint8_t push_high;
    unsigned sum;
    uint8_t anchor;
    uint8_t fraction;
    uint8_t lift;

    push8(rd8(OBJ_VX_LO));
    push8(rd8(OBJ_VX_HI));
    push8(rd8(OBJ_VY_LO));
    push8(rd8(OBJ_VY_HI));
    if (rd8(zPlayerPush) & PUSH_ADDS_X) {
        low = rd8(OBJ_VX_LO);
        push_low = rd8(zPushXLo);
        sum = (unsigned)low + push_low;
        wr8(OBJ_VX_LO, (uint8_t)sum);
        high = rd8(OBJ_VX_HI);
        push_high = rd8(zPushXHi);
        wr8(OBJ_VX_HI, (uint8_t)(high + push_high + (sum >> 8)));
    }
    if (rd8(zPlayerPush) & PUSH_ADDS_Y) {
        low = rd8(OBJ_VY_LO);
        push_low = rd8(zPushYLo);
        sum = (unsigned)low + push_low;
        wr8(OBJ_VY_LO, (uint8_t)sum);
        high = rd8(OBJ_VY_HI);
        push_high = rd8(zPushYHi);
        wr8(OBJ_VY_HI, (uint8_t)(high + push_high + (sum >> 8)));
        anchor = rd8(PlayerAnchorObject);
        fraction = rd8((uint16_t)(OBJ_Y_FRAC + anchor));
        wr8(ScrollYFrac, fraction);
        wr8(OBJ_Y_FRAC, fraction);
    }
    if (rd8(zPlayerPush) & PUSH_LIFTS_SCROLL) {
        wr8(ScrollStepYLo, 0);
        lift = rd8(zPushYHi);
        wr8(ScrollStepYHi, lift);
        high = rd8(OBJ_Y_LO);
        wr8(OBJ_Y_LO, (uint8_t)(high - lift));
    } else {
        jsr_push(0x6665);
        CameraModeDispatch();
        jsr_pop();
    }
    wr8(OBJ_VY_HI, pull8());
    wr8(OBJ_VY_LO, pull8());
    wr8(OBJ_VX_HI, pull8());
    wr8(OBJ_VX_LO, pull8());
    jsr_push(0x6678);
    moved = PlayerMoved();
    jsr_pop();
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

/* What the table's jump leaves for the mode: A and X the mode doubled, N and Z from it, C its top bit. */
static void camera_mode_registers_shim(uint8_t mode)
{
    cpu.a = (uint8_t)(mode << 1);
    cpu.x = cpu.a;
    abi_nz(cpu.a);
    abi_flag(P_C, (mode & 0x80) != 0);
}

/* One call to a camera mode with no C yet, with what the table's jump leaves. */
static void camera_mode_original_shim(uint16_t handler, uint8_t mode)
{
    camera_mode_registers_shim(mode);
    call_original(handler);
}

void abi_CameraModeCarried(void);
void abi_CameraModeLeftWall(void);
void abi_CameraModeFollow(void);
void abi_CameraModeVertical(void);

/*
 * @routine 0047:681 CameraModeDispatch
 * @run 6A:6681
 * WHAT IT DOES: Runs the section's camera mode: the handler the table of 30 words, indexed by
 * CameraMode, names.
 * HOW: CameraMode doubled indexes CameraModeHandlers; a jump through it, so the handler's RTS
 * returns to the caller of this routine.
 * WAITS: no.
 * Entry: nothing. Exit: as the handler leaves them.
 */
void CameraModeDispatch(void)
{
    uint8_t mode = rd8(CameraMode);
    uint16_t entry = (uint16_t)(CameraModeHandlers + (uint8_t)(mode << 1));
    uint16_t handler = read16(entry, (uint16_t)(entry + 1));

    switch (handler) {
    case 0x66C5:
        camera_mode_registers_shim(mode);
        abi_CameraModeCarried();
        break;
    case 0x6722:
        camera_mode_registers_shim(mode);
        abi_CameraModeLeftWall();
        break;
    case 0x678E:
        camera_mode_registers_shim(mode);
        abi_CameraModeFollow();
        break;
    case 0x6A0F:
        camera_mode_registers_shim(mode);
        abi_CameraModeVertical();
        break;
    default:
        camera_mode_original_shim(handler, mode);
        break;
    }
}

void abi_CameraModeDispatch(void)
{
    CameraModeDispatch();
}

/*
 * @routine 0047:6C5 CameraModeCarried
 * @run 6A:66C5
 * WHAT IT DOES: The camera mode where the scroll follows what carries the player, not
 * the player: moves the player on the screen, and takes the push as the scroll step
 * unless bit 4 of zPlayerPush says it was already added to the speed.
 * HOW: Zeroes both steps, MovePlayerYIfMoving, PlayerXOnScreen, copies the push.
 * WAITS: no.
 * Entry: cpu flags current. Exit: A and N, Z the last read; C and V as the moves left
 * them; X and Y as the overlay left them.
 */
CarriedMode CameraModeCarried(void)
{
    CarriedMode mode;

    wr8(ScrollStepXLo, 0);
    wr8(ScrollStepXHi, 0);
    wr8(ScrollStepYLo, 0);
    wr8(ScrollStepYHi, 0);
    jsr_push(0x66D4);
    mode.y = MovePlayerYIfMoving();
    jsr_pop();
    jsr_push(0x66D7);
    mode.x = PlayerXOnScreen();
    jsr_pop();
    mode.copied_push = !(rd8(zPlayerPush) & PUSH_ADDS_X);
    if (mode.copied_push) {
        uint8_t byte = rd8(zPushXLo);

        wr8(ScrollStepXLo, byte);
        byte = rd8(zPushXHi);
        wr8(ScrollStepXHi, byte);
        byte = rd8(zPushYLo);
        wr8(ScrollStepYLo, byte);
        byte = rd8(zPushYHi);
        wr8(ScrollStepYHi, byte);
        mode.push_y_hi = byte;
    }
    return mode;
}

void abi_CameraModeCarried(void)
{
    CarriedMode mode = CameraModeCarried();

    y_move_exit_shim(mode.y);
    x_on_screen_exit_shim(mode.x);
    if (mode.copied_push) {
        cpu.a = mode.push_y_hi;
        abi_nz(cpu.a);
    }
}

/*
 * @routine 0047:6EF PlayerNextX
 * @run 6A:66EF
 * WHAT IT DOES: Works out where the player's speed would put it along X, without moving
 * it.
 * HOW: The 8.8 sum, left in zTemp04-05.
 * WAITS: no.
 * Entry: nothing. Exit: A zTemp05, N, Z, C, V from its add. X and Y kept.
 */
WordPair PlayerNextX(void)
{
    return add_speed_to_x(zTemp04, zTemp05);
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
 * WAITS: no.
 * Entry: nothing. Exit: A and flags as the last compare or load left them. X, Y, V kept.
 */
LevelEnd CheckPlayerAtLevelEnd(void)
{
    uint8_t scroll = rd8(ScrollXHiScreen);
    uint8_t limit = rd8(ScrollXMaxHi);
    uint8_t x;
    LevelEnd end = { scroll, (uint8_t)(scroll - limit), scroll >= limit };

    if (scroll != limit)
        return end;
    x = rd8(OBJ_X_LO);
    end.a = x;
    end.flags_from = (uint8_t)(x - LEVEL_END_FROM_X);
    end.c = x >= LEVEL_END_FROM_X;
    if (x < LEVEL_END_FROM_X)
        return end;
    x = rd8(OBJ_X_LO);
    end.a = x;
    end.flags_from = (uint8_t)(x - LEVEL_END_TO_X);
    end.c = x >= LEVEL_END_TO_X;
    if (x >= LEVEL_END_TO_X)
        return end;
    wr8(OBJ_3510, 0xFF);
    wr8(OBJ_3568, LEVEL_END_TIMER);
    end.a = LEVEL_END_TIMER;
    end.flags_from = LEVEL_END_TIMER;
    return end;
}

/* A compare leaves A, N, Z and C; V stands. */
static void compare_exit_shim(uint8_t a, uint8_t b)
{
    cpu.a = a;
    abi_nz((uint8_t)(a - b));
    abi_flag(P_C, a >= b);
}

/* A and the flags from the compare that decided, or the last load. V stands. */
static void level_end_exit_shim(LevelEnd end)
{
    cpu.a = end.a;
    abi_nz(end.flags_from);
    abi_flag(P_C, end.c);
}

void abi_CheckPlayerAtLevelEnd(void)
{
    level_end_exit_shim(CheckPlayerAtLevelEnd());
}

/*
 * @routine 0047:722 CameraModeLeftWall
 * @run 6A:6722
 * WHAT IT DOES: The camera mode for a section the scroll does not leave to the left:
 * right as in CameraModeFollow, left moves the player on the screen alone. A wall to the
 * left pushes the player right by a table on how deep it went.
 * HOW: Follows the speed's sign, or bits 2 and 1 of zPlayerPush at no speed, then the
 * overlay's wall entry with Y 1.
 * WAITS: no.
 * Entry: cpu flags current. Exit: A and flags as the path taken left them; X and Y as
 * the overlay left them or X the depth.
 */
LeftWallMode CameraModeLeftWall(void)
{
    LeftWallMode mode = { 0 };

    jsr_push(0x6725);
    mode.level = CheckPlayerAtLevelEnd();
    jsr_pop();
    jsr_push(0x6728);
    mode.y = MovePlayerYIfMoving();
    jsr_pop();
    write16(ScrollStepXLo, ScrollStepXHi, 0);
    mode.choice = choose_direction(false);
    if (mode.choice.way == GOES_NOWHERE)
        return mode;
    if (mode.choice.way == GOES_RIGHT) {
        mode.right = MovePlayerRightOrWall();
        return mode;
    }
    jsr_push(0x6751);
    mode.wall = PlayerWall(1);
    jsr_pop();
    if (!wall_found(mode.wall)) {
        jsr_push(0x677F);
        mode.next = PlayerNextX();
        jsr_pop();
        if (next_high(mode.next) <= PLAYER_MIN_X) {
            PlayerAtLeftEdge();
            mode.how = LEFT_WALL_AT_EDGE;
        } else {
            PlayerXFromNextAlt();
            mode.how = LEFT_WALL_MOVED;
        }
        return mode;
    }
    if (rd8(OBJ_STATE) == PLAYER_STATE_NO_WALL_PUSH) {
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

    level_end_exit_shim(mode.level);
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
        compare_exit_shim(next_high(mode.next), PLAYER_MIN_X);
        cpu.a = mode.how == LEFT_WALL_AT_EDGE ? PLAYER_MIN_X : next_high(mode.next);
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
 * WAITS: no.
 * Entry: cpu flags current. Exit: A and flags as the path taken left them.
 */
/* FollowMode holds how each step ended, as plain values: no flags pass through this body. */
FollowMode CameraModeFollow(void)
{
    FollowMode mode = { 0 };

    jsr_push(0x6791);
    mode.y = MovePlayerYIfMoving();
    jsr_pop();
    write16(ScrollStepXLo, ScrollStepXHi, 0);
    mode.choice = choose_direction(false);
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
 * WAITS: no.
 * Entry: cpu flags current. Exit, no wall: A and flags as the overlay left them, Z set.
 * Exit, wall: A 1, N and Z from it.
 */
/* After a wall: stop only, or push back unless that passes the edge or meets another wall. */
/* The frames of the calls a wall push makes, as the assembly returns from them. */
typedef struct { uint16_t stop_first, stop_second, wall_second, edge; } PushFrames;

static const PushFrames push_frames_right = { 0x67D0, 0x67EC, 0x67F3, 0x6800 };
static const PushFrames push_frames_left = { 0x690B, 0x6927, 0x692E, 0x693B };

static void push_from_wall(WallPush *push, const PushFrames *frames, uint8_t other_side)
{
    uint8_t amount;
    uint8_t moved;
    uint8_t kept;
    uint8_t answer;

    if (rd8(OBJ_STATE) == PLAYER_STATE_NO_WALL_PUSH) {
        if (!(rd8(OBJ_VY_HI) & 0x80)) {
            jsr_push(frames->stop_first);
            StopPlayerX();
            jsr_pop();
        }
        push->how = WALL_STOPPED_ONLY;
        return;
    }
    if (push->leftward) {
        push->index = rd8(zTemp00);
        amount = rd8((uint16_t)(WallLeftPush + push->index));
    } else {
        amount = (uint8_t)(0u - rd8(zTemp00));
    }
    push->amount = amount;
    push->position = rd8(OBJ_X_LO);
    moved = (uint8_t)(amount + push->position);
    if (push->leftward ? moved >= PLAYER_MAX_X : moved < PLAYER_MIN_X) {
        jsr_push(frames->edge);
        if (push->leftward)
            PlayerAtRightEdge();
        else
            PlayerAtLeftEdge();
        jsr_pop();
        push->how = WALL_PAST_EDGE;
        return;
    }
    kept = rd8(OBJ_X_LO);
    push8(kept);
    wr8(OBJ_X_LO, moved);
    wr8(OBJ_X_FRAC, 0);
    jsr_push(frames->stop_second);
    StopPlayerX();
    jsr_pop();
    wr8(zPlayerPush, 0);
    jsr_push(frames->wall_second);
    push->second = PlayerWall(other_side);
    jsr_pop();
    kept = pull8();
    answer = wall_answer(push->second);
    if (answer != 0)
        wr8(OBJ_X_LO, kept);
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
            cpu.x = push.index;
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

    jsr_push(0x67BF);
    push.first = PlayerWall(0);
    jsr_pop();
    if (wall_found(push.first))
        push_from_wall(&push, &push_frames_right, 1);
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
 * WAITS: no.
 * Entry: cpu flags current. Exit: as the one that ran last.
 */
RightOrWall MovePlayerRightOrWall(void)
{
    RightOrWall result = { 0 };

    jsr_push(0x6806);
    result.wall = PlayerWallRight();
    jsr_pop();
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
static uint8_t position_from_next(const Axis *axis)
{
    uint8_t frac = rd8(zTemp04);
    uint8_t lo;

    wr8(axis->pos_frac, frac);
    lo = rd8(zTemp05);
    wr8(axis->pos_lo, lo);
    return lo;
}

/*
 * @routine 0048:00C PlayerXFromNext
 * @run 6A:680C
 * WHAT IT DOES: Accepts the position PlayerNextX worked out.
 * HOW: Copies zTemp04-05 to the position.
 * WAITS: no.
 * Entry: zTemp04-05. Exit: A zTemp05, N and Z from it. X, Y, C, V kept.
 */
uint8_t PlayerXFromNext(void)
{
    return position_from_next(&axis_x);
}

void abi_PlayerXFromNext(void)
{
    cpu.a = PlayerXFromNext();
    abi_nz(cpu.a);
}

/* A scroll position against a limit: the high byte given, the low in zTemp03. */
typedef struct { uint8_t a; uint8_t b; bool reached; } LimitTest;

static LimitTest scroll_reaches_limit(uint8_t high, uint16_t limit_hi, uint16_t limit_lo)
{
    LimitTest test;

    test.a = high;
    test.b = rd8(limit_hi);
    if (test.a == test.b) {
        test.a = rd8(zTemp03);
        test.b = rd8(limit_lo);
    }
    test.reached = test.a >= test.b;
    return test;
}

/* A and N, Z, C from the compare that decided: the high bytes, or on equal the low. */
static void limit_exit_shim(LimitTest test)
{
    cpu.a = test.a;
    abi_nz((uint8_t)(test.a - test.b));
    abi_flag(P_C, test.reached);
}

/* What a test against the middle read: the high byte, and at the middle the fraction too. */
typedef struct { uint8_t high; uint8_t frac; bool past; } MiddleTest;

static MiddleTest middle_test(uint8_t high, uint16_t frac)
{
    MiddleTest test;

    test.high = high;
    test.frac = 0;
    if (high == SCREEN_MIDDLE)
        test.frac = rd8(frac);
    test.past = high >= SCREEN_MIDDLE;
    return test;
}

/* A and N, Z from the compare with the middle, or at the middle from the fraction read. */
static void middle_exit_shim(MiddleTest test)
{
    if (test.high == SCREEN_MIDDLE) {
        cpu.a = test.frac;
        abi_nz(cpu.a);
    } else {
        cpu.a = test.high;
        abi_nz((uint8_t)(test.high - SCREEN_MIDDLE));
    }
    abi_flag(P_C, test.past);
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
 * WAITS: no.
 * Entry: nothing. Exit: A the last value, N and Z from it, C and V as
 * right_move_exit_shim gives them. X and Y kept. Clobbers zTemp00-05.
 */
MiddleTest NextPastMiddle(void);
MiddleTest PlayerXPastMiddle(void);
WordPair NextPastMiddleAmount(void);
LimitTest CompareScrollWithRightLimit(uint8_t high);
uint8_t PlayerXFromNext(void);

RightMoveResult MovePlayerRight(void)
{
    RightMoveResult move = { 0 };
    WordPair words;
    MiddleTest middle;
    LimitTest reach;
    uint8_t scroll_lo;
    uint8_t frac;
    uint8_t base;
    unsigned sum;
    uint8_t step_lo;
    uint8_t step_hi;
    unsigned borrow;

    wr8(ScrollStepXLo, 0);
    wr8(ScrollStepXHi, 0);
    jsr_push(0x6820);
    words = PlayerNextX();
    jsr_pop();
    move.speed = words.a;
    move.from = words.b;
    move.next = (uint16_t)(words.a + words.b);
    if ((move.next >> 8) >= PLAYER_MAX_X) {
        PlayerAtRightEdge();
        move.how = MOVED_TO_RIGHT_EDGE;
        return move;
    }
    jsr_push(0x682A);
    middle = NextPastMiddle();
    jsr_pop();
    if (!middle.past) {
        PlayerXFromNext();
        move.how = MOVED_PLAYER;
        return move;
    }
    scroll_lo = rd8(ScrollXLo);
    wr8(zTemp03, scroll_lo);
    move.page = rd8(ScrollXHiScreen);
    jsr_push(0x6837);
    reach = CompareScrollWithRightLimit(move.page);
    jsr_pop();
    if (reach.a == reach.b) {
        PlayerXFromNext();
        move.how = MOVED_PLAYER_AT_SCROLL_END;
        return move;
    }
    jsr_push(0x683C);
    NextPastMiddleAmount();
    jsr_pop();
    jsr_push(0x683F);
    middle = PlayerXPastMiddle();
    jsr_pop();
    if (middle.past) {
        base = rd8(OBJ_VX_LO);
        wr8(zTemp00, base);
    } else {
        base = rd8(zTemp00);
    }
    frac = rd8(ScrollXFrac);
    sum = (unsigned)base + frac;
    wr8(zTemp02, (uint8_t)sum);
    if (middle.past) {
        base = rd8(OBJ_VX_HI);
        wr8(zTemp01, base);
    } else {
        base = rd8(zTemp01);
    }
    scroll_lo = rd8(ScrollXLo);
    sum = (unsigned)base + scroll_lo + (sum >> 8);
    wr8(zTemp03, (uint8_t)sum);
    move.scroll = (uint16_t)((scroll_lo << 8) | frac);
    move.page = rd8(ScrollXHiScreen);
    sum = (unsigned)move.page + (sum >> 8);
    jsr_push(middle.past ? 0x685D : 0x6877);
    reach = CompareScrollWithRightLimit((uint8_t)sum);
    jsr_pop();
    if (!reach.reached) {
        if (!middle.past) {
            wr8(OBJ_X_LO, SCREEN_MIDDLE);
            wr8(OBJ_X_FRAC, 0);
        }
        step_lo = rd8(zTemp00);
        wr8(ScrollStepXLo, step_lo);
        step_hi = rd8(zTemp01);
        wr8(ScrollStepXHi, step_hi);
        move.step = (uint16_t)((step_hi << 8) | step_lo);
        move.how = MOVED_SCROLL;
        return move;
    }
    frac = rd8(ScrollXFrac);
    borrow = frac != 0;
    step_lo = (uint8_t)(0 - frac);
    wr8(ScrollStepXLo, step_lo);
    base = rd8(ScrollXMaxLo);
    scroll_lo = rd8(ScrollXLo);
    step_hi = (uint8_t)(base - scroll_lo - borrow);
    wr8(ScrollStepXHi, step_hi);
    base = rd8(zTemp04);
    step_lo = rd8(ScrollStepXLo);
    borrow = base < step_lo;
    wr8(OBJ_X_FRAC, (uint8_t)(base - step_lo));
    base = rd8(zTemp05);
    step_hi = rd8(ScrollStepXHi);
    wr8(OBJ_X_LO, (uint8_t)(base - step_hi - borrow));
    move.step = (uint16_t)((step_hi << 8) | step_lo);
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
 * WAITS: no.
 * Entry: zTemp05. Exit: C the answer; A and N, Z from the last read or compare. X, Y, V
 * kept.
 */
MiddleTest NextPastMiddle(void)
{
    uint8_t high = rd8(zTemp05);

    return middle_test(high, OBJ_X_FRAC);
}

void abi_NextPastMiddle(void)
{
    middle_exit_shim(NextPastMiddle());
}

/*
 * @routine 0048:0C2 NextFractionSet
 * @run 6A:68C2
 * WHAT IT DOES: Sets C if the next position's fraction, zTemp04, is not zero. Nothing in
 * the kernel reaches it: left over from an earlier NextPastMiddle.
 * HOW: A read and a branch.
 * WAITS: no.
 * Entry: zTemp04. Exit: A zTemp04, N and Z from it; C set if not zero, else kept.
 */
uint8_t NextFractionSet(void)
{
    return rd8(zTemp04);
}

void abi_NextFractionSet(void)
{
    uint8_t fraction = NextFractionSet();

    if (fraction != 0)
        abi_flag(P_C, true);
    cpu.a = fraction;
    abi_nz(fraction);
}

/*
 * @routine 0048:0C9 PlayerXPastMiddle
 * @run 6A:68C9
 * WHAT IT DOES: Says whether the player is at the screen's middle or past it, along X.
 * HOW: As NextPastMiddle, on the position.
 * WAITS: no.
 * Entry: nothing. Exit: as NextPastMiddle.
 */
MiddleTest PlayerXPastMiddle(void)
{
    uint8_t high = rd8(OBJ_X_LO);

    return middle_test(high, OBJ_X_FRAC);
}

void abi_PlayerXPastMiddle(void)
{
    middle_exit_shim(PlayerXPastMiddle());
}

/*
 * @routine 0048:0DC NextPastMiddleAmount
 * @run 6A:68DC
 * WHAT IT DOES: Gives how far the next position lies past the middle.
 * HOW: Subtracts $8000 from zTemp04-05, into zTemp00-01.
 * WAITS: no.
 * Entry: zTemp04-05. Exit: A zTemp01, N, Z, C, V from its subtract. X and Y kept.
 */
WordPair NextPastMiddleAmount(void)
{
    uint8_t lo = rd8(zTemp04);
    uint8_t hi;
    WordPair words;

    wr8(zTemp00, lo);
    hi = rd8(zTemp05);
    wr8(zTemp01, (uint8_t)(hi - SCREEN_MIDDLE));
    words.a = (uint16_t)((hi << 8) | lo);
    words.b = (uint16_t)(SCREEN_MIDDLE << 8);
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
 * WAITS: no.
 * Entry: A high, zTemp03 low. Exit: A and N, Z, C from the last compare. X, Y, V kept.
 */
LimitTest CompareScrollWithRightLimit(uint8_t high)
{
    return scroll_reaches_limit(high, ScrollXMaxHi, ScrollXMaxLo);
}

void abi_CompareScrollWithRightLimit(void)
{
    limit_exit_shim(CompareScrollWithRightLimit(cpu.a));
}

/*
 * @routine 0048:0F5 PlayerWallLeft
 * @run 6A:68F5
 * WHAT IT DOES: Asks the overlay about a wall to the left, and pushes the player back
 * out of one: right by a table on how deep it went, but not into another wall the
 * overlay finds there, and not past the right edge.
 * HOW: As PlayerWallRight, with Y 1 then Y 0.
 * WAITS: no.
 * Entry: cpu flags current. Exit: as PlayerWallRight.
 */
WallPush PlayerWallLeft(void)
{
    WallPush push = { WALL_NOT_THERE, true };

    jsr_push(0x68FA);
    push.first = PlayerWall(1);
    jsr_pop();
    if (wall_found(push.first))
        push_from_wall(&push, &push_frames_left, 0);
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
 * WAITS: no.
 * Entry: cpu flags current. Exit: as the one that ran last.
 */
LeftOrWall MovePlayerLeftOrWall(void)
{
    LeftOrWall result = { 0 };

    jsr_push(0x6949);
    result.wall = PlayerWallLeft();
    jsr_pop();
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
 * WAITS: no.
 * Entry: zTemp04-05. Exit: A zTemp05, N and Z from it. X, Y, C, V kept.
 */
uint8_t PlayerXFromNextAlt(void)
{
    return position_from_next(&axis_x);
}

void abi_PlayerXFromNextAlt(void)
{
    cpu.a = PlayerXFromNextAlt();
    abi_nz(cpu.a);
}

/* The scroll takes the step: at the middle the player is put there (low, then fraction) and the
 * step is the amount in zTemp00-01; else the step is the speed. */
static void step_scroll_back(const Axis *axis, BackMove *move, bool at_middle)
{
    uint8_t lo;
    uint8_t hi;

    if (at_middle) {
        wr8(axis->pos_lo, SCREEN_MIDDLE);
        wr8(axis->pos_frac, 0);
        lo = rd8(zTemp00);
        wr8(axis->step_lo, lo);
        hi = rd8(zTemp01);
        wr8(axis->step_hi, hi);
    } else {
        lo = rd8(axis->vel_lo);
        wr8(axis->step_lo, lo);
        hi = rd8(axis->vel_hi);
        wr8(axis->step_hi, hi);
    }
    move->step = (uint16_t)((hi << 8) | lo);
}

/* The calls a scroll step back makes, as the frames of one axis return from them. */
typedef struct {
    uint16_t amount_ret, far_ret, near_ret;
    MiddleTest (*past_middle)(void);
} BackFrames;

MiddleTest NextPastMiddle(void);
MiddleTest PlayerXPastMiddle(void);
MiddleTest PlayerYPastMiddle(void);
WordPair NextPastMiddleAmount(void);

static const BackFrames back_frames_x = { 0x6971, 0x6986, 0x698D, PlayerXPastMiddle };
static const BackFrames back_frames_y = { 0x6B8D, 0x6BA2, 0x6BA9, PlayerYPastMiddle };

static BackMove scroll_back(const Axis *axis, BackMove move, const BackFrames *frames)
{
    MiddleTest middle;
    uint8_t page;
    uint8_t lowest;
    uint8_t frac;
    uint8_t lo;
    uint8_t base;
    uint8_t low_step;
    uint8_t high_step;
    uint8_t position_frac;
    uint8_t added_lo;
    uint8_t position_lo;
    uint8_t added_hi;
    unsigned sum;
    unsigned carry;

    jsr_push(frames->amount_ret);
    NextPastMiddleAmount();
    jsr_pop();
    page = rd8(axis->scroll_hi);
    lowest = rd8(axis->min_hi);
    move.page = page;
    if (page != lowest) {
        jsr_push(frames->far_ret);
        middle = frames->past_middle();
        jsr_pop();
        move.at_middle = middle.past;
        step_scroll_back(axis, &move, middle.past);
        move.how = BACK_STEP_FAR;
        return move;
    }
    frac = rd8(axis->scroll_frac);
    lo = rd8(axis->scroll_lo);
    if ((frac | lo) == 0) {
        position_from_next(axis);
        move.how = BACK_AT_START;
        return move;
    }
    jsr_push(frames->near_ret);
    middle = frames->past_middle();
    jsr_pop();
    move.at_middle = middle.past;
    base = middle.past ? rd8(zTemp00) : rd8(axis->vel_lo);
    frac = rd8(axis->scroll_frac);
    sum = (unsigned)base + frac;
    wr8(zTemp02, (uint8_t)sum);
    base = middle.past ? rd8(zTemp01) : rd8(axis->vel_hi);
    lo = rd8(axis->scroll_lo);
    sum = (unsigned)base + lo + (sum >> 8);
    wr8(zTemp03, (uint8_t)sum);
    move.carried = (sum >> 8) != 0;
    page = rd8(axis->scroll_hi);
    move.page = page;
    lowest = rd8(axis->min_hi);
    if ((uint8_t)(0xFF + page + (move.carried ? 1 : 0)) == lowest) {
        step_scroll_back(axis, &move, middle.past);
        move.how = BACK_STEP_NEAR;
        return move;
    }
    frac = rd8(axis->scroll_frac);
    low_step = (uint8_t)(0 - frac);
    wr8(axis->step_lo, low_step);
    lo = rd8(axis->scroll_lo);
    high_step = (uint8_t)(0 - lo - (frac != 0 ? 1 : 0));
    wr8(axis->step_hi, high_step);
    position_frac = rd8(axis->pos_frac);
    added_lo = rd8(axis->back_add_lo);
    sum = (unsigned)position_frac + added_lo;
    wr8(axis->pos_frac, (uint8_t)sum);
    carry = sum >> 8;
    position_lo = rd8(axis->pos_lo);
    added_hi = rd8(axis->back_add_hi);
    wr8(axis->pos_lo, (uint8_t)(position_lo + added_hi + carry));
    move.step = (uint16_t)((high_step << 8) | low_step);
    move.position = (uint16_t)((position_lo << 8) | position_frac);
    move.added = (uint16_t)((added_hi << 8) | added_lo);
    move.how = BACK_BOTH;
    return move;
}

/*
 * @routine 0048:15A MovePlayerLeft
 * @run 6A:695A
 * WHAT IT DOES: Moves the player left, or the scroll under it, as MovePlayerRight does
 * to the right, and stops the player at the left edge of the screen.
 * HOW: The next position, the edge, then the scroll's start in place of its limit.
 * WAITS: no.
 * Entry: nothing. Exit: A, flags as the last step left them. X and Y kept. Clobbers
 * zTemp00-05.
 */
BackMove MovePlayerLeft(void)
{
    BackMove move = { 0 };
    WordPair words;
    MiddleTest middle;
    uint8_t high;

    jsr_push(0x695D);
    words = PlayerNextX();
    jsr_pop();
    move.speed = words.a;
    move.from = words.b;
    move.next = (uint16_t)(words.a + words.b);
    high = (uint8_t)(move.next >> 8);
    if (high <= PLAYER_MIN_X) {
        PlayerAtLeftEdge();
        move.how = BACK_AT_EDGE;
        return move;
    }
    jsr_push(0x696C);
    middle = NextPastMiddle();
    jsr_pop();
    if (middle.past) {
        position_from_next(&axis_x);
        move.how = BACK_PLAYER;
        return move;
    }
    return scroll_back(&axis_x, move, &back_frames_x);
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
 * WAITS: no.
 * Entry: cpu flags current. Exit: A and flags as the path taken left them.
 */
VerticalMode CameraModeVertical(void)
{
    VerticalMode mode = { 0 };
    uint8_t speed_hi;
    uint8_t speed_lo;

    jsr_push(0x6A12);
    mode.x = PlayerXOnScreen();
    jsr_pop();
    wr8(ScrollStepYHi, 0);
    wr8(ScrollStepYLo, 0);
    speed_hi = rd8(OBJ_VY_HI);
    speed_lo = rd8(OBJ_VY_LO);
    if ((speed_hi | speed_lo) == 0)
        wr8(ScrollYFrac, 0);
    mode.speed_hi = rd8(OBJ_VY_HI);
    if (mode.speed_hi & 0x80) {
        mode.how = VERTICAL_UP;
        mode.up = MovePlayerUp();
        return mode;
    }
    jsr_push(0x6A2B);
    mode.floor_before = PlayerFloor();
    jsr_pop();
    if (floor_answer(mode.floor_before) != 0) {
        mode.how = VERTICAL_FLOOR_FIRST;
        return mode;
    }
    mode.how = VERTICAL_DOWN;
    jsr_push(0x6A30);
    mode.down = MovePlayerDown();
    jsr_pop();
    wr8(zPlayerPush, 0);
    mode.floor_after = PlayerFloor();
    return mode;
}

void abi_CameraModeVertical(void)
{
    VerticalMode mode = CameraModeVertical();

    x_on_screen_exit_shim(mode.x);
    cpu.a = mode.speed_hi;
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
 * WAITS: no.
 * Entry: cpu flags current. Exit: A and flags as the path taken left them.
 */
OnScreen PlayerXOnScreen(void)
{
    OnScreen screen = { 0 };
    uint8_t x;

    screen.choice = choose_direction(true);
    if (screen.choice.way == GOES_NOWHERE) {
        screen.how = ON_SCREEN_STAYED;
        return screen;
    }
    if (screen.choice.way == GOES_LEFT) {
        jsr_push(0x6A7A);
        screen.wall = PlayerWallLeft();
    } else {
        jsr_push(0x6A59);
        screen.wall = PlayerWallRight();
    }
    jsr_pop();
    if (screen.wall.how != WALL_NOT_THERE) {
        screen.how = ON_SCREEN_WALLED;
        return screen;
    }
    screen.move = add_speed_to_x(OBJ_X_FRAC, OBJ_X_LO);
    x = (uint8_t)((uint16_t)(screen.move.a + screen.move.b) >> 8);
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
 * WAITS: no.
 * Entry: zTemp04-06. Exit: A the high byte, N and Z from it. X, Y, C, V kept.
 */
uint8_t PlayerYFromNextWord(void)
{
    uint8_t high;

    position_from_next(&axis_y);
    high = rd8(zTemp06);
    wr8(OBJ_Y_HI, high);
    return high;
}

void abi_PlayerYFromNextWord(void)
{
    cpu.a = PlayerYFromNextWord();
    abi_nz(cpu.a);
}

/*
 * @routine 0048:298 MovePlayerDown
 * @run 6A:6A98
 * WHAT IT DOES: Moves the player down, or the scroll under it, as MovePlayerRight does
 * to the right.
 * HOW: The 16.8 next position in zTemp04-06, then as MovePlayerRight on the Y fields.
 * WAITS: no.
 * Entry: nothing. Exit: A, flags as the last step left them. X and Y kept. Clobbers
 * zTemp00-06.
 */
/* zTemp00-06 hold the sums here because the routine leaves them: they are RAM it must end with. */
LimitTest CompareScrollWithBottomLimit(uint8_t high);
uint8_t PlayerYFromNextWord(void);

DownMove MovePlayerDown(void)
{
    DownMove move = { 0 };
    MiddleTest middle;
    LimitTest reach;
    uint8_t speed_lo;
    uint8_t speed_hi;
    uint8_t frac;
    uint8_t from_lo;
    uint8_t base;
    uint8_t scroll_lo;
    uint8_t step_lo;
    uint8_t step_hi;
    unsigned sum;

    wr8(ScrollStepYLo, 0);
    wr8(ScrollStepYHi, 0);
    speed_lo = rd8(OBJ_VY_LO);
    frac = rd8(OBJ_Y_FRAC);
    sum = (unsigned)speed_lo + frac;
    wr8(zTemp04, (uint8_t)sum);
    speed_hi = rd8(OBJ_VY_HI);
    from_lo = rd8(OBJ_Y_LO);
    sum = (unsigned)speed_hi + from_lo + (sum >> 8);
    wr8(zTemp05, (uint8_t)sum);
    move.next = (uint16_t)((((uint8_t)sum) << 8) | (uint8_t)(speed_lo + frac));
    move.next_carried = (sum >> 8) != 0;
    move.high = rd8(OBJ_Y_HI);
    move.next_page = (uint8_t)(move.high + (move.next_carried ? 1 : 0));
    wr8(zTemp06, move.next_page);
    jsr_push(0x6AB8);
    middle = NextPastMiddle();
    jsr_pop();
    if (!middle.past) {
        PlayerYFromNextWord();
        move.how = DOWN_PLAYER;
        return move;
    }
    scroll_lo = rd8(ScrollYLo);
    wr8(zTemp03, scroll_lo);
    move.page = rd8(ScrollYHiScreen);
    jsr_push(0x6AC5);
    reach = CompareScrollWithBottomLimit(move.page);
    jsr_pop();
    if (reach.a == reach.b) {
        PlayerYFromNextWord();
        move.how = DOWN_AT_LIMIT;
        return move;
    }
    jsr_push(0x6ACA);
    NextPastMiddleAmount();
    jsr_pop();
    jsr_push(0x6ACD);
    middle = PlayerYPastMiddle();
    jsr_pop();
    if (middle.past) {
        base = rd8(OBJ_VY_LO);
        wr8(zTemp00, base);
    } else {
        base = rd8(zTemp00);
    }
    frac = rd8(ScrollYFrac);
    sum = (unsigned)base + frac;
    wr8(zTemp02, (uint8_t)sum);
    if (middle.past) {
        base = rd8(OBJ_VY_HI);
        wr8(zTemp01, base);
    } else {
        base = rd8(zTemp01);
    }
    scroll_lo = rd8(ScrollYLo);
    sum = (unsigned)base + scroll_lo + (sum >> 8);
    wr8(zTemp03, (uint8_t)sum);
    move.carried = (sum >> 8) != 0;
    move.page = rd8(ScrollYHiScreen);
    sum = (unsigned)move.page + (move.carried ? 1 : 0);
    jsr_push(middle.past ? 0x6AEB : 0x6B05);
    reach = CompareScrollWithBottomLimit((uint8_t)sum);
    jsr_pop();
    if (!reach.reached) {
        if (!middle.past) {
            wr8(OBJ_Y_LO, SCREEN_MIDDLE);
            wr8(OBJ_Y_FRAC, 0);
        }
        step_lo = rd8(zTemp00);
        wr8(ScrollStepYLo, step_lo);
        step_hi = rd8(zTemp01);
        wr8(ScrollStepYHi, step_hi);
        move.step = (uint16_t)((step_hi << 8) | step_lo);
        move.how = DOWN_STEP;
        return move;
    }
    frac = rd8(ScrollYFrac);
    step_lo = (uint8_t)(0 - frac);
    wr8(ScrollStepYLo, step_lo);
    base = rd8(ScrollYMaxLo);
    scroll_lo = rd8(ScrollYLo);
    step_hi = (uint8_t)(base - scroll_lo - (frac != 0 ? 1 : 0));
    wr8(ScrollStepYHi, step_hi);
    base = rd8(zTemp04);
    step_lo = rd8(ScrollStepYLo);
    sum = (unsigned)(base < step_lo);
    wr8(OBJ_Y_FRAC, (uint8_t)(base - step_lo));
    base = rd8(zTemp05);
    step_hi = rd8(ScrollStepYHi);
    wr8(OBJ_Y_LO, (uint8_t)(base - step_hi - sum));
    move.step = (uint16_t)((step_hi << 8) | step_lo);
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
 * WAITS: no.
 * Entry: nothing. Exit: as NextPastMiddle.
 */
MiddleTest PlayerYPastMiddle(void)
{
    uint8_t high = rd8(OBJ_Y_LO);

    return middle_test(high, OBJ_Y_FRAC);
}

void abi_PlayerYPastMiddle(void)
{
    middle_exit_shim(PlayerYPastMiddle());
}

/*
 * @routine 0048:350 CompareScrollWithBottomLimit
 * @run 6A:6B50
 * WHAT IT DOES: Compares a vertical scroll position with the section's bottom limit.
 * HOW: High bytes, then on equal low bytes.
 * WAITS: no.
 * Entry: A high, zTemp03 low. Exit: A and N, Z, C from the last compare. X, Y, V kept.
 */
LimitTest CompareScrollWithBottomLimit(uint8_t high)
{
    return scroll_reaches_limit(high, ScrollYMaxHi, ScrollYMaxLo);
}

void abi_CompareScrollWithBottomLimit(void)
{
    limit_exit_shim(CompareScrollWithBottomLimit(cpu.a));
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
 * WAITS: no.
 * Entry: zTemp04-05. Exit: A zTemp05, N and Z from it. X, Y, C, V kept.
 */
uint8_t PlayerYFromNext(void)
{
    return position_from_next(&axis_y);
}

void abi_PlayerYFromNext(void)
{
    cpu.a = PlayerYFromNext();
    abi_nz(cpu.a);
}

/*
 * @routine 0048:370 MovePlayerUp
 * @run 6A:6B70
 * WHAT IT DOES: Moves the player up, or the scroll under it, as MovePlayerLeft does to
 * the left, and stops the player at the top of the map if it would pass it.
 * HOW: The next position, zero or a borrow meaning the top, then as MovePlayerLeft on the
 * Y fields.
 * WAITS: no.
 * Entry: nothing. Exit: A, flags as the last step left them. X and Y kept. Clobbers
 * zTemp00-05.
 */
BackMove MovePlayerUp(void)
{
    BackMove move = { 0 };
    MiddleTest middle;
    uint8_t speed_lo = rd8(OBJ_VY_LO);
    uint8_t frac = rd8(OBJ_Y_FRAC);
    unsigned sum = (unsigned)speed_lo + frac;
    uint8_t next_lo = (uint8_t)sum;
    uint8_t speed_hi;
    uint8_t from_lo;
    uint8_t next_hi;

    wr8(zTemp04, next_lo);
    speed_hi = rd8(OBJ_VY_HI);
    from_lo = rd8(OBJ_Y_LO);
    sum = (unsigned)speed_hi + from_lo + (sum >> 8);
    next_hi = (uint8_t)sum;
    wr8(zTemp05, next_hi);
    move.speed = (uint16_t)((speed_hi << 8) | speed_lo);
    move.from = (uint16_t)((from_lo << 8) | frac);
    move.next = (uint16_t)((next_hi << 8) | next_lo);
    if (next_hi == 0 || (sum >> 8) == 0) {
        PlayerAtTop();
        move.how = BACK_AT_TOP;
        return move;
    }
    jsr_push(0x6B88);
    middle = NextPastMiddle();
    jsr_pop();
    if (middle.past) {
        position_from_next(&axis_y);
        move.how = BACK_PLAYER;
        return move;
    }
    return scroll_back(&axis_y, move, &back_frames_y);
}

void abi_MovePlayerUp(void)
{
    back_exit_shim(MovePlayerUp());
}
