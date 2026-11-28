/*
 * Maria's objects (her overlay, bank $6E): the companions in slots 1 and 2, their handlers by kind, and the wait on
 * an object's animation that ends its caller.
 *
 * These routines hold the processor, as maria_objects_probe.c's do (they call the stage overlay's read at $8006
 * between their own steps): each is entered with the registers and flags as its caller left them in it, sets in it
 * what its assembly sets where the assembly sets it, gives each typed callee's exit to it as the callee returns, and
 * leaves in it what its assembly leaves. A C caller gives its own exit so far first.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"
#include "maria_objects.h"

enum {
    OBJECT_PLAYER = 0, SLOT_FIRST_COMPANION = 1, SLOT_END_COMPANIONS = 3,
    STATE_HIT = 9, COMPANION_ANIMATION = 3, COMPANION_SPRITE_SET = 3, COMPANION_FOOT_ROWS = 0x10,
    COMPANION_ON_FLOOR = 0x08, COMPANION_HOLD = 0x10, COMPANION_END_SCRIPT = 8, COMPANION_HIT_TIMER = 8,
    SUB_MODE_ENDING = 3,
    KIND3_SPRITE_SET = 1, KIND3_FRAME = 0x2B, KIND3_TIMER = 0x80, KIND4_SPRITE_SET = 3, KIND4_ANIMATION = 0x13,
    HANDLER_NONE = 0xC703, HANDLER_KIND1 = 0xC58D, HANDLER_KIND3 = 0xC645, HANDLER_KIND4 = 0xC67A
};

#define MariaCompanionYOffsets_TBL MariaCompanionYOffsets
#define MariaCompanionXOffsets_TBL MariaCompanionXOffsets
#define MariaCompanionHandlers_TBL MariaCompanionHandlers

uint8_t MariaObjectSlot(void);                                      /* object/richter_objects.c */
uint8_t MariaObjectTake2FE8(uint8_t object);                        /* object/richter_objects.c */
uint8_t RichterObjectAtPlayerPlace(uint8_t object);                 /* object/richter_objects.c */

static void p_lda(uint8_t value)
{
    cpu.a = value;
    abi_nz(value);
}

static void p_sum(Sum s)
{
    p_lda(s.value);
    abi_flag(P_C, s.carry);
    abi_flag(P_V, s.overflow);
}

static void p_cmp(uint8_t reg, uint8_t operand)
{
    abi_nz((uint8_t)(reg - operand));
    abi_flag(P_C, reg >= operand);
}

/*
 * @routine 01E4:584 MariaObjectAnimationWait
 * @run 6E:DD84
 * WHAT IT DOES: Steps an object's animation and, unless the tick left Z set (the animation stopped or a frame of 0),
 * returns to its caller's caller: the caller's own work waits for the animation.
 * HOW: MariaObjectAnimationTick; on Z clear the caller's return address is pulled (PLA PLA).
 * WAITS: no.
 * Entry: X the object. Exit, Z set: as the tick. Otherwise A the return address's high byte, N and Z from it; the
 * rest as the tick left them. Returns true when it dropped the caller's return address: the caller then ends at once.
 */
bool MariaObjectAnimationWait(void)
{
    uint8_t high;

    jsr_push(0xDD87);
    animation_exit_shim(MariaObjectAnimationTick(cpu.x));
    jsr_pop();
    if (cpu.p & P_Z) {
        return false;
    }
    pull8();
    high = pull8();
    p_lda(high);
    return true;
}

void abi_MariaObjectAnimationWait(void)
{
    MariaObjectAnimationWait();
}

/*
 * @routine 01E1:52F MariaObjectAtPlayerPlace
 * @run 6E:C52F
 * WHAT IT DOES: RichterObjectAtPlayerPlace at her address, the same bytes: puts an object where she is.
 * HOW: RichterObjectAtPlayerPlace's body.
 * WAITS: no.
 * Entry: X the object. Exit: A her Y's high byte, N and Z from it. X, Y, C and V kept.
 */
void MariaObjectAtPlayerPlace(void)
{
    p_lda(RichterObjectAtPlayerPlace(cpu.x));
}

void abi_MariaObjectAtPlayerPlace(void)
{
    MariaObjectAtPlayerPlace();
}

/*
 * @routine 01E1:58D MariaObjectKind1
 * @run 6E:C58D
 * WHAT IT DOES: The handler of her companions of kinds 1 and 2. State 0: after OBJ_ANIM_TIMER runs out, animation 3,
 * sprite set 3, state 1, a place beside her by slot and the frame tick (OBJ_WORK_E, MariaCompanionYOffsets and
 * MariaCompanionXOffsets), and a terrain read $10 rows below her: on ground (bit 3) the timer is set to $10. State 1:
 * while its timer runs, rises from its place one row a frame more (OBJ_WORK_D counts the rows) as its animation runs;
 * it ends when its animation ends with her hit timer below 8, her invincibility on or the game in sub-mode 3; at the
 * end of its own timer it starts script 8 and is cleared.
 * HOW: The stage overlay's read at $8006; MariaObjectAnimationWait, which ends this handler while the animation runs.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the last step left it.
 */
void MariaObjectKind1(void)
{
    uint8_t object, value, index;
    Sum s;

    object = RD8(ZP->richter_object);
    cpu.x = object;
    abi_nz(object);
    value = RD8(OBJ->state[object]);
    p_lda(value);
    if (value == 0) {
        value = RD8(OBJ->anim_timer[object]);
        p_lda(value);
        if (value != 0) {
            value = DEC8(OBJ->anim_timer[object]);
            abi_nz(value);
            if (value != 0) {
                return;
            }
        }
        p_lda(COMPANION_ANIMATION);
        jsr_push(0xC5A3);
        animation_exit_shim(MariaObjectSetAnimation(object, COMPANION_ANIMATION));
        jsr_pop();
        p_lda(COMPANION_SPRITE_SET);
        WR8(OBJ->sprite_set[object], COMPANION_SPRITE_SET);
        abi_nz(INC8(OBJ->state[object]));
        p_lda((uint8_t)(object - 1));
        WR8(ZP->temp00, cpu.a);
        value = RD8(ZP->frame_tick);
        p_lda((uint8_t)(value & 0x02));
        value = RD8(ZP->temp00);
        p_lda((uint8_t)(cpu.a | value));
        WR8(OBJ->work_e[object], cpu.a);
        index = cpu.a;
        cpu.y = index;
        value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
        s = add_bytes(value, TBL8(MariaCompanionYOffsets, index), false);
        p_sum(s);
        WR8(OBJ->y_lo[object], s.value);
        value = RD8(OBJ->y_hi[OBJECT_PLAYER]);
        s = add_bytes(0, value, s.carry);
        p_sum(s);
        WR8(OBJ->y_hi[object], s.value);
        value = RD8(OBJ->x_lo[OBJECT_PLAYER]);
        s = add_bytes(value, TBL8(MariaCompanionXOffsets, index), false);
        p_sum(s);
        WR8(OBJ->x_lo[object], s.value);
        value = RD8(OBJ->x_lo[OBJECT_PLAYER]);
        p_lda(value);
        WR8(ZP->temp00, value);
        WR8(ZP->temp01, 0);
        value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
        s = add_bytes(value, COMPANION_FOOT_ROWS, false);
        p_sum(s);
        WR8(ZP->temp02, s.value);
        s = add_bytes(s.value, 0, s.carry);
        p_sum(s);
        WR8(ZP->temp03, s.value);
        if (s.carry) {
            return;
        }
        jsr_push(0xC5EC);
        terrain_read_exit_shim(StageTileRead());
        jsr_pop();
        p_lda((uint8_t)(cpu.a & COMPANION_ON_FLOOR));
        if (cpu.a == 0) {
            return;
        }
        p_lda(COMPANION_HOLD);
        WR8(OBJ->anim_timer[object], COMPANION_HOLD);
        return;
    }
    value = RD8(OBJ->anim_timer[object]);
    p_lda(value);
    if (value != 0) {
        value = DEC8(OBJ->anim_timer[object]);
        abi_nz(value);
        if (value == 0) {
            push8(cpu.x);
            p_lda(COMPANION_END_SCRIPT);
            jsr_push(0xC606);
            MariaStartHitScript(COMPANION_END_SCRIPT);
            hit_script_exit_shim(COMPANION_END_SCRIPT);
            jsr_pop();
            cpu.x = pull8();
            abi_nz(cpu.x);
            ClearObject(cpu.x);
            return;
        }
    }
    index = RD8(OBJ->work_e[object]);
    cpu.y = index;
    abi_nz(index);
    value = RD8(OBJ->work_d[object]);
    p_lda((uint8_t)((uint8_t)(value ^ 0xFF) + 1));
    s = add_bytes(cpu.a, TBL8(MariaCompanionYOffsets, index), false);
    p_sum(s);
    value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
    s = add_bytes(s.value, value, false);
    p_sum(s);
    WR8(OBJ->y_lo[object], s.value);
    abi_nz(INC8(OBJ->work_d[object]));
    jsr_push(0xC624);
    if (MariaObjectAnimationWait()) {
        return;
    }
    jsr_pop();
    value = RD8(OBJ->anim_timer[OBJECT_PLAYER]);
    p_lda(value);
    p_cmp(value, COMPANION_HIT_TIMER);
    if (value >= COMPANION_HIT_TIMER) {
        value = RD8(PVARS->invincible_timer);
        p_lda(value);
        if (value == 0) {
            value = RD8(ZP->game_sub_mode);
            p_lda(value);
            p_cmp(value, SUB_MODE_ENDING);
            if (value != SUB_MODE_ENDING) {
                WR8(OBJ->work_d[object], 0);
                WR8(OBJ->state[object], 0);
                return;
            }
        }
    }
    ClearObject(cpu.x);
}

void abi_MariaObjectKind1(void)
{
    MariaObjectKind1();
}

/*
 * @routine 01E1:645 MariaObjectKind3
 * @run 6E:C645
 * WHAT IT DOES: The handler of her companion of kind 3: cleared when she is hit (her state 9). State 0: where she
 * is, OBJ_SPRITE_FLIP from hers, sprite set 1, frame $2B, timer $80, state 1. State 1: stays where she is, marked,
 * until its timer runs out, then is cleared.
 * HOW: MariaObjectTake2FE8, MariaObjectAtPlayerPlace, MariaObjectMarkOn; the tail JMPs to MariaObjectNextState and
 * MariaObjectClear.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the last step left it.
 */
void MariaObjectKind3(void)
{
    uint8_t object, value, take;

    object = RD8(ZP->richter_object);
    cpu.x = object;
    abi_nz(object);
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    p_lda(value);
    p_cmp(value, STATE_HIT);
    if (value == STATE_HIT) {
        MariaObjectClear(object);
        return;
    }
    value = RD8(OBJ->state[object]);
    p_lda(value);
    if (value == 0) {
        jsr_push(0xC656);
        take = MariaObjectTake2FE8(object);
        jsr_pop();
        p_lda((uint8_t)(take << 1));
        abi_flag(P_C, (take & 0x80) != 0);
        jsr_push(0xC659);
        MariaObjectAtPlayerPlace();
        jsr_pop();
        p_lda(KIND3_SPRITE_SET);
        WR8(OBJ->sprite_set[object], KIND3_SPRITE_SET);
        p_lda(KIND3_FRAME);
        WR8(OBJ->frame[object], KIND3_FRAME);
        p_lda(KIND3_TIMER);
        WR8(OBJ->anim_timer[object], KIND3_TIMER);
        abi_nz(MariaObjectNextState(object));
        return;
    }
    jsr_push(0xC66E);
    MariaObjectAtPlayerPlace();
    jsr_pop();
    jsr_push(0xC671);
    p_lda(MariaObjectMarkOn(object));
    jsr_pop();
    value = DEC8(OBJ->anim_timer[object]);
    abi_nz(value);
    if (value == 0) {
        MariaObjectClear(object);
    }
}

void abi_MariaObjectKind3(void)
{
    MariaObjectKind3();
}

/*
 * @routine 01E1:67A MariaObjectKind4
 * @run 6E:C67A
 * WHAT IT DOES: The handler of her companions of kinds 4 and 5: cleared when she is hit (her state 9). State 0:
 * from her (MariaObjectFromPlayer), marked, at its place beside her, sprite set 3, animation $13, state 1. State 1:
 * moves, and stays at its place while its animation runs; cleared when it ends.
 * HOW: MariaObjectPlaceByState (a BSR, then run on into at the end); MariaObjectMove, MariaObjectAnimationRunning.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the last step left it.
 */
void MariaObjectKind4(void)
{
    uint8_t object, value;
    ObjectFromPlayer from;

    object = RD8(ZP->richter_object);
    cpu.x = object;
    abi_nz(object);
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    p_lda(value);
    p_cmp(value, STATE_HIT);
    if (value == STATE_HIT) {
        MariaObjectClear(object);
        return;
    }
    value = RD8(OBJ->state[object]);
    p_lda(value);
    if (value == 0) {
        jsr_push(0xC68B);
        from = MariaObjectFromPlayer(object);
        jsr_pop();
        p_lda(from.facing);
        abi_flag(P_C, (from.player_2fe8 & 0x80) != 0);
        jsr_push(0xC68E);
        p_lda(MariaObjectMarkOn(object));
        jsr_pop();
        jsr_push(0xC690);
        mx_exit(MariaObjectPlaceByState(object));
        jsr_pop();
        p_lda(KIND4_SPRITE_SET);
        WR8(OBJ->sprite_set[object], KIND4_SPRITE_SET);
        p_lda(KIND4_ANIMATION);
        jsr_push(0xC69A);
        animation_exit_shim(MariaObjectSetAnimation(object, KIND4_ANIMATION));
        jsr_pop();
        abi_nz(MariaObjectNextState(object));
        return;
    }
    jsr_push(0xC6A0);
    abi_ended(ended_of_axis_move(MariaObjectMove(object)));
    jsr_pop();
    jsr_push(0xC6A3);
    animation_exit_shim(MariaObjectAnimationRunning(object));
    jsr_pop();
    if (cpu.p & P_Z) {
        MariaObjectClear(object);
        return;
    }
    mx_exit(MariaObjectPlaceByState(object));
}

void abi_MariaObjectKind4(void)
{
    MariaObjectKind4();
}

/*
 * @routine 01E1:6EF MariaRunCompanionKind
 * @run 6E:C6EF
 * WHAT IT DOES: Runs a companion's handler by its kind (MariaCompanionHandlers: none, kinds 1 and 2, kind 3, kinds 4
 * and 5).
 * HOW: OBJ_KIND doubled; the JMP (table,X). A word past the four handlers goes to the routine at it.
 * WAITS: no.
 * Entry: X the slot. Exit: as the handler.
 */
void MariaRunCompanionKind(void)
{
    uint8_t kind = RD8(OBJ->kind[cpu.x]);
    uint8_t doubled = (uint8_t)(kind << 1);
    uint8_t lo, hi;
    uint16_t target;

    p_lda(doubled);
    abi_flag(P_C, (kind & 0x80) != 0);
    cpu.x = doubled;
    lo = TBL8(MariaCompanionHandlers, doubled);
    hi = TBL8(MariaCompanionHandlers, doubled + 1);
    target = (uint16_t)(lo | (hi << 8));
    switch (target) {
    case HANDLER_NONE:
        cpu.x = MariaObjectSlot();
        abi_nz(cpu.x);
        break;
    case HANDLER_KIND1:
        MariaObjectKind1();
        break;
    case HANDLER_KIND3:
        MariaObjectKind3();
        break;
    case HANDLER_KIND4:
        MariaObjectKind4();
        break;
    default:
        run_routine(target);
        break;
    }
}

void abi_MariaRunCompanionKind(void)
{
    MariaRunCompanionKind();
}

/*
 * @routine 01E1:6E3 MariaRunCompanions
 * @run 6E:C6E3
 * WHAT IT DOES: Runs her companions in slots 1 and 2.
 * HOW: zRichterObject the slot, a BSR to MariaRunCompanionKind; X as the handler left it, one up, until 3.
 * WAITS: no.
 * Entry: nothing. Exit: X 3, Z and C set, N clear; A, Y and V as the last handler left them.
 */
void MariaRunCompanions(void)
{
    cpu.x = SLOT_FIRST_COMPANION;
    abi_nz(cpu.x);
    do {
        WR8(ZP->richter_object, cpu.x);
        jsr_push(0xC6E9);
        MariaRunCompanionKind();
        jsr_pop();
        cpu.x = (uint8_t)(cpu.x + 1);
        p_cmp(cpu.x, SLOT_END_COMPANIONS);
    } while (cpu.x != SLOT_END_COMPANIONS);
}

void abi_MariaRunCompanions(void)
{
    MariaRunCompanions();
}
