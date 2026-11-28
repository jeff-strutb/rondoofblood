/*
 * Item 4's state ($10) and her pad sequence's move ($11), 01DF-01E0 in bank $6D.
 *
 * Item 4 is a state of its own (MariaThrowItem starts it): its steps by OBJ_SUBSTATE (MariaItem4Steps) are the start,
 * which copies its tiles in VRAM four words a frame ($6Fxx to $6Cxx, OBJ_WORK_D counting to $20), a walk with the
 * pad's overrides, and the air. Button II pressed with up held ends it (MariaItem4End).
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"

enum {
    OBJECT_PLAYER = 0, SLOT_ITEM = 5,
    STEP_WALK = 1, STEP_AIR = 2, STEP_FELL = 2,
    ANIMATION_ITEM4_LAND = 0x28, FRAME_ITEM4_STILL = 0x42, FRAME_SEQUENCE_END = 0x46, SEQUENCE_LAND_TIMER = 0x0C,
    ITEM4_END_TIMER = 0x30, ITEM4_LOW_Y = 0xB0, ITEM4_VY_LO = 0x80, ITEM4_VY_HI = 0xFE, ITEM4_GRAVITY = 0x48,
    FALL_SPEED_TOP = 7, ITEM4_HARD = 6, SOUND_ITEM4_HARD = 0x30, SOUND_ITEM4_LAND = 0x4A,
    MARK_FLOOR = 0x24, MARK_DOWN_ONLY = 0x20,
    TILES_FROM_HI = 0x6F, TILES_TO_HI = 0x6C, TILES_ROW = 0x10, TILES_WORDS = 4, TILES_PAGE = 0x10, TILES_HIGH = 0x40,
    TILES_DONE = 0x20,
    PAD_II = 0x02, PAD_UP = 0x10, PAD_RIGHT = 0x20, PAD_LEFT = 0x80,
    ITEM4_STEP_START = 0xB6BD, ITEM4_STEP_WALK = 0xB76B, ITEM4_STEP_AIR = 0xB831
};

#define MariaItem4Steps_TBL MariaItem4Steps
#define MariaItem4WalkSpeeds_TBL MariaItem4WalkSpeeds

/* ------------------------------------------------------------------------------------------------ its tiles */

/*
 * @routine 01DF:728 MariaCopyVramWord
 * @run 6D:B728
 * WHAT IT DOES: Copies one VRAM word: read from zTemp01:zTemp00, written to zTemp03:zTemp00.
 * HOW: The VDC's read address, then its write address, from the data ports; the data selected; the word read from the
 * data ports and written back to them, low byte first.
 * WAITS: no.
 * Entry: zTemp00 the low byte of both addresses, zTemp01 the read's high byte, zTemp03 the write's. Exit: A the high
 * byte copied, N and Z from it. X, Y, C and V as SelectVramData left them (kept).
 */
Regs MariaCopyVramWord(void)
{
    uint8_t value;

    jsr_push(0xB72B);
    SelectVramReadAddress();
    jsr_pop();
    value = RD8(ZP->temp00);
    WR8(VDC->data_lo, value);
    value = RD8(ZP->temp01);
    WR8(VDC->data_hi, value);
    jsr_push(0xB738);
    SelectVramWriteAddress();
    jsr_pop();
    value = RD8(ZP->temp00);
    WR8(VDC->data_lo, value);
    value = RD8(ZP->temp03);
    WR8(VDC->data_hi, value);
    jsr_push(0xB745);
    SelectVramData();
    jsr_pop();
    value = RD8(VDC->data_lo);
    WR8(VDC->data_lo, value);
    value = RD8(VDC->data_hi);
    WR8(VDC->data_hi, value);
    return r_lda(regs_kept(), value);
}

void abi_MariaCopyVramWord(void)
{
    regs_exit(MariaCopyVramWord());
}

/*
 * @routine 01DF:6FA MariaItem4CopyTiles
 * @run 6D:B6FA
 * WHAT IT DOES: Copies four VRAM words of item 4's tiles, $10 apart, from $6Fxx to $6Cxx at the row OBJ_WORK_D gives
 * (from $10 on, $40 more and $10 less), and counts OBJ_WORK_D on; Z set once it reaches $20.
 * HOW: zTemp00, zTemp01 and zTemp03 set up; a BSR to MariaCopyVramWord four times; INC of OBJ_WORK_D, CMP #$20.
 * WAITS: no.
 * Entry: nothing. Exit: A OBJ_WORK_D, N, Z and C from comparing it with $20; Y 4. X kept, V from the subtract when it
 * was made, else kept.
 */
Regs MariaItem4CopyTiles(void)
{
    Regs r = regs_kept();
    uint8_t row = RD8(OBJ->work_d[OBJECT_PLAYER]);
    uint8_t value;
    Sum at;

    r = r_cmp(r_lda(r, row), row, TILES_PAGE);
    if (row >= TILES_PAGE) {
        Sum less = sub_bytes(row, TILES_PAGE, true);

        row = (uint8_t)(less.value | TILES_HIGH);
        r = r_v(r_c(r_lda(r, row), less.carry), less.overflow);
    }
    WR8(ZP->temp00, row);
    WR8(ZP->temp01, TILES_FROM_HI);
    WR8(ZP->temp03, TILES_TO_HI);
    for (uint8_t word = 0; word < TILES_WORDS; word++) {
        jsr_push(0xB713);
        MariaCopyVramWord();
        jsr_pop();
        value = RD8(ZP->temp00);
        at = add_bytes(value, TILES_ROW, false);
        WR8(ZP->temp00, at.value);
        r = r_v(regs_kept(), at.overflow);
    }
    INC8(OBJ->work_d[OBJECT_PLAYER]);
    value = RD8(OBJ->work_d[OBJECT_PLAYER]);
    return r_v(r_cmp(r_lda(r_y_only(regs_kept(), TILES_WORDS), value), value, TILES_DONE), r.v);
}

void abi_MariaItem4CopyTiles(void)
{
    regs_exit(MariaItem4CopyTiles());
}

/* ------------------------------------------------------------------------------------------------ its steps */

/*
 * @routine 01E0:0DA MariaItem4Cancel
 * @run 6D:B8DA
 * WHAT IT DOES: Whether item 4 is called off: button II pressed with up held.
 * HOW: Bit 1 of zPadPressed, bit 4 of zPadHeld.
 * WAITS: no.
 * Entry: nothing. Exit: C the answer; A the last AND, N and Z from it. X, Y and V kept.
 */
Regs MariaItem4Cancel(void)
{
    uint8_t value = (uint8_t)(RD8(ZP->pad_pressed) & PAD_II);
    Regs r = r_lda(regs_kept(), value);

    if (value == 0) {
        return r_c(r, false);
    }
    value = (uint8_t)(RD8(ZP->pad_held) & PAD_UP);
    r = r_lda(r, value);
    return r_c(r, value != 0);
}

void abi_MariaItem4Cancel(void)
{
    regs_exit(MariaItem4Cancel());
}

/*
 * @routine 01E0:0C6 MariaItem4End
 * @run 6D:B8C6
 * WHAT IT DOES: Ends item 4: slot 5 as kind 9 (MariaClearSlot5AsKind9), the hit timer $30; in the air (step 2) she
 * falls, else she jumps.
 * HOW: A BSR to MariaClearSlot5AsKind9; the JMPs to MariaToFall and MariaJump.
 * WAITS: no.
 * Entry: Y the caller's (the sound is given it). Exit: as MariaToFall or MariaJump.
 */
MExit MariaItem4End(uint8_t y)
{
    MExit e = mx_none();
    uint8_t step;
    AnimThen fall;

    mx_regs(&e, r_x_only(r_a_only(regs_kept(), 0x19), SLOT_ITEM));
    jsr_push(0xB8C8);
    mx_sound(&e, 0x19, MariaClearSlot5AsKind9(y));
    jsr_pop();
    WR8(OBJ->anim_timer[OBJECT_PLAYER], ITEM4_END_TIMER);
    step = RD8(OBJ->substate[OBJECT_PLAYER]);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), step), step, STEP_AIR));
    if (step == STEP_AIR) {
        fall = MariaToFall();
        mx_animation(&e, fall.animation);
        mx_regs(&e, fall.regs);
        return e;
    }
    mx_then(&e, MariaJump());
    return e;
}

void abi_MariaItem4End(void)
{
    mx_exit(MariaItem4End(cpu.y));
}

/*
 * @routine 01DF:75F MariaItem4ToAir
 * @run 6D:B75F
 * WHAT IT DOES: Item 4 in the air: the pad's third and fourth overrides cleared, step 2.
 * HOW: The stores.
 * WAITS: no.
 * Entry: nothing. Exit: A 2, N and Z from it. X, Y, C and V kept.
 */
Regs MariaItem4ToAir(void)
{
    WR8(PVARS->pad_overrides[2], 0);
    WR8(PVARS->pad_overrides[3], 0);
    WR8(OBJ->substate[OBJECT_PLAYER], STEP_AIR);
    return r_lda(regs_kept(), STEP_AIR);
}

void abi_MariaItem4ToAir(void)
{
    regs_exit(MariaItem4ToAir());
}

/*
 * @routine 01DF:752 MariaItem4Jump
 * @run 6D:B752
 * WHAT IT DOES: Item 4's jump: lifted three rows, Y speed $FE80, then in the air.
 * HOW: MariaLiftThree, the stores, on into MariaItem4ToAir.
 * WAITS: no.
 * Entry: nothing. Exit: as MariaItem4ToAir; C and V as MariaLiftThree left them.
 */
Regs MariaItem4Jump(void)
{
    Regs r;

    jsr_push(0xB755);
    r = MariaLiftThree();
    jsr_pop();
    WR8(OBJ->vy_lo[OBJECT_PLAYER], ITEM4_VY_LO);
    WR8(OBJ->vy_hi[OBJECT_PLAYER], ITEM4_VY_HI);
    return r_then(r, MariaItem4ToAir());
}

void abi_MariaItem4Jump(void)
{
    regs_exit(MariaItem4Jump());
}

/* LDA OBJ_FACING_LEFT, EOR #1, TAX, STZ PadOverrides+2,X, RTS ($B7FF). */
static Regs item4_other_side(void)
{
    uint8_t other = (uint8_t)(RD8(OBJ->facing_left[OBJECT_PLAYER]) ^ 1);

    WR8(PVARS->pad_overrides[2 + other], 0);
    return r_ldx(r_a_only(regs_kept(), other), other);
}

/*
 * @routine 01DF:7FA MariaItem4Still
 * @run 6D:B7FA
 * WHAT IT DOES: Item 4 standing still: frame $42; then the pad override of the side she faces away from cleared.
 * HOW: The store; OBJ_FACING_LEFT XOR 1 indexes PadOverrides' third byte on.
 * WAITS: no.
 * Entry: nothing. Exit: A the other side, X it, N and Z from it. Y, C and V kept.
 */
Regs MariaItem4Still(void)
{
    WR8(OBJ->frame[OBJECT_PLAYER], FRAME_ITEM4_STILL);
    return item4_other_side();
}

void abi_MariaItem4Still(void)
{
    regs_exit(MariaItem4Still());
}


/*
 * @routine 01E0:00D MariaItem4Blocked
 * @run 6D:B80D
 * WHAT IT DOES: Whether item 4's walk is blocked at the low columns: with a mark under her (bits 2 or 5 of the three
 * columns), any of bits 2 and 5 of MariaLowColumnsBlocked's answer, else its bit 5.
 * HOW: MariaLowColumnsBlocked, the AND; then CPX #2 and CPX #0, whose branches both go to the RTS.
 * WAITS: no.
 * Entry: nothing. Exit: Z clear when blocked, with N, Z and C from the compares of X; A the AND. X, Y and V as
 * MariaLowColumnsBlocked left them.
 */
MExit MariaItem4Blocked(void)
{
    MExit e = mx_none();
    uint8_t marks = RD8(PVARS->column_hit[0]), answer, x;
    LowColumns low;

    marks |= RD8(PVARS->column_hit[1]);
    marks |= RD8(PVARS->column_hit[2]);
    marks &= MARK_FLOOR;
    mx_regs(&e, r_lda(regs_kept(), marks));
    jsr_push(marks != 0 ? 0xB81D : 0xB824);
    low = MariaLowColumnsBlocked();
    jsr_pop();
    mx_low(&e, low);
    answer = (uint8_t)(mx_a_from(0, 0, e) & (marks != 0 ? MARK_FLOOR : MARK_DOWN_ONLY));
    mx_regs(&e, r_lda(regs_kept(), answer));
    if (answer == 0) {
        return e;
    }
    x = mx_x(e);
    mx_regs(&e, r_cmp(regs_kept(), x, 2));
    if (x == 2) {
        return e;
    }
    mx_regs(&e, r_cmp(regs_kept(), x, 0));
    return e;
}

void abi_MariaItem4Blocked(void)
{
    mx_exit(MariaItem4Blocked());
}

/*
 * @routine 01DF:6EC MariaItem4Landed
 * @run 6D:B6EC
 * WHAT IT DOES: Item 4 on the ground: animation $28, standing still, step 1.
 * HOW: MariaSetAnimation, MariaItem4Still, the store.
 * WAITS: no.
 * Entry: nothing. Exit: A 1, N and Z from it; X the other side; Y and C as MariaSetAnimation left them.
 */
MExit MariaItem4Landed(void)
{
    MExit e = mx_none();

    mx_regs(&e, r_lda(regs_kept(), ANIMATION_ITEM4_LAND));
    jsr_push(0xB6F1);
    mx_animation(&e, MariaSetAnimation(ANIMATION_ITEM4_LAND));
    jsr_pop();
    jsr_push(0xB6F4);
    mx_regs(&e, MariaItem4Still());
    jsr_pop();
    WR8(OBJ->substate[OBJECT_PLAYER], STEP_WALK);
    mx_regs(&e, r_lda(regs_kept(), STEP_WALK));
    return e;
}

void abi_MariaItem4Landed(void)
{
    mx_exit(MariaItem4Landed());
}

/*
 * @routine 01DF:6BD MariaItem4Start
 * @run 6D:B6BD
 * WHAT IT DOES: Item 4's step 0: the columns, a landing; while her animation runs, nothing more; then its tiles copied,
 * four words a frame; when they are done, on a floor her column stands on or with a mark under her, she lands
 * (MariaItem4Landed); else standing still, in the air.
 * HOW: MariaReadColumns, MariaLandOnFloor, MariaAnimationWait (which ends the step while it runs); MariaItem4CopyTiles's
 * Z; MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn, the columns' bits 2 and 5; MariaItem4Still and the JMP to
 * MariaItem4ToAir.
 * WAITS: no.
 * Entry: nothing. Exit: as the routine it ended in.
 */
MExit MariaItem4Start(void)
{
    MExit e = mx_none();
    AnimationWaitEnd wait;
    FloorResult floor;
    Regs copied;
    uint8_t marks;

    jsr_push(0xB6C0);
    mx_columns(&e, MariaReadColumns());
    jsr_pop();
    jsr_push(0xB6C3);
    mx_landing(&e, MariaLandOnFloor());
    jsr_pop();
    jsr_push(0xB6C6);
    wait = MariaAnimationWait();
    mx_animation(&e, wait.step);
    if (wait.dropped) {
        return e;
    }
    jsr_pop();
    jsr_push(0xB6C9);
    copied = MariaItem4CopyTiles();
    jsr_pop();
    mx_regs(&e, copied);
    if (copied.nz != 0) {
        return e;
    }
    jsr_push(0xB6CF);
    mx_columns(&e, MariaReadColumns());
    jsr_pop();
    jsr_push(0xB6D2);
    floor = MariaFloorCheck();
    jsr_pop();
    mx_floor(&e, floor);
    if (floor.how != FLOOR_NONE) {
        jsr_push(0xB6D7);
        mx_solid(&e, MariaSolidUnderColumn(mx_x(e)));
        jsr_pop();
        if (mx_zero(e)) {
            mx_then(&e, MariaItem4Landed());
            return e;
        }
    }
    marks = RD8(PVARS->column_hit[0]);
    marks |= RD8(PVARS->column_hit[1]);
    marks |= RD8(PVARS->column_hit[2]);
    marks &= MARK_FLOOR;
    mx_regs(&e, r_lda(regs_kept(), marks));
    if (marks != 0) {
        mx_then(&e, MariaItem4Landed());
        return e;
    }
    jsr_push(0xB6E9);
    mx_regs(&e, MariaItem4Still());
    jsr_pop();
    mx_regs(&e, MariaItem4ToAir());
    return e;
}

void abi_MariaItem4Start(void)
{
    mx_exit(MariaItem4Start());
}

/* A step of item 4's walk toward a side: her X a pixel that way, the blocked check, and her X put back ($B7BC on). */
static bool walk_blocked(MExit *e, uint8_t facing, uint16_t ret)
{
    uint8_t p, pulled;
    MExit blocked;

    if (facing == 0) {
        WR8(OBJ->facing_left[OBJECT_PLAYER], 0);
        INC8(OBJ->x_lo[OBJECT_PLAYER]);
    } else {
        WR8(OBJ->facing_left[OBJECT_PLAYER], 1);
        mx_regs(e, r_lda(regs_kept(), 1));
        DEC8(OBJ->x_lo[OBJECT_PLAYER]);
    }
    jsr_push(ret);
    blocked = MariaItem4Blocked();
    jsr_pop();
    mx_then(e, blocked);
    p = mx_status(*e);
    push8((uint8_t)((p & ~P_T) | P_B));
    if (facing == 0) {
        DEC8(OBJ->x_lo[OBJECT_PLAYER]);
    } else {
        INC8(OBJ->x_lo[OBJECT_PLAYER]);
    }
    pulled = pull8();
    mx_regs(e, r_plp(regs_kept(), pulled));
    return (pulled & P_Z) == 0;
}

/*
 * @routine 01DF:76B MariaItem4Walk
 * @run 6D:B76B
 * WHAT IT DOES: Item 4's walk (step 1): no X speed; called off, it ends; the columns and her footing (a row lower too,
 * else in the air); on a mark the pad's third and fourth overrides follow whether she is low on the screen ($B0); the
 * jump; a landing; then right or left held and not blocked a pixel that way, she walks ($0050 or $FFB0) with her
 * animation; else she stands still.
 * HOW: MariaStopX, MariaItem4Cancel and the JMP to MariaItem4End; MariaReadColumns, MariaHasNoFooting, the columns' bits;
 * MariaJumpPressed and the JMP to MariaItem4Jump; MariaLandOnFloor; BBR5 and BBR7 of zPadHeld; MariaItem4Blocked under
 * PHP and PLP; MariaAnimationTick, MariaItem4WalkSpeeds.
 * WAITS: no.
 * Entry: Y the caller's. Exit: as the routine it ended in, or the store of the other side's override.
 */
MExit MariaItem4Walk(uint8_t y)
{
    MExit e = mx_none();
    Regs cancel;
    FootingCheck footing;
    uint8_t marks, value, facing, index;
    bool jump;

    jsr_push(0xB76E);
    MariaStopX();
    jsr_pop();
    jsr_push(0xB771);
    cancel = MariaItem4Cancel();
    jsr_pop();
    mx_regs(&e, cancel);
    if (cancel.c) {
        mx_then(&e, MariaItem4End(mx_y_from(0, y, e)));
        return e;
    }
    jsr_push(0xB779);
    mx_columns(&e, MariaReadColumns());
    jsr_pop();
    jsr_push(0xB77C);
    footing = MariaHasNoFooting();
    jsr_pop();
    mx_footing(&e, footing);
    if (footing.no_floor) {
        marks = RD8(PVARS->column_hit[0]);
        marks |= RD8(PVARS->column_hit[1]);
        marks |= RD8(PVARS->column_hit[2]);
        marks &= MARK_FLOOR;
        mx_regs(&e, r_lda(regs_kept(), marks));
        if (marks == 0) {
            INC8(OBJ->y_lo[OBJECT_PLAYER]);
            jsr_push(0xB791);
            mx_columns(&e, MariaReadColumns());
            jsr_pop();
            jsr_push(0xB794);
            footing = MariaHasNoFooting();
            jsr_pop();
            mx_footing(&e, footing);
            if (footing.no_floor) {
                mx_regs(&e, MariaItem4ToAir());
                return e;
            }
        } else {
            WR8(PVARS->pad_overrides[2], 0);
            WR8(PVARS->pad_overrides[3], 0);
            value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
            mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, ITEM4_LOW_Y));
            if (value >= ITEM4_LOW_Y) {
                WR8(PVARS->pad_overrides[2], 1);
                WR8(PVARS->pad_overrides[3], 1);
                mx_regs(&e, r_lda(regs_kept(), 1));
            }
        }
    }
    jsr_push(0xB7B1);
    jump = MariaJumpPressed();
    jsr_pop();
    mx_regs(&e, r_c(regs_kept(), jump));
    if (jump) {
        mx_regs(&e, MariaItem4Jump());
        return e;
    }
    jsr_push(0xB7B9);
    mx_landing(&e, MariaLandOnFloor());
    jsr_pop();
    if (RD8(ZP->pad_held) & PAD_RIGHT) {
        if (walk_blocked(&e, 0, 0xB7C4)) {
            jsr_push(0xB7FA);
            MariaStopX();
            jsr_pop();
            mx_regs(&e, MariaItem4Still());
            return e;
        }
    } else if (RD8(ZP->pad_held) & PAD_LEFT) {
        if (walk_blocked(&e, 1, 0xB7DA)) {
            jsr_push(0xB7FA);
            MariaStopX();
            jsr_pop();
            mx_regs(&e, MariaItem4Still());
            return e;
        }
    } else {
        mx_regs(&e, MariaItem4Still());
        return e;
    }
    jsr_push(0xB7E4);
    mx_animation(&e, MariaAnimationTick());
    jsr_pop();
    facing = RD8(OBJ->facing_left[OBJECT_PLAYER]);
    index = (uint8_t)(facing << 1);
    value = TBL8(MariaItem4WalkSpeeds, index);
    WR8(OBJ->vx_lo[OBJECT_PLAYER], value);
    value = TBL8(MariaItem4WalkSpeeds, index + 1);
    WR8(OBJ->vx_hi[OBJECT_PLAYER], value);
    mx_regs(&e, r_c(r_y_only(regs_kept(), index), (facing & 0x80) != 0));
    mx_regs(&e, item4_other_side());
    return e;
}

void abi_MariaItem4Walk(void)
{
    mx_exit(MariaItem4Walk(cpu.y));
}

/*
 * @routine 01E0:031 MariaItem4Air
 * @run 6D:B831
 * WHAT IT DOES: Item 4 in the air (step 2): called off, it ends; falling, on a floor her column stands on or a column
 * with bit 2 or 5 she lands: her Y up by the column's row, slots 1 and 2 as kind 5, sound $30 hard (Y speed $06xx or
 * more) or $4A, no speed, and on the ground; else the bit-3 burst; gravity $0048 held at $07xx; at $06xx or more down,
 * slot 5's hitbox is live.
 * HOW: MariaItem4Cancel and the JMP to MariaItem4End; MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn, TST of
 * RichterColumnHit by X; MariaReplaceSlotsOneTwoKind5, SoundCommand, MariaStopMoving and the JMP to MariaItem4Landed;
 * MariaCheckBit3Under; the 16-bit add.
 * WAITS: no.
 * Entry: Y the caller's. Exit: as the routine it ended in, or the last compare.
 */
MExit MariaItem4Air(uint8_t y)
{
    MExit e = mx_none();
    Regs cancel;
    FloorResult floor;
    uint8_t value, column, sound;
    bool land = false;
    Sum lo, hi, row;

    jsr_push(0xB834);
    cancel = MariaItem4Cancel();
    jsr_pop();
    mx_regs(&e, cancel);
    if (cancel.c) {
        mx_then(&e, MariaItem4End(mx_y_from(0, y, e)));
        return e;
    }
    value = RD8(OBJ->vy_hi[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (!(value & 0x80)) {
        jsr_push(0xB841);
        mx_columns(&e, MariaReadColumns());
        jsr_pop();
        jsr_push(0xB844);
        floor = MariaFloorCheck();
        jsr_pop();
        mx_floor(&e, floor);
        if (floor.how != FLOOR_NONE) {
            jsr_push(0xB849);
            mx_solid(&e, MariaSolidUnderColumn(mx_x(e)));
            jsr_pop();
            land = mx_zero(e);
        }
        if (!land) {
            for (column = 0; column < 3; column++) {
                value = RD8(PVARS->column_hit[column]);
                mx_regs(&e, r_tst(r_x_only(regs_kept(), column), MARK_FLOOR, value));
                if (column != 0) {
                    mx_regs(&e, r_nz(regs_kept(), column));
                    mx_regs(&e, r_tst(regs_kept(), MARK_FLOOR, value));
                }
                if (value & MARK_FLOOR) {
                    land = true;
                    break;
                }
            }
        }
        if (land) {
            column = mx_x(e);
            value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
            row = sub_bytes(value, RD8(PVARS->column_row[column]), true);
            WR8(OBJ->y_lo[OBJECT_PLAYER], row.value);
            mx_regs(&e, r_v(r_c(r_lda(regs_kept(), row.value), row.carry), row.overflow));
            jsr_push(0xB899);
            mx_regs(&e, MariaReplaceSlotsOneTwoKind5());
            jsr_pop();
            value = RD8(OBJ->vy_hi[OBJECT_PLAYER]);
            mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, ITEM4_HARD));
            sound = value >= ITEM4_HARD ? SOUND_ITEM4_HARD : SOUND_ITEM4_LAND;
            mx_regs(&e, r_lda(regs_kept(), sound));
            jsr_push(value >= ITEM4_HARD ? 0xB8A5 : 0xB8AC);
            mx_sound(&e, sound, SoundCommand(sound, mx_x(e), mx_y_from(0, y, e)));
            jsr_pop();
            jsr_push(0xB8AF);
            MariaStopMoving();
            jsr_pop();
            mx_then(&e, MariaItem4Landed());
            return e;
        }
        jsr_push(0xB863);
        mx_then(&e, MariaCheckBit3Under(mx_x(e), mx_y_from(0, y, e)));
        jsr_pop();
    }
    value = RD8(OBJ->vy_lo[OBJECT_PLAYER]);
    lo = add_bytes(value, ITEM4_GRAVITY, false);
    WR8(OBJ->vy_lo[OBJECT_PLAYER], lo.value);
    value = RD8(OBJ->vy_hi[OBJECT_PLAYER]);
    hi = add_bytes(value, 0, lo.carry);
    WR8(OBJ->vy_hi[OBJECT_PLAYER], hi.value);
    mx_regs(&e, r_v(r_c(r_lda(regs_kept(), hi.value), hi.carry), hi.overflow));
    if (!(hi.value & 0x80)) {
        mx_regs(&e, r_cmp(regs_kept(), hi.value, FALL_SPEED_TOP));
        if (hi.value >= FALL_SPEED_TOP) {
            WR8(OBJ->vy_lo[OBJECT_PLAYER], 0);
        }
    }
    value = RD8(OBJ->vy_hi[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value & 0x80) {
        return e;
    }
    mx_regs(&e, r_cmp(regs_kept(), value, ITEM4_HARD));
    if (value < ITEM4_HARD) {
        return e;
    }
    WR8(OBJ->hitbox_on[SLOT_ITEM], 1);
    mx_regs(&e, r_lda(regs_kept(), 1));
    return e;
}

void abi_MariaItem4Air(void)
{
    mx_exit(MariaItem4Air(cpu.y));
}

/*
 * @routine 01DF:6AE MariaStateItem4
 * @run 6D:B6AE
 * WHAT IT DOES: State $10, item 4: slot 5's hitbox off, then the step's handler from MariaItem4Steps (start, walk,
 * air).
 * HOW: STZ, MariaStepIndex, the JMP (table,X).
 * WAITS: no.
 * Entry: Y the caller's. Exit: as the step's handler, after A and X the step doubled and C its bit 7.
 */
MExit MariaStateItem4(uint8_t y)
{
    MExit e = mx_none();
    uint8_t step, doubled, lo, hi;
    uint16_t target;

    WR8(OBJ->hitbox_on[SLOT_ITEM], 0);
    jsr_push(0xB6B4);
    step = MariaStepIndex();
    jsr_pop();
    doubled = (uint8_t)(step << 1);
    mx_regs(&e, r_c(r_ldx(r_lda(regs_kept(), doubled), doubled), (step & 0x80) != 0));
    lo = TBL8(MariaItem4Steps, doubled);
    hi = TBL8(MariaItem4Steps, doubled + 1);
    target = (uint16_t)(lo | (hi << 8));
    switch (target) {
    case ITEM4_STEP_START:
        mx_then(&e, MariaItem4Start());
        break;
    case ITEM4_STEP_WALK:
        mx_then(&e, MariaItem4Walk(y));
        break;
    case ITEM4_STEP_AIR:
        mx_then(&e, MariaItem4Air(y));
        break;
    default:
        mx_exit(e);
        run_routine(target);
        e.elsewhere = true;
        break;
    }
    return e;
}

void abi_MariaStateItem4(void)
{
    mx_exit(MariaStateItem4(cpu.y));
}

/* ------------------------------------------------------------------------------------------------ the sequence's move */

/*
 * @routine 01E0:0EA MariaStateSequence
 * @run 6D:B8EA
 * WHAT IT DOES: State $11, her pad sequence's move: her animation; at frame $46 with under $0C of its time left, she
 * stands on a floor her column stands on, else falls (and the step goes on), the attack's outcome cleared either way;
 * before that, the columns and a landing when she has footing.
 * HOW: MariaAnimationTick; MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn and the JMP to MariaStand, or
 * MariaToFall; MariaReadColumns, MariaHasNoFooting, MariaLandOnFloor.
 * WAITS: no.
 * Entry: nothing. Exit: as the routine it ended in, or the step's INC, or the last check.
 */
MExit MariaStateSequence(void)
{
    MExit e = mx_none();
    FloorResult floor;
    FootingCheck footing;
    uint8_t frame, timer;
    AnimThen moved;

    jsr_push(0xB8ED);
    mx_animation(&e, MariaAnimationTick());
    jsr_pop();
    frame = RD8(OBJ->frame[OBJECT_PLAYER]);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), frame), frame, FRAME_SEQUENCE_END));
    if (frame == FRAME_SEQUENCE_END) {
        timer = RD8(OBJ->hurt_timer[OBJECT_PLAYER]);
        mx_regs(&e, r_cmp(r_lda(regs_kept(), timer), timer, SEQUENCE_LAND_TIMER));
        if (timer < SEQUENCE_LAND_TIMER) {
            jsr_push(0xB8FE);
            mx_columns(&e, MariaReadColumns());
            jsr_pop();
            jsr_push(0xB901);
            floor = MariaFloorCheck();
            jsr_pop();
            mx_floor(&e, floor);
            if (floor.how != FLOOR_NONE) {
                jsr_push(0xB906);
                mx_solid(&e, MariaSolidUnderColumn(mx_x(e)));
                jsr_pop();
                if (mx_zero(e)) {
                    WR8(PVARS->attack_kind, 0);
                    moved = MariaStand();
                    mx_animation(&e, moved.animation);
                    mx_regs(&e, moved.regs);
                    return e;
                }
            }
            WR8(PVARS->attack_kind, 0);
            jsr_push(0xB914);
            moved = MariaToFall();
            jsr_pop();
            mx_animation(&e, moved.animation);
            mx_regs(&e, moved.regs);
            mx_regs(&e, r_nz(regs_kept(), INC8(OBJ->substate[OBJECT_PLAYER])));
            return e;
        }
    }
    jsr_push(0xB91B);
    mx_columns(&e, MariaReadColumns());
    jsr_pop();
    jsr_push(0xB91E);
    footing = MariaHasNoFooting();
    jsr_pop();
    mx_footing(&e, footing);
    if (!footing.no_floor) {
        jsr_push(0xB923);
        mx_landing(&e, MariaLandOnFloor());
        jsr_pop();
    }
    return e;
}

void abi_MariaStateSequence(void)
{
    mx_exit(MariaStateSequence());
}
