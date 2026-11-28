/*
 * Richter's overlay, loaded at $A000 from sector $1B9; Maria has her own at the same
 * addresses, from $1DD. Its jump table is what the kernel and the stage call. Each frame
 * reads the pad, handles damage and runs the handler for the state in OBJ_STATE, whose step
 * is in OBJ_SUBSTATE.
 *
 * Almost every routine here leaves what the next call sees in A and the flags, so all
 * carry them in an Alu, and X and Y go straight to cpu where the assembly loads them.
 * Code of the overlay with no C yet is called by address.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"
#include "access.h"
#include "enemy_objects.h"
#include "player_objects.h"

/* The overlay's routines with no C yet. */
enum {
    RH_STATE_KNOCK = 0xAE5E,
    RH_STATE_A963 = 0xA963, RH_STATE_ABBD = 0xABBD, RH_STATE_AF54 = 0xAF54,
    RH_B180 = 0xB180, RH_STATE_B1B6 = 0xB1B6, RH_STATE_B227 = 0xB227, RH_A9DD = 0xA9DD,
    RH_B611 = 0xB611, RH_READ_PAD = 0xB64C, RH_B659 = 0xB659, RH_SET_IDLE = 0xB669,
    RH_SET_ANIMATION = 0xB67B, RH_PROBE_B6D2 = 0xB6D2, RH_PROBE_B6F9 = 0xB6F9,
    RH_B704 = 0xB704, RH_B73D = 0xB73D, RH_B7CA = 0xB7CA, RH_B7E9 = 0xB7E9,
    RH_B7FC = 0xB7FC, RH_B868 = 0xB868, RH_B87E = 0xB87E, RH_B885 = 0xB885,
    RH_B893 = 0xB893, RH_B929 = 0xB929, RH_BA41 = 0xBA41, RH_BA7A = 0xBA7A,
    RH_BAC2 = 0xBAC2, RH_APPLY_MOVE = 0xBB23, RH_WALL = 0xBB6E, RH_FLOOR = 0xBBC1,
    RH_C023 = 0xC023, RH_C076 = 0xC076, RH_C3A7 = 0xC3A7, RH_CE1D = 0xCE1D,
    RH_CE73 = 0xCE73, RH_B67B = 0xB67B
};

enum {
    PAD_LEFT = 0x80, PAD_RIGHT = 0x20, PAD_DOWN = 0x10, PAD_JUMP = 0x02, PAD_ATTACK = 0x01,
    PAD_UP_B = 0x40, STATE_STAND = 0, STATE_WALKING = 1, STATE_CROUCH = 2, STATE_AIR = 3,
    STATE_FALL = 4, STATE_DYING = 9, SCRIPT_SLOT_PLAYER = 7, LIST_CHANNEL_PLAYER = 7,
    OBJECT_PLAYER = 0, OBJECT_WHIP = 3, SCRIPT_HIT = 8, HIT_KINDS = 0x07, LIFT_STEP = 3,
    LIST_KIND_2 = 2, ANIM_WALK = 0x00, ANIM_STAND = 0x01, ANIM_CROUCH = 0x0C, ANIM_FALL = 0x10,
    ANIM_KNOCK = 0x31, ANIM_WALK_BACK = 0x32, CROUCH_STEP = 3, STATE_FALL_34B8 = 4,
    FALL_SPEED_HI = 0x03, KNOCK_RISE_LO = 0xC0, KNOCK_RISE_HI = 0xFD, KNOCKED = 0x01,
    START_NEW_RUN = 0, START_HEALTH = 1, START_STAND = 2, START_FIELD94 = 0x10,
    FACING_RIGHT = 0, FACING_LEFT = 1, ANIM_AIR = 0x0A, ANIM_AIR_MOVING = 0x02, AIR_SAME_WAY = 0x80,
    SOUND_WHIP = 0x03, FLOOR_MARK_FROM = 0x10, FLOOR_MARKS = 0x24, FLOOR_MARK_ALWAYS = 0x20,
    FALL_PULL = 0x38, MARIA_FALL_PULL = 0x24, FALL_LIMIT = 7, DEATH_SUB_STEP = 5, SCRIPT_DEATH = 9, ANIM_LEDGE_CRASH_A = 0x2B,
    ANIM_LEDGE_CRASH_B = 0x2C, LEDGE_CRASH_STEP = 5, LEDGE_JUMP_LO = 0xC0, LEDGE_JUMP_HI = 0xFA, SOUND_HURT = 0x65, ANIM_CRASH_FIVE = 0x2A, ANIM_HURT_AIR = 0x23,
    CRASH_FIVE_TIMER = 0x60, ITEM_CRASH_FIVE = 5, HURT_TIMER = 0xFF, REACT_FILL_D = 0x0D,
    REACT_FILL_B = 0x0B, REACT_E = 0x0E, REACT_10 = 0x10, REACT_11 = 0x11, REACT_12 = 0x12,
    REACT_13 = 0x13, REACT_11_TIMER = 0x60, ITEM_SLOT_A = 5, ITEM_SLOT_B = 8, ITEM_SLOT_C = 11,
    LEAP_LIST = 0x60, LEAP_SCRIPT = 0x01, LEAP_SCRIPT_UNLESS_HELD = 0x0B, LEAP_HIT_KEEPS = 0x20,
    LEAP_RISE_LO = 0x80, LEAP_RISE_HI = 0xFE, ANIM_LEAP = 0x22, LEAP_CHANNEL = 6,
    OBJECT_FIRST_OTHER = 1, OBJECT_END = 0x10, CAMERA_PUSH_IGNORED = 0x18, PUSH_ON_FLOOR = 0x01,
    HIT_KIND_SPIKE = 6, SPIKE_HIT_TIMER = 0x10, SPIKE_COLUMNS = 3, TILE_SPIKE = 0x02, FLOOR_COLUMNS = 3, TILE_FLOOR_BITS = 0x81,
    DEATH_TIMER = 0x60, SOUND_DEATH = 0x60, SCRIPT_BLINK_START = 0, SCRIPT_BLINK_HIT = 1, HIT_KIND_IGNORED = 5,
    ONE_MORE_POINT = 1, SOUND_LIFE_LOST = 0x64, LIFE_LOST_SUB_STEP = 4, BLINK_PRESSES = 8, BELOW_THE_MAP = 0xFF,
    PAD_DIRECTIONS = 0xF0, HIT_TIMER_TAKEN = 0x30,
    ANIM_LOOK_DOWN = 0x47, TURN_PROGRESS = 3, CROUCH_PROGRESS = 6, WHIP_OUT = 1, ATTACK_CROUCH_FIELD = 6,
    ANIMATION_STOPPED = 0xFF, WAIT_RETURN_HIGH = 0xA3, FIRST_RIGHT_WHIP_STATE = 7, CRASH_FIRST_SLOT = 5,
    CRASH_OBJECT = 0x0C, BACKSTEP_FIELD = 0x32,
    CLIMB_PAST_EDGE = 8, CLIMB_LOW_BELOW = 0x0A, CLIMB_HIGH_BELOW = 0x1E, CLIMB_EDGE_BIT = 0x04, CLIMB_LOW_NEAR = 7,
    STATE_CLIMB_LOW_EDGE = 5, STATE_CLIMB_LOW_OPEN = 6, STATE_CLIMB_HIGH_EDGE = 7, STATE_CLIMB_HIGH_OPEN = 8,
    CLIMB_STEP = 2, ANIM_RISE = 0x0D, CROUCH_RISE_STEP = 4, ANIM_CROUCH_ATTACK = 6, ATTACK_STAND_FIELD = 3,
    ANIM_CROUCHED = 0x0E, CROUCH_ATTACK_RETURN_HIGH = 0xA8, CROUCH_END_RETURN_HIGH = 0xA9,
    INVINCIBLE_FLASH = 2, INVINCIBLE_FRAME_BITS = 0x03, INVINCIBLE_HIT_TIMER = 0x80, INVINCIBLE_START = 0x80,
    INVINCIBLE_ENDING = 0x10, SOUND_INVINCIBLE_START = 0x31, SOUND_INVINCIBLE_ENDING = 0x32, INVINCIBLE_MARKS = 6, WHIP_KEPT = 1, CRASH_FIRST_STATE = 5, HEALTH_FULL = 0x5C, START_LIVE = 0x01
};

/* Called before they are defined. */
typedef struct { uint8_t facing; uint8_t high; } FacingSpeed;
typedef struct { uint8_t facing; uint8_t step; uint8_t was; } Nudge;
uint8_t RichterClearKeepPlace(void);
uint8_t RichterAnimationSound(void);
void RichterHitReact(void);
void RichterStartHitScript(uint8_t script);
void RichterClearWhip(void);
uint8_t RichterStepIndex(void);
FacingSpeed RichterSpeedWalk(void);
FacingSpeed RichterSpeedJump(void);
FacingSpeed RichterSpeedBackstep(void);
void RichterStopMoving(void);
void RichterStopX(void);
void RichterPadOverridesOff(void);
Nudge RichterNudgeA(void);
Nudge RichterNudgeB(void);
Lift RichterJumpSpeed(void);
Lift RichterLiftThree(void);
void abi_RichterClearKeepPlace(void);
typedef struct { bool started; uint8_t held; AnimationStep animation; } WalkStart;
typedef struct { Lift lift; FacingSpeed speed; uint8_t marks; } Knock;
/* The stand's animation, and the animation the sound was started for. */
typedef struct { AnimationStep stand; uint8_t animation; } StandSound;
StandSound RichterStartNew(uint8_t x, uint8_t y);
StandSound RichterStartHealth(uint8_t x, uint8_t y);
StandSound RichterStartClear(uint8_t a, uint8_t x, uint8_t y);
StandSound RichterStartStand(void);
StandSound RichterStart(uint8_t y);
WalkStart RichterStartWalk(uint8_t facing);
AnimationStep RichterCrouch(void);
AnimationStep RichterToFall(void);
Knock RichterKnockBack(void);
AnimationStep RichterStand(void);
void abi_RichterStartNew(void);
void animation_sound_exit_shim(uint8_t animation);
static void stand_sound_exit_shim(StandSound started);
void abi_RichterStartHealth(void);
void abi_RichterStartClear(void);
void abi_RichterStartStand(void);
void abi_RichterStartWalk(void);
void abi_RichterCrouch(void);
void abi_RichterToFall(void);
void abi_RichterKnockBack(void);
void abi_RichterStand(void);
void abi_RichterAttack(void);
void abi_RichterItemCrashFive(void);
void abi_RichterToHurtAir(void);
void abi_RichterDyingStep(void);
void abi_RichterTurnOnPress(void);
void abi_RichterLedgeHurtA(void);
void abi_RichterLedgeHurtB(void);
void abi_RichterNudgeBJump(void);
typedef struct {
    Lift lift;
    bool moving;
    bool same_way;
    AnimationStep animation;
    FacingSpeed speed;
} AirStart;

typedef enum { FLOOR_TOO_HIGH, FLOOR_UNMARKED, FLOOR_MARKED, FLOOR_MARKED_DOWN, FLOOR_NOT_DOWN } FloorHow;

/* A look at the floor's marks: what it found, the value its exit shows, and the pad held. */
typedef struct { FloorHow how; uint8_t value; uint8_t held; } MarkedFloor;

/* An item crash's frame and the add that found its entry. */
typedef struct { Sum entry; uint8_t frame; } CrashFrame;

/* A hurt off a ledge: item 5's crash with its animation, or the nudge and the hurt jump. */
typedef struct { bool crash; AnimationStep animation; Nudge nudge; HurtAir hurt; } LedgeHurt;

/* A jump off a ledge: the nudge, and taking to the air. */
typedef struct { Nudge nudge; AirStart air; } LedgeJump;

LedgeHurt RichterLedgeHurtA(void);
LedgeHurt RichterLedgeHurtB(void);
LedgeJump RichterNudgeBJump(void);
void ledge_hurt_exit_shim(LedgeHurt hurt);
void air_exit_shim(AirStart air);
void ledge_jump_exit_shim(LedgeJump jump);
typedef enum { UP_WHIP_JUMPED, UP_WHIP_NO_LADDER, UP_WHIP_LADDER } UpWhipHow;
/* The whip's reach up a ledge: the pad, the jump off it, or the ladder and the nudge onto it. */
typedef struct {
    UpWhipHow how; uint8_t held; LedgeJump jump; LadderCheck ladder; Nudge nudge; AnimationStep fall;
} UpWhip;
UpWhip RichterUpWhipLeft(void);
UpWhip RichterUpWhipRight(void);
void up_whip_exit_shim(UpWhip whip);
void to_fall_exit_shim(AnimationStep fall);
static Nudge nudge_by_facing(uint16_t table);
static void abi_exit_nudge(Nudge nudge);
AirStart RichterToAir(void);
void abi_RichterToAir(void);
typedef struct { FacingSpeed speed; bool tested; uint8_t hit; } Leap;
typedef struct { uint8_t whip; } OthersCleared;
typedef struct { uint8_t state; uint8_t index; } CrashEnd;
Leap RichterLeapClearingOthers(void);
OthersCleared RichterClearOthersKeepWhip(void);
CrashEnd RichterEndItemCrash(void);
void abi_RichterLeapClearingOthers(void);
void abi_RichterClearOthersKeepWhip(void);
void abi_RichterEndItemCrash(void);
void step_index_exit_shim(uint8_t step);
void walk_start_exit_shim(WalkStart walk);
void crash_end_exit_shim(CrashEnd end);
void turn_exit_shim(TurnPress turn);
void item_crash_exit_shim(ItemCrash crash);
uint8_t RichterStepIndex(void);
/* A ledge taken hold of: the column, the reach and its last add, the state, the animation. */
typedef struct {
    uint8_t column; uint8_t index; Sum reach; uint8_t state; uint8_t was; bool animated; AnimationStep animation;
} ClimbStart;
ClimbStart RichterClimbStart(uint8_t column);
void climb_start_exit_shim(ClimbStart climb);
TileRead RichterReadBelow(uint8_t row, bool carry);
AnimationStep RichterSetAnimation(uint8_t animation);
typedef enum {
    STAND_FELL, STAND_JUMPED, STAND_CRASHED, STAND_ATTACKED, STAND_CLIMBED, STAND_CROUCHED, STAND_WALKED,
    STAND_STILL
} StandHow;
/* How far the standing step's checks went, in their order. */
enum {
    STAND_READ_COLUMNS = 1, STAND_FOOTING, STAND_JUMP, STAND_LANDING, STAND_ITEM_CRASH, STAND_ITEM_USE,
    STAND_LEDGE, STAND_UP, STAND_LOOK, STAND_WALK
};
/* Standing still: each check's answer, and the transition taken. */
typedef struct {
    StandHow how; uint8_t reached;
    ColumnsEnd columns; FootingCheck footing; bool jump; LandingLift landing; ItemCrash crash;
    ItemUse use; LedgeCheck ledge; bool up; uint8_t held; uint8_t right_held; uint8_t left_held; uint8_t look_flag; bool ticked;
    AnimationStep tick;
    AnimationStep animation; AirStart air; HurtAir hurt; AttackStart attack; WalkStart walk;
    ClimbStart climb;
} StandStep;
typedef enum {
    ATTACK_STAND_FELL, ATTACK_STAND_CROUCHED, ATTACK_STAND_WAITING, ATTACK_STAND_STOOD
} AttackStandHow;
/* Standing while attacking: each check, the animation's wait, and how it ended. */
typedef struct {
    AttackStandHow how;
    ColumnsEnd columns; FootingCheck footing; LandingLift landing;
    uint8_t progress; TurnPress turn; uint8_t mark; uint8_t whip; uint8_t late; uint8_t timer; bool up;
    AnimationStep tick; AnimationWaitEnd wait; AnimationStep stand;
} AttackStand;
typedef enum {
    CRASH_STAND_STOOD, CRASH_STAND_ENDED, CRASH_STAND_FELL, CRASH_STAND_JUMPED, CRASH_STAND_UP_WHIP,
    CRASH_STAND_SPAWNED, CRASH_STAND_NO_SLOT
} CrashStandHow;
/* Standing through the item crash: the turn, the timer, each check, and the slot spawned. */
typedef struct {
    CrashStandHow how;
    uint8_t facing; uint8_t held; uint8_t timer; uint8_t state; uint8_t bumped;
    ColumnsEnd columns; FootingCheck footing; bool jump;
    AnimationStep stand; CrashEnd end; AirStart air; UpWhip whip; uint8_t slot; uint8_t seen;
} CrashStand;
/* The stand's step and which handler ran. */
typedef struct {
    uint8_t step; uint16_t handler; StandStep still; AttackStand attack; CrashStand crash;
} StandState;
StandStep RichterStandStep0(void);
AttackStand RichterStandStep1(void);
CrashStand RichterStandStep2(void);
StandState RichterStateStand(void);
void stand_exit_shim(StandStep st);
void attack_stand_exit_shim(AttackStand st);
void crash_stand_exit_shim(CrashStand st);
void stand_state_exit_shim(StandState st);
AnimationStep RichterAnimationTick(void);
AnimationStep RichterAnimationRunning(void);
void animation_running_exit_shim(AnimationStep step);
typedef enum {
    WALK_FELL, WALK_JUMPED, WALK_CRASHED, WALK_ATTACKED, WALK_CLIMBED, WALK_CROUCHED, WALK_ON,
    WALK_BACKSTEP, WALK_STOOD
} WalkHow;
/* Walking: the floor, again a pixel lower, each check, the pad, and the transition taken. */
typedef struct {
    WalkHow how; uint8_t reached;
    ColumnsEnd columns; FloorResult floor; uint8_t stepped; ColumnsEnd columns_below;
    FloorResult floor_below; bool jump; LandingLift landing; ItemCrash crash; ItemUse use;
    LedgeCheck ledge; bool up; AnimationStep tick; uint8_t facing; uint8_t held;
    uint8_t jump_held; uint8_t back_facing;
    AnimationStep fall; AirStart air; HurtAir hurt; AttackStart attack; FacingSpeed speed;
    ClimbStart climb;
} WalkStep;
WalkStep RichterStateWalk(void);
void walk_exit_shim(WalkStep w);
FloorResult RichterFloorCheck(void);
/* A ground check a state's step made, and what it found. */
typedef enum {
    CHECK_COLUMNS, CHECK_FOOTING, CHECK_LANDING, CHECK_ANSWER, CHECK_CRASH, CHECK_USE, CHECK_COMPARE,
    CHECK_TURN, CHECK_RUNNING
} CheckKind;
typedef struct {
    CheckKind kind;
    ColumnsEnd columns; FootingCheck footing; LandingLift landing; bool answer; ItemCrash crash;
    ItemUse use; struct { uint8_t value, with; } compare; TurnPress turn; AnimationStep running;
} CheckRecord;
/* The checks of one step, in the order made. */
typedef struct { uint8_t count; CheckRecord step[10]; } CheckLog;
void check_log_replay_shim(const CheckLog *log);
typedef enum {
    CROUCH_FELL, CROUCH_FELL_ORIGINAL, CROUCH_JUMPED, CROUCH_CRASHED, CROUCH_ATTACKED, CROUCH_RISING,
    CROUCH_HELD, CROUCH_STOOD_TICK, CROUCH_WAITING, CROUCH_DOWN_AGAIN, CROUCH_STEP_BACK, CROUCH_STOOD
} CrouchHow;
/* A step of the crouch: its checks, how it ended, and what the transition started. */
typedef struct {
    CrouchHow how; CheckLog log; uint8_t bits; uint8_t return_high;
    AnimationStep animation; AirStart air; HurtAir hurt; AttackStart attack;
} CrouchStep;
/* The crouch's step and which handler ran. */
typedef struct { uint8_t step; uint16_t handler; CrouchStep run; } CrouchState;
CrouchState RichterStateCrouch(void);
CrouchStep RichterCrouchStep0(void);
CrouchStep RichterCrouchStep1(void);
CrouchStep RichterCrouchStep3(void);
CrouchStep RichterCrouchStep4(void);
AttackStart RichterCrouchAttack(void);
void crouch_exit_shim(CrouchStep c);
void crouch_state_exit_shim(CrouchState st);
static AttackStart attack_with_animation(uint8_t animation);
static void stand_handler_shim(uint8_t step, uint16_t handler);
static FacingSpeed speed_by_facing(uint16_t table);
static void abi_exit_speed(FacingSpeed set);
void abi_RichterAnimationSound(void);
void abi_RichterHitReact(void);
void abi_RichterStartHitScript(void);
void hit_script_exit_shim(uint8_t script);
void abi_RichterClearWhip(void);
void abi_RichterStepIndex(void);
void abi_RichterSpeedWalk(void);
void abi_RichterSpeedJump(void);
void abi_RichterSpeedBackstep(void);
void abi_RichterNudgeA(void);
void abi_RichterNudgeB(void);
void abi_RichterJumpSpeed(void);
void lift_exit_shim(Lift lift);
typedef enum { MOVE_PUSH_BLOCKED, MOVE_PUSH_NOT_PUSHED, MOVE_PUSH_LIFTED, MOVE_PUSH_MAY_LEAVE_TOP } MovePushHow;
/* After the move: the columns read, the first column hit, the push, and the lift it made. */
typedef struct {
    MovePushHow how; ColumnsEnd columns; uint8_t column; uint8_t hit; uint8_t push; ColumnLift lift;
} MovePush;
MovePush RichterApplyMoveAndPush(void);
MovePush MariaApplyMoveAndPush(void);
void abi_MariaApplyMoveAndPush(void);
typedef enum {
    INVINCIBLE_OFF, INVINCIBLE_OFF_FRAME, INVINCIBLE_ENDED, INVINCIBLE_RUNNING, INVINCIBLE_MARK
} InvincibleHow;
/* A frame of the invincibility: how it went, the A compared with $10, the sounds and the mark. */
typedef struct {
    InvincibleHow how; uint8_t value; uint8_t compared; bool first_sound; bool second_sound;
    CommandCall first; CommandCall second; uint8_t mark; uint16_t scripts;
} InvincibleTick;
InvincibleTick RichterInvincibleTick(uint8_t x, uint8_t y);
/* The death started: the sound command it ends with. */
typedef struct { CommandCall sound; } Death;
typedef enum {
    FRAME_DYING, FRAME_DIED, FRAME_LIFE_LOST, FRAME_BLINK_IDLE, FRAME_BLINK_COUNTING, FRAME_HANDLER
} StateFrameHow;
/*
 * A frame of the state: how it ended, and what each step read on the way, in the order the
 * steps ran, so the registers they left can be given to original code and to the caller.
 */
typedef struct {
    StateFrameHow how;
    bool entered_out_of_health;   /* entered at RichterOutOfHealth, not at the frame */
    uint8_t dying;                /* PlayerDying */
    DyingStep dying_step;
    Death death;
    uint8_t y_high;               /* OBJ_Y_HI as the bottom of the map was checked */
    InvincibleTick tick;
    uint8_t hit_timer, timer_left, blink_flags;
    bool timer_script;            /* the hit timer ran out with no blink: script 9 */
    bool hit_looked;
    uint8_t hit, hit_kind, leave_top;   /* OBJ_HIT_TAKEN twice, and PlayerMayLeaveTop for a hit of kind 5 */
    bool paid;
    Sum health;                   /* the hit's damage taken off the health */
    bool out_of_health;
    uint8_t one_more;             /* PlayerByte3BF7 */
    CommandCall sound;            /* the life lost's sound */
    bool taken;
    uint8_t blinking_hit;         /* PlayerByte3BE2 as the hit was taken */
    bool after_hit;
    uint8_t blinking, state, count, pressed, counted;
    uint8_t y_high_late, handler_state;
} StateFrame;
Death RichterDie(uint8_t x, uint8_t y);
Death RichterDeathSound(uint8_t x, uint8_t y);
StateFrame RichterStateFrame(uint8_t x, uint8_t y);
StateFrame RichterOutOfHealth(uint8_t x, uint8_t y);
void death_exit_shim(Death death);
void dying_exit_shim(DyingStep step);
void state_frame_exit_shim(StateFrame f);
void abi_RichterStateStand(void);
void abi_RichterStateWalk(void);
void abi_RichterStateCrouch(void);
uint8_t RichterReadPad(void);
void invincible_exit_shim(InvincibleTick tick);
void move_push_exit_shim(MovePush move);
void columns_exit_shim(ColumnsEnd end);
/* A spike under his feet: whether found, at which point, and the last read. */
typedef struct { bool found; uint8_t column; bool on_the_map; TileRead read; uint8_t state; } SpikeProbe;
typedef enum { PUSH_HIT_CAMERA, PUSH_HIT_NOT_PUSHED, PUSH_HIT_HELD, PUSH_HIT_NO_SPIKE, PUSH_HIT_SPIKE } PushHitHow;
/* A hit while pushed: how far it got, what it read on the way, and the spike probe. */
typedef struct { PushHitHow how; uint8_t camera; uint8_t push; uint8_t held; SpikeProbe spike; } PushHit;
SpikeProbe RichterSpikeBelow(void);
/* X and Y as the spike probe holds them when it asks the overlay to read. */
static void spike_entry_shim(uint8_t column, uint8_t state)
{
    cpu.x = column;
    cpu.y = state;
}
PushHit RichterHitWhilePushed(void);
void spike_exit_shim(SpikeProbe probe);
void push_hit_exit_shim(PushHit hit);
void abi_RichterHitWhilePushed(void);
void abi_RichterInvincibleTick(void);
/* A frame of Richter: the forced pad left, the push check, the state's frame, the sound, the hitbox. */
typedef struct {
    uint8_t forced_left; PushHit push; StateFrame state; bool sounded; uint8_t animation;
    uint8_t index; uint8_t last;
} Frame;
Frame RichterFrame(uint8_t x, uint8_t y);
void abi_RichterFrame(void);
void abi_RichterApplyMoveAndPush(void);
void abi_RichterStart(void);
void abi_ClearObject(void);
void abi_FadeAndStopSound(void);
void abi_SoundCommand(void);

static Alu animation(Alu r, uint8_t id)
{
    alu_load(&r, id);
    return call_with(RH_SET_ANIMATION, r);
}

/* The state and the step to go with it. */
static void set_state(uint8_t state, uint8_t step)
{
    wr8(OBJ_STATE, state);
    wr8(OBJ_SUBSTATE, step);
}

/* LDA, then the state and the step to go with it. */
static void set_state_alu(Alu *r, uint8_t state, uint8_t step)
{
    alu_put(r, OBJ_STATE, state);
    alu_put(r, OBJ_SUBSTATE, step);
}

/* Jump table of the overlay: each is one JMP. */

/*
 * @routine 01B9:000 RichterVecFrame
 * @run 6D:A000
 * WHAT IT DOES: Goes to RichterFrame.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as RichterFrame.
 */
void abi_RichterVecFrame(void)
{
    abi_RichterFrame();
}

/*
 * @routine 01B9:003 RichterVecStart
 * @run 6D:A003
 * WHAT IT DOES: Goes to RichterStart.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as RichterStart.
 */
void abi_RichterVecStart(void)
{
    abi_RichterStart();
}

/*
 * @routine 01B9:006 RichterVecApplyMove
 * @run 6D:A006
 * WHAT IT DOES: Goes to $BB23.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as that code.
 */
void abi_RichterVecApplyMove(void)
{
    call_original(RH_APPLY_MOVE);
}

/*
 * @routine 01B9:009 RichterVecC3A7
 * @run 6D:A009
 * WHAT IT DOES: Goes to $C3A7.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as that code.
 */
void abi_RichterVecC3A7(void)
{
    call_original(RH_C3A7);
}

/*
 * @routine 01B9:00C RichterVecC023
 * @run 6D:A00C
 * WHAT IT DOES: Goes to RichterLeapClearingOthers.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as RichterLeapClearingOthers.
 */
void abi_RichterVecC023(void)
{
    abi_RichterLeapClearingOthers();
}

/*
 * @routine 01B9:00F RichterVecC076
 * @run 6D:A00F
 * WHAT IT DOES: Goes to $C076.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as that code.
 */
void abi_RichterVecC076(void)
{
    call_original(RH_C076);
}

/*
 * @routine 01B9:012 RichterVecAnimationSound
 * @run 6D:A012
 * WHAT IT DOES: Goes to RichterAnimationSound.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as RichterAnimationSound.
 */
void abi_RichterVecAnimationSound(void)
{
    abi_RichterAnimationSound();
}

/*
 * @routine 01B9:015 RichterVecCE1D
 * @run 6D:A015
 * WHAT IT DOES: Goes to RichterClearOthersKeepWhip.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as RichterClearOthersKeepWhip.
 */
void abi_RichterVecCE1D(void)
{
    abi_RichterClearOthersKeepWhip();
}

/*
 * @routine 01B9:018 RichterVecSetAnimation
 * @run 6D:A018
 * WHAT IT DOES: Goes to $B67B.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as that code.
 */
void abi_RichterVecSetAnimation(void)
{
    call_original(RH_SET_ANIMATION);
}

/*
 * @routine 01B9:01B RichterVecSetIdle
 * @run 6D:A01B
 * WHAT IT DOES: Goes to $B669.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as that code.
 */
void abi_RichterVecSetIdle(void)
{
    call_original(RH_SET_IDLE);
}

/*
 * @routine 01B9:01E RichterVecCE73
 * @run 6D:A01E
 * WHAT IT DOES: Goes to $CE73.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as that code.
 */
void abi_RichterVecCE73(void)
{
    call_original(RH_CE73);
}

/*
 * @routine 01B9:021 RichterVecWall
 * @run 6D:A021
 * WHAT IT DOES: Goes to $BB6E, the wall probe.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as that code.
 */
void abi_RichterVecWall(void)
{
    call_original(RH_WALL);
}

/*
 * @routine 01B9:024 RichterVecFloor
 * @run 6D:A024
 * WHAT IT DOES: Goes to $BBC1, the floor probe.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as that code.
 */
void abi_RichterVecFloor(void)
{
    call_original(RH_FLOOR);
}

/*
 * @routine 01B9:02A RichterApplyMoveAndPush
 * @run 6D:A02A
 * WHAT IT DOES: Reads the five columns under him after his move and, if none of the first
 * three is solid or a floor, lets a push on the floor lift him by zFieldCA.
 * HOW: Bits $81 of RichterColumnHit; zFieldCA goes in as column 0's depth for
 * RichterLiftByColumn, then is cleared.
 * WAITS: no.
 * Entry: nothing. Exit: Y as RichterReadColumns left it. A column hit, A its bits, N and Z from
 * them, X the column, C and V from the read. Not pushed, A 0, X 3, Z and C set, N and V the
 * push's bits 7 and 6. Lifted, as RichterLiftByColumn, X 0.
 */
/* A character's overlay's move-and-push: the returns of its two calls and the copies it calls. */
typedef struct {
    uint16_t columns_return, lift_return;
    ColumnsEnd (*columns)(void);
    ColumnLift (*lift)(uint8_t column);
} MovePushLayout;

static const MovePushLayout richter_move_push = { 0xA02D, 0xA048, RichterReadColumns, RichterLiftByColumn };
static const MovePushLayout maria_move_push = { 0xA032, 0xA04D, MariaReadColumns, MariaLiftByColumn };

static MovePush apply_move_and_push(const MovePushLayout *layout)
{
    MovePush move = { MOVE_PUSH_BLOCKED, { 0 }, 0, 0, 0, { false, 0, 0 } };

    jsr_push(layout->columns_return);
    move.columns = layout->columns();
    jsr_pop();

    for (move.column = 0; move.column < FLOOR_COLUMNS; move.column++) {
        move.hit = (uint8_t)(rd8((uint16_t)(RichterColumnHit + move.column)) & TILE_FLOOR_BITS);
        if (move.hit != 0)
            return move;
    }
    move.push = rd8(zPlayerPush);
    move.how = MOVE_PUSH_NOT_PUSHED;
    if (!(move.push & PUSH_ON_FLOOR))
        return move;
    wr8(RichterColumnRow, rd8(zFieldCA));
    jsr_push(layout->lift_return);
    move.lift = layout->lift(0);
    jsr_pop();
    wr8(zFieldCA, 0);
    move.how = MOVE_PUSH_LIFTED;
    return move;
}

MovePush RichterApplyMoveAndPush(void)
{
    return apply_move_and_push(&richter_move_push);
}

void move_push_exit_shim(MovePush move)
{
    if (move.how == MOVE_PUSH_MAY_LEAVE_TOP) {
        cpu.a = move.hit;
        abi_nz(move.hit);
        return;
    }
    columns_exit_shim(move.columns);
    cpu.x = move.column;
    switch (move.how) {
    case MOVE_PUSH_BLOCKED:
        cpu.a = move.hit;
        abi_nz(move.hit);
        /* each column passed by was CPX #3 with X below 3: C clear */
        if (move.column != 0)
            abi_flag(P_C, false);
        break;
    case MOVE_PUSH_NOT_PUSHED:
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_N, (move.push & 0x80) != 0);
        abi_flag(P_V, (move.push & 0x40) != 0);
        abi_flag(P_C, true);
        break;
    case MOVE_PUSH_LIFTED:
        cpu.x = 0;
        column_lift_exit_shim(move.lift);
        break;
    }
}

void abi_RichterApplyMoveAndPush(void)
{
    move_push_exit_shim(RichterApplyMoveAndPush());
}

/*
 * @routine 01B9:027 RichterApplyMoveAndPushGoTo
 * @run 6D:A027
 * WHAT IT DOES: Goes to RichterApplyMoveAndPush.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as RichterApplyMoveAndPush.
 */
void abi_RichterApplyMoveAndPushGoTo(void)
{
    /* Maria's overlay holds the same jump here, to her copy at $A02A. */
    if (resident_at(0xA02A) == DISC(0x01DD, 0x02A)) {
        abi_MariaApplyMoveAndPush();
        return;
    }
    abi_RichterApplyMoveAndPush();
}

/*
 * @routine 01DD:02A MariaApplyMoveAndPush
 * @run 6D:A02A
 * WHAT IT DOES: RichterApplyMoveAndPush in Maria's overlay (01DD), unless she may go above the map
 * (PlayerMayLeaveTop not zero), when it does nothing: reads the five columns under her and, if none of the first
 * three is solid or a floor, lets a push on the floor lift her by zFieldCA.
 * HOW: A test of PlayerMayLeaveTop, then RichterApplyMoveAndPush's instructions calling MariaReadColumns and
 * MariaLiftByColumn.
 * WAITS: no.
 * Entry: nothing. Exit: may go above, A PlayerMayLeaveTop, N and Z from it; else as RichterApplyMoveAndPush.
 */
MovePush MariaApplyMoveAndPush(void)
{
    MovePush move = { MOVE_PUSH_MAY_LEAVE_TOP, { 0 }, 0, 0, 0, { false, 0, 0 } };

    move.hit = rd8(PlayerMayLeaveTop);
    if (move.hit != 0) {
        return move;
    }
    return apply_move_and_push(&maria_move_push);
}

void abi_MariaApplyMoveAndPush(void)
{
    move_push_exit_shim(MariaApplyMoveAndPush());
}

/*
 * @routine 01DD:006 MariaVecApplyMove
 * @run 6D:A006
 * WHAT IT DOES: Goes to MariaReadColumns, the step Maria's overlay makes after she has moved.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as MariaReadColumns.
 */
void abi_MariaVecApplyMove(void)
{
    columns_exit_shim(MariaReadColumns());
}

/*
 * @routine 01DD:021 MariaVecWall
 * @run 6D:A021
 * WHAT IT DOES: Goes to MariaWall, her overlay's wall probe.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as MariaWall.
 */
void abi_MariaVecWall(void)
{
    wall_exit_shim(MariaWall(cpu.y));
}

/*
 * @routine 01DD:024 MariaVecFloor
 * @run 6D:A024
 * WHAT IT DOES: Goes to MariaFloor, her overlay's floor probe.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as MariaFloor.
 */
void abi_MariaVecFloor(void)
{
    floor_exit_shim(MariaFloor());
}

/*
 * @routine 01B9:04B RichterFrame
 * @run 6D:A04B
 * WHAT IT DOES: One frame of Richter: the pad, forced while OBJ_3510 counts down; the
 * pad's overrides; a spike while pushed; the state's frame; the turn window and the item's
 * hearts; the animation's sound on a change; its hitbox.
 * HOW: The animation before and after are compared; the hitbox index comes from the table
 * by animation and is four bytes of AnimationHitbox.
 * WAITS: no.
 * Entry: nothing. Exit: A the last hitbox byte, N and Z from it, X the hitbox offset, C bit 6
 * of the hitbox index; Y and V as RichterAnimationSound left them on a change, else as the
 * state's frame did.
 */
Frame RichterFrame(uint8_t x, uint8_t y)
{
    Frame frame = { 0 };
    uint8_t before;
    uint8_t pressed;
    uint16_t hitbox;

    if (rd8(OBJ_3510) != 0) {
        uint8_t held = rd8(zPadHeld);
        uint8_t forced;

        wr8(zPadPrevious, held);
        forced = rd8(OBJ_3568);
        wr8(zPadHeld, forced);
        wr8(zPadPressed, forced);
        frame.forced_left = (uint8_t)(rd8(OBJ_3510) - 1);
        wr8(OBJ_3510, frame.forced_left);
    }
    if (frame.forced_left == 0) {
        wr8(PlayerByte3BF0, 0);
        jsr_push(0xA066);
        RichterReadPad();
        jsr_pop();
    }
    pressed = rd8(PadForcedPress);
    pressed |= rd8(zPadPressed);
    pressed |= rd8(PadForcedPressB);
    wr8(zPadPressed, pressed);
    before = rd8(OBJ_FRAME);
    push8(before);
    jsr_push(0xA077);
    RichterClearPadOverrides();
    jsr_pop();
    jsr_push(0xA07A);
    frame.push = RichterHitWhilePushed();
    jsr_pop();
    /* Original code runs inside the state's frame, from the registers the push check left. */
    push_hit_exit_shim(frame.push);
    jsr_push(0xA07D);
    frame.state = RichterStateFrame(x, y);
    jsr_pop();
    jsr_push(0xA080);
    RichterCheckTurnWindow();
    jsr_pop();
    jsr_push(0xA083);
    RichterCheckItemHearts();
    jsr_pop();
    before = pull8();
    frame.sounded = rd8(OBJ_FRAME) != before;
    if (frame.sounded) {
        jsr_push(0xA08C);
        frame.animation = RichterAnimationSound();
        jsr_pop();
    }
    frame.index = rd8((uint16_t)(AnimationHitboxIndex + rd8(OBJ_FRAME)));
    hitbox = (uint16_t)(AnimationHitbox + (uint8_t)(frame.index << 2));
    wr8(OBJ_HITBOX_DX, rd8(hitbox));
    wr8(OBJ_HITBOX_DY, rd8((uint16_t)(hitbox + 1)));
    wr8(OBJ_HITBOX_HALF_W, rd8((uint16_t)(hitbox + 2)));
    frame.last = rd8((uint16_t)(hitbox + 3));
    wr8(OBJ_HITBOX_HALF_H, frame.last);
    return frame;
}

void abi_RichterFrame(void)
{
    Frame frame = RichterFrame(cpu.x, cpu.y);

    state_frame_exit_shim(frame.state);
    if (frame.sounded)
        animation_sound_exit_shim(frame.animation);
    cpu.a = frame.last;
    cpu.x = (uint8_t)(frame.index << 2);
    abi_nz(frame.last);
    abi_flag(P_C, (frame.index & 0x40) != 0);
}

/*
 * @routine 01B9:0AE RichterHitWhilePushed
 * @run 6D:A0AE
 * WHAT IT DOES: While pushed (bit 0 of zPlayerPush), outside camera mode $18, with no hit
 * timer, not above the map, a spike under the player is a hit of kind 6.
 * HOW: RichterSpikeBelow, then the hit's fields; the state back to standing.
 * WAITS: no.
 * Entry: nothing. Exit: camera mode $18, A $18, Z and C set. Not pushed, A the camera mode, Z
 * set, N and V the push's bits 7 and 6, C from comparing the mode with $18. Held, A OBJ_ANIM_TIMER
 * or PlayerMayLeaveTop, N and Z from it, C as before, V the push's bit 6. No spike, as
 * RichterSpikeBelow. A spike, as RichterSpikeBelow found it, but A $10 with N and Z from it.
 */
PushHit RichterHitWhilePushed(void)
{
    PushHit hit = { PUSH_HIT_CAMERA, 0, 0, 0, { false, 0, false, { TILE_EMPTY_PAGE, 0, 0, 0, 0, 0 } } };

    hit.camera = rd8(CameraMode);
    if (hit.camera == CAMERA_PUSH_IGNORED)
        return hit;
    hit.push = rd8(zPlayerPush);
    hit.how = PUSH_HIT_NOT_PUSHED;
    if (!(hit.push & PUSH_ON_FLOOR))
        return hit;
    hit.held = (uint8_t)(rd8(OBJ_ANIM_TIMER) | rd8(PlayerMayLeaveTop));
    hit.how = PUSH_HIT_HELD;
    if (hit.held != 0)
        return hit;
    jsr_push(0xA0C5);
    hit.spike = RichterSpikeBelow();
    jsr_pop();
    hit.how = PUSH_HIT_NO_SPIKE;
    if (!hit.spike.found)
        return hit;
    wr8(OBJ_HIT_TAKEN, HIT_KIND_SPIKE);
    wr8(OBJ_HEALTH, SPIKE_HIT_TIMER);
    wr8(OBJ_STATE, STATE_STAND);
    hit.how = PUSH_HIT_SPIKE;
    return hit;
}

void push_hit_exit_shim(PushHit hit)
{
    switch (hit.how) {
    case PUSH_HIT_CAMERA:
        cpu.a = CAMERA_PUSH_IGNORED;
        abi_nz(0);
        abi_flag(P_C, true);
        break;
    case PUSH_HIT_NOT_PUSHED:
        cpu.a = hit.camera;
        abi_nz(0);
        abi_flag(P_N, (hit.push & 0x80) != 0);
        abi_flag(P_V, (hit.push & 0x40) != 0);
        abi_flag(P_C, hit.camera >= CAMERA_PUSH_IGNORED);
        break;
    case PUSH_HIT_HELD:
        cpu.a = hit.held;
        abi_nz(hit.held);
        abi_flag(P_V, (hit.push & 0x40) != 0);
        abi_flag(P_C, hit.camera >= CAMERA_PUSH_IGNORED);
        break;
    case PUSH_HIT_NO_SPIKE:
        spike_exit_shim(hit.spike);
        break;
    case PUSH_HIT_SPIKE:
        spike_exit_shim(hit.spike);
        cpu.a = SPIKE_HIT_TIMER;
        abi_nz(SPIKE_HIT_TIMER);
        break;
    }
}

void abi_RichterHitWhilePushed(void)
{
    push_hit_exit_shim(RichterHitWhilePushed());
}

/*
 * @routine 01B9:0D5 RichterSpikeBelow
 * @run 6D:A0D5
 * WHAT IT DOES: Probes three points at the player's feet for a tile with bit 1 set.
 * HOW: Each point's X from RichterColumnOffset pulled one pixel in, Y from the table by
 * state; the terrain from RichterReadAbove.
 * WAITS: no.
 * Entry: nothing. Exit: Y, C and V as the last read left them (Y the state when the row was
 * off the map). Found, A 2, N and Z from it, X the point. Else A 0, X 3, Z and C set, N clear.
 */
SpikeProbe RichterSpikeBelow(void)
{
    SpikeProbe probe = { false, 0, false, { TILE_EMPTY_PAGE, 0, 0, 0, 0, 0 }, 0 };
    ColumnOffset offset;
    uint8_t across, base, state;
    Sum column, row;

    for (probe.column = 0; probe.column < SPIKE_COLUMNS; probe.column++) {
        jsr_push(0xA0D9);
        offset = RichterColumnOffset(probe.column);
        jsr_pop();
        across = offset.value;
        /* One pixel in toward him, whichever side the column lies. */
        if (across != 0) {
            across = (across & 0x80) ? (uint8_t)(across + 1) : (uint8_t)(across - 1);
        }
        base = RD8(OBJ->x_lo[0]);
        column = add_bytes(across, base, false);
        WR8(ZP->temp00, column.value);
        base = RD8(OBJ->y_lo[0]);
        state = RD8(OBJ->state[0]);
        row = add_bytes(base, rd8((uint16_t)(PlayerFootYByState + state)), false);
        probe.on_the_map = row.carry;
        probe.state = state;
        spike_entry_shim(probe.column, state);
        jsr_push(0xA0F4);
        probe.read = RichterReadAbove(row.value, row.carry);
        jsr_pop();
        if (tile_answer(probe.read) & TILE_SPIKE) {
            probe.found = true;
            return probe;
        }
    }
    return probe;
}

void spike_exit_shim(SpikeProbe probe)
{
    if (!(probe.on_the_map && probe.read.kind == TILE_BY_ORIGINAL)) {
        cpu.y = probe.state;
    }
    read_above_exit_shim(probe.read, probe.on_the_map);
    cpu.x = probe.column;
    if (probe.found) {
        cpu.a = TILE_SPIKE;
        abi_nz(TILE_SPIKE);
        return;
    }
    cpu.a = 0;
    abi_nz(0);
    abi_flag(P_C, true);
}

void abi_RichterSpikeBelow(void)
{
    spike_exit_shim(RichterSpikeBelow());
}

/*
 * @routine 01B9:10F RichterStart
 * @run 6D:A10F
 * WHAT IT DOES: Starts Richter as zField8A says.
 * HOW: The table of four entries: a new run, the health, the stand, the clear; each goes on
 * into the next. zField8A is 0 to 3; the C takes anything past 2 as the clear, where the table
 * would send it elsewhere.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterAnimationSound, and C as RichterStand left it.
 */
StandSound RichterStart(uint8_t y)
{
    uint8_t how = RD8(ZP->field8a);
    uint8_t doubled = (uint8_t)(how << 1);
    uint16_t entry = (uint16_t)(PlayerStartHandlers + doubled);

    /* The jump reads its entry; each start runs on into the next, so all end standing. */
    rd8(entry);
    rd8((uint16_t)(entry + 1));
    switch (how) {
    case START_NEW_RUN:
        return RichterStartNew(doubled, y);
    case START_HEALTH:
        return RichterStartHealth(doubled, y);
    case START_STAND:
        return RichterStartStand();
    default:
        return RichterStartClear(doubled, doubled, y);
    }
}

void abi_RichterStart(void)
{
    stand_sound_exit_shim(RichterStart(cpu.y));
}

/* TAI pushes and pulls Y, A and X: the registers the block move meets are the routine's own at that point. */
static void clear_work_regs_shim(uint8_t a, uint8_t x, uint8_t y)
{
    cpu.a = a;
    cpu.x = x;
    cpu.y = y;
}

/*
 * @routine 01B9:11E RichterClearWork
 * @run 6D:A11E
 * WHAT IT DOES: Clears the player's work bytes and zero page.
 * HOW: Two block moves from the kernel's zeros.
 * WAITS: no.
 * Entry: nothing. Exit: all registers and flags kept.
 */
static void clear_work_body(uint8_t a, uint8_t x, uint8_t y)
{
    clear_work_regs_shim(a, x, y);
    tai(KernelZeros, 0x3BC0, 0x0040);
    tai(KernelZeros, ADDR_OF(ZP->pad_held), 0x0019);
}

void RichterClearWork(uint8_t a, uint8_t x, uint8_t y)
{
    clear_work_body(a, x, y);
}

void abi_RichterClearWork(void)
{
    RichterClearWork(cpu.a, cpu.x, cpu.y);
}

/*
 * @routine 01DD:16B MariaClearWork
 * @run 6D:A16B
 * WHAT IT DOES: RichterClearWork in Maria's player overlay (01DD), the same instructions at her address.
 * HOW: RichterClearWork's body.
 * WAITS: no.
 * Entry, exit: as RichterClearWork.
 */
void MariaClearWork(uint8_t a, uint8_t x, uint8_t y)
{
    clear_work_body(a, x, y);
}

void abi_MariaClearWork(void)
{
    MariaClearWork(cpu.a, cpu.x, cpu.y);
}

/*
 * @routine 01B9:12D RichterClearKeepPlace
 * @run 6D:A12D
 * WHAT IT DOES: Clears object 0, the player, keeping where it is and which way it faces.
 * HOW: The three bytes on the stack around ClearObject.
 * WAITS: no.
 * Entry: nothing. Exit: A the X position, N and Z from it, X 0. Y, C and V kept.
 */
static uint8_t clear_keep_place(uint16_t clear_ret)
{
    uint8_t value;

    value = RD8(OBJ->x_lo[0]);
    push8(value);
    value = RD8(OBJ->y_lo[0]);
    push8(value);
    value = RD8(OBJ->facing_left[0]);
    push8(value);
    jsr_push(clear_ret);
    ClearObject(OBJECT_PLAYER);
    jsr_pop();
    value = pull8();
    WR8(OBJ->facing_left[0], value);
    value = pull8();
    WR8(OBJ->y_lo[0], value);
    value = pull8();
    WR8(OBJ->x_lo[0], value);
    return value;
}

uint8_t RichterClearKeepPlace(void)
{
    return clear_keep_place(0xA13D);
}

void abi_RichterClearKeepPlace(void)
{
    uint8_t x = RichterClearKeepPlace();

    cpu.a = x;
    cpu.x = OBJECT_PLAYER;
    abi_nz(x);
}

/*
 * @routine 01DD:17A MariaClearKeepPlace
 * @run 6D:A17A
 * WHAT IT DOES: RichterClearKeepPlace in Maria's player overlay (01DD), the same bytes.
 * HOW: RichterClearKeepPlace's body; its call of ClearObject returns to $A18A.
 * WAITS: no.
 * Entry, exit: as RichterClearKeepPlace.
 */
uint8_t MariaClearKeepPlace(void)
{
    return clear_keep_place(0xA18A);
}

void abi_MariaClearKeepPlace(void)
{
    uint8_t x = MariaClearKeepPlace();

    cpu.a = x;
    cpu.x = OBJECT_PLAYER;
    abi_nz(x);
}

/*
 * @routine 01B9:14A RichterStartNew
 * @run 6D:A14A
 * WHAT IT DOES: Starts a new run: clears zField93 and sets zField94, then as
 * RichterStartHealth.
 * HOW: Runs on into it.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterAnimationSound, and C as RichterStand left it.
 */
StandSound RichterStartNew(uint8_t x, uint8_t y)
{
    WR8(ZP->field93, 0);
    WR8(ZP->field94, START_FIELD94);
    return RichterStartHealth(x, y);
}

void abi_RichterStartNew(void)
{
    stand_sound_exit_shim(RichterStartNew(cpu.x, cpu.y));
}

/*
 * @routine 01B9:150 RichterStartHealth
 * @run 6D:A150
 * WHAT IT DOES: Fills the health, then as RichterStartClear.
 * HOW: Runs on into it.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterAnimationSound, and C as RichterStand left it.
 */
StandSound RichterStartHealth(uint8_t x, uint8_t y)
{
    WR8(ZP->player_health, HEALTH_FULL);
    return RichterStartClear(HEALTH_FULL, x, y);
}

void abi_RichterStartHealth(void)
{
    stand_sound_exit_shim(RichterStartHealth(cpu.x, cpu.y));
}

/*
 * @routine 01B9:154 RichterStartClear
 * @run 6D:A154
 * WHAT IT DOES: Clears the player's work and object, marks it live, gives a frame of the
 * hit timer, then as RichterStartStand.
 * HOW: Runs on into it.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterAnimationSound, and C as RichterStand left it.
 */
StandSound RichterStartClear(uint8_t a, uint8_t x, uint8_t y)
{
    jsr_push(0xA157);
    RichterClearWork(a, x, y);
    jsr_pop();
    jsr_push(0xA15A);
    RichterClearKeepPlace();
    jsr_pop();
    WR8(OBJ->hitbox_on[0], START_LIVE);
    WR8(OBJ->f3828[0], START_LIVE);
    WR8(OBJ->anim_timer[0], START_LIVE);
    return RichterStartStand();
}

void abi_RichterStartClear(void)
{
    stand_sound_exit_shim(RichterStartClear(cpu.a, cpu.x, cpu.y));
}

/*
 * @routine 01B9:167 RichterStartStand
 * @run 6D:A167
 * WHAT IT DOES: Stands the player, and runs on into RichterAnimationSound.
 * HOW: PlayerByte3BD6 set, RichterStand.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterAnimationSound, and C as RichterStand left it.
 */
StandSound RichterStartStand(void)
{
    StandSound started;

    wr8(PlayerByte3BD6, 1);
    jsr_push(0xA16F);
    started.stand = RichterStand();
    jsr_pop();
    started.animation = RichterAnimationSound();
    return started;
}

void abi_RichterStartStand(void)
{
    stand_sound_exit_shim(RichterStartStand());
}

/*
 * @routine 01B9:16F RichterAnimationSound
 * @run 6D:A16F
 * WHAT IT DOES: Starts the list that goes with the player's animation on its channel.
 * HOW: The table by animation, then StartListChannelKind2 with X 7.
 * WAITS: no.
 * Entry: nothing. Exit: A 2, N 0, Z 0, X 7, Y the animation. C and V kept.
 */
uint8_t RichterAnimationSound(void)
{
    uint8_t animation = rd8(OBJ_FRAME);

    StartListChannelKind2(rd8((uint16_t)(AnimationSounds + animation)), LIST_CHANNEL_PLAYER);
    return animation;
}

void animation_sound_exit_shim(uint8_t animation)
{
    cpu.y = animation;
    cpu.x = LIST_CHANNEL_PLAYER;
    cpu.a = LIST_KIND_2;
    abi_nz(LIST_KIND_2);
}

/* As AnimationSound, with C from the stand's animation before it. */
static void stand_sound_exit_shim(StandSound started)
{
    animation_exit_shim(started.stand);
    animation_sound_exit_shim(started.animation);
}

void abi_RichterAnimationSound(void)
{
    animation_sound_exit_shim(RichterAnimationSound());
}

/*
 * @routine 01B9:17A RichterDyingStep
 * @run 6D:A17A
 * WHAT IT DOES: The death's wait: holds the player still, and when the timer ends clears
 * the objects from 1 up, stopping at one still live, and ends the mode.
 * HOW: OBJ_KIND not zero is live. zSubStep 5 ends it.
 * WAITS: no.
 * Entry: nothing. Exit: still dying, N and Z from the timer, A, X, Y, C and V kept. An object
 * still out, A its OBJ_KIND and X its slot, N and Z from it, C clear past slot 1. Over, as
 * RichterStartHitScript with script 9, and C set.
 */
DyingStep RichterDyingStep(void)
{
    DyingStep step = { DYING_WAITING, 0, 0 };

    jsr_push(0xA17D);
    RichterStopX();
    jsr_pop();
    wr8(OBJ_FRAME, 0);
    step.value = (uint8_t)(rd8(PlayerDyingTimer) - 1);
    wr8(PlayerDyingTimer, step.value);
    if (step.value != 0)
        return step;
    for (step.object = OBJECT_FIRST_OTHER; step.object < OBJECT_END; step.object++) {
        step.value = rd8((uint16_t)(OBJ_KIND + step.object));
        if (step.value != 0) {
            step.how = DYING_OBJECT_LEFT;
            return step;
        }
        jsr_push(0xA18F);
        ClearObject(step.object);
        jsr_pop();
    }
    wr8(zSubStep, DEATH_SUB_STEP);
    wr8(PlayerByte3BD7, 0);
    wr8(PlayerDying, 0);
    jsr_push(0xA1A3);
    RichterStartHitScript(SCRIPT_DEATH);
    jsr_pop();
    step.how = DYING_OVER;
    return step;
}

void dying_exit_shim(DyingStep step)
{
    switch (step.how) {
    case DYING_WAITING:
        abi_nz(step.value);
        break;
    case DYING_OBJECT_LEFT:
        cpu.a = step.value;
        cpu.x = step.object;
        abi_nz(step.value);
        if (step.object != OBJECT_FIRST_OTHER)
            abi_flag(P_C, false);
        break;
    case DYING_OVER:
        hit_script_exit_shim(SCRIPT_DEATH);
        abi_flag(P_C, true);
        break;
    }
}

void abi_RichterDyingStep(void)
{
    dying_exit_shim(RichterDyingStep());
}

/* FadeAndStopSound has no proper C yet: it runs through its wrapper, from the registers as they stand. */
static void fade_and_stop_sound_shim(void)
{
    abi_FadeAndStopSound();
}

/*
 * @routine 01B9:1A4 RichterDie
 * @run 6D:A1A4
 * WHAT IT DOES: Starts the player's death: no health, the timer, the state, the music out
 * and the death's sound.
 * HOW: FadeAndStopSound, then SoundCommand $60.
 * WAITS: no.
 * Entry: nothing. Exit: as SoundCommand with $60.
 */
Death RichterDie(uint8_t x, uint8_t y)
{
    Death death;
    uint8_t value;

    WR8(ZP->player_health, 0);
    value = rd8(PlayerDying);
    wr8(PlayerDying, (uint8_t)(value + 1));
    wr8(PlayerDyingTimer, DEATH_TIMER);
    jsr_push(0xA1B1);
    RichterStopX();
    jsr_pop();
    WR8(OBJ->state[0], STATE_DYING);
    return RichterDeathSound(x, y);
}

/*
 * @routine 01B9:1B6 RichterDeathSound
 * @run 6D:A1B6
 * WHAT IT DOES: Fades the music out and plays the death's sound.
 * HOW: FadeAndStopSound, then SoundCommand $60.
 * WAITS: no.
 * Entry: X, Y kept for the sound. Exit: as SoundCommand with $60.
 */
Death RichterDeathSound(uint8_t x, uint8_t y)
{
    Death death;

    jsr_push(0xA1B9);
    fade_and_stop_sound_shim();
    jsr_pop();
    death.sound = SoundCommand(SOUND_DEATH, x, y);
    return death;
}

void abi_RichterDeathSound(void)
{
    death_exit_shim(RichterDeathSound(cpu.x, cpu.y));
}

void death_exit_shim(Death death)
{
    cpu.a = SOUND_DEATH;
    abi_nz(SOUND_DEATH);
    sound_command_exit_shim(SOUND_DEATH, death.sound);
}

void abi_RichterDie(void)
{
    death_exit_shim(RichterDie(cpu.x, cpu.y));
}

/* Off the bottom by the low byte: its compare with $FF set C, which the fade's original code keeps. */
static void below_map_flags_shim(bool by_low_byte)
{
    if (by_low_byte)
        abi_flag(P_C, true);
}

/* Each step of the frame up to here, as it left the registers, for original code and the exit. */
static void state_frame_replay_shim(const StateFrame *f)
{
    if (!f->entered_out_of_health) {
        if (!(f->y_high & 0x80))
            abi_flag(P_C, false);
        invincible_exit_shim(f->tick);
        if (f->hit_timer != 0) {
            cpu.a = f->blink_flags;
            abi_nz(f->timer_left != 0 ? f->timer_left : f->blink_flags);
            if (f->timer_left != 0)
                cpu.a = f->hit_timer;
            if (f->timer_script)
                hit_script_exit_shim(SCRIPT_DEATH);
        } else {
            cpu.a = 0;
            abi_nz(0);
        }
        if (f->hit_looked) {
            cpu.a = f->hit;
            abi_nz(f->hit);
            if (f->hit != 0) {
                uint8_t kind = (uint8_t)(f->hit & HIT_KINDS);

                cpu.a = kind;
                abi_nz((uint8_t)(kind - HIT_KIND_IGNORED));
                abi_flag(P_C, kind >= HIT_KIND_IGNORED);
                if (kind == HIT_KIND_IGNORED) {
                    cpu.a = f->leave_top;
                    abi_nz(f->leave_top);
                }
            }
        }
        if (f->paid) {
            cpu.a = f->health.value;
            abi_nz(f->health.value);
            abi_flag(P_C, f->health.carry);
            abi_flag(P_V, f->health.overflow);
        }
    }
    if (f->out_of_health) {
        cpu.a = f->one_more;
        abi_nz((uint8_t)(f->one_more - 1));
        abi_flag(P_C, f->one_more >= 1);
        if (f->one_more == ONE_MORE_POINT)
            abi_nz(1);
        else
            cpu.a = SOUND_LIFE_LOST;
    }
    if (f->taken) {
        cpu.a = f->blinking_hit;
        abi_nz(f->blinking_hit);
        hit_script_exit_shim(f->blinking_hit != 0 ? SCRIPT_BLINK_HIT : SCRIPT_HIT);
    }
    if (!f->after_hit)
        return;
    cpu.a = f->blinking;
    abi_nz(f->blinking);
    if (f->blinking == 0)
        return;
    cpu.a = f->state;
    abi_nz((uint8_t)(f->state - STATE_DYING));
    abi_flag(P_C, f->state >= STATE_DYING);
    if (f->state == STATE_DYING)
        return;
    cpu.a = f->count;
    abi_nz(f->count);
    if (f->count == 0) {
        hit_script_exit_shim(SCRIPT_BLINK_START);
        abi_nz(1);
    }
    cpu.a = f->pressed;
    abi_nz(f->pressed);
    if (f->pressed == 0)
        return;
    cpu.a = f->counted;
    abi_nz((uint8_t)(f->counted - BLINK_PRESSES));
    abi_flag(P_C, f->counted >= BLINK_PRESSES);
    if (f->counted == BLINK_PRESSES)
        hit_script_exit_shim(SCRIPT_DEATH);
}

/* The state's handler, entered as the table's jump leaves the registers. */
static void state_handler_shim(const StateFrame *f, uint16_t handler)
{
    state_frame_replay_shim(f);
    cpu.a = f->y_high_late;
    abi_nz((uint8_t)(f->y_high_late - BELOW_THE_MAP));
    abi_flag(P_C, f->y_high_late == BELOW_THE_MAP);
    if (f->y_high_late == BELOW_THE_MAP) {
        cpu.a = (uint8_t)(rd8(zPadPressed));
        abi_nz(cpu.a);
    }
    cpu.a = (uint8_t)(f->handler_state << 1);
    cpu.x = cpu.a;
    abi_nz(cpu.a);
    abi_flag(P_C, (f->handler_state & 0x80) != 0);
    switch (handler) {
    case 0xA3D6:
        abi_RichterStateStand();
        break;
    case 0xA739:
        abi_RichterStateWalk();
        break;
    case 0xA85E:
        abi_RichterStateCrouch();
        break;
    default:
        call_original(handler);
        break;
    }
}

/* From the hit's aftermath on: the blink and its presses, a pad cut off above the map, the state. */
static void after_hit(StateFrame *f)
{
    uint16_t entry;

    f->after_hit = true;
    f->blinking = rd8(PlayerByte3BE2);
    if (f->blinking != 0) {
        f->state = rd8(OBJ_STATE);
        if (f->state != STATE_DYING) {
            f->count = rd8(PlayerByte3BE3);
            if (f->count == 0) {
                jsr_push(0xA249);
                RichterStartHitScript(SCRIPT_BLINK_START);
                jsr_pop();
                wr8(PlayerByte3BE3, (uint8_t)(rd8(PlayerByte3BE3) + 1));
            }
            f->pressed = rd8(zPadPressed);
            if (f->pressed == 0) {
                f->how = FRAME_BLINK_IDLE;
                return;
            }
            wr8(PlayerByte3BE3, (uint8_t)(rd8(PlayerByte3BE3) + 1));
            f->counted = rd8(PlayerByte3BE3);
            if (f->counted != BLINK_PRESSES) {
                f->how = FRAME_BLINK_COUNTING;
                return;
            }
            wr8(PlayerByte3BE2, 0);
            jsr_push(0xA263);
            RichterStartHitScript(SCRIPT_DEATH);
            jsr_pop();
        }
    }
    f->y_high_late = rd8(OBJ_Y_HI);
    if (f->y_high_late == BELOW_THE_MAP) {
        wr8(zPadHeld, (uint8_t)(rd8(zPadHeld) & PAD_DIRECTIONS));
        wr8(zPadPressed, (uint8_t)(rd8(zPadPressed) & PAD_DIRECTIONS));
    }
    f->handler_state = rd8(OBJ_STATE);
    entry = (uint16_t)(PlayerStateHandlers + (uint8_t)(f->handler_state << 1));
    f->how = FRAME_HANDLER;
    state_handler_shim(f, (uint16_t)(rd8(entry) | (rd8((uint16_t)(entry + 1)) << 8)));
}

/* A hit paid for in health: the hit timer, then the blink's own script or the reaction. */
static void hit_taken(StateFrame *f)
{
    wr8(OBJ_ANIM_TIMER, HIT_TIMER_TAKEN);
    f->taken = true;
    f->blinking_hit = rd8(PlayerByte3BE2);
    if (f->blinking_hit != 0) {
        jsr_push(0xA22C);
        RichterStartHitScript(SCRIPT_BLINK_HIT);
        jsr_pop();
        wr8(OBJ_HIT_TAKEN, 0);
    } else {
        jsr_push(0xA234);
        RichterHitReact();
        jsr_pop();
    }
    after_hit(f);
}

/* No health left: one point back with PlayerByte3BF7 at 1, or the life is lost. */
static void out_of_health(StateFrame *f, uint8_t x, uint8_t y)
{
    wr8(zPlayerHealth, 0);
    f->out_of_health = true;
    f->one_more = rd8(PlayerByte3BF7);
    if (f->one_more == ONE_MORE_POINT) {
        wr8(zPlayerHealth, (uint8_t)(rd8(zPlayerHealth) + 1));
        hit_taken(f);
        return;
    }
    wr8(zSubStep, LIFE_LOST_SUB_STEP);
    wr8(PlayerByte3BD7, 0);
    f->how = FRAME_LIFE_LOST;
    f->sound = SoundCommand(SOUND_LIFE_LOST, x, y);
}

/*
 * @routine 01B9:1BE RichterStateFrame
 * @run 6D:A1BE
 * WHAT IT DOES: The state's frame: the death; falling off the map's bottom is death; the
 * potion's invincibility; the hit timer; a hit in OBJ_HIT_TAKEN taken off the health; the blink a
 * hit can leave, which presses shake off; then the state's handler.
 * HOW: Y high 1 or $FF low is below the map. A hit of kind 5 is ignored unless
 * PlayerMayLeaveTop is set. Health gone goes to RichterOutOfHealth. Above the map the pad's
 * buttons are dropped. The handler from PlayerStateHandlers by OBJ_STATE.
 * WAITS: no.
 * Entry: nothing. Exit: dying, as RichterDyingStep. Falling off, as RichterDie. The life
 * lost, as SoundCommand with $64. Blinking with no press, A 0, Z set; with a press short of
 * eight, A the count, N, Z and C from comparing it with 8. Otherwise as the state's handler.
 * In each, the registers the earlier steps left where the last step keeps them.
 */
StateFrame RichterStateFrame(uint8_t x, uint8_t y)
{
    StateFrame f = { 0 };
    uint8_t health, damage;

    f.dying = rd8(PlayerDying);
    if (f.dying != 0) {
        f.how = FRAME_DYING;
        f.dying_step = RichterDyingStep();
        return f;
    }
    f.y_high = rd8(OBJ_Y_HI);
    if (!(f.y_high & 0x80) && (f.y_high == 1 || rd8(OBJ_Y_LO) == 0xFF)) {
        below_map_flags_shim(f.y_high != 1);
        f.how = FRAME_DIED;
        f.death = RichterDie(x, y);
        return f;
    }
    jsr_push(0xA1D5);
    f.tick = RichterInvincibleTick(x, y);
    jsr_pop();
    f.hit_timer = rd8(OBJ_ANIM_TIMER);
    if (f.hit_timer != 0) {
        wr8(OBJ_HIT_TAKEN, 0);
        f.timer_left = (uint8_t)(rd8(OBJ_ANIM_TIMER) - 1);
        wr8(OBJ_ANIM_TIMER, f.timer_left);
        if (f.timer_left != 0) {
            after_hit(&f);
            return f;
        }
        f.blink_flags = (uint8_t)(rd8(PlayerByte3BE2) | rd8(PlayerByte3BE8) | rd8(PlayerByte3BE9));
        if (f.blink_flags == 0) {
            f.timer_script = true;
            jsr_push(0xA1F2);
            RichterStartHitScript(SCRIPT_DEATH);
            jsr_pop();
        }
    }
    f.hit_looked = true;
    f.hit = rd8(OBJ_HIT_TAKEN);
    if (f.hit == 0) {
        after_hit(&f);
        return f;
    }
    f.hit_kind = rd8(OBJ_HIT_TAKEN);   /* LDA again, then AND #7 */
    if ((f.hit_kind & HIT_KINDS) == HIT_KIND_IGNORED) {
        f.leave_top = rd8(PlayerMayLeaveTop);
        if (f.leave_top == 0) {
            wr8(OBJ_HIT_TAKEN, 0);
            after_hit(&f);
            return f;
        }
    }
    f.paid = true;
    health = rd8(zPlayerHealth);
    damage = rd8(OBJ_HEALTH);
    f.health = sub_bytes(health, damage, true);
    wr8(zPlayerHealth, f.health.value);
    if (!f.health.carry || f.health.value == 0)
        out_of_health(&f, x, y);
    else
        hit_taken(&f);
    return f;
}

void state_frame_exit_shim(StateFrame f)
{
    switch (f.how) {
    case FRAME_DYING:
        cpu.a = f.dying;
        dying_exit_shim(f.dying_step);
        break;
    case FRAME_DIED:
        death_exit_shim(f.death);
        break;
    case FRAME_LIFE_LOST:
        state_frame_replay_shim(&f);
        sound_command_exit_shim(SOUND_LIFE_LOST, f.sound);
        break;
    case FRAME_BLINK_IDLE:
    case FRAME_BLINK_COUNTING:
        state_frame_replay_shim(&f);
        break;
    case FRAME_HANDLER:
        break;
    }
}

void abi_RichterStateFrame(void)
{
    state_frame_exit_shim(RichterStateFrame(cpu.x, cpu.y));
}

/*
 * @routine 01B9:298 RichterOutOfHealth
 * @run 6D:A298
 * WHAT IT DOES: The health has run out: with the one-more flag PlayerByte3BF7 at 1 the
 * player keeps one point and takes the hit; otherwise the mode's step 4 ends the life.
 * HOW: SoundCommand $64 on the way out; the hit goes on as RichterStateFrame's.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterStateFrame from the health on.
 */
StateFrame RichterOutOfHealth(uint8_t x, uint8_t y)
{
    StateFrame f = { 0 };

    f.entered_out_of_health = true;
    out_of_health(&f, x, y);
    return f;
}

void abi_RichterOutOfHealth(void)
{
    state_frame_exit_shim(RichterOutOfHealth(cpu.x, cpu.y));
}

/*
 * @routine 01B9:2B2 RichterHitReact
 * @run 6D:A2B2
 * WHAT IT DOES: Starts the reaction to a hit: the invulnerable frames by the hit's kind,
 * the state kept to come back to, the state set to 9, and the hit's script.
 * HOW: Runs on into RichterStartHitScript with 8.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y 8, X 7, N and Z from 8. C and V kept.
 */
void RichterHitReact(void)
{
    uint8_t kind;

    jsr_push(0xA2B5);
    RichterPadOverridesOff();
    jsr_pop();
    kind = (uint8_t)(rd8(OBJ_HIT_TAKEN) & HIT_KINDS);
    wr8(OBJ_ANIM_TIMER, rd8((uint16_t)(HitInvulnerableFrames + kind)));
    wr8(OBJ_35C0, rd8(OBJ_STATE));
    wr8(OBJ_3618, rd8(OBJ_SUBSTATE));
    wr8(OBJ_STATE, STATE_DYING);
    wr8(OBJ_SUBSTATE, 0);
    RichterStartHitScript(SCRIPT_HIT);
}

void abi_RichterHitReact(void)
{
    RichterHitReact();
    hit_script_exit_shim(SCRIPT_HIT);
}

/*
 * @routine 01B9:2D7 RichterStartHitScript
 * @run 6D:A2D7
 * WHAT IT DOES: Starts a script on the player's slot.
 * HOW: StartScriptSlotKind1 with X 7.
 * WAITS: no.
 * Entry: A script. Exit: A and Y the script, X 7, N and Z from the script. C and V kept.
 */
static void start_hit_script_body(uint8_t script)
{
    StartScriptSlotKind1(script, SCRIPT_SLOT_PLAYER);
}

void RichterStartHitScript(uint8_t script)
{
    start_hit_script_body(script);
}

void hit_script_exit_shim(uint8_t script)
{
    cpu.a = script;
    cpu.y = script;
    cpu.x = SCRIPT_SLOT_PLAYER;
    abi_nz(script);
}

void abi_RichterStartHitScript(void)
{
    uint8_t script = cpu.a;

    RichterStartHitScript(script);
    hit_script_exit_shim(script);
}

/*
 * @routine 01DD:359 MariaStartHitScript
 * @run 6D:A359
 * WHAT IT DOES: RichterStartHitScript in Maria's player overlay (01DD), the same instructions at her address.
 * HOW: RichterStartHitScript's body.
 * WAITS: no.
 * Entry, exit: as RichterStartHitScript.
 */
void MariaStartHitScript(uint8_t script)
{
    start_hit_script_body(script);
}

void abi_MariaStartHitScript(void)
{
    uint8_t script = cpu.a;

    MariaStartHitScript(script);
    hit_script_exit_shim(script);
}

/*
 * @routine 01B9:2E4 RichterTurnOnPress
 * @run 6D:A2E4
 * WHAT IT DOES: Turns the player to face the way just pressed.
 * HOW: OBJ_FACING_LEFT is the facing, 0 right, 1 left.
 * WAITS: no.
 * Entry: nothing. Exit: N and V bits 7 and 6 of the presses, Z set if it did not turn; A the
 * facing, or 1 and N and Z from it when it turned left. X, Y and C kept.
 */
TurnPress RichterTurnOnPress(void)
{
    TurnPress turn;

    turn.facing = rd8(OBJ_FACING_LEFT);
    turn.pressed = rd8(zPadPressed);
    turn.turned = (turn.pressed & (turn.facing == 0 ? PAD_LEFT : PAD_RIGHT)) != 0;
    if (turn.turned)
        wr8(OBJ_FACING_LEFT, turn.facing == 0 ? FACING_LEFT : FACING_RIGHT);
    return turn;
}

void turn_exit_shim(TurnPress turn)
{
    cpu.a = turn.facing;
    abi_flag(P_N, (turn.pressed & 0x80) != 0);
    abi_flag(P_V, (turn.pressed & 0x40) != 0);
    abi_flag(P_Z, !turn.turned);
    if (turn.turned && turn.facing == 0) {
        cpu.a = FACING_LEFT;
        abi_nz(FACING_LEFT);
    }
}

void abi_RichterTurnOnPress(void)
{
    turn_exit_shim(RichterTurnOnPress());
}

/* The registers each step of the attack's stand left, in order, up to the one it reached. */
static void attack_stand_replay_shim(const AttackStand *st)
{
    columns_exit_shim(st->columns);
    footing_exit_shim(st->footing);
    if (st->footing.no_floor)
        return;
    landing_exit_shim(st->landing);
    cpu.a = st->progress;
    abi_nz((uint8_t)(st->progress - TURN_PROGRESS));
    abi_flag(P_C, st->progress >= TURN_PROGRESS);
    if (st->progress < TURN_PROGRESS)
        turn_exit_shim(st->turn);
    cpu.a = st->mark;
    abi_nz(st->mark);
    if (st->mark != 0)
        return;
    cpu.a = st->whip;
    abi_nz((uint8_t)(st->whip - WHIP_OUT));
    abi_flag(P_C, st->whip >= WHIP_OUT);
    if (st->whip != WHIP_OUT)
        return;
    cpu.a = st->late;
    abi_nz((uint8_t)(st->late - CROUCH_PROGRESS));
    abi_flag(P_C, st->late >= CROUCH_PROGRESS);
    if (st->late >= CROUCH_PROGRESS) {
        cpu.a = st->timer;
        abi_nz((uint8_t)(st->timer - 4));
        abi_flag(P_C, st->timer >= 4);
    } else {
        abi_flag(P_C, st->up);
    }
}

/* $A9DD has no C yet: its original code, from the registers the checks left. */
static void attack_fall_shim(const AttackStand *st)
{
    attack_stand_replay_shim(st);
    call_original(RH_A9DD);
}

/*
 * @routine 01B9:2FD RichterStandStep1
 * @run 6D:A2FD
 * WHAT IT DOES: The standing state's attacking step: reads the ground, falls if nothing is
 * below; turns with a press while the attack is young; with the whip out and up held early,
 * crouches; otherwise waits for the animation's end, then stands.
 * HOW: OBJ_SCROLL_BAND counts the attack's progress; object 3 is the whip; PlayerByte3BD5
 * set skips to the wait. RichterAnimationWait returns past this routine while it runs.
 * WAITS: no.
 * Entry: nothing. Exit: falling, as $A9DD. Crouched, A 2, N and Z clear, as the animation's
 * tick left the rest. Waiting, as RichterAnimationRunning, A $A3 and N set, the return's high
 * byte. Stood, as RichterStand. In each, what the checks before it left.
 */
AttackStand RichterStandStep1(void)
{
    AttackStand st = { 0 };

    jsr_push(0xA300);
    st.columns = RichterReadColumns();
    jsr_pop();
    jsr_push(0xA303);
    st.footing = RichterHasNoFooting();
    jsr_pop();
    if (st.footing.no_floor) {
        st.how = ATTACK_STAND_FELL;
        attack_fall_shim(&st);
        return st;
    }
    jsr_push(0xA30B);
    st.landing = RichterLandOnFloor();
    jsr_pop();
    st.progress = rd8(OBJ_SCROLL_BAND);
    if (st.progress < TURN_PROGRESS) {
        jsr_push(0xA314);
        st.turn = RichterTurnOnPress();
        jsr_pop();
    }
    st.mark = rd8(PlayerByte3BD5);
    if (st.mark == 0) {
        st.whip = rd8((uint16_t)(OBJ_KIND + OBJECT_WHIP));
        if (st.whip == WHIP_OUT) {
            st.late = rd8(OBJ_SCROLL_BAND);
            if (st.late >= CROUCH_PROGRESS) {
                st.timer = rd8(OBJ_HURT_TIMER);
            } else {
                jsr_push(0xA32A);
                st.up = RichterUpHeld();
                jsr_pop();
                if (st.up) {
                    wr8(OBJ_FIELDS_2EE0, ATTACK_CROUCH_FIELD);
                    jsr_push(0xA33D);
                    st.tick = RichterAnimationTick();
                    jsr_pop();
                    wr8(OBJ_STATE, STATE_CROUCH);
                    st.how = ATTACK_STAND_CROUCHED;
                    return st;
                }
            }
        }
    }
    jsr_push(0xA32F);
    st.wait = RichterAnimationWait();
    if (st.wait.dropped) {
        /* the wait took this frame's return address off: the caller's own return is next */
        st.how = ATTACK_STAND_WAITING;
        return st;
    }
    jsr_pop();
    wr8(PlayerByte3BD5, 0);
    st.stand = RichterStand();
    st.how = ATTACK_STAND_STOOD;
    return st;
}

void attack_stand_exit_shim(AttackStand st)
{
    if (st.how == ATTACK_STAND_FELL)
        return;
    attack_stand_replay_shim(&st);
    switch (st.how) {
    case ATTACK_STAND_CROUCHED:
        animation_exit_shim(st.tick);
        cpu.a = STATE_CROUCH;
        abi_nz(STATE_CROUCH);
        break;
    case ATTACK_STAND_WAITING:
        animation_exit_shim(st.wait.step);
        break;
    default:
        animation_exit_shim(st.wait.step);
        animation_exit_shim(st.stand);
        break;
    }
}

void abi_RichterStandStep1(void)
{
    attack_stand_exit_shim(RichterStandStep1());
}

/*
 * @routine 01B9:34C RichterClearWhip
 * @run 6D:A34C
 * WHAT IT DOES: Clears object 3, the whip.
 * HOW: ClearObject with X 3.
 * WAITS: no.
 * Entry: nothing. Exit: X 3, N and Z from it. A, Y, C and V kept.
 */
void RichterClearWhip(void)
{
    ClearObject(OBJECT_WHIP);
}

void abi_RichterClearWhip(void)
{
    RichterClearWhip();
    cpu.x = OBJECT_WHIP;
    abi_nz(OBJECT_WHIP);
}

/*
 * @routine 01B9:351 RichterOrphanClearWhipAir
 * @run 6D:A351
 * WHAT IT DOES: Clears the whip and takes to the air. Nothing reached calls it.
 * HOW: RichterClearWhip, RichterToAir.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterToAir.
 */
void abi_RichterOrphanClearWhipAir(void)
{
    abi_RichterClearWhip();
    abi_alu(call_abi(abi_RichterToAir, alu_from_cpu()));
}

/* The registers each step of the item crash's stand left, in order, up to the one it reached. */
/* LDA OBJ_STATE and CMP #0. */
static void state_compare_shim(uint8_t state)
{
    cpu.a = state;
    abi_nz(state);
    abi_flag(P_C, true);
}

static void crash_stand_replay_shim(const CrashStand *st)
{
    cpu.a = st->facing;
    abi_nz(st->facing);
    abi_flag(P_N, (st->held & 0x80) != 0);
    abi_flag(P_V, (st->held & 0x40) != 0);
    abi_flag(P_Z, !(st->held & (st->facing == FACING_RIGHT ? PAD_LEFT : PAD_RIGHT)));
    if (st->facing == FACING_RIGHT && (st->held & PAD_LEFT)) {
        cpu.a = FACING_LEFT;
        abi_nz(FACING_LEFT);
    }
    cpu.a = st->timer;
    abi_nz(st->timer);
    if (st->timer == 0) {
        state_compare_shim(st->state);
        return;
    }
    state_compare_shim(st->state);
    if (st->state == STATE_STAND) {
        columns_exit_shim(st->columns);
        footing_exit_shim(st->footing);
        if (st->how == CRASH_STAND_FELL) {
            abi_nz(st->bumped);
            return;
        }
    }
    abi_flag(P_C, st->jump);
    if (!st->jump)
        return;
    state_compare_shim(st->state);
    if (st->state != STATE_STAND) {
        abi_nz((uint8_t)(st->state - FIRST_RIGHT_WHIP_STATE));
        abi_flag(P_C, st->state >= FIRST_RIGHT_WHIP_STATE);
    }
}

/*
 * @routine 01B9:358 RichterStandStep2
 * @run 6D:A358
 * WHAT IT DOES: The standing state's item-crash step: turns with the pad, and while its timer
 * runs reads the ground (in state 0) and falls if nothing is below, jumps on a press, or steps
 * the animation and spawns object $0C in the first free slot from 5. When the timer has run
 * out, stands, or ends the item crash outside state 0.
 * HOW: OBJ_LOOP_FLAGS is the timer. Ending, falling and jumping count PlayerByte3BD6 up. A jump in
 * the crash's states goes up the ledge by the first table below state 7, the second from it.
 * WAITS: no.
 * Entry: nothing. Exit: as the transition taken, after what the steps before it left. Spawned,
 * A $0C, Y the slot, N clear, Z clear. No free slot, A the last slot's OBJ_KIND, Y $10, Z and
 * C set, N clear.
 */
CrashStand RichterStandStep2(void)
{
    CrashStand st = { 0 };

    st.facing = rd8(OBJ_FACING_LEFT);
    st.held = rd8(zPadHeld);
    if (st.facing == FACING_RIGHT && (st.held & PAD_LEFT))
        wr8(OBJ_FACING_LEFT, FACING_LEFT);
    else if (st.facing != FACING_RIGHT && (st.held & PAD_RIGHT))
        wr8(OBJ_FACING_LEFT, FACING_RIGHT);
    st.timer = rd8(OBJ_LOOP_FLAGS);
    if (st.timer == 0) {
        st.bumped = (uint8_t)(rd8(PlayerByte3BD6) + 1);
        wr8(PlayerByte3BD6, st.bumped);
        st.state = rd8(OBJ_STATE);
        if (st.state == STATE_STAND) {
            st.how = CRASH_STAND_STOOD;
            st.stand = RichterStand();
        } else {
            st.how = CRASH_STAND_ENDED;
            st.end = RichterEndItemCrash();
        }
        return st;
    }
    wr8(OBJ_LOOP_FLAGS, (uint8_t)(rd8(OBJ_LOOP_FLAGS) - 1));
    st.state = rd8(OBJ_STATE);
    if (st.state == STATE_STAND) {
        jsr_push(0xA383);
        st.columns = RichterReadColumns();
        jsr_pop();
        jsr_push(0xA386);
        st.footing = RichterHasNoFooting();
        jsr_pop();
        if (st.footing.no_floor) {
            st.bumped = (uint8_t)(rd8(PlayerByte3BD6) + 1);
            wr8(PlayerByte3BD6, st.bumped);
            st.how = CRASH_STAND_FELL;
            st.stand = RichterToFall();
            return st;
        }
    }
    jsr_push(0xA38B);
    st.jump = RichterJumpPressed();
    jsr_pop();
    if (st.jump) {
        st.bumped = (uint8_t)(rd8(PlayerByte3BD6) + 1);
        wr8(PlayerByte3BD6, st.bumped);
        st.state = rd8(OBJ_STATE);
        if (st.state == STATE_STAND) {
            st.how = CRASH_STAND_JUMPED;
            st.air = RichterToAir();
        } else {
            st.how = CRASH_STAND_UP_WHIP;
            st.whip = st.state < FIRST_RIGHT_WHIP_STATE ? RichterUpWhipLeft() : RichterUpWhipRight();
        }
        return st;
    }
    jsr_push(0xA390);
    st.stand = RichterAnimationTick();
    jsr_pop();
    st.how = CRASH_STAND_NO_SLOT;
    for (st.slot = CRASH_FIRST_SLOT; st.slot < OBJECT_END; st.slot++) {
        st.seen = rd8((uint16_t)(OBJ_KIND + st.slot));
        if (st.seen == 0) {
            wr8((uint16_t)(OBJ_KIND + st.slot), CRASH_OBJECT);
            st.how = CRASH_STAND_SPAWNED;
            break;
        }
    }
    return st;
}

void crash_stand_exit_shim(CrashStand st)
{
    crash_stand_replay_shim(&st);
    switch (st.how) {
    case CRASH_STAND_STOOD:
        animation_exit_shim(st.stand);
        break;
    case CRASH_STAND_ENDED:
        crash_end_exit_shim(st.end);
        break;
    case CRASH_STAND_FELL:
        to_fall_exit_shim(st.stand);
        break;
    case CRASH_STAND_JUMPED:
        air_exit_shim(st.air);
        break;
    case CRASH_STAND_UP_WHIP:
        up_whip_exit_shim(st.whip);
        break;
    case CRASH_STAND_SPAWNED:
        animation_exit_shim(st.stand);
        cpu.y = st.slot;
        cpu.a = CRASH_OBJECT;
        abi_nz(CRASH_OBJECT);
        break;
    case CRASH_STAND_NO_SLOT:
        animation_exit_shim(st.stand);
        cpu.y = OBJECT_END;
        cpu.a = st.seen;
        abi_nz(0);
        abi_flag(P_C, true);
        break;
    }
}

void abi_RichterStandStep2(void)
{
    crash_stand_exit_shim(RichterStandStep2());
}

/*
 * @routine 01B9:3D0 RichterStepIndex
 * @run 6D:A3D0
 * WHAT IT DOES: Gives the state's step, for its caller to index a table of words by.
 * HOW: OBJ_SUBSTATE, which the wrapper leaves doubled in A and X.
 * WAITS: no.
 * Entry: nothing. Exit: A and X the offset, N and Z from it, C the step's bit 7. Y, V kept.
 */
static uint8_t step_index_body(void)
{
    return rd8(OBJ_SUBSTATE);
}

uint8_t RichterStepIndex(void)
{
    return step_index_body();
}

void step_index_exit_shim(uint8_t step)
{
    uint8_t offset = (uint8_t)(step << 1);

    cpu.a = offset;
    cpu.x = offset;
    abi_nz(offset);
    abi_flag(P_C, (step & 0x80) != 0);
}

void abi_RichterStepIndex(void)
{
    step_index_exit_shim(RichterStepIndex());
}

/*
 * @routine 01DD:5DC MariaStepIndex
 * @run 6D:A5DC
 * WHAT IT DOES: RichterStepIndex in Maria's player overlay (01DD), the same instructions at her address.
 * HOW: RichterStepIndex's body.
 * WAITS: no.
 * Entry, exit: as RichterStepIndex.
 */
uint8_t MariaStepIndex(void)
{
    return step_index_body();
}

void abi_MariaStepIndex(void)
{
    step_index_exit_shim(MariaStepIndex());
}

/* A handler of the stand with no C, entered as the table's jump leaves the registers. */
static void stand_handler_shim(uint8_t step, uint16_t handler)
{
    step_index_exit_shim(step);
    call_original(handler);
}

/*
 * @routine 01B9:3D6 RichterStateStand
 * @run 6D:A3D6
 * WHAT IT DOES: State 0, standing: runs the step's handler: still, attacking, or the item
 * crash.
 * HOW: PlayerStandHandlers, three words by RichterStepIndex.
 * WAITS: no.
 * Entry: nothing. Exit: as the handler leaves them.
 */
StandState RichterStateStand(void)
{
    StandState st = { 0 };
    uint16_t entry;

    jsr_push(0xA3D8);
    st.step = RichterStepIndex();
    jsr_pop();
    entry = (uint16_t)(PlayerStandHandlers + (uint8_t)(st.step << 1));
    st.handler = (uint16_t)(rd8(entry) | (rd8((uint16_t)(entry + 1)) << 8));
    /* Original code runs in the handlers, from the registers the index left. */
    step_index_exit_shim(st.step);
    switch (st.handler) {
    case 0xA3E1:
        st.still = RichterStandStep0();
        break;
    case 0xA2FD:
        st.attack = RichterStandStep1();
        break;
    case 0xA358:
        st.crash = RichterStandStep2();
        break;
    default:
        stand_handler_shim(st.step, st.handler);
        break;
    }
    return st;
}

void stand_state_exit_shim(StandState st)
{
    switch (st.handler) {
    case 0xA3E1:
        stand_exit_shim(st.still);
        break;
    case 0xA2FD:
        attack_stand_exit_shim(st.attack);
        break;
    case 0xA358:
        crash_stand_exit_shim(st.crash);
        break;
    default:
        break;
    }
}

void abi_RichterStateStand(void)
{
    stand_state_exit_shim(RichterStateStand());
}

/* The column a ledge was found at, which RichterLedgeCheck leaves in X. */
static uint8_t ledge_column(LedgeCheck ledge)
{
    return ledge.how == LEDGE_LOW ? (uint8_t)ledge.low.blocked_column : (uint8_t)ledge.high_column;
}

/* The registers each check of the standing step left, in order, up to the one it reached. */
static void stand_replay_shim(const StandStep *st)
{
    if (st->reached >= STAND_READ_COLUMNS)
        columns_exit_shim(st->columns);
    if (st->reached >= STAND_FOOTING)
        footing_exit_shim(st->footing);
    if (st->reached >= STAND_JUMP)
        abi_flag(P_C, st->jump);
    if (st->reached >= STAND_LANDING)
        landing_exit_shim(st->landing);
    if (st->reached >= STAND_ITEM_CRASH)
        item_crash_exit_shim(st->crash);
    if (st->reached >= STAND_ITEM_USE)
        item_use_exit_shim(st->use);
    if (st->reached >= STAND_LEDGE)
        ledge_check_exit_shim(st->ledge);
    if (st->reached >= STAND_UP)
        abi_flag(P_C, st->up);
    if (st->reached >= STAND_LOOK) {
        abi_flag(P_N, (st->held & 0x80) != 0);
        abi_flag(P_V, (st->held & 0x40) != 0);
        abi_flag(P_Z, !(st->held & PAD_DOWN));
        if (!(st->held & PAD_DOWN)) {
            cpu.a = st->look_flag;
            abi_nz(st->look_flag);
        }
        if (st->ticked) {
            animation_exit_shim(st->tick);
        } else {
            cpu.a = ANIM_LOOK_DOWN;
            abi_nz(ANIM_LOOK_DOWN);
        }
    }
    if (st->reached >= STAND_WALK) {
        cpu.y = FACING_RIGHT;
        abi_flag(P_N, (st->right_held & 0x80) != 0);
        abi_flag(P_V, (st->right_held & 0x40) != 0);
        abi_flag(P_Z, !(st->right_held & PAD_RIGHT));
        if (!(st->right_held & PAD_RIGHT)) {
            cpu.y = FACING_LEFT;
            abi_flag(P_N, (st->left_held & 0x80) != 0);
            abi_flag(P_V, (st->left_held & 0x40) != 0);
            abi_flag(P_Z, !(st->left_held & PAD_LEFT));
        }
    }
}

/*
 * @routine 01B9:3E1 RichterStandStep0
 * @run 6D:A3E1
 * WHAT IT DOES: Standing: reads the ground he moved onto, then the first thing that applies:
 * nothing below is a fall; a jump; the item crash; an item or the whip; a ledge to climb; up
 * crouches; down, or PlayerByte3BF0, looks down; and left or right start walking.
 * HOW: Each check leaves its answer in C. The look down is animation $47 set outright; otherwise
 * the animation steps.
 * WAITS: no.
 * Entry: nothing. Exit: as the transition taken, with what the checks before it left. Still,
 * Y 1, Z set, N and V the pad's bits 7 and 6, C clear, A as the look or the animation left it.
 */
StandStep RichterStandStep0(void)
{
    StandStep st = { 0 };

    jsr_push(0xA3E4);
    st.columns = RichterReadColumns();
    jsr_pop();
    st.reached = STAND_READ_COLUMNS;
    jsr_push(0xA3E7);
    st.footing = RichterHasNoFooting();
    jsr_pop();
    st.reached = STAND_FOOTING;
    if (st.footing.no_floor) {
        st.how = STAND_FELL;
        st.animation = RichterToFall();
        return st;
    }
    jsr_push(0xA3EF);
    st.jump = RichterJumpPressed();
    jsr_pop();
    st.reached = STAND_JUMP;
    if (st.jump) {
        st.how = STAND_JUMPED;
        st.air = RichterToAir();
        return st;
    }
    jsr_push(0xA3F7);
    st.landing = RichterLandOnFloor();
    jsr_pop();
    st.reached = STAND_LANDING;
    jsr_push(0xA3FA);
    st.crash = RichterTryItemCrash();
    jsr_pop();
    st.reached = STAND_ITEM_CRASH;
    if (st.crash.how == CRASH_STARTED) {
        st.how = STAND_CRASHED;
        st.hurt = RichterToHurtAir(0, 0);   /* X and Y of the calling state: passed when that state is re-lifted */
        return st;
    }
    jsr_push(0xA402);
    st.use = RichterUseItem();
    jsr_pop();
    st.reached = STAND_ITEM_USE;
    if (st.use.how != USE_NO_PRESS) {
        st.how = STAND_ATTACKED;
        st.attack = RichterAttack();
        return st;
    }
    jsr_push(0xA40A);
    st.ledge = RichterLedgeCheck();
    jsr_pop();
    st.reached = STAND_LEDGE;
    if (st.ledge.how == LEDGE_LOW || st.ledge.how == LEDGE_HIGH) {
        st.how = STAND_CLIMBED;
        st.climb = RichterClimbStart(ledge_column(st.ledge));
        return st;
    }
    jsr_push(0xA412);
    st.up = RichterUpHeld();
    jsr_pop();
    st.reached = STAND_UP;
    if (st.up) {
        st.how = STAND_CROUCHED;
        st.animation = RichterCrouch();
        return st;
    }
    st.held = rd8(zPadHeld);
    st.reached = STAND_LOOK;
    if (st.held & PAD_DOWN) {
        wr8(OBJ_FRAME, ANIM_LOOK_DOWN);
    } else {
        st.look_flag = rd8(PlayerByte3BF0);
        if (st.look_flag != 0) {
            wr8(OBJ_FRAME, ANIM_LOOK_DOWN);
        } else {
            st.ticked = true;
            jsr_push(0xA432);
            st.tick = RichterAnimationTick();
            jsr_pop();
        }
    }
    st.reached = STAND_WALK;
    /* TST #$20, then TST #$80: the pad is read again for each */
    st.right_held = rd8(zPadHeld);
    if (st.right_held & PAD_RIGHT) {
        st.how = STAND_WALKED;
        st.walk = RichterStartWalk(FACING_RIGHT);
        return st;
    }
    st.left_held = rd8(zPadHeld);
    if (st.left_held & PAD_LEFT) {
        st.how = STAND_WALKED;
        st.walk = RichterStartWalk(FACING_LEFT);
    } else {
        st.how = STAND_STILL;
    }
    return st;
}

void stand_exit_shim(StandStep st)
{
    stand_replay_shim(&st);
    switch (st.how) {
    case STAND_FELL:
        to_fall_exit_shim(st.animation);
        break;
    case STAND_JUMPED:
        air_exit_shim(st.air);
        break;
    case STAND_CRASHED:
        hurt_air_exit_shim(st.hurt);
        break;
    case STAND_ATTACKED:
        attack_exit_shim(st.attack);
        break;
    case STAND_CLIMBED:
        climb_start_exit_shim(st.climb);
        break;
    case STAND_CROUCHED:
        animation_exit_shim(st.animation);
        break;
    case STAND_WALKED:
        walk_start_exit_shim(st.walk);
        break;
    default:
        break;
    }
}

void abi_RichterStandStep0(void)
{
    stand_exit_shim(RichterStandStep0());
}

/*
 * @routine 01B9:442 RichterStartWalk
 * @run 6D:A442
 * WHAT IT DOES: Starts walking the way the pad says, unless a button stops it; a held jump
 * keeps the facing and walks backward.
 * HOW: Y is the new facing, kept unless bit 1 is held. Animation 0 or $32.
 * WAITS: no.
 * Entry: Y the facing. Exit: as RichterSetAnimation; with an attack held, A, X, Y and C kept,
 * N and V the pad's bits 7 and 6, Z clear.
 */
WalkStart RichterStartWalk(uint8_t facing)
{
    WalkStart walk = { false };

    jsr_push(0xA445);
    RichterPadOverridesOff();
    jsr_pop();
    if (!(rd8(zPadHeld) & PAD_JUMP))
        wr8(OBJ_FACING_LEFT, facing);
    walk.held = rd8(zPadHeld);
    if (walk.held & PAD_ATTACK)
        return walk;
    walk.started = true;
    set_state(STATE_WALKING, 0);
    walk.held = rd8(zPadHeld);
    walk.animation = RichterSetAnimation((walk.held & PAD_JUMP) ? ANIM_WALK_BACK : ANIM_WALK);
    return walk;
}

void walk_start_exit_shim(WalkStart walk)
{
    if (walk.started) {
        /* RichterSetAnimation keeps V: it is the last TST of the pad's */
        animation_exit_shim(walk.animation);
        abi_flag(P_V, (walk.held & 0x40) != 0);
        return;
    }
    /* TST of the attack bit, which is set: N and V from the pad's top bits, Z clear. */
    abi_flag(P_N, (walk.held & 0x80) != 0);
    abi_flag(P_V, (walk.held & 0x40) != 0);
    abi_flag(P_Z, false);
}

void abi_RichterStartWalk(void)
{
    walk_start_exit_shim(RichterStartWalk(cpu.y));
}

/* The X speed for the way the player faces, from a table of words by facing. */
static FacingSpeed speed_by_facing(uint16_t table)
{
    FacingSpeed set;
    uint8_t offset;

    set.facing = rd8(OBJ_FACING_LEFT);
    offset = (uint8_t)(set.facing << 1);
    wr8(OBJ_VX_LO, rd8((uint16_t)(table + offset)));
    set.high = rd8((uint16_t)(table + offset + 1));
    wr8(OBJ_VX_HI, set.high);
    return set;
}

/* A the speed's high byte, N and Z from it, Y twice the facing, C the facing's bit 7. */
static void abi_exit_speed(FacingSpeed set)
{
    cpu.a = set.high;
    cpu.y = (uint8_t)(set.facing << 1);
    abi_nz(set.high);
    abi_flag(P_C, (set.facing & 0x80) != 0);
}

/*
 * @routine 01B9:46C RichterSpeedWalk
 * @run 6D:A46C
 * WHAT IT DOES: Sets the X speed to walking, by facing.
 * HOW: WalkSpeeds.
 * WAITS: no.
 * Entry: nothing. Exit: A the speed's high byte, N and Z from it, Y twice the facing, C
 * the facing's bit 7. X and V kept.
 */
FacingSpeed RichterSpeedWalk(void)
{
    return speed_by_facing(WalkSpeeds);
}

void abi_RichterSpeedWalk(void)
{
    abi_exit_speed(RichterSpeedWalk());
}

/*
 * @routine 01B9:47E RichterSpeedJump
 * @run 6D:A47E
 * WHAT IT DOES: Sets the X speed for a jump, by facing.
 * HOW: JumpSpeeds.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterSpeedWalk.
 */
FacingSpeed RichterSpeedJump(void)
{
    return speed_by_facing(JumpSpeeds);
}

void abi_RichterSpeedJump(void)
{
    abi_exit_speed(RichterSpeedJump());
}

/*
 * @routine 01B9:494 RichterSpeedBackstep
 * @run 6D:A494
 * WHAT IT DOES: Sets the X speed for walking backward, by facing.
 * HOW: BackstepSpeeds.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterSpeedWalk.
 */
FacingSpeed RichterSpeedBackstep(void)
{
    return speed_by_facing(BackstepSpeeds);
}

void abi_RichterSpeedBackstep(void)
{
    abi_exit_speed(RichterSpeedBackstep());
}

/*
 * @routine 01B9:4AE RichterCrouch
 * @run 6D:A4AE
 * WHAT IT DOES: Crouches: state 2, step 3, animation $0C, all speed gone.
 * HOW: Runs on into RichterStopMoving.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterSetAnimation. X and V kept.
 */
AnimationStep RichterCrouch(void)
{
    AnimationStep animation;

    jsr_push(0xA4B1);
    RichterPadOverridesOff();
    jsr_pop();
    set_state(STATE_CROUCH, CROUCH_STEP);
    jsr_push(0xA4C0);
    animation = RichterSetAnimation(ANIM_CROUCH);
    jsr_pop();
    RichterStopMoving();
    return animation;
}

void abi_RichterCrouch(void)
{
    animation_exit_shim(RichterCrouch());
}

/*
 * @routine 01B9:4C0 RichterStopMoving
 * @run 6D:A4C0
 * WHAT IT DOES: Stops the player along Y and X.
 * HOW: Runs on into RichterStopX.
 * WAITS: no.
 * Entry: nothing. Exit: all registers and flags kept.
 */
void RichterStopMoving(void)
{
    wr8(OBJ_VY_LO, 0);
    wr8(OBJ_VY_HI, 0);
    RichterStopX();
}

void abi_RichterStopMoving(void)
{
    RichterStopMoving();
}

/*
 * @routine 01B9:4C6 RichterStopX
 * @run 6D:A4C6
 * WHAT IT DOES: Stops the player along X.
 * HOW: Zeroes the speed.
 * WAITS: no.
 * Entry: nothing. Exit: all registers and flags kept.
 */
void RichterStopX(void)
{
    wr8(OBJ_VX_LO, 0);
    wr8(OBJ_VX_HI, 0);
}

void abi_RichterStopX(void)
{
    RichterStopX();
}

/* The attack from its animation on: step 1, then the whip with its sound or the item's object. A jump target of CrouchAttack and what RichterAttack runs on into: not filed as a routine of its own. */
static AttackStart attack_with_animation(uint8_t animation)
{
    AttackStart attack;

    jsr_push(0xA4E7);
    attack.animation = RichterSetAnimation(animation);
    jsr_pop();
    wr8(OBJ_SUBSTATE, 1);
    attack.kind = rd8(PlayerByte3BD5);
    if (attack.kind == 0) {
        jsr_push(0xA4F4);
        RichterClearWhip();
        jsr_pop();
        wr8(OBJ_KIND + OBJECT_WHIP, 1);
        attack.sound = SoundCommand(SOUND_WHIP, OBJECT_WHIP, attack.animation.y);
        return attack;
    }
    attack.item = rd8(zField93);
    attack.tested_b = rd8(OBJ_KIND + ITEM_SLOT_A);
    attack.slot = ITEM_SLOT_A;
    if (attack.tested_b != 0) {
        attack.tested_b = rd8(OBJ_KIND + ITEM_SLOT_B);
        attack.slot = attack.tested_b == 0 ? ITEM_SLOT_B : ITEM_SLOT_C;
    }
    jsr_push(0xA50B);
    ClearObject(attack.slot);
    jsr_pop();
    wr8((uint16_t)(OBJ_KIND + attack.slot), attack.item);
    return attack;
}

void attack_exit_shim(AttackStart attack);

/*
 * @routine 01B9:4CD RichterAttack
 * @run 6D:A4CD
 * WHAT IT DOES: Starts an attack standing: state 0, step 1, the animation by the attack's
 * kind; the whip, with its sound, or the kind's object in a free slot among 5, 8 and $0B.
 * HOW: PlayerByte3BD5 is the kind. zField93 is what goes in the slot. SoundCommand 3 for the whip.
 * WAITS: no.
 * Entry: nothing. Exit: Y and C as RichterSetAnimation. The whip, as SoundCommand with command
 * 3 and X 3; an item, A the item, X its slot, N and Z clear (LDX), V from testing the last slot looked at.
 */
AttackStart RichterAttack(void)
{
    uint8_t kind;

    jsr_push(0xA4D0);
    RichterPadOverridesOff();
    jsr_pop();
    wr8(PlayerByte3BE1, 0);
    jsr_push(0xA4D6);
    RichterFaceByPad();
    jsr_pop();
    WR8(OBJ->state[0], STATE_STAND);
    jsr_push(0xA4DE);
    RichterStopMoving();
    jsr_pop();
    kind = rd8(PlayerByte3BD5);
    return attack_with_animation(rd8((uint16_t)(LandAnimations + kind)));
}

/* The whip's sound's exit after X 3; or the item in A and X its slot, N and Z from the LDX, V from the last test. */
void attack_exit_shim(AttackStart attack)
{
    animation_exit_shim(attack.animation);
    if (attack.kind == 0) {
        cpu.x = OBJECT_WHIP;
        cpu.a = SOUND_WHIP;
        sound_command_exit_shim(SOUND_WHIP, attack.sound);
        return;
    }
    cpu.a = attack.item;
    cpu.x = attack.slot;
    /* LDX of the slot, after the last TST, sets N and Z from X (a slot is never 0); V is the TST's */
    abi_flag(P_Z, false);
    abi_flag(P_N, false);
    abi_flag(P_V, (attack.tested_b & 0x40) != 0);
}

void abi_RichterAttack(void)
{
    attack_exit_shim(RichterAttack());
}

/*
 * @routine 01B9:521 RichterToFall
 * @run 6D:A521
 * WHAT IT DOES: Starts falling: state 4, animation $10, Y speed 3, no X speed.
 * HOW: Runs on into RichterPadOverridesOff.
 * WAITS: no.
 * Entry: nothing. Exit: A 4, N and Z from it; Y and C as RichterSetAnimation. X and V kept.
 */
AnimationStep RichterToFall(void)
{
    AnimationStep animation;

    set_state(STATE_FALL, 0);
    jsr_push(0xA530);
    animation = RichterSetAnimation(ANIM_FALL);
    jsr_pop();
    wr8(OBJ_VY_LO, 0);
    wr8(OBJ_VY_HI, FALL_SPEED_HI);
    wr8(OBJ_34B8, STATE_FALL_34B8);
    jsr_push(0xA542);
    RichterStopX();
    jsr_pop();
    RichterPadOverridesOff();
    return animation;
}

void to_fall_exit_shim(AnimationStep fall)
{
    animation_exit_shim(fall);
    cpu.a = STATE_FALL_34B8;
    abi_nz(STATE_FALL_34B8);
}

void abi_RichterToFall(void)
{
    to_fall_exit_shim(RichterToFall());
}

/*
 * @routine 01B9:542 RichterPadOverridesOff
 * @run 6D:A542
 * WHAT IT DOES: Clears the pad's overrides.
 * HOW: Three bytes zeroed.
 * WAITS: no.
 * Entry: nothing. Exit: all registers and flags kept.
 */
void RichterPadOverridesOff(void)
{
    wr8(PadForcedPress, 0);
    wr8(PadForcedPressB, 0);
    wr8(zFieldD3, 0);
}

void abi_RichterPadOverridesOff(void)
{
    RichterPadOverridesOff();
}

/* The item crash's start: animation, no speed, its step and timer. */
static Alu item_crash(Alu r, uint8_t id)
{
    r = animation(r, id);
    RichterStopMoving();
    alu_put(&r, OBJ_SUBSTATE, 5);
    alu_put(&r, OBJ_LOOP_FLAGS, 0x60);
    return r;
}

/*
 * @routine 01B9:54B RichterLedgeHurtA
 * @run 6D:A54B
 * WHAT IT DOES: A hurt jump off a ledge the first way, or item $05's crash.
 * HOW: Nudges off the ledge and to RichterToHurtAir, or animation $2B.
 * WAITS: no.
 * Entry: nothing. Exit: item 5, A $60, N and Z from it, Y and C as RichterSetAnimation. Otherwise
 * as RichterToHurtAir after RichterNudgeA.
 */
LedgeHurt RichterLedgeHurtA(void)
{
    LedgeHurt hurt;

    jsr_push(0xA54E);
    RichterPadOverridesOff();
    jsr_pop();
    hurt.crash = rd8(zField93) == ITEM_CRASH_FIVE;
    if (hurt.crash) {
        hurt.animation = RichterSetAnimation(ANIM_LEDGE_CRASH_A);
        RichterStopMoving();
        wr8(OBJ_SUBSTATE, LEDGE_CRASH_STEP);
        wr8(OBJ_LOOP_FLAGS, CRASH_FIVE_TIMER);
        return hurt;
    }
    jsr_push(0xA557);
    hurt.nudge = RichterNudgeA();
    jsr_pop();
    hurt.hurt = RichterToHurtAir(0, 0);   /* X and Y of the calling state: passed when that state is re-lifted */
    return hurt;
}

void abi_RichterLedgeHurtA(void)
{
    ledge_hurt_exit_shim(RichterLedgeHurtA());
}

/*
 * @routine 01B9:55A RichterLedgeHurtB
 * @run 6D:A55A
 * WHAT IT DOES: A hurt jump off a ledge the second way, or item $05's crash.
 * HOW: As RichterLedgeHurtA with the other nudge and animation $2C.
 * WAITS: no.
 * Entry: nothing. Exit: item 5, A $60, N and Z from it, Y and C as RichterSetAnimation. Otherwise
 * as RichterToHurtAir after RichterNudgeB.
 */
LedgeHurt RichterLedgeHurtB(void)
{
    LedgeHurt hurt;

    jsr_push(0xA55D);
    RichterPadOverridesOff();
    jsr_pop();
    hurt.crash = rd8(zField93) == ITEM_CRASH_FIVE;
    if (hurt.crash) {
        jsr_push(0xA56E);
        hurt.animation = RichterSetAnimation(ANIM_LEDGE_CRASH_B);
        jsr_pop();
        jsr_push(0xA571);
        RichterStopMoving();
        jsr_pop();
        wr8(OBJ_SUBSTATE, LEDGE_CRASH_STEP);
        wr8(OBJ_LOOP_FLAGS, CRASH_FIVE_TIMER);
        return hurt;
    }
    jsr_push(0xA566);
    hurt.nudge = RichterNudgeB();
    jsr_pop();
    hurt.hurt = RichterToHurtAir(0, 0);   /* X and Y of the calling state: passed when that state is re-lifted */
    return hurt;
}

void abi_RichterLedgeHurtB(void)
{
    ledge_hurt_exit_shim(RichterLedgeHurtB());
}

/* Moves the player along X by the table's entry for its facing. */
static Nudge nudge_by_facing(uint16_t table)
{
    Nudge nudge;

    nudge.facing = rd8(OBJ_FACING_LEFT);
    nudge.step = rd8((uint16_t)(table + nudge.facing));
    nudge.was = rd8(OBJ_X_LO);
    wr8(OBJ_X_LO, (uint8_t)(nudge.step + nudge.was));
    wr8(PlayerMayLeaveTop, 0);
    return nudge;
}

/* A the new position, N, Z, C and V from adding the step, X the facing. */
static void abi_exit_nudge(Nudge nudge)
{
    uint8_t moved = (uint8_t)(nudge.step + nudge.was);

    cpu.a = moved;
    cpu.x = nudge.facing;
    abi_nz(moved);
    abi_flag(P_C, nudge.step + nudge.was > 0xFF);
    abi_flag(P_V, (~(nudge.step ^ nudge.was) & (nudge.step ^ moved) & 0x80) != 0);
}

/*
 * @routine 01B9:5AB RichterNudgeA
 * @run 6D:A5AB
 * WHAT IT DOES: Moves the player off a ledge by the first table, by facing, and clears
 * PlayerMayLeaveTop.
 * HOW: One add to the X position.
 * WAITS: no.
 * Entry: nothing. Exit: A the position, N, Z, C, V from the add, X the facing. Y kept.
 */
Nudge RichterNudgeA(void)
{
    return nudge_by_facing(LedgeNudge);
}

void abi_RichterNudgeA(void)
{
    abi_exit_nudge(RichterNudgeA());
}

/*
 * @routine 01B9:5BC RichterNudgeB
 * @run 6D:A5BC
 * WHAT IT DOES: As RichterNudgeA by the second table.
 * HOW: Goes back into it.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterNudgeA.
 */
Nudge RichterNudgeB(void)
{
    return nudge_by_facing(LedgeNudgeB);
}

void abi_RichterNudgeB(void)
{
    abi_exit_nudge(RichterNudgeB());
}

/* After a nudge off the ledge: into the air, rising at $FAC0. */
static LedgeJump jump_off_ledge(Nudge nudge)
{
    LedgeJump jump;

    jump.nudge = nudge;
    jsr_push(0xA5A0);
    jump.air = RichterToAir();
    jsr_pop();
    wr8(OBJ_VY_LO, LEDGE_JUMP_LO);
    wr8(OBJ_VY_HI, LEDGE_JUMP_HI);
    return jump;
}

/*
 * The whip's reach up a ledge, by the table of nudges for the side: without up held, off the
 * ledge and up into the air; with up held at a ladder, onto it and falling into its grip.
 */
static UpWhip up_whip(uint16_t nudges, bool second)
{
    UpWhip whip = { 0 };

    jsr_push(second ? 0xA5CB : 0xA583);
    RichterClearPadOverrides();
    jsr_pop();
    wr8(PadForcedPressB, 0);
    wr8(zFieldC6, 0);
    whip.held = rd8(zPadHeld);
    if (!(whip.held & PAD_UP_B)) {
        whip.how = UP_WHIP_JUMPED;
        if (second) {
            whip.jump = RichterNudgeBJump();
        } else {
            Nudge nudge;

            jsr_push(0xA59D);
            nudge = RichterNudgeA();
            jsr_pop();
            whip.jump = jump_off_ledge(nudge);
        }
        return whip;
    }
    jsr_push(second ? 0xA5D8 : 0xA590);
    whip.ladder = RichterLadderCheck();
    jsr_pop();
    if (!ladder_found(whip.ladder)) {
        whip.how = UP_WHIP_NO_LADDER;
        return whip;
    }
    whip.how = UP_WHIP_LADDER;
    whip.nudge = nudge_by_facing(nudges);
    whip.fall = RichterToFall();
    return whip;
}

void up_whip_exit_shim(UpWhip whip)
{
    abi_flag(P_N, (whip.held & 0x80) != 0);
    abi_flag(P_V, (whip.held & 0x40) != 0);
    abi_flag(P_Z, !(whip.held & PAD_UP_B));
    switch (whip.how) {
    case UP_WHIP_JUMPED:
        ledge_jump_exit_shim(whip.jump);
        break;
    case UP_WHIP_NO_LADDER:
        ladder_exit_shim(whip.ladder);
        break;
    case UP_WHIP_LADDER:
        ladder_exit_shim(whip.ladder);
        abi_exit_nudge(whip.nudge);
        to_fall_exit_shim(whip.fall);
        break;
    }
}

/*
 * @routine 01B9:580 RichterUpWhipLeft
 * @run 6D:A580
 * WHAT IT DOES: Off a ledge by the first table of nudges: without up held, up into the air at
 * $FAC0; with up held at a ladder, onto it, falling.
 * HOW: The pad's overrides, PadForcedPressB and zFieldC6 cleared; bit 6 of zPadHeld;
 * RichterLadderCheck; LedgeNudge by facing.
 * WAITS: no.
 * Entry: nothing. Exit: the jump, as RichterNudgeBJump with the first nudge. No ladder, as
 * RichterLadderCheck. A ladder, as RichterToFall.
 */
UpWhip RichterUpWhipLeft(void)
{
    return up_whip(LedgeNudge, false);
}

void abi_RichterUpWhipLeft(void)
{
    up_whip_exit_shim(RichterUpWhipLeft());
}

/*
 * @routine 01B9:5C4 RichterNudgeBJump
 * @run 6D:A5C4
 * WHAT IT DOES: The second nudge, and the jump up off the ledge.
 * HOW: RichterNudgeB, then as RichterUpWhipLeft's jump.
 * WAITS: no.
 * Entry: nothing. Exit: A $FA, N set, Z clear; C, V and Y as RichterToAir after the nudge.
 */
LedgeJump RichterNudgeBJump(void)
{
    Nudge nudge;

    jsr_push(0xA5C6);
    nudge = RichterNudgeB();
    jsr_pop();
    return jump_off_ledge(nudge);
}

void ledge_jump_exit_shim(LedgeJump jump)
{
    abi_exit_nudge(jump.nudge);
    air_exit_shim(jump.air);
    cpu.a = LEDGE_JUMP_HI;
    abi_nz(LEDGE_JUMP_HI);
}

void abi_RichterNudgeBJump(void)
{
    ledge_jump_exit_shim(RichterNudgeBJump());
}

/*
 * @routine 01B9:5C8 RichterUpWhipRight
 * @run 6D:A5C8
 * WHAT IT DOES: As RichterUpWhipLeft by the second table of nudges.
 * HOW: The same tests; the jump is RichterNudgeBJump.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterUpWhipLeft.
 */
UpWhip RichterUpWhipRight(void)
{
    return up_whip(LedgeNudgeB, true);
}

void abi_RichterUpWhipRight(void)
{
    up_whip_exit_shim(RichterUpWhipRight());
}

/* What each hit reaction spawns. */
/* The slots from 5 filled with one object. */
static void react_fill_slots(uint8_t kind)
{
    for (uint8_t slot = ITEM_SLOT_A; slot < OBJECT_END; slot++)
        wr8((uint16_t)(OBJ_KIND + slot), kind);
}

/* A, Y $10 and the flags of the loop's last compare. */
static void abi_exit_filled(uint8_t kind)
{
    cpu.a = kind;
    cpu.y = OBJECT_END;
    abi_nz(0);
    abi_flag(P_C, true);
}

/* One object put in slot 5. */
static void react_slot_5(uint8_t kind)
{
    wr8(OBJ_KIND + ITEM_SLOT_A, kind);
}

/*
 * @routine 01B9:5F2 RichterToHurtAir
 * @run 6D:A5F2
 * WHAT IT DOES: Hurt jump: state 3, step 2, the jump's speeds, a long hit timer, animation
 * $23, its sound, and a reaction by zField93. Item $05 goes to RichterItemCrashFive.
 * HOW: Table by zField93, 0 to 8; the C takes anything past 8 as 8. SoundCommand $65.
 * WAITS: no.
 * Entry: nothing. Exit: item 5 first, as RichterItemCrashFive. Otherwise as the reaction, after
 * A and X twice the item, N and Z from it, C its bit 7, Y as the animation and V as the sound left
 * them.
 * @stop A621
 */
HurtAir RichterToHurtAir(uint8_t x, uint8_t y)
{
    HurtAir hurt;
    uint16_t entry;

    jsr_push(0xA5F5);
    RichterPadOverridesOff();
    jsr_pop();
    hurt.crash = rd8(zField93) == ITEM_CRASH_FIVE;
    if (hurt.crash) {
        hurt.crash_five = RichterItemCrashFive(x, y);
        return hurt;
    }
    set_state(STATE_AIR, 2);
    jsr_push(0xA608);
    hurt.lift = RichterJumpSpeed();
    jsr_pop();
    wr8(OBJ_ANIM_TIMER, HURT_TIMER);
    wr8(OBJ_3250, 0);
    jsr_push(0xA615);
    hurt.animation = RichterSetAnimation(ANIM_HURT_AIR);
    jsr_pop();
    jsr_push(0xA61A);
    hurt.sound = SoundCommand(SOUND_HURT, x, hurt.animation.y);
    jsr_pop();
    hurt.item = rd8(zField93);
    entry = (uint16_t)(HitReactionHandlers + (uint8_t)(hurt.item << 1));
    rd8(entry);
    rd8((uint16_t)(entry + 1));
    switch (hurt.item) {
    case 0: RichterReactFillD(); break;
    case 1:
    case 2: RichterReactFillB(); break;
    case 3: RichterReact12(); break;
    case 4: RichterReactE(); break;
    case ITEM_CRASH_FIVE: hurt.crash_five = RichterItemCrashFive(x, y); break;
    case 6: RichterReact10(); break;
    case 7: RichterReact11(); break;
    default: RichterReact13(); break;
    }
    return hurt;
}

void hurt_air_exit_shim(HurtAir hurt)
{
    uint8_t index;

    if (hurt.crash) {
        crash_five_exit_shim(hurt.crash_five);
        return;
    }
    lift_exit_shim(hurt.lift);
    animation_exit_shim(hurt.animation);
    cpu.a = SOUND_HURT;
    sound_command_exit_shim(SOUND_HURT, hurt.sound);
    index = (uint8_t)(hurt.item << 1);
    cpu.a = index;
    cpu.x = index;
    abi_nz(index);
    abi_flag(P_C, (hurt.item & 0x80) != 0);
    switch (hurt.item) {
    case 0: abi_exit_filled(REACT_FILL_D); break;
    case 1:
    case 2: abi_exit_filled(REACT_FILL_B); break;
    case 3: cpu.a = REACT_12; abi_nz(REACT_12); break;
    case 4: cpu.a = REACT_E; abi_nz(REACT_E); break;
    case ITEM_CRASH_FIVE: crash_five_exit_shim(hurt.crash_five); break;
    case 6: cpu.a = REACT_10; abi_nz(REACT_10); break;
    case 7: cpu.a = REACT_11_TIMER; abi_nz(REACT_11_TIMER); break;
    default: cpu.a = REACT_13; abi_nz(REACT_13); break;
    }
}

void abi_RichterToHurtAir(void)
{
    hurt_air_exit_shim(RichterToHurtAir(cpu.x, cpu.y));
}

/* Item 5's ledge crash: the animation's exit, A $60; or the nudge's exit, then the hurt jump's. */
void ledge_hurt_exit_shim(LedgeHurt hurt)
{
    if (hurt.crash) {
        animation_exit_shim(hurt.animation);
        cpu.a = CRASH_FIVE_TIMER;
        abi_nz(CRASH_FIVE_TIMER);
        return;
    }
    abi_exit_nudge(hurt.nudge);
    hurt_air_exit_shim(hurt.hurt);
}

/*
 * @routine 01B9:633 RichterReactFillD
 * @run 6D:A633
 * WHAT IT DOES: Fills the slots from 5 with object $0D.
 * HOW: A loop to $10.
 * WAITS: no.
 * Entry: nothing. Exit: A $0D, Y $10, N, Z, C from comparing it.
 */
void RichterReactFillD(void)
{
    react_fill_slots(REACT_FILL_D);
}

void abi_RichterReactFillD(void)
{
    RichterReactFillD();
    abi_exit_filled(REACT_FILL_D);
}

/*
 * @routine 01B9:640 RichterReactFillB
 * @run 6D:A640
 * WHAT IT DOES: Fills the slots from 5 with object $0B.
 * HOW: Goes back into the loop.
 * WAITS: no.
 * Entry: nothing. Exit: A $0B, Y $10, N, Z, C from comparing it.
 */
void RichterReactFillB(void)
{
    react_fill_slots(REACT_FILL_B);
}

void abi_RichterReactFillB(void)
{
    RichterReactFillB();
    abi_exit_filled(REACT_FILL_B);
}

/*
 * @routine 01B9:644 RichterItemCrashFive
 * @run 6D:A644
 * WHAT IT DOES: Item $05's crash, from a hurt jump: no speed, animation $2A, its sound,
 * state 0, step 2, its timer.
 * HOW: The fields in turn.
 * WAITS: no.
 * Entry: nothing. Exit: A $60, N and Z from it; C and V as SoundCommand left them.
 */
CrashFive RichterItemCrashFive(uint8_t x, uint8_t y)
{
    CrashFive crash;

    jsr_push(0xA647);
    RichterStopMoving();
    jsr_pop();
    jsr_push(0xA64C);
    crash.animation = RichterSetAnimation(ANIM_CRASH_FIVE);
    jsr_pop();
    jsr_push(0xA651);
    crash.sound = SoundCommand(SOUND_HURT, x, crash.animation.y);
    jsr_pop();
    set_state(STATE_STAND, 2);
    wr8(OBJ_LOOP_FLAGS, CRASH_FIVE_TIMER);
    return crash;
}

void crash_five_exit_shim(CrashFive crash)
{
    animation_exit_shim(crash.animation);
    cpu.a = SOUND_HURT;
    sound_command_exit_shim(SOUND_HURT, crash.sound);
    cpu.a = CRASH_FIVE_TIMER;
    abi_nz(CRASH_FIVE_TIMER);
}

void abi_RichterItemCrashFive(void)
{
    crash_five_exit_shim(RichterItemCrashFive(cpu.x, cpu.y));
}

/*
 * @routine 01B9:661 RichterReactE
 * @run 6D:A661
 * WHAT IT DOES: Puts object $0E in slot 5.
 * HOW: One store.
 * WAITS: no.
 * Entry: nothing. Exit: A $0E, N and Z from it. X, Y, C, V kept.
 */
void RichterReactE(void)
{
    react_slot_5(REACT_E);
}

void abi_RichterReactE(void)
{
    RichterReactE();
    cpu.a = REACT_E;
    abi_nz(REACT_E);
}

/*
 * @routine 01B9:667 RichterReact11
 * @run 6D:A667
 * WHAT IT DOES: Puts object $11 in slot 5, and sets the hit timer $60 with the player live.
 * HOW: Three stores.
 * WAITS: no.
 * Entry: nothing. Exit: A $60, N and Z from it. X, Y, C, V kept.
 */
void RichterReact11(void)
{
    react_slot_5(REACT_11);
    wr8(OBJ_3250, 1);
    wr8(OBJ_ANIM_TIMER, REACT_11_TIMER);
}

void abi_RichterReact11(void)
{
    RichterReact11();
    cpu.a = REACT_11_TIMER;
    abi_nz(REACT_11_TIMER);
}

/*
 * @routine 01B9:677 RichterReact10
 * @run 6D:A677
 * WHAT IT DOES: Puts object $10 in slot 5.
 * HOW: Goes back into RichterReactE's store.
 * WAITS: no.
 * Entry: nothing. Exit: A $10, N and Z from it. X, Y, C, V kept.
 */
void RichterReact10(void)
{
    react_slot_5(REACT_10);
}

void abi_RichterReact10(void)
{
    RichterReact10();
    cpu.a = REACT_10;
    abi_nz(REACT_10);
}

/*
 * @routine 01B9:67B RichterReact12
 * @run 6D:A67B
 * WHAT IT DOES: Puts object $12 in slot 5.
 * HOW: Goes back into RichterReactE's store.
 * WAITS: no.
 * Entry: nothing. Exit: A $12, N and Z from it. X, Y, C, V kept.
 */
void RichterReact12(void)
{
    react_slot_5(REACT_12);
}

void abi_RichterReact12(void)
{
    RichterReact12();
    cpu.a = REACT_12;
    abi_nz(REACT_12);
}

/*
 * @routine 01B9:67F RichterReact13
 * @run 6D:A67F
 * WHAT IT DOES: Puts object $13 in slot 5.
 * HOW: Goes back into RichterReactE's store.
 * WAITS: no.
 * Entry: nothing. Exit: A $13, N and Z from it. X, Y, C, V kept.
 */
void RichterReact13(void)
{
    react_slot_5(REACT_13);
}

void abi_RichterReact13(void)
{
    RichterReact13();
    cpu.a = REACT_13;
    abi_nz(REACT_13);
}

/*
 * @routine 01B9:683 RichterKnockBack
 * @run 6D:A683
 * WHAT IT DOES: Knocked into the air: state 3, animation $31, the knock's speeds, marks
 * PlayerByte3BF1 bit 0.
 * HOW: The jump's speed replaced by the knock's.
 * WAITS: no.
 * Entry: nothing. Exit: A PlayerByte3BF1, N and Z from it; Y twice the facing, C the facing's
 * bit 7; V as RichterLiftThree. X kept.
 */
Knock RichterKnockBack(void)
{
    Knock knock;

    wr8(PlayerMayLeaveTop, 0);
    wr8(PadForcedPressB, 0);
    wr8(zFieldC6, 0);
    jsr_push(0xA68E);
    knock.lift = RichterJumpSpeed();
    jsr_pop();
    set_state(STATE_AIR, 0);
    jsr_push(0xA69D);
    RichterSetAnimation(ANIM_KNOCK);
    jsr_pop();
    knock.speed = speed_by_facing(KnockSpeeds);
    wr8(OBJ_VY_LO, KNOCK_RISE_LO);
    wr8(OBJ_VY_HI, KNOCK_RISE_HI);
    knock.marks = (uint8_t)(rd8(PlayerByte3BF1) | KNOCKED);
    wr8(PlayerByte3BF1, knock.marks);
    return knock;
}

void abi_RichterKnockBack(void)
{
    Knock knock = RichterKnockBack();

    lift_exit_shim(knock.lift);
    abi_exit_speed(knock.speed);
    cpu.a = knock.marks;
    abi_nz(knock.marks);
}

/*
 * @routine 01B9:6C5 RichterToAir
 * @run 6D:A6C5
 * WHAT IT DOES: Takes to the air: state 3, the jump's speed; with left or right held, a
 * moving jump, and when that faces the way the player already did, marks it in
 * PlayerByte3BF1; otherwise animation $0A.
 * HOW: The facing the pad asks for is set for the jump's speed, then the old one put back.
 * WAITS: no.
 * Entry: nothing. Exit: V as RichterLiftThree; X kept. Marked, A $80, N set, Z clear, C set, Y
 * twice the facing asked for; otherwise as RichterSetAnimation.
 */
AirStart RichterToAir(void)
{
    AirStart air = { { 0, 0, false }, false, false, { FRAME_STOPPED }, { 0, 0 } };
    uint8_t way;
    uint8_t facing;

    wr8(PlayerMayLeaveTop, 0);
    wr8(PadForcedPressB, 0);
    wr8(zFieldC6, 0);
    set_state(STATE_AIR, 0);
    jsr_push(0xA6D9);
    air.lift = RichterJumpSpeed();
    jsr_pop();
    if (rd8(zPadHeld) & PAD_RIGHT) {
        way = FACING_RIGHT;
    } else if (rd8(zPadHeld) & PAD_LEFT) {
        way = FACING_LEFT;
    } else {
        air.animation = RichterSetAnimation(ANIM_AIR);
        return air;
    }
    air.moving = true;
    facing = rd8(OBJ_FACING_LEFT);
    push8(facing);
    wr8(OBJ_FACING_LEFT, way);
    jsr_push(0xA6F2);
    RichterSetAnimation(ANIM_AIR_MOVING);
    jsr_pop();
    jsr_push(0xA6F5);
    air.speed = RichterSpeedJump();
    jsr_pop();
    facing = pull8();
    air.same_way = facing == rd8(OBJ_FACING_LEFT);
    wr8(OBJ_FACING_LEFT, facing);
    if (air.same_way) {
        wr8(PlayerByte3BF1, AIR_SAME_WAY);
        return air;
    }
    air.animation = RichterSetAnimation(ANIM_AIR);
    return air;
}

void air_exit_shim(AirStart air)
{
    lift_exit_shim(air.lift);
    if (air.moving && air.same_way) {
        abi_exit_speed(air.speed);
        cpu.a = AIR_SAME_WAY;
        abi_nz(AIR_SAME_WAY);
        abi_flag(P_C, true);
    } else {
        animation_exit_shim(air.animation);
    }
}

void abi_RichterToAir(void)
{
    air_exit_shim(RichterToAir());
}

/*
 * @routine 01B9:70C RichterJumpSpeed
 * @run 6D:A70C
 * WHAT IT DOES: Sets a jump's Y speed, stopped along X, and lifts the player off the floor.
 * HOW: Speed $FB50; runs on into RichterLiftThree.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterLiftThree.
 */
Lift RichterJumpSpeed(void)
{
    wr8(PlayerByte3BF1, 0);
    jsr_push(0xA712);
    RichterStopMoving();
    jsr_pop();
    wr8(OBJ_VY_LO, 0x50);
    wr8(OBJ_VY_HI, 0xFB);
    return RichterLiftThree();
}

void abi_RichterJumpSpeed(void)
{
    lift_exit_shim(RichterJumpSpeed());
}

/*
 * @routine 01B9:71C RichterLiftThree
 * @run 6D:A71C
 * WHAT IT DOES: Lifts the player three pixels, stopping at the top of the map.
 * HOW: A 16-bit subtract; high $FF means past the top, and the position becomes zero.
 * WAITS: no.
 * Entry: nothing. Exit: A the position's high byte; N, Z, C from comparing it with $FF; V
 * from the subtract. X and Y kept.
 */
Lift RichterLiftThree(void)
{
    Lift lift;
    uint8_t low = rd8(OBJ_Y_LO);

    lift.borrow = low < LIFT_STEP;
    wr8(OBJ_Y_LO, (uint8_t)(low - LIFT_STEP));
    lift.high_before = rd8(OBJ_Y_HI);
    lift.high = (uint8_t)(lift.high_before - lift.borrow);
    wr8(OBJ_Y_HI, lift.high);
    if (lift.high == 0xFF) {
        wr8(OBJ_Y_LO, 0);
        wr8(OBJ_Y_HI, 0);
    }
    return lift;
}

void lift_exit_shim(Lift lift)
{
    cpu.a = lift.high;
    abi_nz((uint8_t)(lift.high + 1));
    abi_flag(P_C, lift.high == 0xFF);
    abi_flag(P_V, lift.high_before == 0x80 && lift.borrow);
}

void abi_RichterLiftThree(void)
{
    lift_exit_shim(RichterLiftThree());
}

/* The registers each check of the walk left, in order, up to the one it reached. */
static void walk_replay_shim(const WalkStep *w)
{
    columns_exit_shim(w->columns);
    floor_check_exit_shim(w->floor);
    if (w->floor.how == FLOOR_NONE) {
        abi_nz(w->stepped);
        columns_exit_shim(w->columns_below);
        floor_check_exit_shim(w->floor_below);
        if (w->how == WALK_FELL)
            return;
    }
    if (w->reached >= STAND_JUMP)
        abi_flag(P_C, w->jump);
    if (w->reached >= STAND_LANDING)
        landing_exit_shim(w->landing);
    if (w->reached >= STAND_ITEM_CRASH)
        item_crash_exit_shim(w->crash);
    if (w->reached >= STAND_ITEM_USE)
        item_use_exit_shim(w->use);
    if (w->reached >= STAND_LEDGE)
        ledge_check_exit_shim(w->ledge);
    if (w->reached >= STAND_UP)
        abi_flag(P_C, w->up);
    if (w->reached < STAND_LOOK)
        return;
    animation_exit_shim(w->tick);
    cpu.a = w->facing;
    abi_nz(w->facing);
    if (w->how == WALK_ON)
        return;
    abi_flag(P_N, (w->jump_held & 0x80) != 0);
    abi_flag(P_V, (w->jump_held & 0x40) != 0);
    abi_flag(P_Z, !(w->jump_held & PAD_JUMP));
    if (w->jump_held & PAD_JUMP) {
        cpu.a = w->back_facing;
        abi_nz(w->back_facing);
    }
}

/*
 * @routine 01B9:739 RichterStateWalk
 * @run 6D:A739
 * WHAT IT DOES: State 1, walking: a floor under him, or one pixel lower, or he falls; then the
 * first that applies of a jump, the item crash, an item or the whip, a ledge to climb, up to
 * crouch. Otherwise the animation steps, and the pad held his way keeps him walking; held the
 * other way with button II, he steps back; else he stands.
 * HOW: RichterReadColumns and RichterFloorCheck, again after OBJ_Y_LO is stepped down; the
 * checks as in the stand's first step. OBJ_FIELDS_2EE0 0 to walk, $32 to step back.
 * WAITS: no.
 * Entry: nothing. Exit: as the transition taken, with what the checks before it left.
 */
WalkStep RichterStateWalk(void)
{
    WalkStep w = { 0 };

    jsr_push(0xA73C);
    w.columns = RichterReadColumns();
    jsr_pop();
    jsr_push(0xA73F);
    w.floor = RichterFloorCheck();
    jsr_pop();
    if (w.floor.how == FLOOR_NONE) {
        w.stepped = (uint8_t)(rd8(OBJ_Y_LO) + 1);
        wr8(OBJ_Y_LO, w.stepped);
        jsr_push(0xA747);
        w.columns_below = RichterReadColumns();
        jsr_pop();
        jsr_push(0xA74A);
        w.floor_below = RichterFloorCheck();
        jsr_pop();
        if (w.floor_below.how == FLOOR_NONE) {
            w.how = WALK_FELL;
            w.fall = RichterToFall();
            return w;
        }
    }
    jsr_push(0xA752);
    w.jump = RichterJumpPressed();
    jsr_pop();
    w.reached = STAND_JUMP;
    if (w.jump) {
        w.how = WALK_JUMPED;
        w.air = RichterToAir();
        return w;
    }
    jsr_push(0xA75A);
    w.landing = RichterLandOnFloor();
    jsr_pop();
    w.reached = STAND_LANDING;
    jsr_push(0xA75D);
    w.crash = RichterTryItemCrash();
    jsr_pop();
    w.reached = STAND_ITEM_CRASH;
    if (w.crash.how == CRASH_STARTED) {
        w.how = WALK_CRASHED;
        w.hurt = RichterToHurtAir(0, 0);   /* X and Y of the calling state: passed when that state is re-lifted */
        return w;
    }
    jsr_push(0xA765);
    w.use = RichterUseItem();
    jsr_pop();
    w.reached = STAND_ITEM_USE;
    if (w.use.how != USE_NO_PRESS) {
        w.how = WALK_ATTACKED;
        w.attack = RichterAttack();
        return w;
    }
    jsr_push(0xA76D);
    w.ledge = RichterLedgeCheck();
    jsr_pop();
    w.reached = STAND_LEDGE;
    if (w.ledge.how == LEDGE_LOW || w.ledge.how == LEDGE_HIGH) {
        w.how = WALK_CLIMBED;
        w.climb = RichterClimbStart(ledge_column(w.ledge));
        return w;
    }
    jsr_push(0xA775);
    w.up = RichterUpHeld();
    jsr_pop();
    w.reached = STAND_UP;
    if (w.up) {
        w.how = WALK_CROUCHED;
        w.fall = RichterCrouch();
        return w;
    }
    jsr_push(0xA77D);
    w.tick = RichterAnimationTick();
    jsr_pop();
    w.reached = STAND_LOOK;
    w.facing = rd8(OBJ_FACING_LEFT);
    w.held = rd8(zPadHeld);
    if (w.held & (w.facing == FACING_RIGHT ? PAD_RIGHT : PAD_LEFT)) {
        wr8(OBJ_FIELDS_2EE0, 0);
        w.how = WALK_ON;
        w.speed = RichterSpeedWalk();
        return w;
    }
    /* TST #2 of the pad, then the facing and the pad once more for the way back */
    w.jump_held = rd8(zPadHeld);
    if (w.jump_held & PAD_JUMP) {
        uint8_t back;

        w.back_facing = rd8(OBJ_FACING_LEFT);
        back = rd8(zPadHeld);
        if (back & (w.back_facing == FACING_RIGHT ? PAD_LEFT : PAD_RIGHT)) {
            wr8(OBJ_FIELDS_2EE0, BACKSTEP_FIELD);
            w.how = WALK_BACKSTEP;
            w.speed = RichterSpeedBackstep();
            return w;
        }
    }
    w.how = WALK_STOOD;
    w.fall = RichterStand();
    return w;
}

void walk_exit_shim(WalkStep w)
{
    walk_replay_shim(&w);
    switch (w.how) {
    case WALK_FELL:
        to_fall_exit_shim(w.fall);
        break;
    case WALK_JUMPED:
        air_exit_shim(w.air);
        break;
    case WALK_CRASHED:
        hurt_air_exit_shim(w.hurt);
        break;
    case WALK_ATTACKED:
        attack_exit_shim(w.attack);
        break;
    case WALK_CLIMBED:
        climb_start_exit_shim(w.climb);
        break;
    case WALK_CROUCHED:
    case WALK_STOOD:
        animation_exit_shim(w.fall);
        break;
    case WALK_ON:
        abi_nz(0);
        abi_exit_speed(w.speed);
        break;
    case WALK_BACKSTEP:
        abi_exit_speed(w.speed);
        break;
    default:
        break;
    }
}

void abi_RichterStateWalk(void)
{
    walk_exit_shim(RichterStateWalk());
}

/*
 * @routine 01B9:7AC RichterStand
 * @run 6D:A7AC
 * WHAT IT DOES: Stands the player: state 0, step 0, animation 1, no speed.
 * HOW: Clears PlayerMayLeaveTop and PlayerByte3BD5 first.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterSetAnimation. X and V kept.
 */
AnimationStep RichterStand(void)
{
    AnimationStep animation;

    wr8(PlayerMayLeaveTop, 0);
    wr8(PlayerByte3BD5, 0);
    set_state(STATE_STAND, 0);
    jsr_push(0xA7C1);
    animation = RichterSetAnimation(ANIM_STAND);
    jsr_pop();
    RichterStopMoving();
    return animation;
}

void abi_RichterStand(void)
{
    animation_exit_shim(RichterStand());
}

/*
 * @routine 01B9:7C4 RichterClimbStart
 * @run 6D:A7C4
 * WHAT IT DOES: Takes hold of the ledge RichterLedgeCheck found, to climb it: the low ledge
 * (zTemp01 0) or the high one, each in one of two holds by whether the point past the ledge's
 * edge reads bit 2: states 5 and 6 low, 7 and 8 high. PlayerClimbReach is how far he has to
 * go across to the edge. He stops, and the state's step is 2.
 * HOW: The column's offset (RichterColumnOffset) and 8 more past it, read $0A below him for the
 * low ledge, $1E for the high; the reach from the offset less zTemp00 for the low, less the
 * column's RichterColumnX for the high. The stand's animation unless he was walking.
 * WAITS: no.
 * Entry: X the column, zTemp00 the low ledge's X, zTemp01 low or high. Exit: A 2, N and Z clear,
 * X the state; Y the column's index, or as RichterSetAnimation; C set unless he was standing, or as
 * RichterSetAnimation; V from the reach's last add or subtract.
 */
ClimbStart RichterClimbStart(uint8_t column)
{
    ClimbStart climb = { 0 };
    uint8_t across, offset_value, x_value, row;
    ColumnOffset offset;
    bool past;

    jsr_push(0xA7C7);
    RichterPadOverridesOff();
    jsr_pop();
    wr8(zFieldC4, 0);
    wr8(PlayerMayLeaveTop, 1);
    climb.column = column;
    if (rd8(zTemp01) == 0) {
        uint8_t ledge_x = rd8(zTemp00);

        push8(ledge_x);
        jsr_push(0xA7D8);
        offset = RichterColumnOffset(column);
        offset_value = offset.value;
        jsr_pop();
        x_value = rd8(OBJ_X_LO);
        across = add_bytes(add_bytes(offset_value, x_value, false).value, CLIMB_PAST_EDGE, false).value;
        wr8(zTemp00, across);
        row = rd8(OBJ_Y_LO);
        jsr_push(0xA7EA);
        past = tile_answer(RichterReadBelow((uint8_t)(row + CLIMB_LOW_BELOW),
                                            row + CLIMB_LOW_BELOW > 0xFF)) & CLIMB_EDGE_BIT;
        jsr_pop();
        ledge_x = pull8();
        wr8(zTemp00, ledge_x);
        jsr_push(past ? 0xA7F4 : 0xA81C);
        offset = RichterColumnOffset(column);
        offset_value = offset.value;
        jsr_pop();
        x_value = rd8(zTemp00);
        across = sub_bytes(offset_value, x_value, true).value;
        if (past) {
            climb.reach = sub_bytes(across, CLIMB_PAST_EDGE, true);
            climb.state = STATE_CLIMB_LOW_EDGE;
        } else {
            climb.reach = add_bytes(add_bytes(across, CLIMB_LOW_NEAR, false).value,
                                    CLIMB_PAST_EDGE, false);
            climb.state = STATE_CLIMB_LOW_OPEN;
        }
    } else {
        jsr_push(0xA82C);
        offset = RichterColumnOffset(column);
        offset_value = offset.value;
        jsr_pop();
        x_value = rd8(OBJ_X_LO);
        across = add_bytes(add_bytes(offset_value, x_value, false).value, CLIMB_PAST_EDGE, false).value;
        wr8(zTemp00, across);
        row = rd8(OBJ_Y_LO);
        jsr_push(0xA83E);
        past = tile_answer(RichterReadBelow((uint8_t)(row + CLIMB_HIGH_BELOW),
                                            row + CLIMB_HIGH_BELOW > 0xFF)) & CLIMB_EDGE_BIT;
        jsr_pop();
        jsr_push(past ? 0xA845 : 0xA853);
        offset = RichterColumnOffset(column);
        offset_value = offset.value;
        jsr_pop();
        x_value = rd8((uint16_t)(RichterColumnX + column));
        across = sub_bytes(offset_value, x_value, true).value;
        if (past) {
            climb.reach = sub_bytes(across, 1, true);
            climb.state = STATE_CLIMB_HIGH_EDGE;
        } else {
            climb.reach = add_bytes(across, CLIMB_PAST_EDGE, false);
            climb.state = STATE_CLIMB_HIGH_OPEN;
        }
    }
    climb.index = offset.index;
    wr8(PlayerClimbReach, climb.reach.value);
    climb.was = rd8(OBJ_STATE);
    if (climb.was != STATE_WALKING) {
        climb.animated = true;
        jsr_push(0xA80B);
        climb.animation = RichterSetAnimation(ANIM_WALK);
        jsr_pop();
    }
    wr8(OBJ_SUBSTATE, CLIMB_STEP);
    wr8(OBJ_STATE, climb.state);
    RichterStopMoving();
    return climb;
}

void climb_start_exit_shim(ClimbStart climb)
{
    cpu.y = climb.index;
    abi_flag(P_V, climb.reach.overflow);
    abi_flag(P_C, climb.was >= STATE_WALKING);
    if (climb.animated)
        animation_exit_shim(climb.animation);
    cpu.x = climb.state;
    cpu.a = CLIMB_STEP;
    abi_nz(CLIMB_STEP);
}

void abi_RichterClimbStart(void)
{
    climb_start_exit_shim(RichterClimbStart(cpu.x));
}

/*
 * The ground checks a step of his states makes, kept in the order made: each check's answer goes
 * to the step's C, and the log gives the registers each left, to original code and to the exit.
 */
static void log_add(CheckLog *log, CheckRecord record)
{
    log->step[log->count++] = record;
}

static ColumnsEnd check_columns(CheckLog *log, uint16_t ret)
{
    CheckRecord r = { .kind = CHECK_COLUMNS };

    jsr_push(ret);
    r.columns = RichterReadColumns();
    jsr_pop();
    log_add(log, r);
    return r.columns;
}

static bool check_no_footing(CheckLog *log, uint16_t ret)
{
    CheckRecord r = { .kind = CHECK_FOOTING };

    jsr_push(ret);
    r.footing = RichterHasNoFooting();
    jsr_pop();
    log_add(log, r);
    return r.footing.no_floor;
}

static void check_landing(CheckLog *log, uint16_t ret)
{
    CheckRecord r = { .kind = CHECK_LANDING };

    jsr_push(ret);
    r.landing = RichterLandOnFloor();
    jsr_pop();
    log_add(log, r);
}

static bool check_jump(CheckLog *log, uint16_t ret)
{
    CheckRecord r = { .kind = CHECK_ANSWER };

    jsr_push(ret);
    r.answer = RichterJumpPressed();
    jsr_pop();
    log_add(log, r);
    return r.answer;
}

static bool check_up(CheckLog *log, uint16_t ret)
{
    CheckRecord r = { .kind = CHECK_ANSWER };

    jsr_push(ret);
    r.answer = RichterUpHeld();
    jsr_pop();
    log_add(log, r);
    return r.answer;
}

static bool check_item_crash(CheckLog *log, uint16_t ret)
{
    CheckRecord r = { .kind = CHECK_CRASH };

    jsr_push(ret);
    r.crash = RichterTryItemCrash();
    jsr_pop();
    log_add(log, r);
    return r.crash.how == CRASH_STARTED;
}

static bool check_item_use(CheckLog *log, uint16_t ret)
{
    CheckRecord r = { .kind = CHECK_USE };

    jsr_push(ret);
    r.use = RichterUseItem();
    jsr_pop();
    log_add(log, r);
    return r.use.how != USE_NO_PRESS;
}

/* The attack's progress against a mark: LDA OBJ_SCROLL_BAND, CMP. */
static bool check_progress_below(CheckLog *log, uint8_t mark)
{
    CheckRecord r = { .kind = CHECK_COMPARE };

    r.compare.value = rd8(OBJ_SCROLL_BAND);
    r.compare.with = mark;
    log_add(log, r);
    return r.compare.value < mark;
}

static void check_turn(CheckLog *log, uint16_t ret)
{
    CheckRecord r = { .kind = CHECK_TURN };

    jsr_push(ret);
    r.turn = RichterTurnOnPress();
    jsr_pop();
    log_add(log, r);
}

/* The animation's wait: false when it still runs, and the step must return at once. */
static bool check_animation_ended(CheckLog *log, uint16_t ret)
{
    CheckRecord r = { .kind = CHECK_RUNNING };
    AnimationWaitEnd wait;

    jsr_push(ret);
    wait = RichterAnimationWait();
    r.running = wait.step;
    log_add(log, r);
    if (wait.dropped)
        return false;   /* the wait took the return address off: this step's caller is next */
    jsr_pop();
    return true;
}

void check_log_replay_shim(const CheckLog *log)
{
    for (uint8_t i = 0; i < log->count; i++) {
        const CheckRecord *r = &log->step[i];

        switch (r->kind) {
        case CHECK_COLUMNS:
            columns_exit_shim(r->columns);
            break;
        case CHECK_FOOTING:
            footing_exit_shim(r->footing);
            break;
        case CHECK_LANDING:
            landing_exit_shim(r->landing);
            break;
        case CHECK_ANSWER:
            abi_flag(P_C, r->answer);
            break;
        case CHECK_CRASH:
            item_crash_exit_shim(r->crash);
            break;
        case CHECK_USE:
            item_use_exit_shim(r->use);
            break;
        case CHECK_COMPARE:
            cpu.a = r->compare.value;
            abi_nz((uint8_t)(r->compare.value - r->compare.with));
            abi_flag(P_C, r->compare.value >= r->compare.with);
            break;
        case CHECK_TURN:
            turn_exit_shim(r->turn);
            break;
        case CHECK_RUNNING:
            animation_running_exit_shim(r->running);
            break;
        }
    }
}

/* A step left by the animation's wait while it runs: A the high byte of the return it dropped. */
static void wait_exit_shim(const CheckLog *log, uint8_t return_high)
{
    check_log_replay_shim(log);
    cpu.a = return_high;
    abi_nz(return_high);
}

/*
 * @routine 01B9:85E RichterStateCrouch
 * @run 6D:A85E
 * WHAT IT DOES: State 2, crouching: runs the step's handler: down, attacking, nothing, the
 * crouch's end, or rising.
 * HOW: RichterCrouchHandlers, five words by RichterStepIndex; step 2's is the RTS of step 0.
 * WAITS: no.
 * Entry: nothing. Exit: as the handler leaves them; step 2, as RichterStepIndex.
 */
CrouchState RichterStateCrouch(void)
{
    CrouchState st = { 0 };
    uint16_t entry;

    jsr_push(0xA861);
    st.step = RichterStepIndex();
    jsr_pop();
    entry = (uint16_t)(RichterCrouchHandlers + (uint8_t)(st.step << 1));
    st.handler = (uint16_t)(rd8(entry) | (rd8((uint16_t)(entry + 1)) << 8));
    /* Original code runs in the handlers, from the registers the index left. */
    step_index_exit_shim(st.step);
    switch (st.handler) {
    case 0xA86E:
        st.run = RichterCrouchStep0();
        break;
    case 0xA8C7:
        st.run = RichterCrouchStep1();
        break;
    case 0xA8AA:
        break;
    case 0xA907:
        st.run = RichterCrouchStep3();
        break;
    case 0xA937:
        st.run = RichterCrouchStep4();
        break;
    default:
        stand_handler_shim(st.step, st.handler);
        break;
    }
    return st;
}

void crouch_state_exit_shim(CrouchState st)
{
    switch (st.handler) {
    case 0xA86E:
    case 0xA8C7:
    case 0xA907:
    case 0xA937:
        crouch_exit_shim(st.run);
        break;
    default:
        break;
    }
}

void abi_RichterStateCrouch(void)
{
    crouch_state_exit_shim(RichterStateCrouch());
}

/* The transitions out of a crouch's step, each with what it starts. */
static CrouchStep crouch_falls(CrouchStep c)
{
    c.how = CROUCH_FELL;
    c.animation = RichterToFall();
    return c;
}

static CrouchStep crouch_jumps(CrouchStep c)
{
    c.how = CROUCH_JUMPED;
    c.air = RichterToAir();
    return c;
}

static CrouchStep crouch_crashes(CrouchStep c)
{
    c.how = CROUCH_CRASHED;
    c.hurt = RichterToHurtAir(0, 0);   /* X and Y of the calling state: passed when that state is re-lifted */
    return c;
}

static CrouchStep crouch_attacks(CrouchStep c)
{
    c.how = CROUCH_ATTACKED;
    c.attack = RichterCrouchAttack();
    return c;
}

/* $A9DD has no C yet: its original code, from the registers the checks left. */
static void crouch_fall_shim(const CheckLog *log)
{
    check_log_replay_shim(log);
    call_original(RH_A9DD);
}

/*
 * @routine 01B9:86E RichterCrouchStep0
 * @run 6D:A86E
 * WHAT IT DOES: Crouched: reads the ground, falls if nothing is below; then a jump, the item
 * crash, an item or the whip from the crouch; with up let go he starts to rise (animation $0D,
 * step 4); held, left or right alone turns him.
 * HOW: The ground checks in turn, each answering in C. zPadPressed bits 7 and 5: right alone
 * faces right, left alone left, both or neither leave him.
 * WAITS: no.
 * Entry: nothing. Exit: as the transition taken, with what the checks before it left. Rising,
 * A 4, as RichterSetAnimation left the rest. Still down, A the presses' left and right bits,
 * C set, Z clear only for left alone.
 */
CrouchStep RichterCrouchStep0(void)
{
    CrouchStep c = { 0 };

    check_columns(&c.log, 0xA871);
    if (check_no_footing(&c.log, 0xA874))
        return crouch_falls(c);
    check_landing(&c.log, 0xA87C);
    if (check_jump(&c.log, 0xA87F))
        return crouch_jumps(c);
    if (check_item_crash(&c.log, 0xA887))
        return crouch_crashes(c);
    if (check_item_use(&c.log, 0xA88F))
        return crouch_attacks(c);
    if (!check_up(&c.log, 0xA894)) {
        c.how = CROUCH_RISING;
        jsr_push(0xA8B0);
        c.animation = RichterSetAnimation(ANIM_RISE);
        jsr_pop();
        wr8(OBJ_SUBSTATE, CROUCH_RISE_STEP);
        return c;
    }
    c.how = CROUCH_HELD;
    c.bits = (uint8_t)(rd8(zPadPressed) & (PAD_LEFT | PAD_RIGHT));
    if (c.bits != 0 && c.bits != (PAD_LEFT | PAD_RIGHT))
        wr8(OBJ_FACING_LEFT, c.bits == PAD_RIGHT ? FACING_RIGHT : FACING_LEFT);
    return c;
}

/*
 * @routine 01B9:8B6 RichterCrouchAttack
 * @run 6D:A8B6
 * WHAT IT DOES: Starts an attack from the crouch: turned by the pad, stopped, animation 6, and
 * the whip or the item as a standing attack starts them.
 * HOW: As RichterAttack from its animation on, keeping the state.
 * WAITS: no.
 * Entry: nothing. Exit: as RichterAttack.
 */
AttackStart RichterCrouchAttack(void)
{
    jsr_push(0xA8B9);
    RichterPadOverridesOff();
    jsr_pop();
    wr8(PlayerByte3BE1, 0);
    jsr_push(0xA8BF);
    RichterFaceByPad();
    jsr_pop();
    jsr_push(0xA8C2);
    RichterStopMoving();
    jsr_pop();
    return attack_with_animation(ANIM_CROUCH_ATTACK);
}

void abi_RichterCrouchAttack(void)
{
    attack_exit_shim(RichterCrouchAttack());
}

/*
 * @routine 01B9:8C7 RichterCrouchStep1
 * @run 6D:A8C7
 * WHAT IT DOES: Attacking from the crouch: reads the ground (nothing below goes to $A9DD); turns
 * with a press while the attack is young; with up let go before it is far along, stands; else
 * waits for the animation's end, then is down again, animation $0E, step 0.
 * HOW: OBJ_SCROLL_BAND against 3 and 6. RichterAnimationWait returns past this routine while it
 * runs.
 * WAITS: no.
 * Entry: nothing. Exit: falling, as $A9DD. Stood, as RichterAnimationTick, state 0 and
 * OBJ_FIELDS_2EE0 3. Waiting, as RichterAnimationRunning, A $A8 and N set. Down, A $0E, N and Z
 * clear, the rest as RichterAnimationRunning. With what the checks before left.
 */
CrouchStep RichterCrouchStep1(void)
{
    CrouchStep c = { 0 };

    check_columns(&c.log, 0xA8CA);
    if (check_no_footing(&c.log, 0xA8CD)) {
        c.how = CROUCH_FELL_ORIGINAL;
        crouch_fall_shim(&c.log);
        return c;
    }
    check_landing(&c.log, 0xA8D5);
    if (check_progress_below(&c.log, TURN_PROGRESS))
        check_turn(&c.log, 0xA8DF);
    if (check_progress_below(&c.log, CROUCH_PROGRESS) && !check_up(&c.log, 0xA8E9)) {
        wr8(OBJ_STATE, STATE_STAND);
        wr8(OBJ_FIELDS_2EE0, ATTACK_STAND_FIELD);
        c.how = CROUCH_STOOD_TICK;
        c.animation = RichterAnimationTick();
        return c;
    }
    if (!check_animation_ended(&c.log, 0xA8EE)) {
        c.how = CROUCH_WAITING;
        c.return_high = CROUCH_ATTACK_RETURN_HIGH;
        return c;
    }
    wr8(PlayerByte3BD5, 0);
    wr8(OBJ_FRAME, ANIM_CROUCHED);
    wr8(OBJ_SUBSTATE, 0);
    c.how = CROUCH_DOWN_AGAIN;
    return c;
}

/*
 * @routine 01B9:907 RichterCrouchStep3
 * @run 6D:A907
 * WHAT IT DOES: The crouch's end: the pad's overrides off; reads the ground, falls if nothing
 * is below; a jump, the item crash, an item or the whip; else waits for the animation's end
 * and goes back to step 0.
 * HOW: The ground checks in turn. RichterAnimationWait returns past this routine while it runs.
 * WAITS: no.
 * Entry: nothing. Exit: as the transition taken; waiting, as RichterAnimationRunning, A $A9 and
 * N set; ended, as RichterAnimationRunning. With what the checks before left.
 */
CrouchStep RichterCrouchStep3(void)
{
    CrouchStep c = { 0 };

    jsr_push(0xA90A);
    RichterPadOverridesOff();
    jsr_pop();
    check_columns(&c.log, 0xA90D);
    if (check_no_footing(&c.log, 0xA910))
        return crouch_falls(c);
    check_landing(&c.log, 0xA918);
    if (check_jump(&c.log, 0xA91B))
        return crouch_jumps(c);
    if (check_item_crash(&c.log, 0xA923))
        return crouch_crashes(c);
    if (check_item_use(&c.log, 0xA92B))
        return crouch_attacks(c);
    if (!check_animation_ended(&c.log, 0xA933)) {
        c.how = CROUCH_WAITING;
        c.return_high = CROUCH_END_RETURN_HIGH;
        return c;
    }
    wr8(OBJ_SUBSTATE, 0);
    c.how = CROUCH_STEP_BACK;
    return c;
}

/*
 * @routine 01B9:937 RichterCrouchStep4
 * @run 6D:A937
 * WHAT IT DOES: Rising from the crouch: reads the ground, falls if nothing is below; a jump,
 * the item crash, a standing attack; else waits for the animation's end and stands.
 * HOW: The ground checks in turn. RichterAnimationWait returns past this routine while it runs.
 * WAITS: no.
 * Entry: nothing. Exit: as the transition taken; waiting, as RichterAnimationRunning, A $A9 and
 * N set; risen, as RichterStand. With what the checks before left.
 */
CrouchStep RichterCrouchStep4(void)
{
    CrouchStep c = { 0 };

    check_columns(&c.log, 0xA93A);
    if (check_no_footing(&c.log, 0xA93D))
        return crouch_falls(c);
    check_landing(&c.log, 0xA945);
    if (check_jump(&c.log, 0xA948))
        return crouch_jumps(c);
    if (check_item_crash(&c.log, 0xA950))
        return crouch_crashes(c);
    if (check_item_use(&c.log, 0xA958)) {
        c.how = CROUCH_ATTACKED;
        c.attack = RichterAttack();
        return c;
    }
    if (!check_animation_ended(&c.log, 0xA960)) {
        c.how = CROUCH_WAITING;
        c.return_high = CROUCH_END_RETURN_HIGH;
        return c;
    }
    c.how = CROUCH_STOOD;
    c.animation = RichterStand();
    return c;
}

void crouch_exit_shim(CrouchStep c)
{
    if (c.how == CROUCH_FELL_ORIGINAL)
        return;
    if (c.how == CROUCH_WAITING) {
        wait_exit_shim(&c.log, c.return_high);
        return;
    }
    check_log_replay_shim(&c.log);
    switch (c.how) {
    case CROUCH_FELL:
        to_fall_exit_shim(c.animation);
        break;
    case CROUCH_JUMPED:
        air_exit_shim(c.air);
        break;
    case CROUCH_CRASHED:
        hurt_air_exit_shim(c.hurt);
        break;
    case CROUCH_ATTACKED:
        attack_exit_shim(c.attack);
        break;
    case CROUCH_RISING:
        animation_exit_shim(c.animation);
        cpu.a = CROUCH_RISE_STEP;
        abi_nz(CROUCH_RISE_STEP);
        break;
    case CROUCH_HELD:
        cpu.a = c.bits;
        abi_nz(c.bits == PAD_LEFT ? 1 : 0);
        abi_flag(P_C, true);
        break;
    case CROUCH_STOOD_TICK:
    case CROUCH_STOOD:
        animation_exit_shim(c.animation);
        break;
    case CROUCH_DOWN_AGAIN:
        cpu.a = ANIM_CROUCHED;
        abi_nz(ANIM_CROUCHED);
        break;
    default:
        break;
    }
}

void abi_RichterCrouchStep0(void)
{
    crouch_exit_shim(RichterCrouchStep0());
}

void abi_RichterCrouchStep1(void)
{
    crouch_exit_shim(RichterCrouchStep1());
}

void abi_RichterCrouchStep3(void)
{
    crouch_exit_shim(RichterCrouchStep3());
}

void abi_RichterCrouchStep4(void)
{
    crouch_exit_shim(RichterCrouchStep4());
}

/*
 * @routine 01BB:180 RichterEndItemCrash
 * @run 6D:B180
 * WHAT IT DOES: Ends an item crash: no attack, the crash's last frame held, the animation stopped,
 * the step back to 0.
 * HOW: States 5 to 8 are the crashes; two of them share each entry of RichterCrashEndFrames and
 * RichterCrashEndAnimations.
 * WAITS: no.
 * Entry: nothing. Exit: A $FF, N set, Z clear; Y the entry; C bit 0 of the state less 5, V from
 * that subtract. X kept.
 */
CrashEnd RichterEndItemCrash(void)
{
    CrashEnd end;

    wr8(PlayerByte3BD5, 0);
    end.state = rd8(OBJ_STATE);
    end.index = (uint8_t)((uint8_t)(end.state - CRASH_FIRST_STATE) >> 1);
    wr8(OBJ_FRAME, rd8((uint16_t)(RichterCrashEndFrames + end.index)));
    wr8(OBJ_FIELDS_2EE0, rd8((uint16_t)(RichterCrashEndAnimations + end.index)));
    wr8(OBJ_HURT_TIMER, 0xFF);
    wr8(OBJ_SUBSTATE, 0);
    return end;
}

void crash_end_exit_shim(CrashEnd end)
{
    uint8_t less = (uint8_t)(end.state - CRASH_FIRST_STATE);

    cpu.a = 0xFF;
    cpu.y = end.index;
    abi_nz(0xFF);
    abi_flag(P_C, (less & 1) != 0);
    abi_flag(P_V, ((end.state ^ CRASH_FIRST_STATE) & (end.state ^ less) & 0x80) != 0);
}

void abi_RichterEndItemCrash(void)
{
    crash_end_exit_shim(RichterEndItemCrash());
}

/*
 * @routine 01BD:023 RichterLeapClearingOthers
 * @run 6D:C023
 * WHAT IT DOES: Clears every object but him, starts sound list $60 and script 1, and script $0B
 * unless PlayerByte3BE2 is set or bit 5 of his OBJ_HIT_TAKEN is; then leaps up and back: Y speed
 * $FE80, X speed by facing from RichterLeapSpeeds, animation $22, state 0, OBJ_KIND 1.
 * HOW: ClearObject on 1 to 15, StartListChannelKind2 on channel 6, RichterStartHitScript.
 * WAITS: no.
 * Entry: nothing. Exit: A 1, N and Z from it; X 7; Y twice the facing, C the facing's bit 7; V
 * bit 6 of OBJ_HIT_TAKEN if it was tested, else kept.
 */
Leap RichterLeapClearingOthers(void)
{
    Leap leap = { { 0, 0 }, false, 0 };

    for (uint8_t object = OBJECT_FIRST_OTHER; object < OBJECT_END; object++) {
        jsr_push(0xC028);
        ClearObject(object);
        jsr_pop();
    }
    jsr_push(0xC034);
    StartListChannelKind2(LEAP_LIST, LEAP_CHANNEL);
    jsr_pop();
    jsr_push(0xC039);
    RichterStartHitScript(LEAP_SCRIPT);
    jsr_pop();
    if (rd8(PlayerByte3BE2) == 0) {
        leap.tested = true;
        leap.hit = rd8(OBJ_HIT_TAKEN);
        if (!(leap.hit & LEAP_HIT_KEEPS)) {
            jsr_push(0xC049);
            RichterStartHitScript(LEAP_SCRIPT_UNLESS_HELD);
            jsr_pop();
        }
    }
    wr8(OBJ_VY_LO, LEAP_RISE_LO);
    wr8(OBJ_VY_HI, LEAP_RISE_HI);
    jsr_push(0xC058);
    RichterSetAnimation(ANIM_LEAP);
    jsr_pop();
    leap.speed = speed_by_facing(RichterLeapSpeeds);
    wr8(OBJ_STATE, STATE_STAND);
    wr8(OBJ_KIND, 1);
    return leap;
}

void abi_RichterLeapClearingOthers(void)
{
    Leap leap = RichterLeapClearingOthers();

    abi_exit_speed(leap.speed);
    cpu.x = SCRIPT_SLOT_PLAYER;
    cpu.a = 1;
    abi_nz(1);
    if (leap.tested)
        abi_flag(P_V, (leap.hit & 0x40) != 0);
}

/*
 * @routine 01BE:61D RichterClearOthersKeepWhip
 * @run 6D:CE1D
 * WHAT IT DOES: Clears every object but him, and the whip when its OBJ_KIND is 1; drops the
 * forced presses and PlayerByte3BD8.
 * HOW: ClearObject on 1 to 15, passing over 3 when it is kept.
 * WAITS: no.
 * Entry: nothing. Exit: A the whip's OBJ_KIND; X $10; Z and C set, N clear. Y and V kept.
 */
OthersCleared RichterClearOthersKeepWhip(void)
{
    OthersCleared cleared = { 0 };

    for (uint8_t object = OBJECT_FIRST_OTHER; object < OBJECT_END; object++) {
        if (object == OBJECT_WHIP) {
            cleared.whip = rd8((uint16_t)(OBJ_KIND + OBJECT_WHIP));
            if (cleared.whip == WHIP_KEPT)
                continue;
        }
        jsr_push(0xCE2E);
        ClearObject(object);
        jsr_pop();
    }
    wr8(PlayerByte3BD8, 0);
    wr8(PadForcedPress, 0);
    wr8(PadForcedPressB, 0);
    return cleared;
}

void abi_RichterClearOthersKeepWhip(void)
{
    cpu.a = RichterClearOthersKeepWhip().whip;
    cpu.x = OBJECT_END;
    abi_nz(0);
    abi_flag(P_C, true);
}

/*
 * @routine 01BA:389 RichterOnMarkedFloor
 * @run 6D:AB89
 * WHAT IT DOES: Says whether the floor under him is marked: once he is $10 down the screen, a
 * column read with bit 5, or with bit 2 while down is held.
 * HOW: OBJ_Y_LO; RichterColumnHit's bits 5 and 2; bit 4 of zPadHeld.
 * WAITS: no.
 * Entry: nothing. Exit: C set if marked. Too high, A his Y, N and Z from comparing it with $10.
 * No bit, A 0, Z set. Bit 5, A $20, Z clear. Bit 2, A 0, Z clear if down is held, N and V the
 * pad's bits 7 and 6. X, Y kept; V kept but for the pad's test.
 */
MarkedFloor RichterOnMarkedFloor(void)
{
    MarkedFloor floor = { FLOOR_TOO_HIGH, 0, 0 };

    floor.value = rd8(OBJ_Y_LO);
    if (floor.value < FLOOR_MARK_FROM)
        return floor;
    floor.value = (uint8_t)(rd8(RichterColumnHit) & FLOOR_MARKS);
    floor.how = FLOOR_UNMARKED;
    if (floor.value == 0)
        return floor;
    floor.value &= FLOOR_MARK_ALWAYS;
    floor.how = FLOOR_MARKED;
    if (floor.value != 0)
        return floor;
    floor.held = rd8(zPadHeld);
    floor.how = (floor.held & PAD_DOWN) ? FLOOR_MARKED_DOWN : FLOOR_NOT_DOWN;
    return floor;
}

void abi_RichterOnMarkedFloor(void)
{
    MarkedFloor floor = RichterOnMarkedFloor();

    cpu.a = floor.value;
    switch (floor.how) {
    case FLOOR_TOO_HIGH:
        abi_nz((uint8_t)(floor.value - FLOOR_MARK_FROM));
        break;
    case FLOOR_UNMARKED:
    case FLOOR_MARKED:
        abi_nz(floor.value);
        break;
    default:
        abi_flag(P_Z, floor.how == FLOOR_NOT_DOWN);
        abi_flag(P_N, (floor.held & 0x80) != 0);
        abi_flag(P_V, (floor.held & 0x40) != 0);
        break;
    }
    abi_flag(P_C, floor.how == FLOOR_MARKED || floor.how == FLOOR_MARKED_DOWN);
}

/*
 * @routine 01DE:226 MariaOnMarkedFloor
 * @run 6D:AA26
 * WHAT IT DOES: RichterOnMarkedFloor in Maria's player overlay (01DD), the same bytes.
 * HOW: RichterOnMarkedFloor's body.
 * WAITS: no.
 * Entry, exit: as RichterOnMarkedFloor.
 */
MarkedFloor MariaOnMarkedFloor(void)
{
    return RichterOnMarkedFloor();
}

void abi_MariaOnMarkedFloor(void)
{
    MarkedFloor floor = MariaOnMarkedFloor();

    cpu.a = floor.value;
    switch (floor.how) {
    case FLOOR_TOO_HIGH:
        abi_nz((uint8_t)(floor.value - FLOOR_MARK_FROM));
        break;
    case FLOOR_UNMARKED:
    case FLOOR_MARKED:
        abi_nz(floor.value);
        break;
    default:
        abi_flag(P_Z, floor.how == FLOOR_NOT_DOWN);
        abi_flag(P_N, (floor.held & 0x80) != 0);
        abi_flag(P_V, (floor.held & 0x40) != 0);
        break;
    }
    abi_flag(P_C, floor.how == FLOOR_MARKED || floor.how == FLOOR_MARKED_DOWN);
}

/*
 * @routine 01BA:3CB RichterFallGravity
 * @run 6D:ABCB
 * WHAT IT DOES: Pulls him down while he falls: $38 more Y speed a frame, held at $0700.
 * HOW: OBJ_VY_LO and Hi; at 7 and up the low byte is cleared.
 * WAITS: no.
 * Entry: nothing. Exit: A the speed's high byte, V from its add; rising, N, Z and C from the add;
 * falling, N, Z and C from comparing it with 7. X and Y kept.
 */
static Sum fall_gravity(uint8_t pull)
{
    Sum low = add_bytes(rd8(OBJ_VY_LO), pull, false);
    Sum high;

    wr8(OBJ_VY_LO, low.value);
    high = add_bytes(rd8(OBJ_VY_HI), 0, low.carry);
    wr8(OBJ_VY_HI, high.value);
    if (!(high.value & 0x80) && high.value >= FALL_LIMIT)
        wr8(OBJ_VY_LO, 0);
    return high;
}

Sum RichterFallGravity(void)
{
    return fall_gravity(FALL_PULL);
}

static void fall_gravity_exit_shim(Sum high)
{
    cpu.a = high.value;
    abi_flag(P_V, high.overflow);
    if (high.value & 0x80) {
        abi_nz(high.value);
        abi_flag(P_C, high.carry);
        return;
    }
    abi_nz((uint8_t)(high.value - FALL_LIMIT));
    abi_flag(P_C, high.value >= FALL_LIMIT);
}

void abi_RichterFallGravity(void)
{
    fall_gravity_exit_shim(RichterFallGravity());
}

/*
 * @routine 01DE:20B MariaFallGravity
 * @run 6D:AA0B
 * WHAT IT DOES: RichterFallGravity in Maria's player overlay (01DD): $24 more Y speed a frame, not $38, held at $0700.
 * HOW: RichterFallGravity's body with her pull.
 * Untraced: the hold at $0700 ($AA22); no script let her fall that fast.
 * WAITS: no.
 * Entry, exit: as RichterFallGravity.
 */
Sum MariaFallGravity(void)
{
    return fall_gravity(MARIA_FALL_PULL);
}

void abi_MariaFallGravity(void)
{
    fall_gravity_exit_shim(MariaFallGravity());
}

/*
 * @routine 01BA:73A RichterCrashFrame
 * @run 6D:AF3A
 * WHAT IT DOES: The frame an item crash shows: by which pair of crash states he is in, and by
 * the attack's kind.
 * HOW: RichterCrashFrames: states 5 and 6 at 0, 7 and 8 at 4, then PlayerByte3BD5.
 * WAITS: no.
 * Entry: nothing. Exit: A the frame, N and Z from it, X the entry, C and V from adding the kind.
 * Y kept.
 */
CrashFrame RichterCrashFrame(void)
{
    CrashFrame frame;
    uint8_t pair = (uint8_t)(((uint8_t)(rd8(OBJ_STATE) - CRASH_FIRST_STATE) & 0xFE) << 1);

    frame.entry = add_bytes(pair, rd8(PlayerByte3BD5), false);
    frame.frame = rd8((uint16_t)(RichterCrashFrames + frame.entry.value));
    return frame;
}

void abi_RichterCrashFrame(void)
{
    CrashFrame frame = RichterCrashFrame();

    cpu.a = frame.frame;
    cpu.x = frame.entry.value;
    abi_nz(frame.frame);
    abi_flag(P_C, frame.entry.carry);
    abi_flag(P_V, frame.entry.overflow);
}

/*
 * @routine 01BA:62F RichterMoveStopped
 * @run 6D:AE2F
 * WHAT IT DOES: Moves him along X, stops his animation and sets the state's step back to 0.
 * HOW: A added to OBJ_X_LO; OBJ_HURT_TIMER $FF; OBJ_SUBSTATE 0.
 * WAITS: no.
 * Entry: A the move. Exit: A $FF, X the facing, N and Z from it, C and V from the add. Y kept.
 */
Sum RichterMoveStopped(uint8_t move)
{
    Sum moved = add_bytes(move, rd8(OBJ_X_LO), false);

    wr8(OBJ_X_LO, moved.value);
    wr8(OBJ_HURT_TIMER, 0xFF);
    wr8(OBJ_SUBSTATE, 0);
    return moved;
}

void abi_RichterMoveStopped(void)
{
    Sum moved = RichterMoveStopped(cpu.a);

    cpu.a = 0xFF;
    cpu.x = rd8(OBJ_FACING_LEFT);
    abi_nz(cpu.x);
    abi_flag(P_C, moved.carry);
    abi_flag(P_V, moved.overflow);
}

/*
 * @routine 01BB:73D RichterInvincibleTick
 * @run 6D:B73D
 * WHAT IT DOES: Runs the invincibility a potion gives: while its timer runs, every fourth frame
 * he is kept unhittable and the timer counts down, with a sound as it starts and another as it
 * wears off, the flashing scripts at set marks, and his hit timer and flash back to 1 at the end.
 * HOW: PlayerInvincibleTimer; zWaveCounter's low two bits pick the frame. The marks and their
 * scripts are two tables of six. OBJ_ANIM_TIMER $80 holds hits off; OBJ_3828 2 while it runs.
 * WAITS: no.
 * Entry: nothing. Exit: off, A 0, Z set. Not this frame, A the frame's bits, N and Z from them.
 * Otherwise X, Y, C and V as a sound command left them, C from comparing A with $10 when the
 * second sound was dropped or not made. Ended, A 1, N and Z clear. A mark, as
 * RichterStartHitScript with its script, C set. No mark, A the timer, Y 6, Z and C set, N clear.
 */
/* A character's overlay's invincibility tick: its two tables, the returns of its two sound calls and its script start. */
typedef struct {
    uint16_t marks, scripts, first_return, second_return;
    void (*start_script)(uint8_t script);
} InvincibleLayout;

static InvincibleTick invincible_tick(uint8_t x, uint8_t y, const InvincibleLayout *layout)
{
    InvincibleTick tick = { INVINCIBLE_OFF, 0, 0, false, false, { 0 }, { 0 }, 0, layout->scripts };

    tick.value = rd8(PlayerInvincibleTimer);
    if (tick.value == 0)
        return tick;
    wr8(PlayerByte3BE8, 0);
    wr8(PlayerByte3BE9, 0);
    wr8(OBJ_3828, INVINCIBLE_FLASH);
    tick.value = (uint8_t)(rd8(zWaveCounter) & INVINCIBLE_FRAME_BITS);
    tick.how = INVINCIBLE_OFF_FRAME;
    if (tick.value != 0)
        return tick;
    wr8(OBJ_ANIM_TIMER, INVINCIBLE_HIT_TIMER);
    /* The second sound is asked of what the first one's call leaves, not of the timer. */
    tick.compared = rd8(PlayerInvincibleTimer);
    if (tick.compared == INVINCIBLE_START) {
        tick.first_sound = true;
        jsr_push(layout->first_return);
        tick.first = SoundCommand(SOUND_INVINCIBLE_START, x, y);
        jsr_pop();
        tick.compared = tick.first.taken ? tick.first.mpr4 : SOUND_INVINCIBLE_START;
    }
    if (tick.compared == INVINCIBLE_ENDING) {
        tick.second_sound = true;
        jsr_push(layout->second_return);
        tick.second = SoundCommand(SOUND_INVINCIBLE_ENDING, x, y);
        jsr_pop();
    }
    tick.value = (uint8_t)(rd8(PlayerInvincibleTimer) - 1);
    wr8(PlayerInvincibleTimer, tick.value);
    if (tick.value == 0) {
        wr8(OBJ_ANIM_TIMER, 1);
        wr8(OBJ_3828, 1);
        tick.how = INVINCIBLE_ENDED;
        return tick;
    }
    tick.how = INVINCIBLE_RUNNING;
    for (tick.mark = 0; tick.mark < INVINCIBLE_MARKS; tick.mark++) {
        uint8_t timer = rd8(PlayerInvincibleTimer);    /* LDA, then the CMP's read of the mark */

        if (timer == rd8((uint16_t)(layout->marks + tick.mark))) {
            tick.how = INVINCIBLE_MARK;
            layout->start_script(rd8((uint16_t)(layout->scripts + tick.mark)));
            return tick;
        }
    }
    return tick;
}

/* Richter's: tables at $B798 and $B792; Maria's $329 above them, her script start at $A359. */
static const InvincibleLayout richter_invincible = {
    RichterInvincibleMarks, RichterInvincibleScripts, 0xB766, 0xB76F, RichterStartHitScript
};
static const InvincibleLayout maria_invincible = {
    RichterInvincibleMarks + 0x329, RichterInvincibleScripts + 0x329, 0xBA8F, 0xBA98, MariaStartHitScript
};

InvincibleTick RichterInvincibleTick(uint8_t x, uint8_t y)
{
    return invincible_tick(x, y, &richter_invincible);
}

void invincible_exit_shim(InvincibleTick tick)
{
    if (tick.how == INVINCIBLE_OFF || tick.how == INVINCIBLE_OFF_FRAME) {
        cpu.a = tick.value;
        abi_nz(tick.value);
        return;
    }
    if (tick.first_sound)
        sound_command_exit_shim(SOUND_INVINCIBLE_START, tick.first);
    abi_flag(P_C, tick.compared >= INVINCIBLE_ENDING);
    if (tick.second_sound)
        sound_command_exit_shim(SOUND_INVINCIBLE_ENDING, tick.second);
    switch (tick.how) {
    case INVINCIBLE_ENDED:
        cpu.a = 1;
        abi_nz(1);
        break;
    case INVINCIBLE_MARK:
        hit_script_exit_shim(rd8((uint16_t)(tick.scripts + tick.mark)));
        abi_flag(P_C, true);
        break;
    default:
        cpu.a = tick.value;
        cpu.y = INVINCIBLE_MARKS;
        abi_nz(0);
        abi_flag(P_C, true);
        break;
    }
}

void abi_RichterInvincibleTick(void)
{
    invincible_exit_shim(RichterInvincibleTick(cpu.x, cpu.y));
}

/*
 * @routine 01E0:266 MariaInvincibleTick
 * @run 6D:BA66
 * WHAT IT DOES: RichterInvincibleTick in Maria's player overlay (01DD).
 * HOW: The same instructions with her tables of marks and scripts at $BAC1 and $BABB, $329 above his, and her script start, MariaStartHitScript.
 * WAITS: no.
 * Entry, exit: as RichterInvincibleTick.
 */
InvincibleTick MariaInvincibleTick(uint8_t x, uint8_t y)
{
    return invincible_tick(x, y, &maria_invincible);
}

void abi_MariaInvincibleTick(void)
{
    invincible_exit_shim(MariaInvincibleTick(cpu.x, cpu.y));
}

/*
 * Maria's own player code (her overlay 01DD at $A000): routines with no copy in Richter's overlay. Tables of her
 * overlay are read by address: they have no row in config/rondo/tables.csv yet.
 */
enum {
    MARIA_WALK_SPEEDS = 0xA9C5, MARIA_AIR_SPEEDS = 0xA9C1, PAD_TURN_LEFT = 0x80, PAD_TURN_RIGHT = 0x20,
    MARIA_ANIM_TURN_A = 0x0A, MARIA_ANIM_TURN_B = 0x14
};

/*
 * @routine 01DE:095 MariaPadForcesOff
 * @run 6D:A895
 * WHAT IT DOES: Clears the two bytes that force presses onto her pad.
 * HOW: STZ of PadForcedPress and PadForcedPressB.
 * WAITS: no.
 * Entry: any. Exit: every register and flag kept.
 */
void MariaPadForcesOff(void)
{
    wr8(PadForcedPress, 0);
    wr8(PadForcedPressB, 0);
}

void abi_MariaPadForcesOff(void)
{
    MariaPadForcesOff();
}

/* Her X speed for the way she faces from a table of two words; A the high byte, Y twice the facing, C the facing's bit 7. */
static Ended maria_speed_by_facing(uint16_t table)
{
    uint8_t facing = RD8(OBJ->facing_left[0]);
    uint8_t y = (uint8_t)(facing << 1);
    uint8_t value;

    value = rd8((uint16_t)(table + y));
    WR8(OBJ->vx_lo[0], value);
    value = rd8((uint16_t)(table + 1 + y));
    WR8(OBJ->vx_hi[0], value);
    return ended_with_carry(ended_with_y(ended_load(value), y), (facing & 0x80) != 0);
}

/*
 * @routine 01DE:143 MariaTurnOnPress
 * @run 6D:A943
 * WHAT IT DOES: A press of the way she faces sets her walking speed; a press of the other way turns her round
 * first, counting the turn in zFieldC6.
 * HOW: Bits 5 (right) and 7 (left) of zPadPressed by OBJ_FACING_LEFT; the speed from her table at $A9C5.
 * WAITS: no.
 * Entry: nothing. Exit: no press, A the facing, Z set, N clear, V bit 6 of the pad; otherwise A the speed's high
 * byte, N and Z from it, Y twice the facing, C clear, V bit 6 of the pad. X kept.
 */
EndedTst MariaTurnOnPress(void)
{
    EndedTst r;
    uint8_t facing, pressed;

    facing = RD8(OBJ->facing_left[0]);
    r = ended_tst_none(ended_load(facing));
    if (facing == 0) {
        pressed = RD8(ZP->pad_pressed);
        r = ended_tst(r.end, pressed, PAD_TURN_RIGHT);
        if (!(pressed & PAD_TURN_RIGHT)) {
            pressed = RD8(ZP->pad_pressed);
            r = ended_tst(r.end, pressed, PAD_TURN_LEFT);
            if (!(pressed & PAD_TURN_LEFT)) {
                return r;
            }
            INC8(ZP->field_c6);
            WR8(OBJ->facing_left[0], 1);
        }
    } else {
        pressed = RD8(ZP->pad_pressed);
        r = ended_tst(r.end, pressed, PAD_TURN_LEFT);
        if (!(pressed & PAD_TURN_LEFT)) {
            pressed = RD8(ZP->pad_pressed);
            r = ended_tst(r.end, pressed, PAD_TURN_RIGHT);
            if (!(pressed & PAD_TURN_RIGHT)) {
                return r;
            }
            INC8(ZP->field_c6);
            WR8(OBJ->facing_left[0], 0);
        }
    }
    r.end = ended_then(r.end, maria_speed_by_facing(MARIA_WALK_SPEEDS));
    r.v_only = true;
    return r;
}

void abi_MariaTurnOnPress(void)
{
    abi_ended_tst(MariaTurnOnPress());
}

/*
 * @routine 01DE:17F MariaAirTurn
 * @run 6D:A97F
 * WHAT IT DOES: Her turn in the air: in animations $0A and $14 as MariaTurnOnPress; otherwise a press against the
 * way she moves turns her and sets her air speed, counting the turn in zFieldC6.
 * HOW: OBJ_FIELDS_2EE0; the sign of OBJ_VX_HI picks the press looked for; the speed from her table at $A9C1.
 * Untraced: everything for a speed to the left ($A9A0 to $A9AC), and the return when she faces left while moving
 * right ($A992); no script turned her in the air that way.
 * WAITS: no.
 * Entry: nothing. Exit: as MariaTurnOnPress in those animations, C set; else no turn, A the facing or 0, N and Z
 * from it or the press, C from comparing the animation with $14; a turn, A the speed's high byte, Y twice the
 * facing, C clear; V bit 6 of the pad after a press was looked at. X kept.
 */
EndedTst MariaAirTurn(void)
{
    EndedTst r;
    uint8_t animation, value, pressed;

    animation = RD8(OBJ->fields_2ee0[0]);
    if (animation == MARIA_ANIM_TURN_A || animation == MARIA_ANIM_TURN_B) {
        r = MariaTurnOnPress();
        r.end = ended_then(ended_with_carry(ended_kept(), true), r.end);
        return r;
    }
    r = ended_tst_none(ended_compare(animation, MARIA_ANIM_TURN_B));
    value = RD8(OBJ->vx_hi[0]);
    if (!(value & 0x80)) {
        value = RD8(OBJ->facing_left[0]);
        r.end = ended_then(r.end, ended_load(value));
        if (value != 0) {
            return r;
        }
        pressed = RD8(ZP->pad_pressed);
        r = ended_tst(r.end, pressed, PAD_TURN_LEFT);
        if (!(pressed & PAD_TURN_LEFT)) {
            return r;
        }
        WR8(OBJ->facing_left[0], 1);
    } else {
        value = RD8(OBJ->facing_left[0]);
        r.end = ended_then(r.end, ended_load(value));
        if (value == 0) {
            return r;
        }
        pressed = RD8(ZP->pad_pressed);
        r = ended_tst(r.end, pressed, PAD_TURN_RIGHT);
        if (!(pressed & PAD_TURN_RIGHT)) {
            return r;
        }
        WR8(OBJ->facing_left[0], 0);
    }
    INC8(ZP->field_c6);
    r.end = ended_then(r.end, maria_speed_by_facing(MARIA_AIR_SPEEDS));
    r.v_only = true;
    return r;
}

void abi_MariaAirTurn(void)
{
    abi_ended_tst(MariaAirTurn());
}
