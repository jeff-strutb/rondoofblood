/*
 * Richter's overlay, loaded at $A000 from sector $1B9; Maria has her own at the same
 * addresses, from $1DD. Its jump table is what the kernel and the stage call. Each frame
 * reads the pad, handles damage and runs the handler for the state in OBJ_27A8, whose step
 * is in OBJ_2800.
 *
 * Almost every routine here leaves what the next call sees in A and the flags, so all
 * carry them in an Alu, and X and Y go straight to cpu where the assembly loads them.
 * Code of the overlay with no C yet is called by address.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"

/* The overlay's routines with no C yet. */
enum {
    RH_CLIMB_START = 0xA7C4, RH_STATE_FALL = 0xA85E, RH_STATE_KNOCK = 0xAE5E,
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
    FALL_PULL = 0x38, FALL_LIMIT = 7, DEATH_SUB_STEP = 5, SCRIPT_DEATH = 9, ANIM_LEDGE_CRASH_A = 0x2B,
    ANIM_LEDGE_CRASH_B = 0x2C, LEDGE_CRASH_STEP = 5, LEDGE_JUMP_LO = 0xC0, LEDGE_JUMP_HI = 0xFA, SOUND_HURT = 0x65, ANIM_CRASH_FIVE = 0x2A, ANIM_HURT_AIR = 0x23,
    CRASH_FIVE_TIMER = 0x60, ITEM_CRASH_FIVE = 5, HURT_TIMER = 0xFF, REACT_FILL_D = 0x0D,
    REACT_FILL_B = 0x0B, REACT_E = 0x0E, REACT_10 = 0x10, REACT_11 = 0x11, REACT_12 = 0x12,
    REACT_13 = 0x13, REACT_11_TIMER = 0x60, ITEM_SLOT_A = 5, ITEM_SLOT_B = 8, ITEM_SLOT_C = 11,
    LEAP_LIST = 0x60, LEAP_SCRIPT = 0x01, LEAP_SCRIPT_UNLESS_HELD = 0x0B, LEAP_HIT_KEEPS = 0x20,
    LEAP_RISE_LO = 0x80, LEAP_RISE_HI = 0xFE, ANIM_LEAP = 0x22, LEAP_CHANNEL = 6,
    OBJECT_FIRST_OTHER = 1, OBJECT_END = 0x10, WHIP_KEPT = 1, CRASH_FIRST_STATE = 5, HEALTH_FULL = 0x5C, START_LIVE = 0x01
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
StandSound RichterStartNew(void);
StandSound RichterStartHealth(void);
StandSound RichterStartClear(void);
StandSound RichterStartStand(void);
StandSound RichterStart(void);
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
Alu RichterApplyMoveAndPush(Alu r);
Alu RichterFrame(Alu r);
Alu RichterHitWhilePushed(Alu r);
Alu RichterSpikeBelow(Alu r);
Alu RichterDie(Alu r);
Alu RichterStateFrame(Alu r);
Alu RichterOutOfHealth(Alu r);
Alu RichterStandStep1(Alu r);
Alu RichterStandStep2(Alu r);
Alu RichterStateStand(Alu r);
Alu RichterStandStep0(Alu r);
Alu RichterUpWhipLeft(Alu r);
Alu RichterUpWhipRight(Alu r);
Alu RichterStateWalk(Alu r);
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
    wr8(OBJ_27A8, state);
    wr8(OBJ_2800, step);
}

/* LDA, then the state and the step to go with it. */
static void set_state_alu(Alu *r, uint8_t state, uint8_t step)
{
    alu_put(r, OBJ_27A8, state);
    alu_put(r, OBJ_2800, step);
}

/* Jump table of the overlay: each is one JMP. */

/*
 * @routine 01B9:000 RichterVecFrame
 * @run 6D:A000
 * WHAT IT DOES: Goes to RichterFrame.
 * HOW: One jump.
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
 * Entry, exit: as that code.
 */
void abi_RichterVecFloor(void)
{
    call_original(RH_FLOOR);
}

/*
 * @routine 01B9:02A RichterApplyMoveAndPush
 * @run 6D:A02A
 * WHAT IT DOES: Applies the move and, if nothing in three probes touches, takes the push
 * from $CA into $3BC8 when the push flag says so. Called from outside what has been read
 * of the overlay so far.
 * HOW: Bits $81 of the three bytes at $3BC3 stop it.
 * Entry: nothing. Exit: as the path taken leaves them.
 */
Alu RichterApplyMoveAndPush(Alu r)
{
    uint8_t byte;

    r = call_with(RH_APPLY_MOVE, r);
    cpu.x = 0;
    do {
        byte = rd8((uint16_t)(0x3BC3 + cpu.x));
        alu_load(&r, (uint8_t)(byte & 0x81));
        if (r.a != 0)
            return r;
        cpu.x++;
        alu_compare(&r, cpu.x, 3);
        r.a = (uint8_t)(byte & 0x81);
    } while (cpu.x != 3);
    alu_tst(&r, 0x01, rd8(zPlayerPush));
    if (r.nz == 0)
        return r;
    byte = rd8(0x20CA);
    alu_load(&r, byte);
    wr8(0x3BC8, byte);
    cpu.x = 0;
    r = call_with(0xBA80, r);
    wr8(0x20CA, 0);
    return r;
}

void abi_RichterApplyMoveAndPush(void)
{
    abi_alu(RichterApplyMoveAndPush(alu_from_cpu()));
}

/*
 * @routine 01B9:027 RichterApplyMoveAndPushGoTo
 * @run 6D:A027
 * WHAT IT DOES: Goes to RichterApplyMoveAndPush.
 * HOW: One jump.
 * Entry, exit: as RichterApplyMoveAndPush.
 */
void abi_RichterApplyMoveAndPushGoTo(void)
{
    abi_RichterApplyMoveAndPush();
}

/*
 * @routine 01B9:04B RichterFrame
 * @run 6D:A04B
 * WHAT IT DOES: One frame of Richter: the pad, forced while OBJ_3510 counts down; the
 * pad's overrides; the state's handler; the animation's sound on a change; its hitbox.
 * HOW: The animation before and after are compared; the hitbox index comes from the table
 * by animation and is four bytes of AnimationHitbox.
 * Entry: nothing. Exit: A the last hitbox byte, N and Z from it, X the hitbox offset; Y, C
 * and V as the last code left them.
 */
Alu RichterFrame(Alu r)
{
    uint8_t byte = rd8(OBJ_3510);
    uint8_t before;
    uint8_t other;
    bool forced_more = false;

    alu_load(&r, byte);
    if (byte != 0) {
        byte = rd8(zPadHeld);
        wr8(zPadPrevious, byte);
        byte = rd8(OBJ_3568);
        r.a = byte;
        wr8(zPadHeld, byte);
        wr8(zPadPressed, byte);
        byte = rd8(OBJ_3510);
        byte = (uint8_t)(byte - 1);
        wr8(OBJ_3510, byte);
        alu_nz(&r, byte);
        forced_more = byte != 0;
    }
    if (!forced_more) {
        wr8(PlayerByte3BF0, 0);
        r = call_with(RH_B611, r);
    }
    byte = rd8(PadForcedPress);
    other = rd8(zPadPressed);
    byte |= other;
    other = rd8(PadForcedPressB);
    byte |= other;
    wr8(zPadPressed, byte);
    before = rd8(OBJ_2A68);
    alu_load(&r, before);
    r = call_with(RH_READ_PAD, r);
    r = RichterHitWhilePushed(r);
    r = RichterStateFrame(r);
    r = call_with(RH_B704, r);
    r = call_with(RH_B7E9, r);
    byte = rd8(OBJ_2A68);
    alu_load(&r, (uint8_t)(before ^ byte));
    if (r.a != 0)
        r = call_abi(abi_RichterAnimationSound, r);
    cpu.x = rd8(OBJ_2A68);
    byte = rd8((uint16_t)(AnimationHitboxIndex + cpu.x));
    r.c = (byte & 0x40) != 0;
    r.c_kept = false;
    cpu.x = (uint8_t)(byte << 2);
    byte = rd8((uint16_t)(AnimationHitbox + cpu.x));
    wr8(OBJ_3930, byte);
    byte = rd8((uint16_t)(AnimationHitbox + 1 + cpu.x));
    wr8(OBJ_39E0, byte);
    byte = rd8((uint16_t)(AnimationHitbox + 2 + cpu.x));
    wr8(OBJ_3988, byte);
    byte = rd8((uint16_t)(AnimationHitbox + 3 + cpu.x));
    alu_load(&r, byte);
    wr8(OBJ_3A38, byte);
    return r;
}

void abi_RichterFrame(void)
{
    abi_alu(RichterFrame(alu_from_cpu()));
}

/*
 * @routine 01B9:0AE RichterHitWhilePushed
 * @run 6D:A0AE
 * WHAT IT DOES: While pushed (bit 0 of zPlayerPush), outside camera mode $18, with no hit
 * timer, not above the map, a spike under the player is a hit of kind 6.
 * HOW: RichterSpikeBelow, then the hit's fields; the state back to standing.
 * Entry: nothing. Exit: as the last step left them.
 */
Alu RichterHitWhilePushed(Alu r)
{
    uint8_t byte = rd8(CameraMode);
    uint8_t other;

    alu_compare(&r, byte, 0x18);
    if (byte == 0x18)
        return r;
    alu_tst(&r, 0x01, rd8(zPlayerPush));
    if (r.nz == 0)
        return r;
    byte = rd8(OBJ_3460);
    other = rd8(PlayerMayLeaveTop);
    alu_load(&r, (uint8_t)(byte | other));
    if (r.a != 0)
        return r;
    r = RichterSpikeBelow(r);
    if (r.nz == 0)
        return r;
    alu_put(&r, OBJ_33B0, 6);
    alu_put(&r, OBJ_32A8, 0x10);
    wr8(OBJ_27A8, 0);
    return r;
}

void abi_RichterHitWhilePushed(void)
{
    abi_alu(RichterHitWhilePushed(alu_from_cpu()));
}

/*
 * @routine 01B9:0D5 RichterSpikeBelow
 * @run 6D:A0D5
 * WHAT IT DOES: Probes three points at the player's feet for a tile with bit 1 set.
 * HOW: Each point's X from $B6F9 pulled one pixel in, Y from the table by state; the
 * terrain from $B6D2.
 * Entry: nothing. Exit: on finding one, A bit 1, N and Z from it, X the point. Else A
 * zero, X 3, N, Z and C from comparing X with 3.
 */
Alu RichterSpikeBelow(Alu r)
{
    uint8_t byte;
    uint8_t other;

    cpu.x = 0;
    do {
        r = call_with(RH_PROBE_B6F9, r);
        if (r.nz != 0)
            r.a = (r.nz & 0x80) ? (uint8_t)(r.a + 1) : (uint8_t)(r.a - 1);
        byte = rd8(OBJ_X_LO);
        r = adc8(r.a, byte, false);
        wr8(zTemp00, r.a);
        byte = rd8(OBJ_Y_LO);
        cpu.y = rd8(OBJ_27A8);
        other = rd8((uint16_t)(PlayerFootYByState + cpu.y));
        r = adc8(byte, other, false);
        r = call_with(RH_PROBE_B6D2, r);
        alu_load(&r, (uint8_t)(r.a & 0x02));
        if (r.a != 0)
            return r;
        cpu.x++;
        byte = r.a;
        alu_compare(&r, cpu.x, 3);
        r.a = byte;
    } while (cpu.x != 3);
    return r;
}

void abi_RichterSpikeBelow(void)
{
    abi_alu(RichterSpikeBelow(alu_from_cpu()));
}

/*
 * @routine 01B9:10F RichterStart
 * @run 6D:A10F
 * WHAT IT DOES: Starts Richter as zField8A says.
 * HOW: The table of four entries: a new run, the health, the stand, the clear; each goes on
 * into the next. zField8A is 0 to 3; the C takes anything past 2 as the clear, where the table
 * would send it elsewhere.
 * Entry: nothing. Exit: as RichterAnimationSound, and C as RichterStand left it.
 */
StandSound RichterStart(void)
{
    uint8_t how = rd8(zField8A);
    uint16_t entry = (uint16_t)(PlayerStartHandlers + (uint8_t)(how << 1));

    /* The jump reads its entry; each start runs on into the next, so all end standing. */
    rd8(entry);
    rd8((uint16_t)(entry + 1));
    switch (how) {
    case START_NEW_RUN:
        return RichterStartNew();
    case START_HEALTH:
        return RichterStartHealth();
    case START_STAND:
        return RichterStartStand();
    default:
        return RichterStartClear();
    }
}

void abi_RichterStart(void)
{
    stand_sound_exit_shim(RichterStart());
}

/*
 * @routine 01B9:11E RichterClearWork
 * @run 6D:A11E
 * WHAT IT DOES: Clears the player's work bytes and zero page.
 * HOW: Two block moves from the kernel's zeros.
 * Entry: nothing. Exit: all registers and flags kept.
 */
void RichterClearWork(void)
{
    tai(KernelZeros, 0x3BC0, 0x0040);
    tai(KernelZeros, 0x20C0, 0x0019);
}

void abi_RichterClearWork(void)
{
    RichterClearWork();
}

/*
 * @routine 01B9:12D RichterClearKeepPlace
 * @run 6D:A12D
 * WHAT IT DOES: Clears object 0, the player, keeping where it is and which way it faces.
 * HOW: The three bytes on the stack around ClearObject.
 * Entry: nothing. Exit: A the X position, N and Z from it, X 0. Y, C and V kept.
 */
uint8_t RichterClearKeepPlace(void)
{
    uint8_t x = rd8(OBJ_X_LO);
    uint8_t y = rd8(OBJ_Y_LO);
    uint8_t facing = rd8(OBJ_2AC0);

    ClearObject(OBJECT_PLAYER);
    wr8(OBJ_2AC0, facing);
    wr8(OBJ_Y_LO, y);
    wr8(OBJ_X_LO, x);
    return x;
}

void abi_RichterClearKeepPlace(void)
{
    uint8_t x = RichterClearKeepPlace();

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
 * Entry: nothing. Exit: as RichterAnimationSound, and C as RichterStand left it.
 */
StandSound RichterStartNew(void)
{
    wr8(zField93, 0);
    wr8(zField94, START_FIELD94);
    return RichterStartHealth();
}

void abi_RichterStartNew(void)
{
    stand_sound_exit_shim(RichterStartNew());
}

/*
 * @routine 01B9:150 RichterStartHealth
 * @run 6D:A150
 * WHAT IT DOES: Fills the health, then as RichterStartClear.
 * HOW: Runs on into it.
 * Entry: nothing. Exit: as RichterAnimationSound, and C as RichterStand left it.
 */
StandSound RichterStartHealth(void)
{
    wr8(zPlayerHealth, HEALTH_FULL);
    return RichterStartClear();
}

void abi_RichterStartHealth(void)
{
    stand_sound_exit_shim(RichterStartHealth());
}

/*
 * @routine 01B9:154 RichterStartClear
 * @run 6D:A154
 * WHAT IT DOES: Clears the player's work and object, marks it live, gives a frame of the
 * hit timer, then as RichterStartStand.
 * HOW: Runs on into it.
 * Entry: nothing. Exit: as RichterAnimationSound, and C as RichterStand left it.
 */
StandSound RichterStartClear(void)
{
    RichterClearWork();
    RichterClearKeepPlace();
    wr8(OBJ_3250, START_LIVE);
    wr8(OBJ_3828, START_LIVE);
    wr8(OBJ_3460, START_LIVE);
    return RichterStartStand();
}

void abi_RichterStartClear(void)
{
    stand_sound_exit_shim(RichterStartClear());
}

/*
 * @routine 01B9:167 RichterStartStand
 * @run 6D:A167
 * WHAT IT DOES: Stands the player, and runs on into RichterAnimationSound.
 * HOW: PlayerByte3BD6 set, RichterStand.
 * Entry: nothing. Exit: as RichterAnimationSound, and C as RichterStand left it.
 */
StandSound RichterStartStand(void)
{
    StandSound started;

    wr8(PlayerByte3BD6, 1);
    started.stand = RichterStand();
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
 * Entry: nothing. Exit: A 2, N 0, Z 0, X 7, Y the animation. C and V kept.
 */
uint8_t RichterAnimationSound(void)
{
    uint8_t animation = rd8(OBJ_2A68);

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
 * HOW: OBJ_2750 not zero is live. zSubStep 5 ends it.
 * Entry: nothing. Exit: still dying, N and Z from the timer, A, X, Y, C and V kept. An object
 * still out, A its OBJ_2750 and X its slot, N and Z from it, C clear past slot 1. Over, as
 * RichterStartHitScript with script 9, and C set.
 */
DyingStep RichterDyingStep(void)
{
    DyingStep step = { DYING_WAITING, 0, 0 };

    RichterStopX();
    wr8(OBJ_2A68, 0);
    step.value = (uint8_t)(rd8(PlayerDyingTimer) - 1);
    wr8(PlayerDyingTimer, step.value);
    if (step.value != 0)
        return step;
    for (step.object = OBJECT_FIRST_OTHER; step.object < OBJECT_END; step.object++) {
        step.value = rd8((uint16_t)(OBJ_2750 + step.object));
        if (step.value != 0) {
            step.how = DYING_OBJECT_LEFT;
            return step;
        }
        ClearObject(step.object);
    }
    wr8(zSubStep, DEATH_SUB_STEP);
    wr8(PlayerByte3BD7, 0);
    wr8(PlayerDying, 0);
    RichterStartHitScript(SCRIPT_DEATH);
    step.how = DYING_OVER;
    return step;
}

void abi_RichterDyingStep(void)
{
    DyingStep step = RichterDyingStep();

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

/*
 * @routine 01B9:1A4 RichterDie
 * @run 6D:A1A4
 * WHAT IT DOES: Starts the player's death: no health, the timer, the state, the music out
 * and the death's sound.
 * HOW: FadeAndStopSound, then SoundCommand $60.
 * Entry: nothing. Exit: as SoundCommand.
 */
Alu RichterDie(Alu r)
{
    uint8_t byte;

    wr8(zPlayerHealth, 0);
    byte = rd8(PlayerDying);
    byte = (uint8_t)(byte + 1);
    wr8(PlayerDying, byte);
    alu_put(&r, PlayerDyingTimer, 0x60);
    RichterStopX();
    alu_put(&r, OBJ_27A8, STATE_DYING);
    r = call_abi(abi_FadeAndStopSound, r);
    alu_load(&r, 0x60);
    return call_abi(abi_SoundCommand, r);
}

void abi_RichterDie(void)
{
    abi_alu(RichterDie(alu_from_cpu()));
}

/* The state's handler. */
static Alu run_state(Alu r)
{
    uint8_t byte = rd8(OBJ_27A8);
    uint8_t lo;
    uint8_t hi;
    uint16_t handler;

    r.a = (uint8_t)(byte << 1);
    alu_nz(&r, r.a);
    r.c = (byte & 0x80) != 0;
    r.c_kept = false;
    cpu.x = r.a;
    lo = rd8((uint16_t)(PlayerStateHandlers + cpu.x));
    hi = rd8((uint16_t)(PlayerStateHandlers + cpu.x + 1));
    handler = (uint16_t)(lo | (hi << 8));
    switch (handler) {
    case 0xA3D6:
        return RichterStateStand(r);
    case 0xA739:
        return RichterStateWalk(r);
    default:
        return call_with(handler, r);
    }
}

/* From the hit's aftermath on: the blink, a pad cut off above the map, the state. */
static Alu after_hit(Alu r)
{
    uint8_t byte = rd8(PlayerByte3BE2);

    alu_load(&r, byte);
    if (byte != 0) {
        byte = rd8(OBJ_27A8);
        alu_compare(&r, byte, STATE_DYING);
        if (byte != STATE_DYING) {
            byte = rd8(PlayerByte3BE3);
            alu_load(&r, byte);
            if (byte == 0) {
                r.a = 0;
                r = call_abi(abi_RichterStartHitScript, r);
                byte = rd8(PlayerByte3BE3);
                alu_nz(&r, (uint8_t)(byte + 1));
                wr8(PlayerByte3BE3, (uint8_t)(byte + 1));
            }
            byte = rd8(zPadPressed);
            alu_load(&r, byte);
            if (byte == 0)
                return r;
            byte = rd8(PlayerByte3BE3);
            alu_nz(&r, (uint8_t)(byte + 1));
            wr8(PlayerByte3BE3, (uint8_t)(byte + 1));
            byte = rd8(PlayerByte3BE3);
            alu_compare(&r, byte, 8);
            if (byte != 8)
                return r;
            wr8(PlayerByte3BE2, 0);
            alu_load(&r, 9);
            r = call_abi(abi_RichterStartHitScript, r);
        }
    }
    byte = rd8(OBJ_Y_HI);
    alu_compare(&r, byte, 0xFF);
    if (byte == 0xFF) {
        byte = rd8(zPadHeld);
        alu_load(&r, (uint8_t)(byte & 0xF0));
        wr8(zPadHeld, r.a);
        byte = rd8(zPadPressed);
        alu_load(&r, (uint8_t)(byte & 0xF0));
        wr8(zPadPressed, r.a);
    }
    return run_state(r);
}

/* A hit that has been paid for in health. */
static Alu hit_taken(Alu r)
{
    uint8_t byte;

    alu_put(&r, OBJ_3460, 0x30);
    byte = rd8(PlayerByte3BE2);
    alu_load(&r, byte);
    if (byte != 0) {
        alu_load(&r, 1);
        r = call_abi(abi_RichterStartHitScript, r);
        wr8(OBJ_33B0, 0);
    } else {
        r = call_abi(abi_RichterHitReact, r);
    }
    return after_hit(r);
}

/*
 * @routine 01B9:1BE RichterStateFrame
 * @run 6D:A1BE
 * WHAT IT DOES: The state's frame: the death; falling off the map's bottom is death; the
 * hit timer; a hit in OBJ_33B0 taken off the health; then the state's handler.
 * HOW: Y high 1 or $FF low is below the map. A hit of kind 5 is ignored unless
 * PlayerMayLeaveTop is set. Health gone goes to RichterOutOfHealth.
 * Entry: nothing. Exit: as the path taken left them.
 */
Alu RichterStateFrame(Alu r)
{
    uint8_t byte = rd8(PlayerDying);
    uint8_t other;

    alu_load(&r, byte);
    if (byte != 0)
        return call_abi(abi_RichterDyingStep, r);
    byte = rd8(OBJ_Y_HI);
    alu_load(&r, byte);
    if (!(byte & 0x80)) {
        byte = (uint8_t)(byte - 1);
        alu_load(&r, byte);
        if (byte == 0)
            return RichterDie(r);
        byte = rd8(OBJ_Y_LO);
        alu_compare(&r, byte, 0xFF);
        if (byte >= 0xFF)
            return RichterDie(r);
    }
    r = call_with(RH_B73D, r);
    byte = rd8(OBJ_3460);
    alu_load(&r, byte);
    if (byte != 0) {
        wr8(OBJ_33B0, 0);
        byte = rd8(OBJ_3460);
        byte = (uint8_t)(byte - 1);
        wr8(OBJ_3460, byte);
        alu_nz(&r, byte);
        if (byte != 0)
            return after_hit(r);
        byte = rd8(PlayerByte3BE2);
        other = rd8(PlayerByte3BE8);
        byte |= other;
        other = rd8(PlayerByte3BE9);
        alu_load(&r, (uint8_t)(byte | other));
        if (r.a == 0) {
            alu_load(&r, 9);
            r = call_abi(abi_RichterStartHitScript, r);
        }
    }
    byte = rd8(OBJ_33B0);
    alu_load(&r, byte);
    if (byte == 0)
        return after_hit(r);
    byte = rd8(OBJ_33B0);
    alu_load(&r, (uint8_t)(byte & 0x07));
    alu_compare(&r, r.a, 5);
    if (r.a == 5) {
        byte = rd8(PlayerMayLeaveTop);
        alu_load(&r, byte);
        if (byte == 0) {
            wr8(OBJ_33B0, 0);
            return after_hit(r);
        }
    }
    byte = rd8(zPlayerHealth);
    other = rd8(OBJ_32A8);
    r = sbc8(byte, other, true);
    wr8(zPlayerHealth, r.a);
    if (!r.c || r.a == 0)
        return RichterOutOfHealth(r);
    return hit_taken(r);
}

void abi_RichterStateFrame(void)
{
    abi_alu(RichterStateFrame(alu_from_cpu()));
}

/*
 * @routine 01B9:298 RichterOutOfHealth
 * @run 6D:A298
 * WHAT IT DOES: The health has run out: with the one-more flag PlayerByte3BF7 at 1 the
 * player keeps one point and takes the hit; otherwise the mode's step 4 ends the life.
 * HOW: SoundCommand $64 on the way out.
 * Entry: nothing. Exit: as the path taken left them.
 */
Alu RichterOutOfHealth(Alu r)
{
    uint8_t byte;

    wr8(zPlayerHealth, 0);
    byte = rd8(PlayerByte3BF7);
    alu_compare(&r, byte, 1);
    if (byte == 1) {
        byte = rd8(zPlayerHealth);
        alu_nz(&r, (uint8_t)(byte + 1));
        wr8(zPlayerHealth, (uint8_t)(byte + 1));
        return hit_taken(r);
    }
    alu_put(&r, zSubStep, 4);
    wr8(PlayerByte3BD7, 0);
    alu_load(&r, 0x64);
    return call_abi(abi_SoundCommand, r);
}

void abi_RichterOutOfHealth(void)
{
    abi_alu(RichterOutOfHealth(alu_from_cpu()));
}

/*
 * @routine 01B9:2B2 RichterHitReact
 * @run 6D:A2B2
 * WHAT IT DOES: Starts the reaction to a hit: the invulnerable frames by the hit's kind,
 * the state kept to come back to, the state set to 9, and the hit's script.
 * HOW: Runs on into RichterStartHitScript with 8.
 * Entry: nothing. Exit: A and Y 8, X 7, N and Z from 8. C and V kept.
 */
void RichterHitReact(void)
{
    uint8_t kind;

    RichterPadOverridesOff();
    kind = (uint8_t)(rd8(OBJ_33B0) & HIT_KINDS);
    wr8(OBJ_3460, rd8((uint16_t)(HitInvulnerableFrames + kind)));
    wr8(OBJ_35C0, rd8(OBJ_27A8));
    wr8(OBJ_3618, rd8(OBJ_2800));
    wr8(OBJ_27A8, STATE_DYING);
    wr8(OBJ_2800, 0);
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
 * Entry: A script. Exit: A and Y the script, X 7, N and Z from the script. C and V kept.
 */
void RichterStartHitScript(uint8_t script)
{
    StartScriptSlotKind1(script, SCRIPT_SLOT_PLAYER);
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
 * @routine 01B9:2E4 RichterTurnOnPress
 * @run 6D:A2E4
 * WHAT IT DOES: Turns the player to face the way just pressed.
 * HOW: OBJ_2AC0 is the facing, 0 right, 1 left.
 * Entry: nothing. Exit: N and V bits 7 and 6 of the presses, Z set if it did not turn; A the
 * facing, or 1 and N and Z from it when it turned left. X, Y and C kept.
 */
TurnPress RichterTurnOnPress(void)
{
    TurnPress turn;

    turn.facing = rd8(OBJ_2AC0);
    turn.pressed = rd8(zPadPressed);
    turn.turned = (turn.pressed & (turn.facing == 0 ? PAD_LEFT : PAD_RIGHT)) != 0;
    if (turn.turned)
        wr8(OBJ_2AC0, turn.facing == 0 ? FACING_LEFT : FACING_RIGHT);
    return turn;
}

void abi_RichterTurnOnPress(void)
{
    TurnPress turn = RichterTurnOnPress();

    cpu.a = turn.facing;
    abi_flag(P_N, (turn.pressed & 0x80) != 0);
    abi_flag(P_V, (turn.pressed & 0x40) != 0);
    abi_flag(P_Z, !turn.turned);
    if (turn.turned && turn.facing == 0) {
        cpu.a = FACING_LEFT;
        abi_nz(FACING_LEFT);
    }
}

/*
 * @routine 01B9:2FD RichterStandStep1
 * @run 6D:A2FD
 * WHAT IT DOES: The standing state's attacking step: moves, falls if nothing is below;
 * turns while the attack is young, and when it ends stands or crouches.
 * HOW: OBJ_FLAGS_2F38 counts the attack's progress; object 3, the whip, ends it.
 * Entry: nothing. Exit: as the path taken left them.
 */
Alu RichterStandStep1(Alu r)
{
    uint8_t byte;

    r = call_with(RH_APPLY_MOVE, r);
    r = call_with(RH_B885, r);
    if (r.c)
        return call_with(RH_A9DD, r);
    r = call_with(RH_BA7A, r);
    byte = rd8(OBJ_FLAGS_2F38);
    alu_compare(&r, byte, 3);
    if (byte < 3)
        r = call_abi(abi_RichterTurnOnPress, r);
    byte = rd8(PlayerByte3BD5);
    alu_load(&r, byte);
    if (byte == 0) {
        byte = rd8((uint16_t)(OBJ_2750 + 3));
        alu_compare(&r, byte, 1);
        if (byte == 1) {
            byte = rd8(OBJ_FLAGS_2F38);
            alu_compare(&r, byte, 6);
            if (byte >= 6) {
                byte = rd8(OBJ_2F90);
                alu_compare(&r, byte, 4);
            } else {
                r = call_with(RH_B868, r);
                if (r.c) {
                    alu_put(&r, OBJ_FIELDS_2EE0, 6);
                    r = call_with(RH_SET_IDLE, r);
                    alu_put(&r, OBJ_27A8, STATE_CROUCH);
                    return r;
                }
            }
        }
    }
    r = call_with(RH_B659, r);
    wr8(PlayerByte3BD5, 0);
    return call_abi(abi_RichterStand, r);
}

void abi_RichterStandStep1(void)
{
    abi_alu(RichterStandStep1(alu_from_cpu()));
}

/*
 * @routine 01B9:34C RichterClearWhip
 * @run 6D:A34C
 * WHAT IT DOES: Clears object 3, the whip.
 * HOW: ClearObject with X 3.
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
 * Entry: nothing. Exit: as RichterToAir.
 */
void abi_RichterOrphanClearWhipAir(void)
{
    abi_RichterClearWhip();
    abi_alu(call_abi(abi_RichterToAir, alu_from_cpu()));
}

/*
 * @routine 01B9:358 RichterStandStep2
 * @run 6D:A358
 * WHAT IT DOES: The standing state's item-crash step: turns with the pad, moves while its
 * timer runs and spawns object $0C in the first free slot from 5; when it ends, stands or
 * falls.
 * HOW: OBJ_30F0 is the timer. Coming off a floor or jumping sets PlayerByte3BD6.
 * Entry: nothing. Exit: as the path taken left them.
 */
Alu RichterStandStep2(Alu r)
{
    uint8_t byte = rd8(OBJ_2AC0);

    alu_load(&r, byte);
    if (byte == 0) {
        alu_tst(&r, PAD_LEFT, rd8(zPadHeld));
        if (r.nz != 0)
            alu_put(&r, OBJ_2AC0, 1);
    } else {
        alu_tst(&r, PAD_RIGHT, rd8(zPadHeld));
        if (r.nz != 0)
            wr8(OBJ_2AC0, 0);
    }
    byte = rd8(OBJ_30F0);
    alu_load(&r, byte);
    if (byte == 0) {
        byte = rd8(PlayerByte3BD6);
        wr8(PlayerByte3BD6, (uint8_t)(byte + 1));
        byte = rd8(OBJ_27A8);
        alu_compare(&r, byte, 0);
        if (byte == 0)
            return call_abi(abi_RichterStand, r);
        return call_abi(abi_RichterEndItemCrash, r);
    }
    byte = rd8(OBJ_30F0);
    wr8(OBJ_30F0, (uint8_t)(byte - 1));
    byte = rd8(OBJ_27A8);
    alu_compare(&r, byte, 0);
    if (byte == 0) {
        r = call_with(RH_APPLY_MOVE, r);
        r = call_with(RH_B885, r);
        if (r.c) {
            byte = rd8(PlayerByte3BD6);
            wr8(PlayerByte3BD6, (uint8_t)(byte + 1));
            return call_abi(abi_RichterToFall, r);
        }
    }
    r = call_with(RH_B87E, r);
    if (r.c) {
        byte = rd8(PlayerByte3BD6);
        wr8(PlayerByte3BD6, (uint8_t)(byte + 1));
        byte = rd8(OBJ_27A8);
        alu_compare(&r, byte, 0);
        if (byte == 0)
            return call_abi(abi_RichterToAir, r);
        alu_compare(&r, byte, 7);
        if (byte < 7)
            return RichterUpWhipLeft(r);
        return RichterUpWhipRight(r);
    }
    r = call_with(RH_SET_IDLE, r);
    cpu.y = 5;
    alu_nz(&r, 5);
    do {
        byte = rd8((uint16_t)(OBJ_2750 + cpu.y));
        alu_load(&r, byte);
        if (byte == 0) {
            alu_load(&r, 0x0C);
            wr8((uint16_t)(OBJ_2750 + cpu.y), 0x0C);
            return r;
        }
        cpu.y++;
        alu_compare(&r, cpu.y, 0x10);
        r.a = byte;
    } while (cpu.y != 0x10);
    return r;
}

void abi_RichterStandStep2(void)
{
    abi_alu(RichterStandStep2(alu_from_cpu()));
}

/*
 * @routine 01B9:3D0 RichterStepIndex
 * @run 6D:A3D0
 * WHAT IT DOES: Gives the state's step, for its caller to index a table of words by.
 * HOW: OBJ_2800, which the wrapper leaves doubled in A and X.
 * Entry: nothing. Exit: A and X the offset, N and Z from it, C the step's bit 7. Y, V kept.
 */
uint8_t RichterStepIndex(void)
{
    return rd8(OBJ_2800);
}

void abi_RichterStepIndex(void)
{
    uint8_t step = RichterStepIndex();
    uint8_t offset = (uint8_t)(step << 1);

    cpu.a = offset;
    cpu.x = offset;
    abi_nz(offset);
    abi_flag(P_C, (step & 0x80) != 0);
}

/*
 * @routine 01B9:3D6 RichterStateStand
 * @run 6D:A3D6
 * WHAT IT DOES: State 0: runs the step's handler.
 * HOW: A table of three by RichterStepIndex.
 * Entry: nothing. Exit: as the handler leaves them.
 */
Alu RichterStateStand(Alu r)
{
    uint8_t lo;
    uint8_t hi;
    uint16_t handler;

    r = call_abi(abi_RichterStepIndex, r);
    lo = rd8((uint16_t)(PlayerStandHandlers + cpu.x));
    hi = rd8((uint16_t)(PlayerStandHandlers + cpu.x + 1));
    handler = (uint16_t)(lo | (hi << 8));
    switch (handler) {
    case 0xA3E1:
        return RichterStandStep0(r);
    case 0xA2FD:
        return RichterStandStep1(r);
    case 0xA358:
        return RichterStandStep2(r);
    default:
        return call_with(handler, r);
    }
}

void abi_RichterStateStand(void)
{
    abi_alu(RichterStateStand(alu_from_cpu()));
}

/*
 * @routine 01B9:3E1 RichterStandStep0
 * @run 6D:A3E1
 * WHAT IT DOES: Standing: moves, then the first thing that applies: nothing below is a
 * fall; a jump; a hit in the air; an attack; a climb; a crouch; down looks down, and left
 * or right start walking.
 * HOW: Each check is a routine leaving its answer in C.
 * Entry: nothing. Exit: as the path taken left them.
 */
Alu RichterStandStep0(Alu r)
{
    r = call_with(RH_APPLY_MOVE, r);
    r = call_with(RH_B885, r);
    if (r.c)
        return call_abi(abi_RichterToFall, r);
    r = call_with(RH_B87E, r);
    if (r.c)
        return call_abi(abi_RichterToAir, r);
    r = call_with(RH_BA7A, r);
    r = call_with(RH_B7FC, r);
    if (r.c)
        return call_abi(abi_RichterToHurtAir, r);
    r = call_with(RH_B929, r);
    if (r.c)
        return call_abi(abi_RichterAttack, r);
    r = call_with(RH_BA41, r);
    if (r.c)
        return call_with(RH_CLIMB_START, r);
    r = call_with(RH_B868, r);
    if (r.c)
        return call_abi(abi_RichterCrouch, r);
    alu_tst(&r, PAD_DOWN, rd8(zPadHeld));
    if (r.nz != 0) {
        alu_put(&r, OBJ_2A68, 0x47);
    } else {
        uint8_t byte = rd8(PlayerByte3BF0);

        alu_load(&r, byte);
        if (byte != 0)
            alu_put(&r, OBJ_2A68, 0x47);
        else
            r = call_with(RH_SET_IDLE, r);
    }
    cpu.y = 0;
    alu_tst(&r, PAD_RIGHT, rd8(zPadHeld));
    if (r.nz == 0) {
        cpu.y = 1;
        alu_nz(&r, 1);
        alu_tst(&r, PAD_LEFT, rd8(zPadHeld));
        if (r.nz == 0)
            return r;
    }
    return call_abi(abi_RichterStartWalk, r);
}

void abi_RichterStandStep0(void)
{
    abi_alu(RichterStandStep0(alu_from_cpu()));
}

/*
 * @routine 01B9:442 RichterStartWalk
 * @run 6D:A442
 * WHAT IT DOES: Starts walking the way the pad says, unless a button stops it; a held jump
 * keeps the facing and walks backward.
 * HOW: Y is the new facing, kept unless bit 1 is held. Animation 0 or $32.
 * Entry: Y the facing. Exit: as RichterSetAnimation; with an attack held, A, X, Y and C kept,
 * N and V the pad's bits 7 and 6, Z clear.
 */
WalkStart RichterStartWalk(uint8_t facing)
{
    WalkStart walk = { false };

    RichterPadOverridesOff();
    if (!(rd8(zPadHeld) & PAD_JUMP))
        wr8(OBJ_2AC0, facing);
    walk.held = rd8(zPadHeld);
    if (walk.held & PAD_ATTACK)
        return walk;
    walk.started = true;
    set_state(STATE_WALKING, 0);
    walk.animation = RichterSetAnimation((rd8(zPadHeld) & PAD_JUMP) ? ANIM_WALK_BACK : ANIM_WALK);
    return walk;
}

void abi_RichterStartWalk(void)
{
    WalkStart walk = RichterStartWalk(cpu.y);

    if (walk.started) {
        animation_exit_shim(walk.animation);
        return;
    }
    /* TST of the attack bit, which is set: N and V from the pad's top bits, Z clear. */
    abi_flag(P_N, (walk.held & 0x80) != 0);
    abi_flag(P_V, (walk.held & 0x40) != 0);
    abi_flag(P_Z, false);
}

/* The X speed for the way the player faces, from a table of words by facing. */
static FacingSpeed speed_by_facing(uint16_t table)
{
    FacingSpeed set;
    uint8_t offset;

    set.facing = rd8(OBJ_2AC0);
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
 * Entry: nothing. Exit: as RichterSetAnimation. X and V kept.
 */
AnimationStep RichterCrouch(void)
{
    AnimationStep animation;

    RichterPadOverridesOff();
    set_state(STATE_CROUCH, CROUCH_STEP);
    animation = RichterSetAnimation(ANIM_CROUCH);
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

/* The attack from its animation on: step 1, then the whip with its sound or the item's object. */
static AttackStart attack_with_animation(uint8_t animation)
{
    AttackStart attack;

    attack.animation = RichterSetAnimation(animation);
    wr8(OBJ_2800, 1);
    attack.kind = rd8(PlayerByte3BD5);
    if (attack.kind == 0) {
        RichterClearWhip();
        wr8(OBJ_2750 + OBJECT_WHIP, 1);
        attack.sound = SoundCommand(SOUND_WHIP);
        return attack;
    }
    attack.item = rd8(zField93);
    attack.tested_b = rd8(OBJ_2750 + ITEM_SLOT_A);
    attack.slot = ITEM_SLOT_A;
    if (attack.tested_b != 0) {
        attack.tested_b = rd8(OBJ_2750 + ITEM_SLOT_B);
        attack.slot = attack.tested_b == 0 ? ITEM_SLOT_B : ITEM_SLOT_C;
    }
    ClearObject(attack.slot);
    wr8((uint16_t)(OBJ_2750 + attack.slot), attack.item);
    return attack;
}

/*
 * @routine 01B9:4CD RichterAttack
 * @run 6D:A4CD
 * WHAT IT DOES: Starts an attack standing: state 0, step 1, the animation by the attack's
 * kind; the whip, with its sound, or the kind's object in a free slot among 5, 8 and $0B.
 * HOW: PlayerByte3BD5 is the kind. zField93 is what goes in the slot. SoundCommand 3 for the whip.
 * Entry: nothing. Exit: Y and C as RichterSetAnimation. The whip, as SoundCommand with command
 * 3 and X 3; an item, A the item, X its slot, N, Z and V from testing the last slot looked at.
 */
AttackStart RichterAttack(void)
{
    RichterPadOverridesOff();
    wr8(PlayerByte3BE1, 0);
    RichterFaceByPad();
    wr8(OBJ_27A8, STATE_STAND);
    RichterStopMoving();
    return attack_with_animation(rd8((uint16_t)(LandAnimations + rd8(PlayerByte3BD5))));
}

/* The whip's sound's exit after X 3; or the item in A and X its slot, N, Z and V from the last test. */
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
    if (attack.slot == ITEM_SLOT_C) {
        abi_flag(P_Z, false);
        abi_flag(P_N, (attack.tested_b & 0x80) != 0);
        abi_flag(P_V, (attack.tested_b & 0x40) != 0);
    } else {
        abi_flag(P_Z, true);
        abi_flag(P_N, false);
        abi_flag(P_V, false);
    }
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
 * Entry: nothing. Exit: A 4, N and Z from it; Y and C as RichterSetAnimation. X and V kept.
 */
AnimationStep RichterToFall(void)
{
    AnimationStep animation;

    set_state(STATE_FALL, 0);
    animation = RichterSetAnimation(ANIM_FALL);
    wr8(OBJ_VY_LO, 0);
    wr8(OBJ_VY_HI, FALL_SPEED_HI);
    wr8(OBJ_34B8, STATE_FALL_34B8);
    RichterStopX();
    RichterPadOverridesOff();
    return animation;
}

void abi_RichterToFall(void)
{
    animation_exit_shim(RichterToFall());
    cpu.a = STATE_FALL_34B8;
    abi_nz(STATE_FALL_34B8);
}

/*
 * @routine 01B9:542 RichterPadOverridesOff
 * @run 6D:A542
 * WHAT IT DOES: Clears the pad's overrides.
 * HOW: Three bytes zeroed.
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
    alu_put(&r, OBJ_2800, 5);
    alu_put(&r, OBJ_30F0, 0x60);
    return r;
}

/*
 * @routine 01B9:54B RichterLedgeHurtA
 * @run 6D:A54B
 * WHAT IT DOES: A hurt jump off a ledge the first way, or item $05's crash.
 * HOW: Nudges off the ledge and to RichterToHurtAir, or animation $2B.
 * Entry: nothing. Exit: item 5, A $60, N and Z from it, Y and C as RichterSetAnimation. Otherwise
 * as RichterToHurtAir after RichterNudgeA.
 */
LedgeHurt RichterLedgeHurtA(void)
{
    LedgeHurt hurt;

    RichterPadOverridesOff();
    hurt.crash = rd8(zField93) == ITEM_CRASH_FIVE;
    if (hurt.crash) {
        hurt.animation = RichterSetAnimation(ANIM_LEDGE_CRASH_A);
        RichterStopMoving();
        wr8(OBJ_2800, LEDGE_CRASH_STEP);
        wr8(OBJ_30F0, CRASH_FIVE_TIMER);
        return hurt;
    }
    hurt.nudge = RichterNudgeA();
    hurt.hurt = RichterToHurtAir();
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
 * Entry: nothing. Exit: item 5, A $60, N and Z from it, Y and C as RichterSetAnimation. Otherwise
 * as RichterToHurtAir after RichterNudgeB.
 */
LedgeHurt RichterLedgeHurtB(void)
{
    LedgeHurt hurt;

    RichterPadOverridesOff();
    hurt.crash = rd8(zField93) == ITEM_CRASH_FIVE;
    if (hurt.crash) {
        hurt.animation = RichterSetAnimation(ANIM_LEDGE_CRASH_B);
        RichterStopMoving();
        wr8(OBJ_2800, LEDGE_CRASH_STEP);
        wr8(OBJ_30F0, CRASH_FIVE_TIMER);
        return hurt;
    }
    hurt.nudge = RichterNudgeB();
    hurt.hurt = RichterToHurtAir();
    return hurt;
}

void abi_RichterLedgeHurtB(void)
{
    ledge_hurt_exit_shim(RichterLedgeHurtB());
}

/* Off a ledge upward: a nudge, then the fall that starts with an up speed. */
static Alu jump_off_ledge(Alu r)
{
    r = call_abi(abi_RichterToAir, r);
    alu_put(&r, OBJ_VY_LO, 0xC0);
    alu_put(&r, OBJ_VY_HI, 0xFA);
    return r;
}

/* Off a ledge with $40 held: only if $B893 says, and into a fall. */
static Alu ledge_fall(Alu r, uint16_t nudges)
{
    uint8_t byte;
    uint8_t position;

    cpu.x = rd8(OBJ_2AC0);
    byte = rd8((uint16_t)(nudges + cpu.x));
    position = rd8(OBJ_X_LO);
    r = adc8(byte, position, false);
    wr8(OBJ_X_LO, r.a);
    wr8(PlayerMayLeaveTop, 0);
    return call_abi(abi_RichterToFall, r);
}

/*
 * @routine 01B9:580 RichterUpWhipLeft
 * @run 6D:A580
 * WHAT IT DOES: Jumps up off a ledge the first way; with button $40 held and $B893's
 * answer, drops off it instead.
 * HOW: Reads the pad; clears PadForcedPressB and zFieldC6.
 * Entry: nothing. Exit: as the path taken left them.
 */
Alu RichterUpWhipLeft(Alu r)
{
    r = call_with(RH_READ_PAD, r);
    wr8(PadForcedPressB, 0);
    wr8(zFieldC6, 0);
    alu_tst(&r, PAD_UP_B, rd8(zPadHeld));
    if (r.nz == 0) {
        r = call_abi(abi_RichterNudgeA, r);
        return jump_off_ledge(r);
    }
    r = call_with(RH_B893, r);
    if (!r.c)
        return r;
    return ledge_fall(r, LedgeNudge);
}

void abi_RichterUpWhipLeft(void)
{
    abi_alu(RichterUpWhipLeft(alu_from_cpu()));
}

/* Moves the player along X by the table's entry for its facing. */
static Nudge nudge_by_facing(uint16_t table)
{
    Nudge nudge;

    nudge.facing = rd8(OBJ_2AC0);
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

/*
 * @routine 01B9:5C4 RichterNudgeBJump
 * @run 6D:A5C4
 * WHAT IT DOES: The second nudge, and the jump up off the ledge.
 * HOW: RichterNudgeB, then as RichterUpWhipLeft's jump.
 * Entry: nothing. Exit: A $FA, N set, Z clear; C, V and Y as RichterToAir after the nudge.
 */
LedgeJump RichterNudgeBJump(void)
{
    LedgeJump jump;

    jump.nudge = RichterNudgeB();
    jump.air = RichterToAir();
    wr8(OBJ_VY_LO, LEDGE_JUMP_LO);
    wr8(OBJ_VY_HI, LEDGE_JUMP_HI);
    return jump;
}

void abi_RichterNudgeBJump(void)
{
    LedgeJump jump = RichterNudgeBJump();

    abi_exit_nudge(jump.nudge);
    air_exit_shim(jump.air);
    cpu.a = LEDGE_JUMP_HI;
    abi_nz(LEDGE_JUMP_HI);
}

/*
 * @routine 01B9:5C8 RichterUpWhipRight
 * @run 6D:A5C8
 * WHAT IT DOES: As RichterUpWhipLeft by the second table.
 * HOW: The same tests.
 * Entry: nothing. Exit: as the path taken left them.
 */
Alu RichterUpWhipRight(Alu r)
{
    r = call_with(RH_READ_PAD, r);
    wr8(PadForcedPressB, 0);
    wr8(zFieldC6, 0);
    alu_tst(&r, PAD_UP_B, rd8(zPadHeld));
    if (r.nz == 0)
        return call_abi(abi_RichterNudgeBJump, r);
    r = call_with(RH_B893, r);
    if (!r.c)
        return r;
    return ledge_fall(r, LedgeNudgeB);
}

void abi_RichterUpWhipRight(void)
{
    abi_alu(RichterUpWhipRight(alu_from_cpu()));
}

/* What each hit reaction spawns. */
/* The slots from 5 filled with one object. */
static void react_fill_slots(uint8_t kind)
{
    for (uint8_t slot = ITEM_SLOT_A; slot < OBJECT_END; slot++)
        wr8((uint16_t)(OBJ_2750 + slot), kind);
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
    wr8(OBJ_2750 + ITEM_SLOT_A, kind);
}

/*
 * @routine 01B9:5F2 RichterToHurtAir
 * @run 6D:A5F2
 * WHAT IT DOES: Hurt jump: state 3, step 2, the jump's speeds, a long hit timer, animation
 * $23, its sound, and a reaction by zField93. Item $05 goes to RichterItemCrashFive.
 * HOW: Table by zField93, 0 to 8; the C takes anything past 8 as 8. SoundCommand $65.
 * Entry: nothing. Exit: item 5 first, as RichterItemCrashFive. Otherwise as the reaction, after
 * A and X twice the item, N and Z from it, C its bit 7, Y as the animation and V as the sound left
 * them.
 * @stop A621
 */
HurtAir RichterToHurtAir(void)
{
    HurtAir hurt;
    uint16_t entry;

    RichterPadOverridesOff();
    hurt.crash = rd8(zField93) == ITEM_CRASH_FIVE;
    if (hurt.crash) {
        hurt.crash_five = RichterItemCrashFive();
        return hurt;
    }
    set_state(STATE_AIR, 2);
    hurt.lift = RichterJumpSpeed();
    wr8(OBJ_3460, HURT_TIMER);
    wr8(OBJ_3250, 0);
    hurt.animation = RichterSetAnimation(ANIM_HURT_AIR);
    hurt.sound = SoundCommand(SOUND_HURT);
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
    case ITEM_CRASH_FIVE: hurt.crash_five = RichterItemCrashFive(); break;
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
    hurt_air_exit_shim(RichterToHurtAir());
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
 * Entry: nothing. Exit: A $60, N and Z from it; C and V as SoundCommand left them.
 */
CrashFive RichterItemCrashFive(void)
{
    CrashFive crash;

    RichterStopMoving();
    crash.animation = RichterSetAnimation(ANIM_CRASH_FIVE);
    crash.sound = SoundCommand(SOUND_HURT);
    set_state(STATE_STAND, 2);
    wr8(OBJ_30F0, CRASH_FIVE_TIMER);
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
    crash_five_exit_shim(RichterItemCrashFive());
}

/*
 * @routine 01B9:661 RichterReactE
 * @run 6D:A661
 * WHAT IT DOES: Puts object $0E in slot 5.
 * HOW: One store.
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
 * Entry: nothing. Exit: A $60, N and Z from it. X, Y, C, V kept.
 */
void RichterReact11(void)
{
    react_slot_5(REACT_11);
    wr8(OBJ_3250, 1);
    wr8(OBJ_3460, REACT_11_TIMER);
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
 * Entry: nothing. Exit: A PlayerByte3BF1, N and Z from it; Y twice the facing, C the facing's
 * bit 7; V as RichterLiftThree. X kept.
 */
Knock RichterKnockBack(void)
{
    Knock knock;

    wr8(PlayerMayLeaveTop, 0);
    wr8(PadForcedPressB, 0);
    wr8(zFieldC6, 0);
    knock.lift = RichterJumpSpeed();
    set_state(STATE_AIR, 0);
    RichterSetAnimation(ANIM_KNOCK);
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
    air.lift = RichterJumpSpeed();
    if (rd8(zPadHeld) & PAD_RIGHT) {
        way = FACING_RIGHT;
    } else if (rd8(zPadHeld) & PAD_LEFT) {
        way = FACING_LEFT;
    } else {
        air.animation = RichterSetAnimation(ANIM_AIR);
        return air;
    }
    air.moving = true;
    facing = rd8(OBJ_2AC0);
    wr8(OBJ_2AC0, way);
    RichterSetAnimation(ANIM_AIR_MOVING);
    air.speed = RichterSpeedJump();
    air.same_way = facing == rd8(OBJ_2AC0);
    wr8(OBJ_2AC0, facing);
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
 * Entry: nothing. Exit: as RichterLiftThree.
 */
Lift RichterJumpSpeed(void)
{
    wr8(PlayerByte3BF1, 0);
    RichterStopMoving();
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

/*
 * @routine 01B9:739 RichterStateWalk
 * @run 6D:A739
 * WHAT IT DOES: State 1, walking: moves, a pixel down onto a slope, falls if nothing is
 * below; then the checks of standing; then keeps walking while the pad says, walking
 * forward or, with the jump held, backward; else stands.
 * HOW: $BAC2's C says there is a floor. The direction held must be the facing's.
 * Entry: nothing. Exit: as the path taken left them.
 */
Alu RichterStateWalk(Alu r)
{
    uint8_t byte;
    uint8_t facing;
    bool held;

    r = call_with(RH_APPLY_MOVE, r);
    r = call_with(RH_BAC2, r);
    if (!r.c) {
        byte = rd8(OBJ_Y_LO);
        alu_nz(&r, (uint8_t)(byte + 1));
        wr8(OBJ_Y_LO, (uint8_t)(byte + 1));
        r = call_with(RH_APPLY_MOVE, r);
        r = call_with(RH_BAC2, r);
        if (!r.c)
            return call_abi(abi_RichterToFall, r);
    }
    r = call_with(RH_B87E, r);
    if (r.c)
        return call_abi(abi_RichterToAir, r);
    r = call_with(RH_BA7A, r);
    r = call_with(RH_B7FC, r);
    if (r.c)
        return call_abi(abi_RichterToHurtAir, r);
    r = call_with(RH_B929, r);
    if (r.c)
        return call_abi(abi_RichterAttack, r);
    r = call_with(RH_BA41, r);
    if (r.c)
        return call_with(RH_CLIMB_START, r);
    r = call_with(RH_B868, r);
    if (r.c)
        return call_abi(abi_RichterCrouch, r);
    r = call_with(RH_SET_IDLE, r);
    facing = rd8(OBJ_2AC0);
    alu_load(&r, facing);
    held = (rd8(zPadHeld) & (facing != 0 ? PAD_LEFT : PAD_RIGHT)) != 0;
    if (held) {
        alu_put(&r, OBJ_FIELDS_2EE0, 0);
        return call_abi(abi_RichterSpeedWalk, r);
    }
    alu_tst(&r, PAD_JUMP, rd8(zPadHeld));
    if (r.nz == 0)
        return call_abi(abi_RichterStand, r);
    facing = rd8(OBJ_2AC0);
    alu_load(&r, facing);
    held = (rd8(zPadHeld) & (facing != 0 ? PAD_RIGHT : PAD_LEFT)) != 0;
    if (!held)
        return call_abi(abi_RichterStand, r);
    alu_put(&r, OBJ_FIELDS_2EE0, 0x32);
    return call_abi(abi_RichterSpeedBackstep, r);
}

void abi_RichterStateWalk(void)
{
    abi_alu(RichterStateWalk(alu_from_cpu()));
}

/*
 * @routine 01B9:7AC RichterStand
 * @run 6D:A7AC
 * WHAT IT DOES: Stands the player: state 0, step 0, animation 1, no speed.
 * HOW: Clears PlayerMayLeaveTop and PlayerByte3BD5 first.
 * Entry: nothing. Exit: as RichterSetAnimation. X and V kept.
 */
AnimationStep RichterStand(void)
{
    AnimationStep animation;

    wr8(PlayerMayLeaveTop, 0);
    wr8(PlayerByte3BD5, 0);
    set_state(STATE_STAND, 0);
    animation = RichterSetAnimation(ANIM_STAND);
    RichterStopMoving();
    return animation;
}

void abi_RichterStand(void)
{
    animation_exit_shim(RichterStand());
}

/*
 * @routine 01BB:180 RichterEndItemCrash
 * @run 6D:B180
 * WHAT IT DOES: Ends an item crash: no attack, the crash's last frame held, the animation stopped,
 * the step back to 0.
 * HOW: States 5 to 8 are the crashes; two of them share each entry of RichterCrashEndFrames and
 * RichterCrashEndAnimations.
 * Entry: nothing. Exit: A $FF, N set, Z clear; Y the entry; C bit 0 of the state less 5, V from
 * that subtract. X kept.
 */
CrashEnd RichterEndItemCrash(void)
{
    CrashEnd end;

    wr8(PlayerByte3BD5, 0);
    end.state = rd8(OBJ_27A8);
    end.index = (uint8_t)((uint8_t)(end.state - CRASH_FIRST_STATE) >> 1);
    wr8(OBJ_2A68, rd8((uint16_t)(RichterCrashEndFrames + end.index)));
    wr8(OBJ_FIELDS_2EE0, rd8((uint16_t)(RichterCrashEndAnimations + end.index)));
    wr8(OBJ_2F90, 0xFF);
    wr8(OBJ_2800, 0);
    return end;
}

void abi_RichterEndItemCrash(void)
{
    CrashEnd end = RichterEndItemCrash();
    uint8_t less = (uint8_t)(end.state - CRASH_FIRST_STATE);

    cpu.a = 0xFF;
    cpu.y = end.index;
    abi_nz(0xFF);
    abi_flag(P_C, (less & 1) != 0);
    abi_flag(P_V, ((end.state ^ CRASH_FIRST_STATE) & (end.state ^ less) & 0x80) != 0);
}

/*
 * @routine 01BD:023 RichterLeapClearingOthers
 * @run 6D:C023
 * WHAT IT DOES: Clears every object but him, starts sound list $60 and script 1, and script $0B
 * unless PlayerByte3BE2 is set or bit 5 of his OBJ_33B0 is; then leaps up and back: Y speed
 * $FE80, X speed by facing from RichterLeapSpeeds, animation $22, state 0, OBJ_2750 1.
 * HOW: ClearObject on 1 to 15, StartListChannelKind2 on channel 6, RichterStartHitScript.
 * Entry: nothing. Exit: A 1, N and Z from it; X 7; Y twice the facing, C the facing's bit 7; V
 * bit 6 of OBJ_33B0 if it was tested, else kept.
 */
Leap RichterLeapClearingOthers(void)
{
    Leap leap = { { 0, 0 }, false, 0 };

    for (uint8_t object = OBJECT_FIRST_OTHER; object < OBJECT_END; object++)
        ClearObject(object);
    StartListChannelKind2(LEAP_LIST, LEAP_CHANNEL);
    RichterStartHitScript(LEAP_SCRIPT);
    if (rd8(PlayerByte3BE2) == 0) {
        leap.tested = true;
        leap.hit = rd8(OBJ_33B0);
        if (!(leap.hit & LEAP_HIT_KEEPS))
            RichterStartHitScript(LEAP_SCRIPT_UNLESS_HELD);
    }
    wr8(OBJ_VY_LO, LEAP_RISE_LO);
    wr8(OBJ_VY_HI, LEAP_RISE_HI);
    RichterSetAnimation(ANIM_LEAP);
    leap.speed = speed_by_facing(RichterLeapSpeeds);
    wr8(OBJ_27A8, STATE_STAND);
    wr8(OBJ_2750, 1);
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
 * WHAT IT DOES: Clears every object but him, and the whip when its OBJ_2750 is 1; drops the
 * forced presses and PlayerByte3BD8.
 * HOW: ClearObject on 1 to 15, passing over 3 when it is kept.
 * Entry: nothing. Exit: A the whip's OBJ_2750; X $10; Z and C set, N clear. Y and V kept.
 */
OthersCleared RichterClearOthersKeepWhip(void)
{
    OthersCleared cleared = { 0 };

    for (uint8_t object = OBJECT_FIRST_OTHER; object < OBJECT_END; object++) {
        if (object == OBJECT_WHIP) {
            cleared.whip = rd8((uint16_t)(OBJ_2750 + OBJECT_WHIP));
            if (cleared.whip == WHIP_KEPT)
                continue;
        }
        ClearObject(object);
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
 * @routine 01BA:3CB RichterFallGravity
 * @run 6D:ABCB
 * WHAT IT DOES: Pulls him down while he falls: $38 more Y speed a frame, held at $0700.
 * HOW: OBJ_VY_LO and Hi; at 7 and up the low byte is cleared.
 * Entry: nothing. Exit: A the speed's high byte, V from its add; rising, N, Z and C from the add;
 * falling, N, Z and C from comparing it with 7. X and Y kept.
 */
Sum RichterFallGravity(void)
{
    Sum low = add_bytes(rd8(OBJ_VY_LO), FALL_PULL, false);
    Sum high;

    wr8(OBJ_VY_LO, low.value);
    high = add_bytes(rd8(OBJ_VY_HI), 0, low.carry);
    wr8(OBJ_VY_HI, high.value);
    if (!(high.value & 0x80) && high.value >= FALL_LIMIT)
        wr8(OBJ_VY_LO, 0);
    return high;
}

void abi_RichterFallGravity(void)
{
    Sum high = RichterFallGravity();

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

/*
 * @routine 01BA:73A RichterCrashFrame
 * @run 6D:AF3A
 * WHAT IT DOES: The frame an item crash shows: by which pair of crash states he is in, and by
 * the attack's kind.
 * HOW: RichterCrashFrames: states 5 and 6 at 0, 7 and 8 at 4, then PlayerByte3BD5.
 * Entry: nothing. Exit: A the frame, N and Z from it, X the entry, C and V from adding the kind.
 * Y kept.
 */
CrashFrame RichterCrashFrame(void)
{
    CrashFrame frame;
    uint8_t pair = (uint8_t)(((uint8_t)(rd8(OBJ_27A8) - CRASH_FIRST_STATE) & 0xFE) << 1);

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
 * HOW: A added to OBJ_X_LO; OBJ_2F90 $FF; OBJ_2800 0.
 * Entry: A the move. Exit: A $FF, X the facing, N and Z from it, C and V from the add. Y kept.
 */
Sum RichterMoveStopped(uint8_t move)
{
    Sum moved = add_bytes(move, rd8(OBJ_X_LO), false);

    wr8(OBJ_X_LO, moved.value);
    wr8(OBJ_2F90, 0xFF);
    wr8(OBJ_2800, 0);
    return moved;
}

void abi_RichterMoveStopped(void)
{
    Sum moved = RichterMoveStopped(cpu.a);

    cpu.a = 0xFF;
    cpu.x = rd8(OBJ_2AC0);
    abi_nz(cpu.x);
    abi_flag(P_C, moved.carry);
    abi_flag(P_V, moved.overflow);
}
