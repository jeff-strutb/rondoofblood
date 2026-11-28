/*
 * Maria walking (state 1) and crouching (state 2), and the slide she starts from a crouch (01DD-01DF, bank $6D).
 *
 * The crouch's steps by OBJ_SUBSTATE (MariaCrouchSteps): crouched, the crouching attack, nothing, the crouch's start,
 * standing up. Its ground checks, like the stand's, end the step themselves when one fires (PLA, PLA and a JMP), and
 * MariaAnimationWait ends a step while the animation runs the same way.
 *
 * The slide: jump pressed while crouched with a direction held, not at that edge of the screen, and nothing of bit 1
 * ahead of her. It is state $0D with the direction still held, else $0F, at X speed $02C0 the way she faces. A tile
 * of bit 3 under her (or 8 rows less below) starts MariaBit3Burst; otherwise slots 1 and 2 are replaced as kind 4.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"

enum {
    OBJECT_PLAYER = 0,
    STATE_SLIDE_HELD = 0x0D, STATE_SLIDE = 0x0F,
    STEP_ATTACK = 1, STEP_RISE = 4,
    ANIMATION_CROUCH_RISE = 0x0F, ANIMATION_CROUCH_ATTACK = 0x21, ANIMATION_SLIDE_HELD = 0x16, ANIMATION_SLIDE = 0x17,
    PAD_RIGHT = 0x20, PAD_LEFT = 0x80, PAD_SIDES = 0xA0,
    EDGE_LEFT = 0x10, EDGE_RIGHT = 0xF0, AHEAD_BELOW = 0x10, NEAR_BELOW = 0x08, TILE_AHEAD_BLOCKS = 0x02,
    TILE_BIT3 = 0x08, BURST_SOUND = 0x23, BURST_KIND = 1, BURST_SCRIPT = 8, BURST_DROP = 0x10,
    BURST_TYPE = 0x9A, BURST_OBJECTS = 6, SCRIPT_SLOT_PLAYER = 7, KIND_REPLACED = 4, KIND_REPLACED_5 = 5,
    CROUCH_STEP_DOWN = 0xA8AD, CROUCH_STEP_ATTACK = 0xA912, CROUCH_STEP_NONE = 0xA8AC, CROUCH_STEP_START = 0xA923,
    CROUCH_STEP_RISE = 0xA93A
};

#define MariaCrouchSteps_TBL MariaCrouchSteps
#define MariaSlideAhead_TBL MariaSlideAhead
#define MariaSlideSpeeds_TBL MariaSlideSpeeds

/* PLA, PLA: the caller's return address taken off the stack; A the second byte pulled, N and Z from it. */
static Regs drop_caller(void)
{
    uint8_t value;

    pull8();
    value = pull8();
    return r_lda(regs_kept(), value);
}

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

/* The A a chain leaves (the read's answer, for the AND after it). */
static uint8_t a_after(uint8_t x, uint8_t y, MExit e)
{
    MExit t = mx_none();
    Cpu saved = cpu;
    uint8_t a;

    mx_regs(&t, r_y_only(r_x_only(regs_kept(), x), y));
    mx_then(&t, e);
    mx_exit(t);
    a = cpu.a;
    cpu = saved;
    return a;
}

/* ------------------------------------------------------------------------------------------------ walking */

/*
 * @routine 01DD:7EB MariaStateWalk
 * @run 6D:A7EB
 * WHAT IT DOES: State 1, walking: the columns under her and a floor (one row lower if not), else she falls; the jump,
 * a landing, the item crash, the attack, a ledge, MariaUpHeld's crouch; then her animation ticks and, while the way she
 * faces is held, she keeps the walking speed; let go, she stands.
 * HOW: MariaReadColumns and MariaFloorCheck (twice, with INC of her Y between), MariaJumpPressed, MariaLandOnFloor,
 * MariaTryItemCrash, MariaUseItem, MariaLedgeCheck, MariaUpHeld, each carry set ending in its JMP; MariaAnimationTick;
 * BBR5 or BBR7 of zPadHeld by OBJ_FACING_LEFT; the tail JMPs to MariaSpeedWalk and MariaStand.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the routine it ended in.
 */
MExit MariaStateWalk(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    FloorResult floor;
    ItemCrash crash;
    TurnWindow use;
    MariaLedge ledge;
    bool jump, up, held;
    uint8_t facing;
    AnimThen moved;

    jsr_push(0xA7EE);
    mx_columns(&e, MariaReadColumns());
    jsr_pop();
    jsr_push(0xA7F1);
    floor = MariaFloorCheck();
    jsr_pop();
    mx_floor(&e, floor);
    if (floor.how == FLOOR_NONE) {
        INC8(OBJ->y_lo[OBJECT_PLAYER]);
        jsr_push(0xA7F9);
        mx_columns(&e, MariaReadColumns());
        jsr_pop();
        jsr_push(0xA7FC);
        floor = MariaFloorCheck();
        jsr_pop();
        mx_floor(&e, floor);
        if (floor.how == FLOOR_NONE) {
            moved = MariaToFall();
            mx_animation(&e, moved.animation);
            mx_regs(&e, moved.regs);
            return e;
        }
    }
    jsr_push(0xA804);
    jump = MariaJumpPressed();
    jsr_pop();
    mx_regs(&e, r_c(regs_kept(), jump));
    if (jump) {
        mx_then(&e, MariaJump());
        return e;
    }
    jsr_push(0xA80C);
    mx_landing(&e, MariaLandOnFloor());
    jsr_pop();
    jsr_push(0xA80F);
    crash = MariaTryItemCrash();
    jsr_pop();
    mx_crash(&e, crash);
    if (crash.how == CRASH_STARTED) {
        mx_then(&e, MariaCrashOnGround(x_after(x, y, e), y_after(x, y, e)));
        return e;
    }
    jsr_push(0xA817);
    use = MariaUseItem();
    jsr_pop();
    mx_turn(&e, use);
    if (use.end.c_set && use.end.c) {
        mx_then(&e, MariaAttack(x_after(x, y, e), y_after(x, y, e)));
        return e;
    }
    jsr_push(0xA81F);
    ledge = MariaLedgeCheck();
    jsr_pop();
    mx_ledge(&e, ledge);
    if (mx_carry(e)) {
        mx_then(&e, MariaClimbStart(x_after(x, y, e)));
        return e;
    }
    jsr_push(0xA827);
    up = MariaUpHeld();
    jsr_pop();
    mx_regs(&e, r_c(regs_kept(), up));
    if (up) {
        moved = MariaCrouch();
        mx_animation(&e, moved.animation);
        mx_regs(&e, moved.regs);
        return e;
    }
    jsr_push(0xA82F);
    mx_animation(&e, MariaAnimationTick());
    jsr_pop();
    facing = RD8(OBJ->facing_left[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), facing));
    held = (RD8(ZP->pad_held) & (facing != 0 ? PAD_LEFT : PAD_RIGHT)) != 0;
    if (held) {
        mx_regs(&e, MariaSpeedWalk());
        return e;
    }
    moved = MariaStand();
    mx_animation(&e, moved.animation);
    mx_regs(&e, moved.regs);
    return e;
}

void abi_MariaStateWalk(void)
{
    mx_exit(MariaStateWalk(cpu.x, cpu.y));
}

/* ------------------------------------------------------------------------------------------------ the burst */

/*
 * @routine 01DF:028 MariaStart9AObjects
 * @run 6D:B028
 * WHAT IT DOES: Starts six objects of type $9A where object X is, in free slots from $30: each with OBJ_WORK_D the
 * value given and OBJ_WORK_E its number, 0 to 5. With no free slot, stops there.
 * HOW: zTemp00 the value, zBurstCount the count; FindFreeSlotFrom30, StartObjectAtPlace (both through the kernel's
 * vectors). Y is kept on the stack.
 * WAITS: no.
 * Entry: A the value, X the object they start at, Y kept. Exit: Y as given, N and Z from it. Six started, A 5, C set;
 * else A the last type FindFreeSlotFrom30 read, C clear. X and V kept.
 */
Regs MariaStart9AObjects(uint8_t value, uint8_t from, uint8_t y)
{
    uint8_t count = 0, slot, last = 0;
    bool found;
    Regs r = regs_kept();

    WR8(ZP->temp00, value);
    push8(y);
    for (;;) {
        WR8(ZP->burst_count, count);
        jsr_push(0xB031);
        slot = FindFreeSlotFrom30(&last, &found);
        jsr_pop();
        if (!found) {
            r = r_c(r_a_only(r, last), false);
            break;
        }
        value = RD8(ZP->temp00);
        WR8(OBJ->work_d[slot], value);
        jsr_push(0xB03D);
        StartObjectAtPlace(BURST_TYPE, from, slot);
        jsr_pop();
        value = RD8(ZP->burst_count);
        WR8(OBJ->work_e[slot], value);
        count = RD8(ZP->burst_count);
        count++;
        if (count >= BURST_OBJECTS) {
            r = r_c(r_a_only(r, value), true);
            break;
        }
    }
    y = pull8();
    return r_ldy(r, y);
}

void abi_MariaStart9AObjects(void)
{
    regs_exit(MariaStart9AObjects(cpu.a, cpu.x, cpu.y));
}

/*
 * @routine 01DE:7F1 MariaBit3Burst
 * @run 6D:AFF1
 * WHAT IT DOES: Sound $23; each of slots 2 and 1 that holds kind 1 is emptied and starts the hit's script 8; then six
 * objects of type $9A start where she is, 16 rows below her (MariaStart9AObjects with 0), her Y put back after.
 * HOW: SoundCommand; OBJ_KIND less one; ClearObject, MariaStartHitScript; her Y bytes on the stack around the 16-bit add.
 * The slots are counted down in X, which MariaStartHitScript leaves 7: after a hit the count goes on from slot 6.
 * WAITS: no.
 * Entry: X and Y the caller's (SoundCommand is given them). Exit: A her Y's low byte, N and Z from it, X 0; Y and C as
 * MariaStart9AObjects left them; V from the add to her Y's high byte.
 */
MExit MariaBit3Burst(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t slot, kind, lo, hi;
    Sum low, high;

    jsr_push(0xAFF6);
    mx_sound(&e, BURST_SOUND, SoundCommand(BURST_SOUND, x, y));
    jsr_pop();
    /* X counts the slots down; MariaStartHitScript leaves X 7, so after a hit the count runs on from slot 6. */
    slot = 2;
    do {
        kind = RD8(OBJ->kind[slot]);
        if ((uint8_t)(kind - 1) == 0) {
            jsr_push(0xB001);
            ClearObject(slot);
            jsr_pop();
            jsr_push(0xB006);
            MariaStartHitScript(BURST_SCRIPT);
            jsr_pop();
            mx_script(&e, BURST_SCRIPT);
            slot = SCRIPT_SLOT_PLAYER;
        }
        slot--;
    } while (slot != 0);
    mx_regs(&e, r_nz(r_x_only(regs_kept(), 0), 0));
    lo = RD8(OBJ->y_lo[OBJECT_PLAYER]);
    push8(lo);
    low = add_bytes(lo, BURST_DROP, false);
    WR8(OBJ->y_lo[OBJECT_PLAYER], low.value);
    hi = RD8(OBJ->y_hi[OBJECT_PLAYER]);
    push8(hi);
    high = add_bytes(hi, 0, low.carry);
    WR8(OBJ->y_hi[OBJECT_PLAYER], high.value);
    mx_regs(&e, r_v(r_a_only(regs_kept(), 0), high.overflow));
    jsr_push(0xB01F);
    mx_regs(&e, MariaStart9AObjects(0, 0, y_after(x, y, e)));
    jsr_pop();
    hi = pull8();
    WR8(OBJ->y_hi[OBJECT_PLAYER], hi);
    lo = pull8();
    WR8(OBJ->y_lo[OBJECT_PLAYER], lo);
    mx_regs(&e, r_lda(regs_kept(), lo));
    return e;
}

void abi_MariaBit3Burst(void)
{
    mx_exit(MariaBit3Burst(cpu.x, cpu.y));
}

/*
 * @routine 01DE:7E3 MariaCheckBit3Under
 * @run 6D:AFE3
 * WHAT IT DOES: Starts MariaBit3Burst when any of the three columns under her read bit 3.
 * HOW: RichterColumnHit's first three ORed; a branch into MariaBit3Burst.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: none, A 0, Z set; else as MariaBit3Burst.
 */
MExit MariaCheckBit3Under(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t bits = RD8(PVARS->column_hit[0]);

    bits |= RD8(PVARS->column_hit[1]);
    bits |= RD8(PVARS->column_hit[2]);
    bits &= TILE_BIT3;
    mx_regs(&e, r_lda(regs_kept(), bits));
    if (bits == 0) {
        return e;
    }
    mx_then(&e, MariaBit3Burst(x, y));
    return e;
}

void abi_MariaCheckBit3Under(void)
{
    mx_exit(MariaCheckBit3Under(cpu.x, cpu.y));
}

/*
 * @routine 01DE:7DF MariaReplaceSlotsOneTwoKind5
 * @run 6D:AFDF
 * WHAT IT DOES: As MariaReplaceSlotsOneTwo, with kind 5: slots 1 and 2 cleared as kind 5, each unless it holds kind 1
 * or 2.
 * HOW: Y 5, then a branch into MariaReplaceSlotsOneTwo past its LDY: OBJ_KIND compared with 1 and 2; a BSR to
 * MariaClearSlotAsKind for slot 1, a run on into it for slot 2.
 * WAITS: no.
 * Entry: nothing. Exit: Y 5, X 2; slot 2 kept, A its kind, N, Z and C from comparing it with 1 or 2; else A 5, N and
 * Z from it, C from comparing the kind with 2. V kept.
 */
Regs MariaReplaceSlotsOneTwoKind5(void)
{
    Regs r = r_y_only(regs_kept(), KIND_REPLACED_5);
    uint8_t slot, kind;

    for (slot = 1; slot <= 2; slot++) {
        r = r_x_only(r, slot);
        if (slot == 2) {
            r = r_nz(r, slot);
        }
        kind = RD8(OBJ->kind[slot]);
        r = r_cmp(r_lda(r, kind), kind, 1);
        if (kind == 1) {
            continue;
        }
        r = r_cmp(r, kind, 2);
        if (kind == 2) {
            continue;
        }
        if (slot == 1) {
            jsr_push(0xAFCB);
            MariaClearSlotAsKind(slot, KIND_REPLACED_5);
            jsr_pop();
        } else {
            MariaClearSlotAsKind(slot, KIND_REPLACED_5);
        }
        r = r_lda(r, KIND_REPLACED_5);
    }
    return r;
}

void abi_MariaReplaceSlotsOneTwoKind5(void)
{
    regs_exit(MariaReplaceSlotsOneTwoKind5());
}

/* ------------------------------------------------------------------------------------------------ the slide */

/*
 * @routine 01DF:083 MariaReadAhead
 * @run 6D:B083
 * WHAT IT DOES: Reads the terrain a given distance across from her and 16 rows below, for bit 1.
 * HOW: The distance plus her X to zTemp00, her Y plus $10 and its carry, MariaReadBelow; AND #2.
 * WAITS: no.
 * Entry: A the distance, X and Y the caller's (the read's handler is entered with them). Exit: A bit 1 of the read,
 * N and Z from it; Y, C and V as MariaReadBelow left them. X kept.
 */
MExit MariaReadAhead(uint8_t distance, uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t value = RD8(OBJ->x_lo[OBJECT_PLAYER]);
    Sum across = add_bytes(distance, value, false);
    Sum row;
    uint8_t answer;

    WR8(ZP->temp00, across.value);
    value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
    row = add_bytes(value, AHEAD_BELOW, false);
    mx_regs(&e, r_v(r_c(r_lda(regs_kept(), row.value), row.carry), row.overflow));
    cpu.x = x;
    cpu.y = y;
    jsr_push(0xB092);
    mx_read_below(&e, MariaReadBelow(row.value, row.carry), row.carry);
    jsr_pop();
    answer = (uint8_t)(a_after(x, y, e) & TILE_AHEAD_BLOCKS);
    mx_regs(&e, r_lda(regs_kept(), answer));
    return e;
}

void abi_MariaReadAhead(void)
{
    mx_exit(MariaReadAhead(cpu.a, cpu.x, cpu.y));
}

/*
 * @routine 01DF:04B MariaSlideCheck
 * @run 6D:B04B
 * WHAT IT DOES: Whether she slides from the crouch: she faces the way the pad holds (MariaFaceByPad); with the jump
 * pressed, short of the screen's edge that way ($10 left, $F0 right) and with no bit 1 ahead of her (7 across), C
 * set. Else C clear and her facing put back.
 * HOW: The facing on the stack; MariaFaceByPad with zPadHeld, MariaJumpPressed, the edge, MariaSlideAhead by the
 * facing and a BSR to MariaReadAhead.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: A the facing she had, N and Z from it; C set to slide; Y as the last call left
 * it (the facing when the read was made after it), V likewise. X kept.
 */
MExit MariaSlideCheck(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t before, value, facing, ahead;
    bool jump, room = false;
    MExit read;

    before = RD8(OBJ->facing_left[OBJECT_PLAYER]);
    push8(before);
    value = RD8(ZP->pad_held);
    jsr_push(0xB054);
    mx_regs(&e, MariaFaceByPad(value));
    jsr_pop();
    jsr_push(0xB057);
    jump = MariaJumpPressed();
    jsr_pop();
    mx_regs(&e, r_c(regs_kept(), jump));
    if (jump) {
        facing = RD8(OBJ->facing_left[OBJECT_PLAYER]);
        value = RD8(OBJ->x_lo[OBJECT_PLAYER]);
        if (facing != 0) {
            mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, EDGE_LEFT));
            room = value > EDGE_LEFT;
        } else {
            mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, EDGE_RIGHT));
            room = value < EDGE_RIGHT;
        }
        if (room) {
            facing = RD8(OBJ->facing_left[OBJECT_PLAYER]);
            ahead = TBL8(MariaSlideAhead, facing);
            mx_regs(&e, r_lda(r_ldy(regs_kept(), facing), ahead));
            jsr_push(0xB078);
            read = MariaReadAhead(ahead, x, facing);
            jsr_pop();
            mx_then(&e, read);
            if (mx_zero(read)) {
                value = pull8();
                mx_regs(&e, r_c(r_lda(regs_kept(), value), true));
                return e;
            }
        }
    }
    value = pull8();
    WR8(OBJ->facing_left[OBJECT_PLAYER], value);
    mx_regs(&e, r_c(r_lda(regs_kept(), value), false));
    (void)y;
    return e;
}

void abi_MariaSlideCheck(void)
{
    mx_exit(MariaSlideCheck(cpu.x, cpu.y));
}

/*
 * @routine 01DF:097 MariaSlideStart
 * @run 6D:B097
 * WHAT IT DOES: Starts her slide: a tile of bit 3 under her (16 rows below, or else 8) starts MariaBit3Burst,
 * otherwise slots 1 and 2 are replaced as kind 4; she stops, takes the slide's speed the way she faces, and is in
 * state $0D (animation $16) with left or right still held, else state $0F (animation $17), step 0.
 * HOW: MariaReadBelow twice at her X; MariaBit3Burst or MariaReplaceSlotsOneTwo; MariaStopMoving; MariaSlideSpeeds by
 * the facing doubled; TST of zPadHeld; the tail JMP to MariaSetAnimation.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaSetAnimation, after A the animation, Y the facing doubled, C its bit 7.
 */
MExit MariaSlideStart(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t value, facing, index, lo, hi, held;
    Sum row;
    bool burst = false;

    value = RD8(OBJ->x_lo[OBJECT_PLAYER]);
    WR8(ZP->temp00, value);
    value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
    row = add_bytes(value, AHEAD_BELOW, false);
    mx_regs(&e, r_v(r_c(r_lda(regs_kept(), row.value), row.carry), row.overflow));
    cpu.x = x;
    cpu.y = y;
    jsr_push(0xB0A5);
    mx_read_below(&e, MariaReadBelow(row.value, row.carry), row.carry);
    jsr_pop();
    value = (uint8_t)(a_after(x, y, e) & TILE_BIT3);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value != 0) {
        burst = true;
        jsr_push(0xB0AC);
        mx_then(&e, MariaBit3Burst(x_after(x, y, e), y_after(x, y, e)));
        jsr_pop();
    } else {
        value = RD8(OBJ->x_lo[OBJECT_PLAYER]);
        WR8(ZP->temp00, value);
        value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
        row = add_bytes(value, NEAR_BELOW, false);
        mx_regs(&e, r_v(r_c(r_lda(regs_kept(), row.value), row.carry), row.overflow));
        cpu.x = x_after(x, y, e);
        cpu.y = y_after(x, y, e);
        jsr_push(0xB0BC);
        mx_read_below(&e, MariaReadBelow(row.value, row.carry), row.carry);
        jsr_pop();
        value = (uint8_t)(a_after(x, y, e) & TILE_BIT3);
        mx_regs(&e, r_lda(regs_kept(), value));
        if (value != 0) {
            burst = true;
            jsr_push(0xB0C3);
            mx_then(&e, MariaBit3Burst(x_after(x, y, e), y_after(x, y, e)));
            jsr_pop();
        }
    }
    if (!burst) {
        Ended replaced;

        jsr_push(0xB0C8);
        replaced = MariaReplaceSlotsOneTwo();
        jsr_pop();
        mx_regs(&e, r_x_only(regs_kept(), 2));
        {
            Regs re = regs_kept();

            if (replaced.a_set) {
                re = r_a_only(re, replaced.a);
            }
            if (replaced.nz_set) {
                re = r_nz(re, replaced.nz);
            }
            if (replaced.c_set) {
                re = r_c(re, replaced.c);
            }
            if (replaced.v_set) {
                re = r_v(re, replaced.v);
            }
            if (replaced.y_set) {
                re = r_y_only(re, replaced.y);
            }
            mx_regs(&e, re);
        }
    }
    jsr_push(0xB0CB);
    MariaStopMoving();
    jsr_pop();
    facing = RD8(OBJ->facing_left[OBJECT_PLAYER]);
    index = (uint8_t)(facing << 1);
    lo = TBL8(MariaSlideSpeeds, index);
    WR8(OBJ->vx_lo[OBJECT_PLAYER], lo);
    hi = TBL8(MariaSlideSpeeds, index + 1);
    WR8(OBJ->vx_hi[OBJECT_PLAYER], hi);
    mx_regs(&e, r_c(r_y_only(regs_kept(), index), (facing & 0x80) != 0));
    held = RD8(ZP->pad_held);
    mx_regs(&e, r_tst(regs_kept(), PAD_SIDES, held));
    if (held & PAD_SIDES) {
        WR8(OBJ->state[OBJECT_PLAYER], STATE_SLIDE_HELD);
        WR8(OBJ->substate[OBJECT_PLAYER], 0);
        mx_regs(&e, r_lda(regs_kept(), ANIMATION_SLIDE_HELD));
        mx_animation(&e, MariaSetAnimation(ANIMATION_SLIDE_HELD));
        return e;
    }
    WR8(OBJ->state[OBJECT_PLAYER], STATE_SLIDE);
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    mx_regs(&e, r_lda(regs_kept(), ANIMATION_SLIDE));
    mx_animation(&e, MariaSetAnimation(ANIMATION_SLIDE));
    return e;
}

void abi_MariaSlideStart(void)
{
    mx_exit(MariaSlideStart(cpu.x, cpu.y));
}

/* ------------------------------------------------------------------------------------------------ crouching */

/*
 * @routine 01DE:0BF MariaFaceByPad
 * @run 6D:A8BF
 * WHAT IT DOES: Faces her the way a pad byte holds right or left (bit 5 right, bit 7 left); neither or both, no change.
 * HOW: AND #$A0 and two compares; STZ then INC of OBJ_FACING_LEFT.
 * WAITS: no.
 * Entry: A the pad byte. Exit: A its bits 5 and 7; N and Z from the last test of them, or from the facing's INC when
 * left; C from the last compare (kept when neither is held). X, Y and V kept.
 */
Regs MariaFaceByPad(uint8_t pad)
{
    uint8_t sides = (uint8_t)(pad & PAD_SIDES);
    Regs r = r_lda(regs_kept(), sides);

    if (sides == 0) {
        return r;
    }
    r = r_cmp(r, sides, PAD_SIDES);
    if (sides == PAD_SIDES) {
        return r;
    }
    WR8(OBJ->facing_left[OBJECT_PLAYER], 0);
    r = r_cmp(r, sides, PAD_RIGHT);
    if (sides == PAD_RIGHT) {
        return r;
    }
    return r_nz(r, INC8(OBJ->facing_left[OBJECT_PLAYER]));
}

void abi_MariaFaceByPad(void)
{
    regs_exit(MariaFaceByPad(cpu.a));
}

/*
 * @routine 01DE:0D2 MariaCrouchRise
 * @run 6D:A8D2
 * WHAT IT DOES: Starts her standing up from the crouch: animation $0F, step 4.
 * HOW: MariaSetAnimation, then the store.
 * WAITS: no.
 * Entry: nothing. Exit: A 4, N and Z from it; Y and C as MariaSetAnimation left them. X and V kept.
 */
MExit MariaCrouchRise(void)
{
    MExit e = mx_none();

    mx_regs(&e, r_lda(regs_kept(), ANIMATION_CROUCH_RISE));
    jsr_push(0xA8D7);
    mx_animation(&e, MariaSetAnimation(ANIMATION_CROUCH_RISE));
    jsr_pop();
    WR8(OBJ->substate[OBJECT_PLAYER], STEP_RISE);
    mx_regs(&e, r_lda(regs_kept(), STEP_RISE));
    return e;
}

void abi_MariaCrouchRise(void)
{
    mx_exit(MariaCrouchRise());
}

/*
 * @routine 01DE:102 MariaCrouchAttack
 * @run 6D:A902
 * WHAT IT DOES: Her attack from the crouch: what the attack button said (MariaAttack); if it started her attack's
 * object, step 1 and animation $21.
 * HOW: MariaAttack's Z; the tail JMP to MariaSetAnimation.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaAttack when nothing started; else as MariaSetAnimation.
 */
MExit MariaCrouchAttack(uint8_t x, uint8_t y)
{
    MExit e;

    jsr_push(0xA905);
    e = MariaAttack(x, y);
    jsr_pop();
    if (!mx_zero(e)) {
        return e;
    }
    WR8(OBJ->substate[OBJECT_PLAYER], STEP_ATTACK);
    mx_regs(&e, r_lda(regs_kept(), ANIMATION_CROUCH_ATTACK));
    mx_animation(&e, MariaSetAnimation(ANIMATION_CROUCH_ATTACK));
    return e;
}

void abi_MariaCrouchAttack(void)
{
    mx_exit(MariaCrouchAttack(cpu.x, cpu.y));
}

/*
 * @routine 01DE:0DD MariaCrouchChecks
 * @run 6D:A8DD
 * WHAT IT DOES: The checks a crouch's step makes first: the columns under her; no footing, she falls; a landing; then
 * the item crash and the attack button. When one fires, the step ends there: its return address is pulled and the
 * fall, the item crash or the crouching attack runs in its place.
 * HOW: MariaReadColumns, MariaHasNoFooting, MariaLandOnFloor, MariaTryItemCrash, MariaUseItem; PLA, PLA and the JMP to
 * MariaToFall, MariaCrashOnGround or MariaCrouchAttack.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: nothing fired, as MariaUseItem with C clear. Fired, the caller's return address
 * gone and S two higher; as the routine jumped to, after A the return address's high byte.
 */
MariaDrop MariaCrouchChecks(uint8_t x, uint8_t y)
{
    MariaDrop r = { mx_none(), false };
    FootingCheck footing;
    ItemCrash crash;
    TurnWindow use;
    AnimThen fall;

    jsr_push(0xA8E0);
    mx_columns(&r.e, MariaReadColumns());
    jsr_pop();
    jsr_push(0xA8E3);
    footing = MariaHasNoFooting();
    jsr_pop();
    mx_footing(&r.e, footing);
    if (footing.no_floor) {
        r.dropped = true;
        mx_regs(&r.e, drop_caller());
        fall = MariaToFall();
        mx_animation(&r.e, fall.animation);
        mx_regs(&r.e, fall.regs);
        return r;
    }
    jsr_push(0xA8E8);
    mx_landing(&r.e, MariaLandOnFloor());
    jsr_pop();
    jsr_push(0xA8EB);
    crash = MariaTryItemCrash();
    jsr_pop();
    mx_crash(&r.e, crash);
    if (crash.how == CRASH_STARTED) {
        r.dropped = true;
        mx_regs(&r.e, drop_caller());
        mx_then(&r.e, MariaCrashOnGround(x_after(x, y, r.e), y_after(x, y, r.e)));
        return r;
    }
    jsr_push(0xA8F0);
    use = MariaUseItem();
    jsr_pop();
    mx_turn(&r.e, use);
    if (use.end.c_set && use.end.c) {
        r.dropped = true;
        mx_regs(&r.e, drop_caller());
        mx_then(&r.e, MariaCrouchAttack(x_after(x, y, r.e), y_after(x, y, r.e)));
    }
    return r;
}

void abi_MariaCrouchChecks(void)
{
    mx_exit(MariaCrouchChecks(cpu.x, cpu.y).e);
}

/*
 * @routine 01DE:0AD MariaCrouchFrame
 * @run 6D:A8AD
 * WHAT IT DOES: Step 0 of the crouch: the crouch's checks; with neither up nor a mark above her (MariaUpOrMarkedAbove
 * clear), she starts to stand; a slide, she slides; else she faces the way the pad was pressed.
 * HOW: MariaCrouchChecks, MariaUpOrMarkedAbove's carry, MariaSlideCheck's carry and the JMP to MariaSlideStart; zPadPressed
 * on into MariaFaceByPad.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the routine it ended in, or MariaFaceByPad with zPadPressed.
 */
MExit MariaCrouchFrame(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    MariaDrop checks;
    UpOrMarked up;
    MExit slide;

    jsr_push(0xA8B0);
    checks = MariaCrouchChecks(x, y);
    if (checks.dropped) {
        return checks.e;
    }
    jsr_pop();
    mx_then(&e, checks.e);
    jsr_push(0xA8B3);
    up = MariaUpOrMarkedAbove();
    jsr_pop();
    mx_up_marked(&e, up);
    if (!up.c) {
        mx_then(&e, MariaCrouchRise());
        return e;
    }
    jsr_push(0xA8B8);
    slide = MariaSlideCheck(x_after(x, y, e), y_after(x, y, e));
    jsr_pop();
    mx_then(&e, slide);
    if (mx_carry(e)) {
        mx_then(&e, MariaSlideStart(x_after(x, y, e), y_after(x, y, e)));
        return e;
    }
    mx_regs(&e, MariaFaceByPad(RD8(ZP->pad_pressed)));
    return e;
}

void abi_MariaCrouchFrame(void)
{
    mx_exit(MariaCrouchFrame(cpu.x, cpu.y));
}

/*
 * @routine 01DE:0AC MariaCrouchIdle
 * @run 6D:A8AC
 * WHAT IT DOES: Step 2 of the crouch: nothing.
 * HOW: RTS.
 * WAITS: no.
 * Entry, exit: nothing; all registers and flags kept.
 */
void MariaCrouchIdle(void)
{
}

void abi_MariaCrouchIdle(void)
{
    MariaCrouchIdle();
}

/*
 * @routine 01DE:112 MariaCrouchAttackFrame
 * @run 6D:A912
 * WHAT IT DOES: Step 1 of the crouch, the attack: while her animation runs, as step 0; when it has stopped, the attack
 * button's outcome cleared and back to step 0.
 * HOW: MariaAnimationRunning's Z; the JMP to MariaCrouchFrame.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: running, as MariaCrouchFrame; else A 0, Z set, the rest as MariaAnimationRunning
 * left it.
 */
MExit MariaCrouchAttackFrame(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    AnimationStep running;

    jsr_push(0xA915);
    running = MariaAnimationRunning();
    jsr_pop();
    mx_animation(&e, running);
    if (!mx_zero(mx_one_animation(running))) {
        mx_then(&e, MariaCrouchFrame(x_after(x, y, e), y_after(x, y, e)));
        return e;
    }
    WR8(PVARS->attack_kind, 0);
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    mx_regs(&e, r_lda(regs_kept(), 0));
    return e;
}

void abi_MariaCrouchAttackFrame(void)
{
    mx_exit(MariaCrouchAttackFrame(cpu.x, cpu.y));
}

/*
 * @routine 01DE:123 MariaCrouchStart
 * @run 6D:A923
 * WHAT IT DOES: Step 3 of the crouch, going down: the pad's forced presses off, the crouch's checks, a slide; while her
 * animation runs, nothing more; when it has stopped, step 0.
 * HOW: MariaPadForcesOff, MariaCrouchChecks, MariaSlideCheck and the JMP to MariaSlideStart; MariaAnimationWait, which
 * ends the step while the animation runs.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the routine it ended in; at the animation's end, A 0, Z set.
 */
MExit MariaCrouchStart(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    MariaDrop checks;
    MExit slide;
    AnimationWaitEnd wait;

    jsr_push(0xA926);
    MariaPadForcesOff();
    jsr_pop();
    jsr_push(0xA929);
    checks = MariaCrouchChecks(x, y);
    if (checks.dropped) {
        return checks.e;
    }
    jsr_pop();
    mx_then(&e, checks.e);
    jsr_push(0xA92C);
    slide = MariaSlideCheck(x_after(x, y, e), y_after(x, y, e));
    jsr_pop();
    mx_then(&e, slide);
    if (mx_carry(e)) {
        mx_then(&e, MariaSlideStart(x_after(x, y, e), y_after(x, y, e)));
        return e;
    }
    jsr_push(0xA934);
    wait = MariaAnimationWait();
    mx_animation(&e, wait.step);
    if (wait.dropped) {
        return e;
    }
    jsr_pop();
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    mx_regs(&e, r_lda(regs_kept(), 0));
    return e;
}

void abi_MariaCrouchStart(void)
{
    mx_exit(MariaCrouchStart(cpu.x, cpu.y));
}

/*
 * @routine 01DE:13A MariaStandUp
 * @run 6D:A93A
 * WHAT IT DOES: Step 4 of the crouch, standing up: the stand's ground checks; while her animation runs, nothing more;
 * when it has stopped, she stands.
 * HOW: MariaGroundChecks, MariaAnimationWait, then the tail JMP to MariaStand.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the routine it ended in, or MariaStand.
 */
MExit MariaStandUp(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    MariaDrop checks;
    AnimationWaitEnd wait;
    AnimThen stand;

    jsr_push(0xA93D);
    checks = MariaGroundChecks(x, y);
    if (checks.dropped) {
        return checks.e;
    }
    jsr_pop();
    mx_then(&e, checks.e);
    jsr_push(0xA940);
    wait = MariaAnimationWait();
    mx_animation(&e, wait.step);
    if (wait.dropped) {
        return e;
    }
    jsr_pop();
    stand = MariaStand();
    mx_animation(&e, stand.animation);
    mx_regs(&e, stand.regs);
    return e;
}

void abi_MariaStandUp(void)
{
    mx_exit(MariaStandUp(cpu.x, cpu.y));
}

/*
 * @routine 01DE:09C MariaStateCrouch
 * @run 6D:A89C
 * WHAT IT DOES: State 2, crouching: the step's handler from MariaCrouchSteps (crouched, the attack, nothing, going
 * down, standing up).
 * HOW: MariaStepIndex, then the JMP (table,X).
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the step's handler, after A and X the step doubled and C its bit 7.
 */
MExit MariaStateCrouch(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    uint8_t step, doubled, lo, hi;
    uint16_t target;

    jsr_push(0xA89F);
    step = MariaStepIndex();
    jsr_pop();
    doubled = (uint8_t)(step << 1);
    mx_regs(&e, r_c(r_ldx(r_lda(regs_kept(), doubled), doubled), (step & 0x80) != 0));
    lo = TBL8(MariaCrouchSteps, doubled);
    hi = TBL8(MariaCrouchSteps, doubled + 1);
    target = (uint16_t)(lo | (hi << 8));
    switch (target) {
    case CROUCH_STEP_DOWN:
        mx_then(&e, MariaCrouchFrame(doubled, y));
        break;
    case CROUCH_STEP_ATTACK:
        mx_then(&e, MariaCrouchAttackFrame(doubled, y));
        break;
    case CROUCH_STEP_NONE:
        MariaCrouchIdle();
        break;
    case CROUCH_STEP_START:
        mx_then(&e, MariaCrouchStart(doubled, y));
        break;
    case CROUCH_STEP_RISE:
        mx_then(&e, MariaStandUp(doubled, y));
        break;
    default:
        mx_exit(e);
        run_routine(target);
        e.elsewhere = true;
        break;
    }
    (void)x;
    return e;
}

void abi_MariaStateCrouch(void)
{
    mx_exit(MariaStateCrouch(cpu.x, cpu.y));
}
