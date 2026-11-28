/*
 * Maria's attack and her items (01DD, bank $6D): the attack button's three outcomes (PlayerAttackKind: an attack, an
 * item thrown, her pad sequence's move), the item crash from the ground or in the air and its phases, and the jump
 * and ledge hold the crash ends in.
 *
 * Her attack is an object: slot 3, or 4 when 3 is out, of kind 1, with sound 4. An item thrown is an object of the
 * item's number in slot 5 (8 when 5 is out); item 7 also takes an attack slot and an object of kind 7 in 5 to 14
 * that keeps the attack slot in OBJ_WORK_D; item 4 is a state of its own, $10. The item crash puts the item's crash
 * object in slot 5 (MariaCrashKinds), slows time (TimeSlowMode 2) and runs a frame handler by its kind.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"

enum {
    OBJECT_PLAYER = 0, SLOT_ATTACK = 3, SLOT_ATTACK_2 = 4, SLOT_ITEM = 5, SLOT_ITEM_2 = 8, SLOT_ITEM4_EXTRA = 9,
    SLOT_SEQUENCE = 0x0F, SLOT_ITEM7_END = 0x0F,
    KIND_ATTACK = 1, KIND_ITEM7_PART = 7, KIND_ITEM4 = 4, KIND_ITEM4_EXTRA = 0x0C, KIND_SEQUENCE = 0x0A,
    SOUND_ATTACK = 4, SOUND_THROW = 0x69, SOUND_SEQUENCE = 0x6A,
    ITEM_FOUR = 4, ITEM_SEVEN = 7, ITEM4_HEARTS = 0x50, ITEM4_NONE = 2,
    STATE_STAND = 0, STATE_JUMP = 3, STATE_ITEM4 = 0x10, STATE_SEQUENCE = 0x11, STATE_CLIMB_FIRST = 5,
    STEP_ATTACK = 1, STEP_CRASH = 2, STEP_CRASH_AIR = 5,
    ANIMATION_ATTACK = 0x20, ANIMATION_CRASH = 0x2A, ANIMATION_CRASH_AIR = 0x2B, ANIMATION_SEQUENCE = 0x29,
    ANIMATION_ITEM4 = 0x26, ANIMATION_ITEM4_WALK = 0x27, ANIMATION_CRASH_RISE = 0x2C, ANIMATION_CRASH_LAND = 0x2D,
    ANIMATION_CRASH_TURN = 0x2F, ANIMATION_JUMP_UP = 0x0A, ANIMATION_JUMP_ACROSS = 0x0B,
    CRASH_KIND_BASE = 0x0D, CRASH_TIME_SLOW = 2, CRASH_FRAME_HELD = 0x52, CRASH_SCRIPT = 0x1B, CRASH_SIGNALLED = 2,
    TIMER_STOPPED = 0xFF, RECOVER_TIMER = 0x20, JUMP_VY_LO = 0x80, JUMP_VY_HI = 0xFC, LIFT_STEP = 3,
    ABOVE_THE_MAP = 0xFF, CRASH_RISE_ROWS = 0xF0,
    PAD_RIGHT = 0x20, PAD_LEFT = 0x80,
    CRASH_PHASE_WAIT = 0xA559, CRASH_PHASE_SIGNAL = 0xA56B, CRASH_PHASE_RISE = 0xA58B, CRASH_PHASE_LAND = 0xA5CD,
    CRASH_FRAME_B = 0xA5C0, CRASH_FRAME_A = 0xA54C, CRASH_FRAME_C = 0xA5AD, CRASH_FRAME_END = 0xA507,
    CRASH_FRAME_D = 0xA530
};

#define MariaCrashKinds_TBL MariaCrashKinds
#define MariaCrashSounds_TBL MariaCrashSounds
#define MariaItem4ByState_TBL MariaItem4ByState
#define MariaCrashHandlers_TBL MariaCrashHandlers
#define MariaCrashPhases_TBL MariaCrashPhases
#define MariaCrashPhasesB_TBL MariaCrashPhasesB
#define MariaWalkSpeeds_TBL MariaWalkSpeeds
#define MariaClimbFrames_TBL MariaClimbFrames
#define MariaClimbWork_TBL MariaClimbWork

/* The JMP (table,X) of a dispatch to a word that is none of the dispatcher's own: that routine's C, by its address. */
static void jump_to_routine(uint16_t target, Regs at_jump)
{
    regs_exit(at_jump);
    run_routine(target);
}

/* The X and Y a chain leaves from known registers at its start, for a call made after it that is given them. */
static uint8_t x_after(uint8_t x, uint8_t y, MExit e)
{
    MExit t = mx_none();

    mx_regs(&t, r_y_only(r_x_only(regs_kept(), x), y));
    mx_then(&t, e);
    return mx_x(t);
}

static uint8_t y_after(uint8_t x, uint8_t y, MExit e)
{
    MExit t = mx_none();

    mx_regs(&t, r_y_only(r_x_only(regs_kept(), x), y));
    mx_then(&t, e);
    return mx_y(t);
}

/* LDA #0, STA PadForcedPressB, RTS: the end of a claim ($A3FF). */
static void forced_press_b_off(MExit *e)
{
    WR8(PVARS->forced_press_b, 0);
    mx_regs(e, r_lda(regs_kept(), 0));
}

/* ------------------------------------------------------------------------------------------------ the attack */

/*
 * @routine 01DD:3EB MariaAttackSlot
 * @run 6D:A3EB
 * WHAT IT DOES: Starts her attack's object: the first of slots 3 and 4 that is empty becomes kind 1, with sound 4, and
 * the pad's second forced press is cleared. With both out, nothing.
 * HOW: OBJ_KIND not zero is out; ClearObject, then SoundCommand.
 * WAITS: no.
 * Entry: Y the caller's (SoundCommand is given it). Exit: both out, A slot 4's kind, N and Z from it, X 4. Started,
 * A 0, Z set, N clear; X, Y, C and V as SoundCommand left them (X the slot when the command was dropped).
 */
MariaClaim MariaAttackSlot(uint8_t y)
{
    MariaClaim claim = { mx_none(), false };
    uint8_t slot = SLOT_ATTACK;
    uint8_t kind = RD8(OBJ->kind[SLOT_ATTACK]);

    mx_regs(&claim.e, r_lda(r_ldx(regs_kept(), SLOT_ATTACK), kind));
    if (kind != 0) {
        slot = SLOT_ATTACK_2;
        kind = RD8(OBJ->kind[SLOT_ATTACK_2]);
        mx_regs(&claim.e, r_lda(r_ldx(regs_kept(), SLOT_ATTACK_2), kind));
        if (kind != 0) {
            return claim;
        }
    }
    claim.claimed = true;
    jsr_push(0xA3F5);
    ClearObject(slot);
    jsr_pop();
    WR8(OBJ->kind[slot], KIND_ATTACK);
    jsr_push(0xA3FF);
    mx_sound(&claim.e, SOUND_ATTACK, SoundCommand(SOUND_ATTACK, slot, y));
    jsr_pop();
    forced_press_b_off(&claim.e);
    return claim;
}

void abi_MariaAttackSlot(void)
{
    mx_exit(MariaAttackSlot(cpu.y).e);
}

/*
 * @routine 01DD:41B MariaStandAttack
 * @run 6D:A41B
 * WHAT IT DOES: Puts her in the standing attack: state 0, step 1, animation $20.
 * HOW: MariaSetAnimation between the stores.
 * WAITS: no.
 * Entry: nothing. Exit: A 1, N and Z from it; Y and C as MariaSetAnimation left them; X and V kept.
 */
/*
 * MariaStandAttack from its JSR on ($A422): an animation set, step 1. MariaClimbAttack jumps in here with $20, keeping
 * the state it is in.
 */
MExit stand_attack_animation(uint8_t animation_number)
{
    MExit e = mx_none();
    AnimationStep animation;

    jsr_push(0xA425);
    animation = MariaSetAnimation(animation_number);
    jsr_pop();
    mx_animation(&e, animation);
    WR8(OBJ->substate[OBJECT_PLAYER], STEP_ATTACK);
    mx_regs(&e, r_lda(regs_kept(), STEP_ATTACK));
    return e;
}

MExit MariaStandAttack(void)
{
    MExit e = mx_none();

    WR8(OBJ->state[OBJECT_PLAYER], STATE_STAND);
    mx_regs(&e, r_lda(regs_kept(), ANIMATION_ATTACK));
    mx_then(&e, stand_attack_animation(ANIMATION_ATTACK));
    return e;
}

void abi_MariaStandAttack(void)
{
    mx_exit(MariaStandAttack());
}

/*
 * @routine 01DD:3C0 MariaSequenceMoveStart
 * @run 6D:A3C0
 * WHAT IT DOES: Starts the move of her pad sequence: sound $6A, state $11 step 0, object slot 15 of kind $0A, animation
 * $29, the pad's forced presses off, all speed gone.
 * HOW: SoundCommand, ClearObject, MariaSetAnimation, MariaPadForcesOff, then the tail JMP to MariaStopMoving.
 * WAITS: no.
 * Entry: X and Y the caller's (SoundCommand is given them). Exit: as MariaSetAnimation, after A $29 and X 15.
 */
MExit MariaSequenceMoveStart(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    AnimationStep animation;

    jsr_push(0xA3C5);
    mx_sound(&e, SOUND_SEQUENCE, SoundCommand(SOUND_SEQUENCE, x, y));
    jsr_pop();
    WR8(OBJ->state[OBJECT_PLAYER], STATE_SEQUENCE);
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    jsr_push(0xA3D4);
    ClearObject(SLOT_SEQUENCE);
    jsr_pop();
    WR8(OBJ->kind[SLOT_SEQUENCE], KIND_SEQUENCE);
    mx_regs(&e, r_lda(r_ldx(regs_kept(), SLOT_SEQUENCE), ANIMATION_SEQUENCE));
    jsr_push(0xA3DE);
    animation = MariaSetAnimation(ANIMATION_SEQUENCE);
    jsr_pop();
    mx_animation(&e, animation);
    jsr_push(0xA3E1);
    MariaPadForcesOff();
    jsr_pop();
    MariaStopMoving();
    return e;
}

void abi_MariaSequenceMoveStart(void)
{
    mx_exit(MariaSequenceMoveStart(cpu.x, cpu.y));
}

/*
 * @routine 01DD:42D MariaThrowItem
 * @run 6D:A42D
 * WHAT IT DOES: Throws the item in hand (zField93), with sound $69. Item 7 starts her attack and, with it, an object
 * of kind 7 in the first empty slot of 5 to 14 that keeps the attack's slot in OBJ_WORK_D. Item 4 is a state of its
 * own: state $10 with animation $26 standing or $27 walking (nothing in a state MariaItem4ByState gives 2), no speed,
 * an object of kind 4 in slot 5 and, with $50 hearts or more, one of kind $0C in slot 9. Any other item is an object
 * of the item's number in slot 5, or 8 when 5 is out, and the pad's second forced press is cleared.
 * HOW: SoundCommand, then by item; MariaAttackSlot, ClearObject, MariaSetAnimation and MariaStopMoving.
 * WAITS: no.
 * Entry: X and Y the caller's (SoundCommand is given them). Exit: item 7, as MariaAttackSlot when it started nothing;
 * with no slot from 5 to 14, A slot 14's kind, X 15, Y as MariaAttackSlot left X, Z and C set; else A and Y that X,
 * N and Z from it, X the slot, C clear past slot 5. Item 4, in a state giving 2: A 2, X the state, Z and C set; else
 * X 5 and A zField94, N and Z from it less $50, C clear, or with $50 hearts A $0C, X 9, C set. Other items: A 0, Z
 * set, X the slot, C item 4 or more, V bit 6 of slot 5's kind, Y as SoundCommand left it.
 */
MExit MariaThrowItem(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t item, slot, kind, state, choice, hearts, attack;
    MariaClaim claim;
    AnimationStep animation;

    jsr_push(0xA432);
    mx_sound(&e, SOUND_THROW, SoundCommand(SOUND_THROW, x, y));
    jsr_pop();
    item = RD8(ZP->field93);
    if (item == ITEM_SEVEN) {
        mx_regs(&e, r_cmp(r_lda(regs_kept(), item), item, ITEM_SEVEN));
        jsr_push(0xA453);
        claim = MariaAttackSlot(y_after(x, y, e));
        jsr_pop();
        mx_then(&e, claim.e);
        if (!claim.claimed) {
            return e;
        }
        attack = x_after(x, y, e);
        mx_regs(&e, r_ldy(r_lda(regs_kept(), attack), attack));
        mx_regs(&e, r_ldx(regs_kept(), SLOT_ITEM));
        for (slot = SLOT_ITEM; slot != SLOT_ITEM7_END; slot++) {
            kind = RD8(OBJ->kind[slot]);
            mx_regs(&e, r_lda(regs_kept(), kind));
            if (kind == 0) {
                jsr_push(0xA468);
                ClearObject(slot);
                jsr_pop();
                WR8(OBJ->kind[slot], KIND_ITEM7_PART);
                WR8(OBJ->work_d[slot], attack);
                mx_regs(&e, r_lda(r_x_only(regs_kept(), slot), attack));
                return e;
            }
            mx_regs(&e, r_cmp(r_ldx(regs_kept(), (uint8_t)(slot + 1)), (uint8_t)(slot + 1), SLOT_ITEM7_END));
        }
        return e;
    }
    if (item == ITEM_FOUR) {
        mx_regs(&e, r_cmp(r_lda(regs_kept(), item), item, ITEM_FOUR));
        state = RD8(OBJ->state[OBJECT_PLAYER]);
        choice = TBL8(MariaItem4ByState, state);
        mx_regs(&e, r_lda(r_ldx(regs_kept(), state), choice));
        if (choice == 0) {
            mx_regs(&e, r_lda(regs_kept(), ANIMATION_ITEM4));
        } else {
            mx_regs(&e, r_cmp(regs_kept(), choice, ITEM4_NONE));
            if (choice == ITEM4_NONE) {
                return e;
            }
            mx_regs(&e, r_lda(regs_kept(), ANIMATION_ITEM4_WALK));
        }
        jsr_push(0xA487);
        animation = MariaSetAnimation(choice == 0 ? ANIMATION_ITEM4 : ANIMATION_ITEM4_WALK);
        jsr_pop();
        mx_animation(&e, animation);
        WR8(OBJ->work_d[OBJECT_PLAYER], 0);
        WR8(OBJ->state[OBJECT_PLAYER], STATE_ITEM4);
        WR8(OBJ->substate[OBJECT_PLAYER], 0);
        jsr_push(0xA499);
        ClearObject(SLOT_ITEM);
        jsr_pop();
        WR8(OBJ->kind[SLOT_ITEM], KIND_ITEM4);
        jsr_push(0xA4A1);
        MariaStopMoving();
        jsr_pop();
        hearts = RD8(ZP->field94);
        mx_regs(&e, r_cmp(r_lda(r_ldx(regs_kept(), SLOT_ITEM), hearts), hearts, ITEM4_HEARTS));
        if (hearts < ITEM4_HEARTS) {
            return e;
        }
        jsr_push(0xA4AC);
        ClearObject(SLOT_ITEM4_EXTRA);
        jsr_pop();
        WR8(OBJ->kind[SLOT_ITEM4_EXTRA], KIND_ITEM4_EXTRA);
        mx_regs(&e, r_lda(r_ldx(regs_kept(), SLOT_ITEM4_EXTRA), KIND_ITEM4_EXTRA));
        return e;
    }
    mx_regs(&e, r_cmp(r_lda(regs_kept(), item), item, ITEM_FOUR));
    kind = RD8(OBJ->kind[SLOT_ITEM]);
    mx_regs(&e, r_tst(regs_kept(), 0xFF, kind));
    slot = kind != 0 ? SLOT_ITEM_2 : SLOT_ITEM;
    mx_regs(&e, r_ldx(regs_kept(), slot));
    jsr_push(0xA447);
    ClearObject(slot);
    jsr_pop();
    WR8(OBJ->kind[slot], item);
    forced_press_b_off(&e);
    return e;
}

void abi_MariaThrowItem(void)
{
    mx_exit(MariaThrowItem(cpu.x, cpu.y));
}

/*
 * @routine 01DD:42B MariaAttackAndThrow
 * @run 6D:A42B
 * WHAT IT DOES: The standing attack, then the item thrown.
 * HOW: A BSR to MariaStandAttack, then on into MariaThrowItem.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaThrowItem.
 */
MExit MariaAttackAndThrow(uint8_t x, uint8_t y)
{
    MExit e;

    jsr_push(0xA42D);
    e = MariaStandAttack();
    jsr_pop();
    mx_then(&e, MariaThrowItem(x_after(x, y, e), y_after(x, y, e)));
    return e;
}

void abi_MariaAttackAndThrow(void)
{
    mx_exit(MariaAttackAndThrow(cpu.x, cpu.y));
}

/*
 * @routine 01DD:3E4 MariaAttack
 * @run 6D:A3E4
 * WHAT IT DOES: Does what the attack button said (PlayerAttackKind): her pad sequence's move ($FF), the item thrown
 * (1), or her attack's object (0).
 * HOW: The sign, then zero, of PlayerAttackKind; on into MariaAttackSlot.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaSequenceMoveStart, MariaThrowItem or MariaAttackSlot.
 */
MExit MariaAttack(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t kind = RD8(PVARS->attack_kind);

    mx_regs(&e, r_lda(regs_kept(), kind));
    if (kind & 0x80) {
        mx_then(&e, MariaSequenceMoveStart(x, y));
    } else if (kind != 0) {
        mx_then(&e, MariaThrowItem(x, y));
    } else {
        mx_then(&e, MariaAttackSlot(y).e);
    }
    return e;
}

void abi_MariaAttack(void)
{
    mx_exit(MariaAttack(cpu.x, cpu.y));
}

/*
 * @routine 01DD:40D MariaAttackFromUse
 * @run 6D:A40D
 * WHAT IT DOES: The attack button's press as MariaUseItem left it: her pad sequence's move; the standing attack with
 * the item thrown; or the attack's object, and if it started, the standing attack.
 * HOW: PlayerAttackKind read for its sign and again for zero; a BSR to MariaAttack, whose Z says the object started;
 * on into MariaStandAttack.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaSequenceMoveStart, MariaAttackAndThrow, MariaAttack when nothing started,
 * or MariaStandAttack.
 */
MExit MariaAttackFromUse(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    MExit attack;
    uint8_t kind = RD8(PVARS->attack_kind);

    mx_regs(&e, r_lda(regs_kept(), kind));
    if (kind & 0x80) {
        mx_then(&e, MariaSequenceMoveStart(x, y));
        return e;
    }
    kind = RD8(PVARS->attack_kind);
    mx_regs(&e, r_lda(regs_kept(), kind));
    if (kind != 0) {
        mx_then(&e, MariaAttackAndThrow(x, y));
        return e;
    }
    jsr_push(0xA419);
    attack = MariaAttack(x, y);
    jsr_pop();
    mx_then(&e, attack);
    if (!mx_zero(e)) {
        return e;
    }
    mx_then(&e, MariaStandAttack());
    return e;
}

void abi_MariaAttackFromUse(void)
{
    mx_exit(MariaAttackFromUse(cpu.x, cpu.y));
}

/* ------------------------------------------------------------------------------------------------ the item crash */

/*
 * The crash's start once the state and step are set ($A37B): the animation, no speed, slot 5 the crash's object by
 * item, zCrashKind its kind less $0D, the crash's sound, zCrashPhase and zFieldD2 cleared, time slowed, her hit timer
 * $FF, and the tail JMP to MariaPadForcesOff.
 */
static MExit crash_start(uint8_t animation_number, uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    MExit at_sound = mx_none();
    AnimationStep animation;
    uint8_t item, kind, sound;
    Sum index;

    mx_regs(&e, r_lda(regs_kept(), animation_number));
    jsr_push(0xA37E);
    animation = MariaSetAnimation(animation_number);
    jsr_pop();
    mx_animation(&e, animation);
    jsr_push(0xA381);
    MariaStopMoving();
    jsr_pop();
    jsr_push(0xA386);
    ClearObject(SLOT_ITEM);
    jsr_pop();
    item = RD8(ZP->field93);
    kind = TBL8(MariaCrashKinds, item);
    WR8(OBJ->kind[SLOT_ITEM], kind);
    index = sub_bytes(kind, CRASH_KIND_BASE, true);
    WR8(ZP->crash_kind, index.value);
    sound = TBL8(MariaCrashSounds, index.value);
    mx_regs(&e, r_v(r_c(r_lda(r_ldx(regs_kept(), index.value), sound), index.carry), index.overflow));
    if (sound != 0) {
        at_sound = e;
        jsr_push(0xA39C);
        mx_sound(&e, sound, SoundCommand(sound, index.value, y_after(x, y, at_sound)));
        jsr_pop();
    }
    WR8(ZP->crash_phase, 0);
    WR8(ZP->fieldd2, 0);
    WR8(PVARS->time_slow_mode, CRASH_TIME_SLOW);
    WR8(OBJ->anim_timer[OBJECT_PLAYER], TIMER_STOPPED);
    mx_regs(&e, r_lda(regs_kept(), TIMER_STOPPED));
    MariaPadForcesOff();
    return e;
}

/*
 * @routine 01DD:36F MariaCrashOnGround
 * @run 6D:A36F
 * WHAT IT DOES: Starts her item crash from the ground: state 0 step 2, animation $2A, then the crash's object, kind,
 * sound and slowed time.
 * HOW: The stores, then the crash's start (MariaSetAnimation, MariaStopMoving, ClearObject, SoundCommand when the
 * crash has a sound, the tail JMP to MariaPadForcesOff).
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: A $FF, N and Z from it; X zCrashKind and C and V from its subtract, or X, Y, C
 * and V as SoundCommand left them; Y as MariaSetAnimation left it with no sound.
 */
MExit MariaCrashOnGround(uint8_t x, uint8_t y)
{
    MExit e = mx_none();

    WR8(OBJ->state[OBJECT_PLAYER], STATE_STAND);
    WR8(OBJ->substate[OBJECT_PLAYER], STEP_CRASH);
    mx_then(&e, crash_start(ANIMATION_CRASH, x, y));
    return e;
}

void abi_MariaCrashOnGround(void)
{
    mx_exit(MariaCrashOnGround(cpu.x, cpu.y));
}

/*
 * @routine 01DD:366 MariaCrashInAir
 * @run 6D:A366
 * WHAT IT DOES: Starts her item crash in the air: step 5 of the state she is in, animation $2B, then as
 * MariaCrashOnGround.
 * HOW: The store, then a branch into MariaCrashOnGround's start.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaCrashOnGround.
 */
MExit MariaCrashInAir(uint8_t x, uint8_t y)
{
    WR8(OBJ->substate[OBJECT_PLAYER], STEP_CRASH_AIR);
    return crash_start(ANIMATION_CRASH_AIR, x, y);
}

void abi_MariaCrashInAir(void)
{
    mx_exit(MariaCrashInAir(cpu.x, cpu.y));
}

/*
 * @routine 01DD:522 MariaCrashRecover
 * @run 6D:A522
 * WHAT IT DOES: The end of a crash for her: the item's first use to come again, and no hit timer.
 * HOW: PlayerItemFirstUse 1; OBJ_ANIM_TIMER $20, then 0.
 * WAITS: no.
 * Entry: nothing. Exit: A $20, N and Z from it. X, Y, C and V kept.
 */
Regs MariaCrashRecover(void)
{
    WR8(PVARS->item_first_use, 1);
    WR8(OBJ->anim_timer[OBJECT_PLAYER], RECOVER_TIMER);
    WR8(OBJ->anim_timer[OBJECT_PLAYER], 0);
    return r_lda(regs_kept(), RECOVER_TIMER);
}

void abi_MariaCrashRecover(void)
{
    regs_exit(MariaCrashRecover());
}

/*
 * @routine 01DE:707 MariaClimbHold
 * @run 6D:AF07
 * WHAT IT DOES: Puts her back in her ledge's hold (states 5 to 8): the attack button's outcome cleared, her frame and
 * OBJ_WORK_A by low or high ledge, her animation stopped (OBJ_HURT_TIMER $FF), step 0.
 * HOW: (state - 5) / 2 picks MariaClimbFrames and MariaClimbWork.
 * WAITS: no.
 * Entry: nothing. Exit: A $FF, N and Z from it; Y the ledge's index; C bit 0 of state less 5, V from that subtract.
 * X kept.
 */
Regs MariaClimbHold(void)
{
    uint8_t state, frame, work;
    Sum less;
    uint8_t index;

    WR8(PVARS->attack_kind, 0);
    state = RD8(OBJ->state[OBJECT_PLAYER]);
    less = sub_bytes(state, STATE_CLIMB_FIRST, true);
    index = (uint8_t)(less.value >> 1);
    frame = TBL8(MariaClimbFrames, index);
    WR8(OBJ->frame[OBJECT_PLAYER], frame);
    work = TBL8(MariaClimbWork, index);
    WR8(OBJ->work_a[OBJECT_PLAYER], work);
    WR8(OBJ->hurt_timer[OBJECT_PLAYER], TIMER_STOPPED);
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    return r_v(r_c(r_lda(r_y_only(regs_kept(), index), TIMER_STOPPED), (less.value & 1) != 0), less.overflow);
}

void abi_MariaClimbHold(void)
{
    regs_exit(MariaClimbHold());
}

/* The crash's end once its animation has run ($A50F): MariaCrashRecover, then standing, or her ledge's hold. */
static MExit crash_end(void)
{
    MExit e = mx_none();
    uint8_t state;

    jsr_push(0xA511);
    mx_regs(&e, MariaCrashRecover());
    jsr_pop();
    state = RD8(OBJ->state[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), state));
    if (state == STATE_STAND) {
        WR8(PVARS->may_leave_top, 0);
        AnimThen stand = MariaStand();
        mx_animation(&e, stand.animation);
        mx_regs(&e, stand.regs);
        return e;
    }
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    mx_regs(&e, MariaClimbHold());
    return e;
}

/*
 * @routine 01DD:507 MariaCrashEnd
 * @run 6D:A507
 * WHAT IT DOES: The crash frame of kinds 4 and 5: time back to normal; when her animation has stopped, the crash's end
 * (MariaCrashRecover, then standing in state 0, or her ledge's hold).
 * HOW: MariaAnimationRunning's Z; the tail JMPs to MariaStand and MariaClimbHold.
 * WAITS: no.
 * Entry: nothing. Exit: running, as MariaAnimationRunning; else as MariaStand, or MariaClimbHold.
 */
MExit MariaCrashEnd(void)
{
    MExit e = mx_none();
    AnimationStep running;

    WR8(PVARS->time_slow_mode, 0);
    jsr_push(0xA50D);
    running = MariaAnimationRunning();
    jsr_pop();
    mx_animation(&e, running);
    if (!mx_zero(mx_one_animation(running))) {
        return e;
    }
    mx_then(&e, crash_end());
    return e;
}

void abi_MariaCrashEnd(void)
{
    mx_exit(MariaCrashEnd());
}

/* INC zCrashPhase, RTS ($A568). */
static void crash_next_phase(MExit *e)
{
    mx_regs(e, r_nz(regs_kept(), INC8(ZP->crash_phase)));
}

/*
 * @routine 01DD:530 MariaCrashTurn
 * @run 6D:A530
 * WHAT IT DOES: The crash frame of kind 6: in phase 0 her animation runs until frame $52, then turns to animation $2F
 * and the phase goes on; after that, when the animation has stopped, the crash's end.
 * HOW: MariaAnimationTick and the frame, MariaSetAnimation; MariaAnimationRunning's Z, then MariaCrashEnd's tail.
 * WAITS: no.
 * Entry: nothing. Exit: phase 0, A the frame, N, Z and C from comparing it with $52, or after the turn as
 * MariaSetAnimation with N and Z from the new phase; later, as MariaAnimationRunning, or the crash's end.
 */
MExit MariaCrashTurn(void)
{
    MExit e = mx_none();
    uint8_t phase = RD8(ZP->crash_phase);
    uint8_t frame;
    AnimationStep animation;

    mx_regs(&e, r_lda(regs_kept(), phase));
    if (phase != 0) {
        jsr_push(0xA548);
        animation = MariaAnimationRunning();
        jsr_pop();
        mx_animation(&e, animation);
        if (!mx_zero(mx_one_animation(animation))) {
            return e;
        }
        mx_then(&e, crash_end());
        return e;
    }
    jsr_push(0xA537);
    mx_animation(&e, MariaAnimationTick());
    jsr_pop();
    frame = RD8(OBJ->frame[OBJECT_PLAYER]);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), frame), frame, CRASH_FRAME_HELD));
    if (frame != CRASH_FRAME_HELD) {
        return e;
    }
    mx_regs(&e, r_lda(regs_kept(), ANIMATION_CRASH_TURN));
    jsr_push(0xA543);
    mx_animation(&e, MariaSetAnimation(ANIMATION_CRASH_TURN));
    jsr_pop();
    crash_next_phase(&e);
    return e;
}

void abi_MariaCrashTurn(void)
{
    mx_exit(MariaCrashTurn());
}

/*
 * @routine 01DD:559 MariaCrashWait
 * @run 6D:A559
 * WHAT IT DOES: A crash's first phase: her animation runs until frame $52, then script $1B starts and the phase goes
 * on.
 * HOW: MariaAnimationTick and the frame; MariaStartHitScript; INC zCrashPhase.
 * WAITS: no.
 * Entry: nothing. Exit: A the frame, N, Z and C from comparing it with $52; at frame $52, as MariaStartHitScript with
 * $1B, N and Z from the new phase, C set.
 */
MExit MariaCrashWait(void)
{
    MExit e = mx_none();
    uint8_t frame;

    jsr_push(0xA55C);
    mx_animation(&e, MariaAnimationTick());
    jsr_pop();
    frame = RD8(OBJ->frame[OBJECT_PLAYER]);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), frame), frame, CRASH_FRAME_HELD));
    if (frame != CRASH_FRAME_HELD) {
        return e;
    }
    jsr_push(0xA568);
    MariaStartHitScript(CRASH_SCRIPT);
    jsr_pop();
    mx_script(&e, CRASH_SCRIPT);
    crash_next_phase(&e);
    return e;
}

void abi_MariaCrashWait(void)
{
    mx_exit(MariaCrashWait());
}

/*
 * @routine 01DD:56B MariaCrashSignal
 * @run 6D:A56B
 * WHAT IT DOES: A crash's phase that waits on zFieldD2: once set, animation $2C, zFieldD2 2, her animation stopped,
 * time back to normal, she may leave the top of the map with no push, and the phase goes on.
 * HOW: The test, MariaSetAnimation, the stores, INC zCrashPhase.
 * WAITS: no.
 * Entry: nothing. Exit: waiting, A 0, Z set. Else A 1, N and Z from the new phase; Y and C as MariaSetAnimation left
 * them.
 */
MExit MariaCrashSignal(void)
{
    MExit e = mx_none();
    uint8_t signal = RD8(ZP->fieldd2);

    mx_regs(&e, r_lda(regs_kept(), signal));
    if (signal == 0) {
        return e;
    }
    mx_regs(&e, r_lda(regs_kept(), ANIMATION_CRASH_RISE));
    jsr_push(0xA575);
    mx_animation(&e, MariaSetAnimation(ANIMATION_CRASH_RISE));
    jsr_pop();
    WR8(ZP->fieldd2, CRASH_SIGNALLED);
    WR8(OBJ->anim_timer[OBJECT_PLAYER], TIMER_STOPPED);
    WR8(PVARS->time_slow_mode, 0);
    WR8(PVARS->may_leave_top, 1);
    WR8(ZP->player_push, 0);
    mx_regs(&e, r_lda(regs_kept(), 1));
    crash_next_phase(&e);
    return e;
}

void abi_MariaCrashSignal(void)
{
    mx_exit(MariaCrashSignal());
}

/*
 * @routine 01DD:58B MariaCrashRise
 * @run 6D:A58B
 * WHAT IT DOES: A crash's last phase: her animation runs and she may leave the top of the map; once zFieldD2 is set,
 * the crash's end for her and a jump, from no higher than row 0.
 * HOW: MariaAnimationTick; MariaCrashRecover; her Y below $10 is set to 0; the tail JMP to MariaJump.
 * WAITS: no.
 * Entry: nothing. Exit: waiting, A 0, Z set, and C and Y as MariaAnimationTick left them. Else as MariaJump.
 */
MExit MariaCrashRise(void)
{
    MExit e = mx_none();
    uint8_t signal, y;
    Sum lifted;

    jsr_push(0xA58E);
    mx_animation(&e, MariaAnimationTick());
    jsr_pop();
    WR8(PVARS->may_leave_top, 1);
    WR8(ZP->player_push, 0);
    signal = RD8(ZP->fieldd2);
    mx_regs(&e, r_lda(regs_kept(), signal));
    if (signal == 0) {
        return e;
    }
    jsr_push(0xA59C);
    mx_regs(&e, MariaCrashRecover());
    jsr_pop();
    WR8(PVARS->may_leave_top, 0);
    y = RD8(OBJ->y_lo[OBJECT_PLAYER]);
    lifted = add_bytes(y, CRASH_RISE_ROWS, false);
    mx_regs(&e, r_v(r_c(r_lda(regs_kept(), lifted.value), lifted.carry), lifted.overflow));
    if (!lifted.carry) {
        WR8(OBJ->y_lo[OBJECT_PLAYER], 0);
    }
    mx_then(&e, MariaJump());
    return e;
}

void abi_MariaCrashRise(void)
{
    mx_exit(MariaCrashRise());
}

/*
 * @routine 01DD:5AD MariaCrashFall
 * @run 6D:A5AD
 * WHAT IT DOES: The crash frame of kinds 2 and 3: phase 0 as MariaCrashWait; after it, when her animation has stopped,
 * time back to normal and the crash's end.
 * HOW: zCrashPhase; the tail JMP to MariaCrashWait; MariaAnimationRunning's Z, then MariaCrashEnd's tail.
 * WAITS: no.
 * Entry: nothing. Exit: as MariaCrashWait, MariaAnimationRunning, or the crash's end.
 */
MExit MariaCrashFall(void)
{
    MExit e = mx_none();
    uint8_t phase = RD8(ZP->crash_phase);
    AnimationStep animation;

    mx_regs(&e, r_lda(regs_kept(), phase));
    if (phase == 0) {
        mx_then(&e, MariaCrashWait());
        return e;
    }
    jsr_push(0xA5B7);
    animation = MariaAnimationRunning();
    jsr_pop();
    mx_animation(&e, animation);
    if (!mx_zero(mx_one_animation(animation))) {
        return e;
    }
    WR8(PVARS->time_slow_mode, 0);
    mx_then(&e, crash_end());
    return e;
}

void abi_MariaCrashFall(void)
{
    mx_exit(MariaCrashFall());
}

/*
 * @routine 01DD:5CD MariaCrashLand
 * @run 6D:A5CD
 * WHAT IT DOES: Phase 1 of crash kind 0: MariaCrashSignal, and once that has moved the phase on, animation $2D.
 * HOW: MariaCrashSignal, then zCrashPhase compared with 1; the tail JMP to MariaSetAnimation.
 * WAITS: no.
 * Entry: nothing. Exit: still phase 1, A 1, Z and C set. Else as MariaSetAnimation.
 */
MExit MariaCrashLand(void)
{
    MExit e;
    uint8_t phase;

    jsr_push(0xA5D0);
    e = MariaCrashSignal();
    jsr_pop();
    phase = RD8(ZP->crash_phase);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), phase), phase, 1));
    if (phase == 1) {
        return e;
    }
    mx_regs(&e, r_lda(regs_kept(), ANIMATION_CRASH_LAND));
    mx_animation(&e, MariaSetAnimation(ANIMATION_CRASH_LAND));
    return e;
}

void abi_MariaCrashLand(void)
{
    mx_exit(MariaCrashLand());
}

/* A crash's phase handler by the word its table gave, as the JMP (table,X) reaches it. */
static MExit crash_phase_handler(uint16_t target, Regs at_jump)
{
    MExit e = mx_none();

    switch (target) {
    case CRASH_PHASE_WAIT:
        return MariaCrashWait();
    case CRASH_PHASE_SIGNAL:
        return MariaCrashSignal();
    case CRASH_PHASE_RISE:
        return MariaCrashRise();
    case CRASH_PHASE_LAND:
        return MariaCrashLand();
    default:
        jump_to_routine(target, at_jump);
        e.elsewhere = true;
        return e;
    }
}

/*
 * @routine 01DD:54C MariaCrashPhase
 * @run 6D:A54C
 * WHAT IT DOES: The crash frame of kind 1: its phase's handler, from MariaCrashPhases (wait, signal, rise).
 * HOW: zCrashPhase doubled; the JMP (table,X).
 * WAITS: no.
 * Entry: nothing. Exit: as the phase's handler.
 */
MExit MariaCrashPhase(void)
{
    uint8_t phase = RD8(ZP->crash_phase);
    uint8_t doubled = (uint8_t)(phase << 1);
    uint8_t lo = TBL8(MariaCrashPhases, doubled);
    uint8_t hi = TBL8(MariaCrashPhases, doubled + 1);

    return crash_phase_handler((uint16_t)(lo | (hi << 8)),
                               r_c(r_ldx(r_lda(regs_kept(), doubled), doubled), (phase & 0x80) != 0));
}

void abi_MariaCrashPhase(void)
{
    mx_exit(MariaCrashPhase());
}

/*
 * @routine 01DD:5C0 MariaCrashPhaseB
 * @run 6D:A5C0
 * WHAT IT DOES: The crash frame of kind 0: its phase's handler, from MariaCrashPhasesB (wait, land, rise).
 * HOW: zCrashPhase doubled; the JMP (table,X).
 * WAITS: no.
 * Entry: nothing. Exit: as the phase's handler.
 */
MExit MariaCrashPhaseB(void)
{
    uint8_t phase = RD8(ZP->crash_phase);
    uint8_t doubled = (uint8_t)(phase << 1);
    uint8_t lo = TBL8(MariaCrashPhasesB, doubled);
    uint8_t hi = TBL8(MariaCrashPhasesB, doubled + 1);

    return crash_phase_handler((uint16_t)(lo | (hi << 8)),
                               r_c(r_ldx(r_lda(regs_kept(), doubled), doubled), (phase & 0x80) != 0));
}

void abi_MariaCrashPhaseB(void)
{
    mx_exit(MariaCrashPhaseB());
}

/*
 * @routine 01DD:4E7 MariaCrashFrame
 * @run 6D:A4E7
 * WHAT IT DOES: A frame of her item crash (state 0 step 2): the columns under her, a landing when she has footing,
 * then the crash's frame handler by its kind (MariaCrashHandlers).
 * HOW: MariaReadColumns, MariaHasNoFooting's carry, MariaLandOnFloor; zCrashKind doubled, the JMP (table,X).
 * WAITS: no.
 * Entry: nothing. Exit: as the kind's handler, after A and X the doubled kind and C its bit 7.
 */
MExit MariaCrashFrame(void)
{
    MExit e = mx_none();
    MExit handler = mx_none();
    FootingCheck footing;
    uint8_t kind, doubled, lo, hi;
    uint16_t target;
    Regs at_jump;

    jsr_push(0xA4EA);
    mx_columns(&e, MariaReadColumns());
    jsr_pop();
    jsr_push(0xA4ED);
    footing = MariaHasNoFooting();
    jsr_pop();
    mx_footing(&e, footing);
    if (!footing.no_floor) {
        jsr_push(0xA4F2);
        mx_landing(&e, MariaLandOnFloor());
        jsr_pop();
    }
    kind = RD8(ZP->crash_kind);
    doubled = (uint8_t)(kind << 1);
    lo = TBL8(MariaCrashHandlers, doubled);
    hi = TBL8(MariaCrashHandlers, doubled + 1);
    target = (uint16_t)(lo | (hi << 8));
    at_jump = r_c(r_ldx(r_lda(regs_kept(), doubled), doubled), (kind & 0x80) != 0);
    mx_regs(&e, at_jump);
    switch (target) {
    case CRASH_FRAME_B:
        handler = MariaCrashPhaseB();
        break;
    case CRASH_FRAME_A:
        handler = MariaCrashPhase();
        break;
    case CRASH_FRAME_C:
        handler = MariaCrashFall();
        break;
    case CRASH_FRAME_END:
        handler = MariaCrashEnd();
        break;
    case CRASH_FRAME_D:
        handler = MariaCrashTurn();
        break;
    default:
        mx_exit(e);
        run_routine(target);
        handler.elsewhere = true;
        e = handler;
        return e;
    }
    mx_then(&e, handler);
    return e;
}

void abi_MariaCrashFrame(void)
{
    mx_exit(MariaCrashFrame());
}

/* ------------------------------------------------------------------------------------------------ the jump */

/*
 * @routine 01DD:6AD MariaSetAnimationZero
 * @run 6D:A6AD
 * WHAT IT DOES: Sets her animation 0.
 * HOW: The tail JMP to MariaSetAnimation.
 * WAITS: no.
 * Entry: nothing. Exit: as MariaSetAnimation.
 */
MExit MariaSetAnimationZero(void)
{
    MExit e = mx_none();

    mx_regs(&e, r_lda(regs_kept(), 0));
    mx_animation(&e, MariaSetAnimation(0));
    return e;
}

void abi_MariaSetAnimationZero(void)
{
    mx_exit(MariaSetAnimationZero());
}

/*
 * @routine 01DD:6B2 MariaSpeedWalk
 * @run 6D:A6B2
 * WHAT IT DOES: Gives her the walking speed the way she faces ($0160 right, $FEA0 left).
 * HOW: MariaWalkSpeeds by OBJ_FACING_LEFT doubled.
 * WAITS: no.
 * Entry: nothing. Exit: A the speed's high byte, N and Z from it; Y the facing doubled; C bit 7 of the facing. X and V
 * kept.
 */
Regs MariaSpeedWalk(void)
{
    uint8_t facing = RD8(OBJ->facing_left[OBJECT_PLAYER]);
    uint8_t index = (uint8_t)(facing << 1);
    uint8_t lo = TBL8(MariaWalkSpeeds, index);
    uint8_t hi;

    WR8(OBJ->vx_lo[OBJECT_PLAYER], lo);
    hi = TBL8(MariaWalkSpeeds, index + 1);
    WR8(OBJ->vx_hi[OBJECT_PLAYER], hi);
    return r_c(r_lda(r_y_only(regs_kept(), index), hi), (facing & 0x80) != 0);
}

void abi_MariaSpeedWalk(void)
{
    regs_exit(MariaSpeedWalk());
}

/*
 * @routine 01DD:729 MariaLiftThree
 * @run 6D:A729
 * WHAT IT DOES: Moves her up three rows, no higher than the top of the map (Y $0000).
 * HOW: A 16-bit subtract; a high byte of $FF is put back to zero.
 * WAITS: no.
 * Entry: nothing. Exit: A her Y's high byte, N, Z and C from comparing it with $FF, V from the subtract; at the top A
 * $FF, Z and C set. X and Y kept.
 */
Regs MariaLiftThree(void)
{
    uint8_t value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
    Sum lo = sub_bytes(value, LIFT_STEP, true);
    Sum hi;

    WR8(OBJ->y_lo[OBJECT_PLAYER], lo.value);
    value = RD8(OBJ->y_hi[OBJECT_PLAYER]);
    hi = sub_bytes(value, 0, lo.carry);
    WR8(OBJ->y_hi[OBJECT_PLAYER], hi.value);
    if (hi.value == ABOVE_THE_MAP) {
        WR8(OBJ->y_lo[OBJECT_PLAYER], 0);
        WR8(OBJ->y_hi[OBJECT_PLAYER], 0);
    }
    return r_v(r_cmp(r_lda(regs_kept(), hi.value), hi.value, ABOVE_THE_MAP), hi.overflow);
}

void abi_MariaLiftThree(void)
{
    regs_exit(MariaLiftThree());
}

/*
 * @routine 01DD:70A MariaJumpSpeed
 * @run 6D:A70A
 * WHAT IT DOES: Starts her jump's rise: keeps her Y on the map (PlayerJumpStartY), stops her, gives her Y speed $FC80
 * (up), and lifts her three rows.
 * HOW: Her Y plus ScrollYLo, the scroll's screen and the carry; MariaStopMoving; on into MariaLiftThree.
 * WAITS: no.
 * Entry: nothing. Exit: as MariaLiftThree.
 */
Regs MariaJumpSpeed(void)
{
    uint8_t value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
    uint8_t scroll = RD8(SCROLL->y_lo[0]);
    Sum lo = add_bytes(value, scroll, false);
    Sum hi;

    WR8(PVARS2->jump_start_y.lo, lo.value);
    value = RD8(SCROLLE->y_hi_screen[0]);
    hi = add_bytes(value, 0, lo.carry);
    WR8(PVARS2->jump_start_y.hi, hi.value);
    jsr_push(0xA71F);
    MariaStopMoving();
    jsr_pop();
    WR8(OBJ->vy_lo[OBJECT_PLAYER], JUMP_VY_LO);
    WR8(OBJ->vy_hi[OBJECT_PLAYER], JUMP_VY_HI);
    return MariaLiftThree();
}

void abi_MariaJumpSpeed(void)
{
    regs_exit(MariaJumpSpeed());
}

/*
 * @routine 01DD:6C8 MariaJump
 * @run 6D:A6C8
 * WHAT IT DOES: Starts her jump (state 3, step 0): may not leave the top, the pad's second forced press and zFieldC6
 * cleared, the rise begun. With right or left held, she faces that way for animation $0B and takes the walking speed;
 * jumping the way she did not face, she keeps her facing and the animation is $0A, as for a jump straight up.
 * HOW: A BSR to MariaJumpSpeed; BBS5 and BBS7 of zPadHeld (two reads) give Y 0 or 1 for the facing; MariaSetAnimation,
 * MariaSpeedWalk; her old facing from the stack compared with the new.
 * WAITS: no.
 * Entry: nothing. Exit: straight up or across backward, as MariaSetAnimation with $0A. Across forward, A the facing, Z
 * and C set; Y as MariaSpeedWalk left it.
 */
MExit MariaJump(void)
{
    MExit e = mx_none();
    uint8_t facing = 0;
    uint8_t old, now;

    WR8(PVARS->may_leave_top, 0);
    WR8(PVARS->forced_press_b, 0);
    WR8(ZP->fieldc6, 0);
    WR8(OBJ->state[OBJECT_PLAYER], STATE_JUMP);
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    jsr_push(0xA6DC);
    mx_regs(&e, MariaJumpSpeed());
    jsr_pop();
    mx_regs(&e, r_y_only(regs_kept(), 0));
    if (!(RD8(ZP->pad_held) & PAD_RIGHT)) {
        facing = 1;
        mx_regs(&e, r_ldy(regs_kept(), 1));
        if (!(RD8(ZP->pad_held) & PAD_LEFT)) {
            mx_regs(&e, r_lda(regs_kept(), ANIMATION_JUMP_UP));
            mx_animation(&e, MariaSetAnimation(ANIMATION_JUMP_UP));
            return e;
        }
    }
    old = RD8(OBJ->facing_left[OBJECT_PLAYER]);
    push8(old);
    WR8(OBJ->facing_left[OBJECT_PLAYER], facing);
    mx_regs(&e, r_lda(regs_kept(), ANIMATION_JUMP_ACROSS));
    jsr_push(0xA6F5);
    mx_animation(&e, MariaSetAnimation(ANIMATION_JUMP_ACROSS));
    jsr_pop();
    jsr_push(0xA6F8);
    mx_regs(&e, MariaSpeedWalk());
    jsr_pop();
    old = pull8();
    now = RD8(OBJ->facing_left[OBJECT_PLAYER]);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), old), old, now));
    WR8(OBJ->facing_left[OBJECT_PLAYER], old);
    if (old == now) {
        return e;
    }
    mx_regs(&e, r_lda(regs_kept(), ANIMATION_JUMP_UP));
    mx_animation(&e, MariaSetAnimation(ANIMATION_JUMP_UP));
    return e;
}

void abi_MariaJump(void)
{
    mx_exit(MariaJump());
}
