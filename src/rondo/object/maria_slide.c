/*
 * Maria's slide (01DF, bank $6D): state $0D, sliding with the direction held; $0F, the slide let go, and from it the
 * crawl on a mark under her feet (PlayerFeetOnMark); $0E, a slide gone off a floor. The slide slows each frame
 * (MariaSlideFriction) and ends in the crouch (state 2, frame $1B) when her animation has run.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"

enum {
    OBJECT_PLAYER = 0,
    STATE_CROUCH = 2, STATE_SLIDE_AIR = 0x0E, STATE_SLIDE = 0x0F, STEP_CRAWL = 1,
    FRAME_CROUCHED = 0x1B, ANIMATION_CRAWL = 0x18, SLIDE_STEPS_HELD = 5, SLIDE_GRAVITY = 0x38, FALL_SPEED_TOP = 7,
    SLIDE_LAND_SLOW = 4, PAD_I = 0x01, PAD_DOWN = 0x40, PAD_RIGHT = 0x20, PAD_SIDES = 0xA0
};

#define MariaSlideFriction_TBL MariaSlideFriction
#define MariaSlideAirSpeeds_TBL MariaSlideAirSpeeds
#define MariaCrawlSpeeds_TBL MariaCrawlSpeeds

/*
 * @routine 01DF:150 MariaSlideEnd
 * @run 6D:B150
 * WHAT IT DOES: Ends her slide in the crouch: no speed, frame $1B, state 2, step 0.
 * HOW: MariaStopMoving, the stores.
 * WAITS: no.
 * Entry: nothing. Exit: A 0, Z set, N clear. X, Y, C and V kept.
 */
Regs MariaSlideEnd(void)
{
    jsr_push(0xB153);
    MariaStopMoving();
    jsr_pop();
    WR8(OBJ->frame[OBJECT_PLAYER], FRAME_CROUCHED);
    WR8(OBJ->state[OBJECT_PLAYER], STATE_CROUCH);
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    return r_lda(regs_kept(), 0);
}

void abi_MariaSlideEnd(void)
{
    regs_exit(MariaSlideEnd());
}

/*
 * @routine 01DF:163 MariaToSlideAir
 * @run 6D:B163
 * WHAT IT DOES: Her slide goes off the floor: the X speed the way she faces (MariaSlideAirSpeeds), state $0E, step 0.
 * HOW: The table by the facing doubled; the stores.
 * WAITS: no.
 * Entry: nothing. Exit: A 0, Z set, N clear; Y the facing doubled; C bit 7 of the facing. X and V kept.
 */
Regs MariaToSlideAir(void)
{
    uint8_t facing = RD8(OBJ->facing_left[OBJECT_PLAYER]);
    uint8_t index = (uint8_t)(facing << 1);
    uint8_t value = TBL8(MariaSlideAirSpeeds, index);

    WR8(OBJ->vx_lo[OBJECT_PLAYER], value);
    value = TBL8(MariaSlideAirSpeeds, index + 1);
    WR8(OBJ->vx_hi[OBJECT_PLAYER], value);
    WR8(OBJ->state[OBJECT_PLAYER], STATE_SLIDE_AIR);
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    return r_c(r_lda(r_y_only(regs_kept(), index), 0), (facing & 0x80) != 0);
}

void abi_MariaToSlideAir(void)
{
    regs_exit(MariaToSlideAir());
}

/*
 * @routine 01DF:17F MariaSlideSlow
 * @run 6D:B17F
 * WHAT IT DOES: Slows her slide: while she has X speed, the friction the way she faces is added (MariaSlideFriction);
 * when the speed's sign turns over, she stops along X.
 * HOW: A 16-bit add; the high byte's sign before (in zTemp00) and after; MariaStopX.
 * WAITS: no.
 * Entry: nothing. Exit: no speed, A 0, Z set; else A the sign bits that changed, N and Z from them, Y the facing
 * doubled, C and V from the add. X kept.
 */
Regs MariaSlideSlow(void)
{
    uint8_t value = RD8(OBJ->vx_hi[OBJECT_PLAYER]);
    uint8_t facing, index, before;
    Sum lo, hi;

    value |= RD8(OBJ->vx_lo[OBJECT_PLAYER]);
    if (value == 0) {
        return r_lda(regs_kept(), 0);
    }
    facing = RD8(OBJ->facing_left[OBJECT_PLAYER]);
    index = (uint8_t)(facing << 1);
    value = RD8(OBJ->vx_lo[OBJECT_PLAYER]);
    lo = add_bytes(value, TBL8(MariaSlideFriction, index), false);
    WR8(OBJ->vx_lo[OBJECT_PLAYER], lo.value);
    before = RD8(OBJ->vx_hi[OBJECT_PLAYER]);
    WR8(ZP->temp00, before);
    hi = add_bytes(before, TBL8(MariaSlideFriction, index + 1), lo.carry);
    WR8(OBJ->vx_hi[OBJECT_PLAYER], hi.value);
    value = (uint8_t)((hi.value ^ RD8(ZP->temp00)) & 0x80);
    if (value != 0) {
        jsr_push(0xB1AA);
        MariaStopX();
        jsr_pop();
    }
    return r_v(r_c(r_lda(r_y_only(regs_kept(), index), value), hi.carry), hi.overflow);
}

void abi_MariaSlideSlow(void)
{
    regs_exit(MariaSlideSlow());
}

/*
 * @routine 01DF:103 MariaStateSlideHeld
 * @run 6D:B103
 * WHAT IT DOES: State $0D, sliding with the direction held: with footing at her Y she slides on; with none there,
 * footing one row lower takes her off the floor (MariaToSlideAir), and so does none one row higher either; footing
 * one row higher keeps her sliding there. Then a landing, a ledge to take hold of; from the slide's fifth
 * animation step on, while MariaUpHeld holds it, the slide slows and may slide again; else when her animation has
 * run, she crouches.
 * HOW: MariaReadColumns and MariaHasNoFooting at three heights (INC and DEC of her Y); MariaLandOnFloor,
 * MariaLedgeCheck and the JMP to MariaClimbStart; OBJ_ANIMATION_AT compared with 5; MariaUpHeld, MariaSlideSlow,
 * MariaSlideCheck and the JMP to MariaSlideStart; MariaAnimationWait, which ends the step while it runs; on into
 * MariaSlideEnd.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the routine it ended in.
 */
MExit MariaStateSlideHeld(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    FootingCheck footing;
    MariaLedge ledge;
    AnimationWaitEnd wait;
    uint8_t step;
    bool up;
    MExit slide;

    jsr_push(0xB106);
    mx_columns(&e, MariaReadColumns());
    jsr_pop();
    jsr_push(0xB109);
    footing = MariaHasNoFooting();
    jsr_pop();
    mx_footing(&e, footing);
    if (footing.no_floor) {
        INC8(OBJ->y_lo[OBJECT_PLAYER]);
        jsr_push(0xB111);
        mx_columns(&e, MariaReadColumns());
        jsr_pop();
        jsr_push(0xB114);
        footing = MariaHasNoFooting();
        jsr_pop();
        mx_footing(&e, footing);
        if (!footing.no_floor) {
            mx_regs(&e, MariaToSlideAir());
            return e;
        }
        DEC8(OBJ->y_lo[OBJECT_PLAYER]);
        DEC8(OBJ->y_lo[OBJECT_PLAYER]);
        jsr_push(0xB11F);
        mx_columns(&e, MariaReadColumns());
        jsr_pop();
        jsr_push(0xB122);
        footing = MariaHasNoFooting();
        jsr_pop();
        mx_footing(&e, footing);
        if (footing.no_floor) {
            mx_regs(&e, MariaToSlideAir());
            return e;
        }
    }
    jsr_push(0xB127);
    mx_landing(&e, MariaLandOnFloor());
    jsr_pop();
    jsr_push(0xB12A);
    ledge = MariaLedgeCheck();
    jsr_pop();
    mx_ledge(&e, ledge);
    if (mx_carry(e)) {
        mx_then(&e, MariaClimbStart(mx_x_from(x, y, e)));
        return e;
    }
    step = RD8(OBJ->animation_at[OBJECT_PLAYER]);
    mx_regs(&e, r_cmp(r_lda(regs_kept(), step), step, SLIDE_STEPS_HELD));
    if (step >= SLIDE_STEPS_HELD) {
        jsr_push(0xB139);
        up = MariaUpHeld();
        jsr_pop();
        mx_regs(&e, r_c(regs_kept(), up));
        if (!up) {
            mx_regs(&e, MariaSlideEnd());
            return e;
        }
        step = RD8(OBJ->animation_at[OBJECT_PLAYER]);
        mx_regs(&e, r_cmp(r_lda(regs_kept(), step), step, SLIDE_STEPS_HELD));
        if (step >= SLIDE_STEPS_HELD) {
            jsr_push(0xB145);
            mx_regs(&e, MariaSlideSlow());
            jsr_pop();
            jsr_push(0xB148);
            slide = MariaSlideCheck(mx_x_from(x, y, e), mx_y_from(x, y, e));
            jsr_pop();
            mx_then(&e, slide);
            if (mx_carry(e)) {
                mx_then(&e, MariaSlideStart(mx_x_from(x, y, e), mx_y_from(x, y, e)));
                return e;
            }
        }
    }
    jsr_push(0xB150);
    wait = MariaAnimationWait();
    mx_animation(&e, wait.step);
    if (wait.dropped) {
        return e;
    }
    jsr_pop();
    mx_regs(&e, MariaSlideEnd());
    return e;
}

void abi_MariaStateSlideHeld(void)
{
    mx_exit(MariaStateSlideHeld(cpu.x, cpu.y));
}

/*
 * @routine 01DF:1B5 MariaStateSlide
 * @run 6D:B1B5
 * WHAT IT DOES: State $0F. Step 0, the slide let go: the mark under her feet noted, then as the held slide; if that
 * crouched her with a mark under her feet, she crawls (animation $18, state $0F, step 1). Step 1, the crawl: footing
 * at her Y or a row lower, else off the floor; the mark noted, her animation; off the mark the crawl ends in the
 * crouch, on it she stops, and with right or left held crawls that way ($0080 or $FF80).
 * HOW: MariaFeetMarked, MariaStateSlideHeld, PlayerFeetOnMark; MariaReadColumns and MariaHasNoFooting, MariaAnimationTick,
 * MariaStopX, TST of zPadHeld, MariaCrawlSpeeds; the JMPs to MariaToSlideAir and MariaSlideEnd.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the routine it ended in, or the last test or store.
 */
MExit MariaStateSlide(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    FootingCheck footing;
    uint8_t value, index;

    value = RD8(OBJ->substate[OBJECT_PLAYER]);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value == 0) {
        jsr_push(0xB1BD);
        mx_marked(&e, MariaFeetMarked());
        jsr_pop();
        jsr_push(0xB1C0);
        mx_then(&e, MariaStateSlideHeld(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        jsr_pop();
        value = RD8(OBJ->state[OBJECT_PLAYER]);
        mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, STATE_CROUCH));
        if (value != STATE_CROUCH) {
            return e;
        }
        value = RD8(PVARS->feet_on_mark);
        mx_regs(&e, r_lda(regs_kept(), value));
        if (value == 0) {
            return e;
        }
        mx_regs(&e, r_lda(regs_kept(), ANIMATION_CRAWL));
        jsr_push(0xB1D1);
        mx_animation(&e, MariaSetAnimation(ANIMATION_CRAWL));
        jsr_pop();
        WR8(OBJ->state[OBJECT_PLAYER], STATE_SLIDE);
        WR8(OBJ->substate[OBJECT_PLAYER], STEP_CRAWL);
        mx_regs(&e, r_lda(regs_kept(), STEP_CRAWL));
        return e;
    }
    jsr_push(0xB1E0);
    mx_columns(&e, MariaReadColumns());
    jsr_pop();
    jsr_push(0xB1E3);
    footing = MariaHasNoFooting();
    jsr_pop();
    mx_footing(&e, footing);
    if (footing.no_floor) {
        INC8(OBJ->y_lo[OBJECT_PLAYER]);
        jsr_push(0xB1EB);
        mx_columns(&e, MariaReadColumns());
        jsr_pop();
        jsr_push(0xB1EE);
        footing = MariaHasNoFooting();
        jsr_pop();
        mx_footing(&e, footing);
        if (footing.no_floor) {
            mx_regs(&e, MariaToSlideAir());
            return e;
        }
    }
    jsr_push(0xB1F6);
    mx_marked(&e, MariaFeetMarked());
    jsr_pop();
    jsr_push(0xB1F9);
    mx_animation(&e, MariaAnimationTick());
    jsr_pop();
    value = RD8(PVARS->feet_on_mark);
    mx_regs(&e, r_lda(regs_kept(), value));
    if (value == 0) {
        mx_regs(&e, MariaSlideEnd());
        return e;
    }
    jsr_push(0xB201);
    MariaStopX();
    jsr_pop();
    value = RD8(ZP->pad_held);
    mx_regs(&e, r_tst(regs_kept(), PAD_SIDES, value));
    if (!(value & PAD_SIDES)) {
        return e;
    }
    index = 0;
    mx_regs(&e, r_y_only(regs_kept(), index));
    value = RD8(ZP->pad_held);
    mx_regs(&e, r_tst(regs_kept(), PAD_RIGHT, value));
    if (!(value & PAD_RIGHT)) {
        index = 2;
        mx_regs(&e, r_ldy(regs_kept(), index));
    }
    value = TBL8(MariaCrawlSpeeds, index);
    WR8(OBJ->vx_lo[OBJECT_PLAYER], value);
    value = TBL8(MariaCrawlSpeeds, index + 1);
    WR8(OBJ->vx_hi[OBJECT_PLAYER], value);
    mx_regs(&e, r_lda(regs_kept(), value));
    return e;
}

void abi_MariaStateSlide(void)
{
    mx_exit(MariaStateSlide(cpu.x, cpu.y));
}

/*
 * @routine 01DF:222 MariaStateSlideAir
 * @run 6D:B222
 * WHAT IT DOES: State $0E, a slide gone off the floor: a floor her column stands on lands her, and slow enough, with
 * button I and down held, she slides on from it (her Y up by the column's row); the marked-floor landings, the bit-3
 * burst; gravity $0038, held at $07xx; the jump pressed is her second jump; her animation.
 * HOW: MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn's Z; the JMPs to MariaSlideStart and MariaJumpLand;
 * MariaOnMarkedFloor, MariaFeetOnMarkedFloor, MariaCheckBit3Under; the 16-bit add; MariaJumpPressed and the JMP to
 * MariaSecondJump; the tail JMP to MariaAnimationTick.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as the routine it ended in.
 */
MExit MariaStateSlideAir(uint8_t x, uint8_t y)
{
    MExit e = mx_none();
    FloorResult floor;
    MarkedFloor marked;
    MExit feet;
    uint8_t value, column;
    Sum lo, hi;
    bool jump;

    jsr_push(0xB225);
    mx_columns(&e, MariaReadColumns());
    jsr_pop();
    jsr_push(0xB228);
    floor = MariaFloorCheck();
    jsr_pop();
    mx_floor(&e, floor);
    if (floor.how != FLOOR_NONE) {
        column = mx_x_from(x, y, e);
        jsr_push(0xB22D);
        mx_solid(&e, MariaSolidUnderColumn(column));
        jsr_pop();
        if (mx_zero(e)) {
            column = mx_x_from(x, y, e);
            value = RD8(OBJ->vy_hi[OBJECT_PLAYER]);
            mx_regs(&e, r_cmp(r_lda(regs_kept(), value), value, SLIDE_LAND_SLOW));
            if (value < SLIDE_LAND_SLOW) {
                value = RD8(ZP->pad_held);
                mx_regs(&e, r_tst(regs_kept(), PAD_I, value));
                if (value & PAD_I) {
                    value = RD8(ZP->pad_held);
                    mx_regs(&e, r_tst(regs_kept(), PAD_DOWN, value));
                    if (value & PAD_DOWN) {
                        Sum row;

                        value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
                        row = sub_bytes(value, RD8(PVARS->column_row[column]), true);
                        WR8(OBJ->y_lo[OBJECT_PLAYER], row.value);
                        mx_regs(&e, r_v(r_c(r_lda(regs_kept(), row.value), row.carry), row.overflow));
                        mx_then(&e, MariaSlideStart(column, mx_y_from(x, y, e)));
                        return e;
                    }
                }
            }
            mx_then(&e, MariaJumpLand(column, mx_y_from(x, y, e)));
            return e;
        }
    }
    jsr_push(0xB232);
    marked = MariaOnMarkedFloor();
    jsr_pop();
    mx_on_marked(&e, marked);
    if (marked.how == FLOOR_MARKED || marked.how == FLOOR_MARKED_DOWN) {
        mx_then(&e, MariaLandOnMarked(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    jsr_push(0xB23A);
    feet = MariaFeetOnMarkedFloor();
    jsr_pop();
    mx_then(&e, feet);
    if (mx_carry(e)) {
        mx_then(&e, MariaLandOnMarkedFeet(mx_x_from(x, y, e), mx_y_from(x, y, e)));
        return e;
    }
    jsr_push(0xB242);
    mx_then(&e, MariaCheckBit3Under(mx_x_from(x, y, e), mx_y_from(x, y, e)));
    jsr_pop();
    value = RD8(OBJ->vy_lo[OBJECT_PLAYER]);
    lo = add_bytes(value, SLIDE_GRAVITY, false);
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
    jsr_push(0xB25F);
    jump = MariaJumpPressed();
    jsr_pop();
    mx_regs(&e, r_c(regs_kept(), jump));
    if (jump) {
        mx_then(&e, MariaSecondJump());
        return e;
    }
    mx_animation(&e, MariaAnimationTick());
    return e;
}

void abi_MariaStateSlideAir(void)
{
    mx_exit(MariaStateSlideAir(cpu.x, cpu.y));
}
