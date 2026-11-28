/* The stage's ending effect: two objects fall and a fade is drawn from random bytes, stepped each frame. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum {
    OBJECT_X = 84, OBJECT_Y = 85, LAST_STEP = 6, STEPS_DONE = 7, MODE_CD_FIRST = 0x0C, MODE_CD_SECOND = 0x10,
    SPEED_TABLE = 0x9049, TIMER_TABLE = 0x904F, FADE_TABLE = 0x9056, FLAG_DONE = 0x01, BLEND_KEEP = 0x1F,
    CD_RETURN = 0x8FD8, VELOCITY_Y_RETURN = 0x901B, VELOCITY_X_RETURN = 0x903D, RANDOM_RETURN = 0x909F,
    MASK_RETURN = 0x90A1, MASK_RUN = 0x90B1, FADE_ROWS = 0x10
};

/* The overflow and carry of an axis move, as the original's add leaves them. */
static void move_flags(AxisMove move, bool *c, bool *v)
{
    uint32_t added = (uint32_t)(int32_t)move.velocity & 0xFFFFFF;
    uint32_t sum = move.before + added;
    uint8_t sign = (uint8_t)(added >> 16);
    uint8_t high = (uint8_t)(move.before >> 16);
    uint8_t result = (uint8_t)(sum >> 16);

    *c = sum > 0xFFFFFF;
    *v = (~(sign ^ high) & (sign ^ result) & 0x80) != 0;
}

static uint8_t move_high(AxisMove move)
{
    uint32_t added = (uint32_t)(int32_t)move.velocity & 0xFFFFFF;

    return (uint8_t)((move.before + added) >> 16);
}

/* Draws the fade from the step's table row: random bytes the mask routine takes, the address moving on a column each. */
static StageExit draw_fade(uint8_t pick)
{
    StageExit end = { 0, 0, 0, false, false, 0, EXIT_X | EXIT_C | EXIT_V };
    uint8_t a = pick;
    uint8_t m;
    uint8_t x;
    Sum sum;
    RandomResult random;
    VramMaskExit mask = { 0, false, { 0, false, false }, 0 };
    bool carry = false;

    WR8(ZP->sprites_left, a);
    if (a >= 0x20) {
        a = (uint8_t)(a & BLEND_KEEP);
        carry = (a & 0x01) != 0;
        a = (uint8_t)(a >> 1);
        if (carry) {
            a = (uint8_t)(a + 0x10);
        }
        a = (uint8_t)(a + 0x20);
    }
    push8(a);
    a = (uint8_t)(a & 0x0F);
    WR8(ZP->temp10, a);
    a = FADE_ROWS;
    sum = sub_bytes(a, RD8(ZP->temp10), true);
    WR8(ZP->sprites_left, sum.value);
    a = pull8();
    a = (uint8_t)(a >> 2);
    x = (uint8_t)(a & 0x0C);
    a = rd8((uint16_t)(FADE_TABLE + x));
    WR8(ZP->temp10, a);
    a = rd8((uint16_t)(FADE_TABLE + 1 + x));
    WR8(ZP->temp11, a);
    a = rd8((uint16_t)(FADE_TABLE + 2 + x));
    WR8(ZP->temp12, a);
    a = rd8((uint16_t)(FADE_TABLE + 3 + x));
    WR8(ZP->product.lo, a);
    do {
        jsr_push(RANDOM_RETURN);
        random = NextRandom();
        jsr_pop();
        (void)random;
        jsr_push(MASK_RETURN);
        if (resident_at(MASK_RUN) == DISC(0x12CF, 0x0B1)) {
            mask = MaskVramRowsRandomLong();
        } else {
            mask = MaskVramRowsRandom();
        }
        jsr_pop();
        a = RD8(ZP->temp10);
        m = RD8(ZP->product.lo);
        sum = add_bytes(a, m, false);
        WR8(ZP->temp10, sum.value);
        if (sum.carry) {
            INC8(ZP->temp11);
        }
        m = DEC8(ZP->temp12);
        end.a = sum.value;
    } while (m != 0);
    end.x = 0;
    end.y = mask.y;
    end.set |= EXIT_Y;
    end.c = sum.carry;
    end.v = sum.overflow;
    end.nz = 0;
    return end;
}

/* One step of the ending effect: the CD track when the stage's mode asks, the two objects' motion and the fade. */
static StageExit step_stage_ending(uint8_t x, uint8_t y)
{
    StageExit end = { 0, x, y, false, false, 0, 0 };
    StageExit played;
    AxisMove move;
    uint8_t a;
    uint8_t m;
    bool carry;

    a = RD8(OBJ->angle_lo[OBJECT_X]);
    if (a == 0) {
        a = RD8(ZP->stage);
        if (a != 0 && a != MODE_CD_FIRST && a != MODE_CD_SECOND) {
            jsr_push(CD_RETURN);
            play_track_exit_shim(CdPlayAgain(x, y));
            jsr_pop();
            played = stage_exit_from_cpu_shim();
            a = played.a;
            end.x = played.x;
            end.y = played.y;
            end.c = played.c;
            end.v = played.v;
            end.set = EXIT_X | EXIT_Y | EXIT_C | EXIT_V;
            x = played.x;
            y = played.y;
        }
        INC8(OBJ->angle_lo[OBJECT_X]);
    }
    m = DEC8(OBJ->work_e[OBJECT_Y]);
    if (m == 0) {
        m = INC8(OBJ->life_timer[OBJECT_Y]);
        y = RD8(OBJ->life_timer[OBJECT_Y]);
        end.y = y;
        end.set |= EXIT_Y;
        if (y == STEPS_DONE) {
            WR8(OBJ->frame[OBJECT_Y], 0);
            RMW8(ZP->field8e, flags, flags | FLAG_DONE);
            end.a = a;
            end.c = true;
            end.nz = 0;
            end.set |= EXIT_C;
            return end;
        }
        a = rd8((uint16_t)(SPEED_TABLE + y));
        carry = (a & 0x01) != 0;
        a = (uint8_t)(a >> 1);
        WR8(OBJ->vy_hi[OBJECT_Y], a);
        a = carry ? 0x80 : 0;
        WR8(OBJ->vy_lo[OBJECT_Y], a);
        a = rd8((uint16_t)(TIMER_TABLE + y));
        WR8(OBJ->work_e[OBJECT_Y], a);
    }
    a = RD8(OBJ->life_timer[OBJECT_Y]);
    if (a == LAST_STEP) {
        a = RD8(OBJ->work_e[OBJECT_Y]);
        if (a == BLEND_KEEP) {
            WR8(OBJ->frame[OBJECT_X], 0);
        }
        WR8(OBJ->angle_hi[OBJECT_X], a);
    }
    jsr_push(VELOCITY_Y_RETURN);
    move = ApplyVelocityY(OBJECT_Y);
    jsr_pop();
    m = DEC8(OBJ->work_e[OBJECT_X]);
    if (m == 0) {
        INC8(OBJ->life_timer[OBJECT_X]);
        y = RD8(OBJ->life_timer[OBJECT_X]);
        end.y = y;
        end.set |= EXIT_Y;
        a = rd8((uint16_t)(SPEED_TABLE + y));
        carry = (a & 0x01) != 0;
        a = (uint8_t)(a >> 1);
        WR8(OBJ->vx_hi[OBJECT_X], a);
        a = carry ? 0x80 : 0;
        WR8(OBJ->vx_lo[OBJECT_X], a);
        a = rd8((uint16_t)(TIMER_TABLE + y));
        WR8(OBJ->work_e[OBJECT_X], a);
    }
    jsr_push(VELOCITY_X_RETURN);
    move = ApplyVelocityX(OBJECT_X);
    jsr_pop();
    end.a = move_high(move);
    move_flags(move, &end.c, &end.v);
    end.x = OBJECT_X;
    end.set |= EXIT_X | EXIT_C | EXIT_V;
    a = RD8(OBJ->life_timer[OBJECT_X]);
    end.c = a >= LAST_STEP;
    end.nz = (uint8_t)(a - LAST_STEP);
    if (a != LAST_STEP) {
        end.a = a;
        return end;
    }
    a = RD8(OBJ->angle_hi[OBJECT_X]);
    played = draw_fade(a);
    played.set |= EXIT_X | EXIT_C | EXIT_V;
    return played;
}

/*
 * @routine 024E:7C4 StepStageEnding
 * @run 6B:8FC4
 * WHAT IT DOES: One step of a stage's ending: starts the CD track again on the first step unless
 * the stage's mode is one of two, drops two objects along tables of speeds and times (a step
 * each when their timers run out; the last step of the first sets the done flag in zField8E), and
 * on the sixth step of the second draws a fade of random columns.
 * HOW: Two timers count down in the f3510 column's slots 84 and 85; the tables at $9049 and $904F
 * give each step's speed and timer; the fade is MaskVramRowsRandom called with the table row
 * the picked step names.
 * Every stage overlay holds the same code here (the fade picks the overlay's mask routine).
 * WAITS: no.
 * Entry: X and Y as the CD track routine keeps them. Exit: as the last step left it.
 */
StageExit StepStageEnding(uint8_t x, uint8_t y)
{
    return step_stage_ending(x, y);
}

void abi_StepStageEnding(void)
{
    stage_exit_shim(StepStageEnding(cpu.x, cpu.y));
}
