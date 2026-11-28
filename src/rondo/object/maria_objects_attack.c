/*
 * Maria's objects (her overlay, bank $6E): kind 1, her attack's object (MariaAttackSlot starts it in slot 3 or 4),
 * and kind 5, item 5 (MariaThrowItem puts it in slot 5, or 8; the second of its pair, in the next slot, flies the
 * other way).
 *
 * The attack's object starts beside her, flies out falling and slowing, turns and falls, then homes back on her
 * hitbox's middle; it is cleared when it reaches her.
 *
 * These routines hold the processor (see maria_objects_slots.c): each typed callee's exit is given to it as the
 * callee returns.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"
#include "maria_objects.h"

enum {
    OBJECT_PLAYER = 0, STATE_CROUCH = 2, ITEM5_FIRST = 5, ITEM5_SECOND = 8, KIND_ITEM5 = 5,
    START_ACROSS = 0x0A, START_DOWN = 0xF3, CROUCH_DOWN = 0x03, ATTACK_HALF = 0x08, ATTACK_DX = 0xFC,
    FALL_START_LO = 0xF0, FALL_START_HI = 0xFF, ATTACK_WORK_N = 5,
    TURN_VY = 2, TURN_FALL_LO = 0x06, RETURN_SPEED_HI = 0x03, RETURN_STANDING_DOWN = 0xF3,
    ITEM5_VX = 4, ITEM5_VY = 5, ITEM5_FALL_LO = 0x44, ITEM5_ATTR = 0x85, STATE2 = 2, STATE2_WORK_A = 6,
    ATTACK_START = 0xC77B, ATTACK_FLY = 0xC7DA, ATTACK_FALL = 0xC7FF, ATTACK_RETURN = 0xC825,
    ITEM5_START = 0xC8F6, ITEM5_FLY = 0xC934, ITEM5_FALL = 0xC945, ITEM5_RETURN = 0xC948
};

#define MariaAttackStates_TBL MariaAttackStates
#define MariaAttackAngles_TBL MariaAttackAngles
#define MariaItem5States_TBL MariaItem5States

ObjArm MariaStateDoubled(void);                                     /* object/player_leaves.c */

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

/* A velocity from a table: A the high byte, C the entry's bit 7, Y the doubled entry. */
static void p_obj_arm(ObjArm r)
{
    regs_exit(regs_of_velocity_table(r));
}

/* LDX zRichterObject. */
static uint8_t p_ldx_object(void)
{
    cpu.x = RD8(ZP->richter_object);
    abi_nz(cpu.x);
    return cpu.x;
}

/* JSR MariaStateDoubled, then the JMP (table,X) by the doubled state: the word read, for the caller's switch. */
static uint16_t state_handler(uint16_t ret, uint16_t table)
{
    ObjArm r;
    uint8_t lo, hi;

    jsr_push(ret);
    r = MariaStateDoubled();
    jsr_pop();
    p_lda(r.first);
    abi_flag(P_C, r.flag);
    cpu.x = r.first;
    lo = rd8((uint16_t)(table + r.first));
    hi = rd8((uint16_t)(table + r.first + 1));
    return (uint16_t)(lo | (hi << 8));
}

/*
 * @routine 01E1:77B MariaAttackStart
 * @run 6E:C77B
 * WHAT IT DOES: Starts her attack's object: from her (facing and sprite flip), $0A across and $0D up from her ($0A
 * across and 3 down when she crouches), an 8 by 8 hitbox $04 to the left, live; its X speed from entry 0 the way it
 * faces, Y speed entry 1, falling by $FFF0 a frame, OBJ_WORK_N 5; then animation 0 and its next state.
 * HOW: MariaObjectFromPlayer, MariaObjectAtPlayer, the velocity tables, MariaObjectFacingVelocity; on into
 * MariaObjectStartAnimation.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as MariaObjectStartAnimation.
 */
void MariaAttackStart(void)
{
    uint8_t object = p_ldx_object();
    uint8_t state;
    ObjectFromPlayer from;

    jsr_push(0xC780);
    from = MariaObjectFromPlayer(object);
    jsr_pop();
    p_lda(from.facing);
    abi_flag(P_C, (from.player_2fe8 & 0x80) != 0);
    p_lda(START_ACROSS);
    WR8(ZP->temp04, START_ACROSS);
    p_lda(START_DOWN);
    WR8(ZP->temp06, START_DOWN);
    jsr_push(0xC78B);
    p_sum(MariaObjectAtPlayer(object));
    jsr_pop();
    state = RD8(OBJ->state[OBJECT_PLAYER]);
    p_lda(state);
    p_cmp(state, STATE_CROUCH);
    if (state == STATE_CROUCH) {
        p_lda(START_ACROSS);
        WR8(ZP->temp04, START_ACROSS);
        p_lda(CROUCH_DOWN);
        WR8(ZP->temp06, CROUCH_DOWN);
        jsr_push(0xC79D);
        p_sum(MariaObjectAtPlayer(object));
        jsr_pop();
    }
    p_lda(ATTACK_HALF);
    WR8(OBJ->hitbox_half_h[object], ATTACK_HALF);
    p_lda(ATTACK_HALF);
    WR8(OBJ->hitbox_half_w[object], ATTACK_HALF);
    p_lda(ATTACK_DX);
    WR8(OBJ->hitbox_dx[object], ATTACK_DX);
    p_lda(1);
    WR8(OBJ->hitbox_on[object], 1);
    p_lda(0);
    jsr_push(0xC7B6);
    p_obj_arm(MariaSetVelocityXFromTable(object, 0));
    jsr_pop();
    jsr_push(0xC7B9);
    abi_ended(MariaObjectFacingVelocity(object));
    jsr_pop();
    p_lda(1);
    jsr_push(0xC7BE);
    p_obj_arm(MariaSetVelocityYFromTable(object, 1));
    jsr_pop();
    p_lda(FALL_START_LO);
    WR8(OBJ->fall_lo[object], FALL_START_LO);
    p_lda(FALL_START_HI);
    WR8(OBJ->fall_hi[object], FALL_START_HI);
    p_lda(ATTACK_WORK_N);
    WR8(OBJ->work_n[object], ATTACK_WORK_N);
    p_lda(0);
    mx_exit(MariaObjectStartAnimation(object, 0));
}

void abi_MariaAttackStart(void)
{
    MariaAttackStart();
}

/*
 * @routine 01E1:7DA MariaAttackFly
 * @run 6E:C7DA
 * WHAT IT DOES: Her attack's object flying out: its animation runs, it moves and falls, and it slows; when its X
 * speed changes sign it turns: Y speed entry 2, falling by $0006 a frame, animation 1 and its next state.
 * HOW: MariaObjectAnimationTick, MariaObjectMove, MariaObjectFallCapped, MariaObjectAccelerate (Z clear on the sign's
 * change); the tail JMP to MariaObjectStartAnimation.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as MariaObjectAccelerate, or MariaObjectStartAnimation.
 */
void MariaAttackFly(void)
{
    uint8_t object = p_ldx_object();

    jsr_push(0xC7DF);
    animation_exit_shim(MariaObjectAnimationTick(object));
    jsr_pop();
    jsr_push(0xC7E2);
    abi_ended(ended_of_axis_move(MariaObjectMove(object)));
    jsr_pop();
    jsr_push(0xC7E5);
    abi_ended(MariaObjectFallCapped(object));
    jsr_pop();
    jsr_push(0xC7E8);
    abi_ended(MariaObjectAccelerate(object));
    jsr_pop();
    if (cpu.p & P_Z) {
        return;
    }
    p_lda(TURN_VY);
    jsr_push(0xC7EF);
    p_obj_arm(MariaSetVelocityYFromTable(object, TURN_VY));
    jsr_pop();
    p_lda(TURN_FALL_LO);
    WR8(OBJ->fall_lo[object], TURN_FALL_LO);
    p_lda(0);
    WR8(OBJ->fall_hi[object], 0);
    p_lda(1);
    mx_exit(MariaObjectStartAnimation(object, 1));
}

void abi_MariaAttackFly(void)
{
    MariaAttackFly();
}

/*
 * @routine 01E1:7FF MariaAttackFall
 * @run 6E:C7FF
 * WHAT IT DOES: Her attack's object after its turn: cleared when it reaches her; otherwise it moves and falls, and
 * when its animation ends it takes the angle back toward her by its facing (MariaAttackAngles, into OBJ_WORK_D),
 * animation 2 and its next state.
 * HOW: MariaObjectNearPlayer (C set when it is at her), MariaObjectApplyVelocity, MariaObjectFallCapped,
 * MariaObjectAnimationRunning; the tail JMPs to ClearObject and MariaObjectStartAnimation.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the last step left the processor.
 */
void MariaAttackFall(void)
{
    uint8_t object = p_ldx_object();
    uint8_t facing, angle;

    jsr_push(0xC804);
    abi_ended(MariaObjectNearPlayer(object));
    jsr_pop();
    if (cpu.p & P_C) {
        ClearObject(object);
        return;
    }
    jsr_push(0xC809);
    abi_ended(ended_of_axis_move(MariaObjectApplyVelocity(object)));
    jsr_pop();
    jsr_push(0xC80C);
    abi_ended(MariaObjectFallCapped(object));
    jsr_pop();
    jsr_push(0xC80F);
    animation_exit_shim(MariaObjectAnimationRunning(object));
    jsr_pop();
    if (!(cpu.p & P_Z)) {
        return;
    }
    facing = RD8(OBJ->facing_left[object]);
    cpu.y = facing;
    abi_nz(facing);
    angle = TBL8(MariaAttackAngles, facing);
    p_lda(angle);
    WR8(OBJ->work_d[object], angle);
    p_lda(2);
    mx_exit(MariaObjectStartAnimation(object, 2));
}

void abi_MariaAttackFall(void)
{
    MariaAttackFall();
}

/*
 * @routine 01E1:825 MariaAttackReturn
 * @run 6E:C825
 * WHAT IT DOES: Her attack's object homing back: cleared when it reaches her; otherwise it moves, its animation
 * runs, and it is pointed at speed $0300 at her hitbox's middle (her Y moved by her OBJ_HITBOX_DY, and $0D up unless
 * she crouches, for the angle, then put back).
 * HOW: MariaObjectNearPlayer, MariaObjectApplyVelocity, MariaObjectAnimationTick; her Y on the stack around
 * AngleToPlayer and SetVelocityFromAngle (the kernel's vectors at $404B and $4048).
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: A her Y's low byte, N and Z from it; C and V as SetVelocityFromAngle left
 * them, Y 0.
 */
void MariaAttackReturn(void)
{
    uint8_t object = p_ldx_object();
    uint8_t value, angle;
    Sum s;
    AngleExit toward;

    jsr_push(0xC82A);
    abi_ended(MariaObjectNearPlayer(object));
    jsr_pop();
    if (cpu.p & P_C) {
        ClearObject(object);
        return;
    }
    jsr_push(0xC82F);
    abi_ended(ended_of_axis_move(MariaObjectApplyVelocity(object)));
    jsr_pop();
    jsr_push(0xC832);
    animation_exit_shim(MariaObjectAnimationTick(object));
    jsr_pop();
    WR8(ZP->temp00, 0);
    value = RD8(OBJ->hitbox_dy[OBJECT_PLAYER]);
    p_lda(value);
    if (value & 0x80) {
        abi_nz(DEC8(ZP->temp00));
    }
    value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
    p_lda(value);
    push8(value);
    s = add_bytes(value, RD8(OBJ->hitbox_dy[OBJECT_PLAYER]), false);
    p_sum(s);
    WR8(OBJ->y_lo[OBJECT_PLAYER], s.value);
    value = RD8(OBJ->y_hi[OBJECT_PLAYER]);
    p_lda(value);
    push8(value);
    s = add_bytes(value, RD8(ZP->temp00), s.carry);
    p_sum(s);
    WR8(OBJ->y_hi[OBJECT_PLAYER], s.value);
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    p_lda(value);
    p_cmp(value, STATE_CROUCH);
    if (value != STATE_CROUCH) {
        value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
        s = add_bytes(value, RETURN_STANDING_DOWN, false);
        p_sum(s);
        WR8(OBJ->y_lo[OBJECT_PLAYER], s.value);
        p_lda(0xFF);
        s = add_bytes(0xFF, RD8(OBJ->y_hi[OBJECT_PLAYER]), s.carry);
        p_sum(s);
        WR8(OBJ->y_hi[OBJECT_PLAYER], s.value);
    }
    jsr_push(0xC86A);
    toward = AngleToPlayer(object);
    jsr_pop();
    angle_exit_shim(toward);
    push8(cpu.a);
    p_lda(0);
    WR8(ZP->temp00, 0);
    p_lda(RETURN_SPEED_HI);
    WR8(ZP->temp01, RETURN_SPEED_HI);
    angle = pull8();
    p_lda(angle);
    jsr_push(0xC877);
    regs_exit(regs_of_scale(SetVelocityFromAngle(angle, object)));
    jsr_pop();
    value = pull8();
    p_lda(value);
    WR8(OBJ->y_hi[OBJECT_PLAYER], value);
    value = pull8();
    p_lda(value);
    WR8(OBJ->y_lo[OBJECT_PLAYER], value);
}

void abi_MariaAttackReturn(void)
{
    MariaAttackReturn();
}

/*
 * @routine 01E1:76D MariaAttackObject
 * @run 6E:C76D
 * WHAT IT DOES: The handler of kind 1, her attack's object: its state's step (MariaAttackStates: start, fly, fall,
 * return).
 * HOW: MariaStateDoubled, then the JMP (table,X).
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the state's step.
 */
void MariaAttackObject(void)
{
    uint16_t target = state_handler(0xC770, MariaAttackStates);

    switch (target) {
    case ATTACK_START:
        MariaAttackStart();
        break;
    case ATTACK_FLY:
        MariaAttackFly();
        break;
    case ATTACK_FALL:
        MariaAttackFall();
        break;
    case ATTACK_RETURN:
        MariaAttackReturn();
        break;
    default:
        run_routine(target);
        break;
    }
}

void abi_MariaAttackObject(void)
{
    MariaAttackObject();
}

/* From $C910: the attack's start, then item 5's speeds and attribute, and her facing back from the stack. */
static void item5_start_tail(void)
{
    uint8_t object, value;

    jsr_push(0xC913);
    MariaAttackStart();
    jsr_pop();
    object = cpu.x;
    p_lda(ITEM5_VX);
    jsr_push(0xC918);
    p_obj_arm(MariaSetVelocityXFromTable(object, ITEM5_VX));
    jsr_pop();
    jsr_push(0xC91B);
    abi_ended(MariaObjectFacingVelocity(object));
    jsr_pop();
    p_lda(ITEM5_VY);
    jsr_push(0xC920);
    p_obj_arm(MariaSetVelocityYFromTable(object, ITEM5_VY));
    jsr_pop();
    p_lda(ITEM5_FALL_LO);
    WR8(OBJ->fall_lo[object], ITEM5_FALL_LO);
    p_lda(0);
    WR8(OBJ->fall_hi[object], 0);
    p_lda(ITEM5_ATTR);
    WR8(OBJ->sprite_attr[object], ITEM5_ATTR);
    value = pull8();
    p_lda(value);
    WR8(OBJ->facing_left[OBJECT_PLAYER], value);
}

/*
 * @routine 01E1:8ED MariaItem5Pair
 * @run 6E:C8ED
 * WHAT IT DOES: The first of item 5's pair (slot 5 or 8): puts the second in the next slot, then starts this one
 * facing the other way from her.
 * HOW: Entered by MariaItem5Start's branch, with her facing on the stack: OBJ_KIND 5 one slot up, her facing
 * flipped, then the start's tail.
 * WAITS: no.
 * Entry: X the slot, her facing pushed. Exit: as MariaItem5Start.
 */
void MariaItem5Pair(void)
{
    uint8_t object = cpu.x;
    uint8_t value;

    cpu.x = (uint8_t)(object + 1);
    abi_nz(cpu.x);
    p_lda(KIND_ITEM5);
    WR8(OBJ->kind[cpu.x], KIND_ITEM5);
    cpu.x = object;
    abi_nz(cpu.x);
    value = RD8(OBJ->facing_left[OBJECT_PLAYER]);
    p_lda(value);
    p_lda((uint8_t)(value ^ 0x01));
    p_lda((uint8_t)(cpu.a & 0x01));
    WR8(OBJ->facing_left[OBJECT_PLAYER], cpu.a);
    item5_start_tail();
}

void abi_MariaItem5Pair(void)
{
    MariaItem5Pair();
}

/*
 * @routine 01E1:8F6 MariaItem5Start
 * @run 6E:C8F6
 * WHAT IT DOES: Starts item 5 as her attack's object starts, with X speed entry 4, Y speed entry 5, falling by $0044
 * a frame and sprite attribute $85. From slot 5 or 8 it is the first of a pair (MariaItem5Pair).
 * HOW: Her facing on the stack; MariaAttackStart; the velocity tables and MariaObjectFacingVelocity.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: A her facing, N and Z from it; the rest as the velocity table left them.
 */
void MariaItem5Start(void)
{
    uint8_t value, object;

    value = RD8(OBJ->facing_left[OBJECT_PLAYER]);
    p_lda(value);
    push8(value);
    object = p_ldx_object();
    p_cmp(object, ITEM5_FIRST);
    if (object != ITEM5_FIRST) {
        p_cmp(object, ITEM5_SECOND);
    }
    if (object == ITEM5_FIRST || object == ITEM5_SECOND) {
        MariaItem5Pair();
        return;
    }
    item5_start_tail();
}

void abi_MariaItem5Start(void)
{
    MariaItem5Start();
}

/*
 * @routine 01E2:134 MariaItem5Fly
 * @run 6E:C934
 * WHAT IT DOES: Item 5 flying out: as her attack's object.
 * HOW: The tail JMP to MariaAttackFly.
 * WAITS: no.
 * Entry, exit: as MariaAttackFly.
 */
void MariaItem5Fly(void)
{
    MariaAttackFly();
}

void abi_MariaItem5Fly(void)
{
    MariaItem5Fly();
}

/*
 * @routine 01E2:137 MariaObjectState2Work6
 * @run 6E:C937
 * WHAT IT DOES: In state 2, sets an object's OBJ_WORK_A to 6. Nothing is known to reach it: it follows
 * MariaItem5Fly's jump and no call, jump or table found goes to it.
 * HOW: A compare of OBJ_STATE with 2.
 * WAITS: no.
 * Entry: X the object. Exit: not state 2, A the state, N, Z and C from comparing it with 2. Else A 6, N and Z from
 * it, C set. X, Y and V kept.
 */
void MariaObjectState2Work6(void)
{
    uint8_t object = cpu.x;
    uint8_t state = RD8(OBJ->state[object]);

    p_lda(state);
    p_cmp(state, STATE2);
    if (state != STATE2) {
        return;
    }
    p_lda(STATE2_WORK_A);
    WR8(OBJ->work_a[object], STATE2_WORK_A);
}

void abi_MariaObjectState2Work6(void)
{
    MariaObjectState2Work6();
}

/*
 * @routine 01E2:145 MariaItem5Fall
 * @run 6E:C945
 * WHAT IT DOES: Item 5 after its turn: as her attack's object.
 * HOW: The tail JMP to MariaAttackFall.
 * WAITS: no.
 * Entry, exit: as MariaAttackFall.
 */
void MariaItem5Fall(void)
{
    MariaAttackFall();
}

void abi_MariaItem5Fall(void)
{
    MariaItem5Fall();
}

/*
 * @routine 01E2:148 MariaItem5Return
 * @run 6E:C948
 * WHAT IT DOES: Item 5 homing back: as her attack's object.
 * HOW: The tail JMP to MariaAttackReturn.
 * WAITS: no.
 * Entry, exit: as MariaAttackReturn.
 */
void MariaItem5Return(void)
{
    MariaAttackReturn();
}

void abi_MariaItem5Return(void)
{
    MariaItem5Return();
}

/*
 * @routine 01E1:8DF MariaItem5
 * @run 6E:C8DF
 * WHAT IT DOES: The handler of kind 5, item 5: its state's step (MariaItem5States: start, fly, fall, return).
 * HOW: MariaStateDoubled, then the JMP (table,X).
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as the state's step.
 */
void MariaItem5(void)
{
    uint16_t target = state_handler(0xC8E2, MariaItem5States);

    switch (target) {
    case ITEM5_START:
        MariaItem5Start();
        break;
    case ITEM5_FLY:
        MariaItem5Fly();
        break;
    case ITEM5_FALL:
        MariaItem5Fall();
        break;
    case ITEM5_RETURN:
        MariaItem5Return();
        break;
    default:
        run_routine(target);
        break;
    }
}

void abi_MariaItem5(void)
{
    MariaItem5();
}
