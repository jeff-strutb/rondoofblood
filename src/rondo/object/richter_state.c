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
    START_NEW_RUN = 0, START_HEALTH = 1, START_STAND = 2, START_FIELD94 = 0x10, HEALTH_FULL = 0x5C, START_LIVE = 0x01
};

/* Called before they are defined. */
typedef struct { uint8_t facing; uint8_t high; } FacingSpeed;
typedef struct { uint8_t facing; uint8_t step; uint8_t was; } Nudge;
typedef struct { uint8_t high_before; uint8_t high; bool borrow; } Lift;
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
Alu RichterDyingStep(Alu r);
Alu RichterDie(Alu r);
Alu RichterStateFrame(Alu r);
Alu RichterOutOfHealth(Alu r);
Alu RichterTurnOnPress(Alu r);
Alu RichterStandStep1(Alu r);
Alu RichterStandStep2(Alu r);
Alu RichterStateStand(Alu r);
Alu RichterStandStep0(Alu r);
Alu RichterAttack(Alu r);
Alu RichterLedgeHurtA(Alu r);
Alu RichterLedgeHurtB(Alu r);
Alu RichterUpWhipLeft(Alu r);
Alu RichterNudgeBJump(Alu r);
Alu RichterUpWhipRight(Alu r);
Alu RichterToHurtAir(Alu r);
Alu RichterItemCrashFive(Alu r);
Alu RichterToAir(Alu r);
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
 * WHAT IT DOES: Goes to $C023.
 * HOW: One jump.
 * Entry, exit: as that code.
 */
void abi_RichterVecC023(void)
{
    call_original(RH_C023);
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
 * WHAT IT DOES: Goes to $CE1D.
 * HOW: One jump.
 * Entry, exit: as that code.
 */
void abi_RichterVecCE1D(void)
{
    call_original(RH_CE1D);
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
 * Entry: nothing. Exit: as the last step left them.
 */
Alu RichterDyingStep(Alu r)
{
    uint8_t byte;

    RichterStopX();
    wr8(OBJ_2A68, 0);
    byte = rd8(PlayerDyingTimer);
    byte = (uint8_t)(byte - 1);
    wr8(PlayerDyingTimer, byte);
    alu_nz(&r, byte);
    if (byte != 0)
        return r;
    cpu.x = 1;
    alu_nz(&r, 1);
    do {
        byte = rd8((uint16_t)(OBJ_2750 + cpu.x));
        alu_load(&r, byte);
        if (byte != 0)
            return r;
        r = call_abi(abi_ClearObject, r);
        cpu.x++;
        byte = r.a;
        alu_compare(&r, cpu.x, 0x10);
        r.a = byte;
    } while (cpu.x != 0x10);
    alu_put(&r, zSubStep, 5);
    wr8(PlayerByte3BD7, 0);
    wr8(PlayerDying, 0);
    alu_load(&r, 9);
    return call_abi(abi_RichterStartHitScript, r);
}

void abi_RichterDyingStep(void)
{
    abi_alu(RichterDyingStep(alu_from_cpu()));
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
        return RichterDyingStep(r);
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
 * Entry: nothing. Exit: A the facing or 1, flags as the last test left them. X, Y, C kept.
 */
Alu RichterTurnOnPress(Alu r)
{
    uint8_t facing = rd8(OBJ_2AC0);

    alu_load(&r, facing);
    if (facing == 0) {
        alu_tst(&r, PAD_LEFT, rd8(zPadPressed));
        if (r.nz != 0)
            alu_put(&r, OBJ_2AC0, 1);
    } else {
        alu_tst(&r, PAD_RIGHT, rd8(zPadPressed));
        if (r.nz != 0)
            wr8(OBJ_2AC0, 0);
    }
    return r;
}

void abi_RichterTurnOnPress(void)
{
    abi_alu(RichterTurnOnPress(alu_from_cpu()));
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
        r = RichterTurnOnPress(r);
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
    abi_alu(RichterToAir(alu_from_cpu()));
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
        return call_with(RH_B180, r);
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
            return RichterToAir(r);
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
        return RichterToAir(r);
    r = call_with(RH_BA7A, r);
    r = call_with(RH_B7FC, r);
    if (r.c)
        return RichterToHurtAir(r);
    r = call_with(RH_B929, r);
    if (r.c)
        return RichterAttack(r);
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

/*
 * @routine 01B9:4CD RichterAttack
 * @run 6D:A4CD
 * WHAT IT DOES: Starts an attack standing: state 0, step 1, the animation by the attack's
 * kind; the whip, with its sound, or the kind's object in a free slot among 5, 8 and $0B.
 * HOW: PlayerByte3BD5 is the kind. zField93 is what goes in the slot.
 * Entry: nothing. Exit: as the path taken left them.
 */
Alu RichterAttack(Alu r)
{
    uint8_t byte;

    RichterPadOverridesOff();
    wr8(PlayerByte3BE1, 0);
    r = call_with(RH_B7CA, r);
    alu_put(&r, OBJ_27A8, STATE_STAND);
    RichterStopMoving();
    cpu.x = rd8(PlayerByte3BD5);
    byte = rd8((uint16_t)(LandAnimations + cpu.x));
    r = animation(r, byte);
    alu_put(&r, OBJ_2800, 1);
    byte = rd8(PlayerByte3BD5);
    alu_load(&r, byte);
    if (byte == 0) {
        r = call_abi(abi_RichterClearWhip, r);
        alu_put(&r, (uint16_t)(OBJ_2750 + 3), 1);
        alu_load(&r, 3);
        return call_abi(abi_SoundCommand, r);
    }
    byte = rd8(zField93);
    alu_load(&r, byte);
    alu_tst(&r, 0xFF, rd8((uint16_t)(OBJ_2750 + 5)));
    if (r.nz == 0) {
        cpu.x = 5;
    } else {
        alu_tst(&r, 0xFF, rd8((uint16_t)(OBJ_2750 + 8)));
        cpu.x = r.nz == 0 ? 8 : 0x0B;
    }
    alu_nz(&r, cpu.x);
    r = call_abi(abi_ClearObject, r);
    wr8((uint16_t)(OBJ_2750 + cpu.x), r.a);
    return r;
}

void abi_RichterAttack(void)
{
    abi_alu(RichterAttack(alu_from_cpu()));
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
 * Entry: nothing. Exit: as the path taken left them.
 */
Alu RichterLedgeHurtA(Alu r)
{
    uint8_t byte;

    RichterPadOverridesOff();
    byte = rd8(zField93);
    alu_compare(&r, byte, 5);
    if (byte == 5)
        return item_crash(r, 0x2B);
    r = call_abi(abi_RichterNudgeA, r);
    return RichterToHurtAir(r);
}

void abi_RichterLedgeHurtA(void)
{
    abi_alu(RichterLedgeHurtA(alu_from_cpu()));
}

/*
 * @routine 01B9:55A RichterLedgeHurtB
 * @run 6D:A55A
 * WHAT IT DOES: A hurt jump off a ledge the second way, or item $05's crash.
 * HOW: As RichterLedgeHurtA with the other nudge and animation $2C.
 * Entry: nothing. Exit: as the path taken left them.
 */
Alu RichterLedgeHurtB(Alu r)
{
    uint8_t byte;

    RichterPadOverridesOff();
    byte = rd8(zField93);
    alu_compare(&r, byte, 5);
    if (byte == 5)
        return item_crash(r, 0x2C);
    r = call_abi(abi_RichterNudgeB, r);
    return RichterToHurtAir(r);
}

void abi_RichterLedgeHurtB(void)
{
    abi_alu(RichterLedgeHurtB(alu_from_cpu()));
}

/* Off a ledge upward: a nudge, then the fall that starts with an up speed. */
static Alu jump_off_ledge(Alu r)
{
    r = RichterToAir(r);
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
 * Entry: nothing. Exit: as the jump left them.
 */
Alu RichterNudgeBJump(Alu r)
{
    r = call_abi(abi_RichterNudgeB, r);
    return jump_off_ledge(r);
}

void abi_RichterNudgeBJump(void)
{
    abi_alu(RichterNudgeBJump(alu_from_cpu()));
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
        return RichterNudgeBJump(r);
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
static Alu react_fill_slots(Alu r, uint8_t kind)
{
    alu_load(&r, kind);
    cpu.y = 5;
    alu_nz(&r, 5);
    do {
        wr8((uint16_t)(OBJ_2750 + cpu.y), kind);
        cpu.y++;
        alu_compare(&r, cpu.y, 0x10);
        r.a = kind;
    } while (cpu.y != 0x10);
    return r;
}

static Alu react_slot_5(Alu r, uint8_t kind)
{
    alu_put(&r, (uint16_t)(OBJ_2750 + 5), kind);
    return r;
}

/*
 * @routine 01B9:5F2 RichterToHurtAir
 * @run 6D:A5F2
 * WHAT IT DOES: Hurt jump: state 3, step 2, the jump's speeds, a long hit timer, animation
 * $23, its sound, and a reaction by zField93. Item $05 goes to RichterItemCrashFive.
 * HOW: Table by zField93.
 * Entry: nothing. Exit: as the reaction left them.
 * @stop A621
 */
Alu RichterToHurtAir(Alu r)
{
    uint8_t byte;

    RichterPadOverridesOff();
    byte = rd8(zField93);
    alu_compare(&r, byte, 5);
    if (byte == 5)
        return RichterItemCrashFive(r);
    set_state_alu(&r, STATE_AIR, 2);
    r = call_abi(abi_RichterJumpSpeed, r);
    alu_put(&r, OBJ_3460, 0xFF);
    wr8(OBJ_3250, 0);
    r = animation(r, 0x23);
    alu_load(&r, 0x65);
    r = call_abi(abi_SoundCommand, r);
    byte = rd8(zField93);
    r.a = (uint8_t)(byte << 1);
    alu_nz(&r, r.a);
    r.c = (byte & 0x80) != 0;
    r.c_kept = false;
    cpu.x = r.a;
    {
        uint8_t lo = rd8((uint16_t)(HitReactionHandlers + cpu.x));
        uint8_t hi = rd8((uint16_t)(HitReactionHandlers + cpu.x + 1));

        switch ((uint16_t)(lo | (hi << 8))) {
        case 0xA633: return react_fill_slots(r, 0x0D);
        case 0xA640: return react_fill_slots(r, 0x0B);
        case 0xA644: return RichterItemCrashFive(r);
        case 0xA661: return react_slot_5(r, 0x0E);
        case 0xA667:
            r = react_slot_5(r, 0x11);
            alu_put(&r, OBJ_3250, 1);
            alu_put(&r, OBJ_3460, 0x60);
            return r;
        case 0xA677: return react_slot_5(r, 0x10);
        case 0xA67B: return react_slot_5(r, 0x12);
        case 0xA67F: return react_slot_5(r, 0x13);
        default:
            return call_with((uint16_t)(lo | (hi << 8)), r);
        }
    }
}

void abi_RichterToHurtAir(void)
{
    abi_alu(RichterToHurtAir(alu_from_cpu()));
}

/*
 * @routine 01B9:633 RichterReactFillD
 * @run 6D:A633
 * WHAT IT DOES: Fills the slots from 5 with object $0D.
 * HOW: A loop to $10.
 * Entry: nothing. Exit: A $0D, Y $10, N, Z, C from comparing it.
 */
void abi_RichterReactFillD(void)
{
    abi_alu(react_fill_slots(alu_from_cpu(), 0x0D));
}

/*
 * @routine 01B9:640 RichterReactFillB
 * @run 6D:A640
 * WHAT IT DOES: Fills the slots from 5 with object $0B.
 * HOW: Goes back into the loop.
 * Entry: nothing. Exit: A $0B, Y $10, N, Z, C from comparing it.
 */
void abi_RichterReactFillB(void)
{
    abi_alu(react_fill_slots(alu_from_cpu(), 0x0B));
}

/*
 * @routine 01B9:644 RichterItemCrashFive
 * @run 6D:A644
 * WHAT IT DOES: Item $05's crash, from a hurt jump: no speed, animation $2A, its sound,
 * state 0, step 2, its timer.
 * HOW: The fields in turn.
 * Entry: nothing. Exit: A $60, N and Z from it; C and V as SoundCommand left them.
 */
Alu RichterItemCrashFive(Alu r)
{
    RichterStopMoving();
    r = animation(r, 0x2A);
    alu_load(&r, 0x65);
    r = call_abi(abi_SoundCommand, r);
    set_state_alu(&r, STATE_STAND, 2);
    alu_put(&r, OBJ_30F0, 0x60);
    return r;
}

void abi_RichterItemCrashFive(void)
{
    abi_alu(RichterItemCrashFive(alu_from_cpu()));
}

/*
 * @routine 01B9:661 RichterReactE
 * @run 6D:A661
 * WHAT IT DOES: Puts object $0E in slot 5.
 * HOW: One store.
 * Entry: nothing. Exit: A $0E, N and Z from it. X, Y, C, V kept.
 */
void abi_RichterReactE(void)
{
    abi_alu(react_slot_5(alu_from_cpu(), 0x0E));
}

/*
 * @routine 01B9:667 RichterReact11
 * @run 6D:A667
 * WHAT IT DOES: Puts object $11 in slot 5, and sets the hit timer $60 with the player live.
 * HOW: Three stores.
 * Entry: nothing. Exit: A $60, N and Z from it. X, Y, C, V kept.
 */
void abi_RichterReact11(void)
{
    Alu r = react_slot_5(alu_from_cpu(), 0x11);

    alu_put(&r, OBJ_3250, 1);
    alu_put(&r, OBJ_3460, 0x60);
    abi_alu(r);
}

/*
 * @routine 01B9:677 RichterReact10
 * @run 6D:A677
 * WHAT IT DOES: Puts object $10 in slot 5.
 * HOW: Goes back into RichterReactE's store.
 * Entry: nothing. Exit: A $10, N and Z from it. X, Y, C, V kept.
 */
void abi_RichterReact10(void)
{
    abi_alu(react_slot_5(alu_from_cpu(), 0x10));
}

/*
 * @routine 01B9:67B RichterReact12
 * @run 6D:A67B
 * WHAT IT DOES: Puts object $12 in slot 5.
 * HOW: Goes back into RichterReactE's store.
 * Entry: nothing. Exit: A $12, N and Z from it. X, Y, C, V kept.
 */
void abi_RichterReact12(void)
{
    abi_alu(react_slot_5(alu_from_cpu(), 0x12));
}

/*
 * @routine 01B9:67F RichterReact13
 * @run 6D:A67F
 * WHAT IT DOES: Puts object $13 in slot 5.
 * HOW: Goes back into RichterReactE's store.
 * Entry: nothing. Exit: A $13, N and Z from it. X, Y, C, V kept.
 */
void abi_RichterReact13(void)
{
    abi_alu(react_slot_5(alu_from_cpu(), 0x13));
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
 * HOW: Y is the facing the pad asks for, tried and put back.
 * Entry: nothing. Exit: as the animation's code, or A $80 and its flags.
 */
Alu RichterToAir(Alu r)
{
    uint8_t facing;
    uint8_t now;

    wr8(PlayerMayLeaveTop, 0);
    wr8(PadForcedPressB, 0);
    wr8(zFieldC6, 0);
    set_state_alu(&r, STATE_AIR, 0);
    r = call_abi(abi_RichterJumpSpeed, r);
    cpu.y = 0;
    if (!(rd8(zPadHeld) & PAD_RIGHT)) {
        cpu.y = 1;
        alu_nz(&r, 1);
        if (!(rd8(zPadHeld) & PAD_LEFT))
            return animation(r, 0x0A);
    }
    facing = rd8(OBJ_2AC0);
    alu_load(&r, facing);
    wr8(OBJ_2AC0, cpu.y);
    r = animation(r, 0x02);
    r = call_abi(abi_RichterSpeedJump, r);
    alu_load(&r, facing);
    now = rd8(OBJ_2AC0);
    alu_compare(&r, facing, now);
    wr8(OBJ_2AC0, facing);
    if (facing == now) {
        alu_put(&r, PlayerByte3BF1, 0x80);
        return r;
    }
    return animation(r, 0x0A);
}

void abi_RichterToAir(void)
{
    abi_alu(RichterToAir(alu_from_cpu()));
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
        return RichterToAir(r);
    r = call_with(RH_BA7A, r);
    r = call_with(RH_B7FC, r);
    if (r.c)
        return RichterToHurtAir(r);
    r = call_with(RH_B929, r);
    if (r.c)
        return RichterAttack(r);
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
