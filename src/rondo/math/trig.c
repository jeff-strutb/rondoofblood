/*
 * Angles, sines, the multiply under them, and the game's random numbers.
 *
 * The bodies work in whole words and leave the original's results in the original's RAM.
 * What the assembly leaves in A and the flags is worked out from those values in the
 * *_exit_shim functions, for the abi_ wrappers, and nowhere else.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "common_objects.h"

#define PRODUCT_MASK 0xFFFFFFu
#define PRODUCT_TOP 0x800000u

enum {
    MULTIPLY_ROUNDS = 9,
    MULTIPLIER_BITS = 0x1FF,
    ANGLE_NEGATIVE = 0x80,
    QUARTER_TURN = 0x40,
    HALF_TURN = 0x80,
    ATAN_AXIS_MAX = 0x0F
};

/*
 * @routine 0045:69A MultiplyBySineWord
 * @run 69:569A
 * WHAT IT DOES: Multiplies the 16-bit value in zTemp00 by the 9-bit word in
 * zMultiplierLo, for a 24-bit product in zProduct0-2; the word is a sine, so
 * the product is the value scaled by that sine as 8.8.
 * HOW: Shift and add, nine rounds: the multiplier shifts right through RAM, a
 * 24-bit copy of the value in zTemp10-12 shifts left, and each bit shifted out
 * set adds the copy to the product, byte by byte with the carry running on.
 * WAITS: no.
 * Entry: zTemp00-01 value, zMultiplierLo-Hi word, D clear. Exit: A the last byte
 * loaded (the top of the product if any bit was set, else zTemp01), Y 0, N 0,
 * Z 1, C the top bit shifted out of zTemp12, V from the last add if any.
 * X kept. Clobbers zTemp10-12, zMultiplierLo-Hi.
 */
ScaleResult MultiplyBySineWord(void)
{
    ScaleResult result = { 0, 0, false, false, false };
    uint8_t value, operand;
    bool bit, carry;
    Sum sum;

    WR8(ZP->product.lo, 0);
    WR8(ZP->product.mid, 0);
    WR8(ZP->product.hi, 0);
    value = RD8(ZP->temp00);
    WR8(ZP->temp10, value);
    value = RD8(ZP->temp01);
    WR8(ZP->temp11, value);
    result.a = value;
    WR8(ZP->temp12, 0);
    for (int round = 0; round < MULTIPLY_ROUNDS; round++) {
        bit = LSR8(ZP->multiplier.hi);
        bit = ROR8(ZP->multiplier.lo, bit);
        if (bit) {
            value = RD8(ZP->product.lo);
            operand = RD8(ZP->temp10);
            sum = add_bytes(value, operand, false);
            WR8(ZP->product.lo, sum.value);
            value = RD8(ZP->product.mid);
            operand = RD8(ZP->temp11);
            sum = add_bytes(value, operand, sum.carry);
            WR8(ZP->product.mid, sum.value);
            value = RD8(ZP->product.hi);
            operand = RD8(ZP->temp12);
            sum = add_bytes(value, operand, sum.carry);
            WR8(ZP->product.hi, sum.value);
            result.a = sum.value;
            result.v_set = true;
            result.overflow = sum.overflow;
        }
        carry = ASL8(ZP->temp10);
        carry = ROL8(ZP->temp11, carry);
        carry = ROL8(ZP->temp12, carry);
        result.carry = carry;
    }
    return result;
}

/* A the multiply's last byte, N 0, Z 1, C, V if an add ran, Y 0. */
void scale_exit_shim(ScaleResult result)
{
    cpu.a = result.a;
    abi_nz(result.nz);
    abi_flag(P_C, result.carry);
    if (result.v_set) {
        abi_flag(P_V, result.overflow);
    }
    cpu.y = 0;
}

void abi_MultiplyBySineWord(void)
{
    scale_exit_shim(MultiplyBySineWord());
}

/*
 * The half wave's word for an angle goes to the multiplier (the angle doubled, its bit 7 out
 * as the sign), then the multiply runs; the return address of its call is given.
 */
static ScaleResult scale_by_sine_body(uint8_t angle)
{
    uint8_t index = (uint8_t)(angle << 1);
    bool negative = (angle & ANGLE_NEGATIVE) != 0;
    ScaleResult result;
    Sum sum;
    uint8_t value;

    WR8(ZP->multiplier.lo, rd8((uint16_t)(SineTable + index)));
    WR8(ZP->multiplier.hi, rd8((uint16_t)(SineTable + 1 + index)));
    jsr_push(negative ? 0x5680 : 0x5671);
    result = MultiplyBySineWord();
    jsr_pop();
    if (!negative) {
        return result;
    }
    value = RD8(ZP->product.lo);
    sum = add_bytes((uint8_t)(value ^ 0xFF), 0, true);
    WR8(ZP->product.lo, sum.value);
    value = RD8(ZP->product.mid);
    sum = add_bytes((uint8_t)(value ^ 0xFF), 0, sum.carry);
    WR8(ZP->product.mid, sum.value);
    value = RD8(ZP->product.hi);
    sum = add_bytes((uint8_t)(value ^ 0xFF), 0, sum.carry);
    WR8(ZP->product.hi, sum.value);
    result.a = sum.value;
    result.nz = sum.value;
    result.carry = sum.carry;
    result.v_set = true;
    result.overflow = sum.overflow;
    return result;
}

/*
 * @routine 0045:660 ScaleBySine
 * @run 69:5660
 * WHAT IT DOES: Scales the value in zTemp00 by the sine of an angle: 256 steps
 * to the turn, angle 64 the peak, angles from 128 up negative.
 * HOW: Doubles the angle for a word index: bit 7 goes out as the sign, the
 * rest indexes the half wave in SineTable. Multiplies, and for the negative
 * half takes the two's complement of the 24-bit product.
 * WAITS: no.
 * Entry: A angle, zTemp00-01 value, D clear. Exit: zProduct0-2 result; A, N, Z, C and V
 * from the multiply, or from the complement's last add when negative; Y 0.
 * X kept. Clobbers zTemp10-12, zMultiplierLo-Hi.
 */
ScaleResult ScaleBySine(uint8_t angle)
{
    return scale_by_sine_body(angle);
}

void abi_ScaleBySine(void)
{
    scale_exit_shim(ScaleBySine(cpu.a));
}

/*
 * @routine 0045:65D ScaleByCosine
 * @run 69:565D
 * WHAT IT DOES: Scales the value in zTemp00 by the cosine of an angle.
 * HOW: A quarter turn added to the angle, then ScaleBySine, which it runs into.
 * WAITS: no.
 * Entry: A angle, zTemp00-01 value, D clear. Exit: as ScaleBySine; V from the angle's
 * add when no later add set it.
 */
ScaleResult ScaleByCosine(uint8_t angle)
{
    uint8_t turned = (uint8_t)(angle + QUARTER_TURN);
    ScaleResult result = scale_by_sine_body(turned);

    if (!result.v_set) {
        result.v_set = true;
        result.overflow = add_bytes(angle, QUARTER_TURN, false).overflow;
    }
    return result;
}

void abi_ScaleByCosine(void)
{
    scale_exit_shim(ScaleByCosine(cpu.a));
}

/*
 * @routine 0045:640 SetVelocityFromAngle
 * @run 69:5640
 * WHAT IT DOES: Points an object along an angle at a given speed: its X
 * velocity becomes speed times cosine, its Y velocity speed times sine.
 * HOW: Keeps the angle on the stack, ScaleByCosine and ScaleBySine in turn, taking the top
 * two bytes of each product as an 8.8 velocity.
 * WAITS: no.
 * Entry: A angle, X object, zTemp00-01 speed, D clear. Exit: A the Y velocity's low
 * byte, N and Z from it, C and V from the sine (V from the cosine if the sine set none), Y 0.
 * X kept. Clobbers zTemp10-12, zProduct0-2, zMultiplierLo-Hi.
 */
ScaleResult SetVelocityFromAngle(uint8_t angle, uint8_t object)
{
    ScaleResult cosine, sine;
    uint8_t byte;

    push8(angle);
    jsr_push(0x5644);
    cosine = ScaleByCosine(angle);
    jsr_pop();
    byte = RD8(ZP->product.hi);
    WR8(OBJ->vx_hi[object], byte);
    byte = RD8(ZP->product.mid);
    WR8(OBJ->vx_lo[object], byte);
    angle = pull8();
    jsr_push(0x5652);
    sine = ScaleBySine(angle);
    jsr_pop();
    byte = RD8(ZP->product.hi);
    WR8(OBJ->vy_hi[object], byte);
    byte = RD8(ZP->product.mid);
    WR8(OBJ->vy_lo[object], byte);
    sine.a = byte;
    sine.nz = byte;
    if (!sine.v_set) {
        sine.v_set = true;
        sine.overflow = cosine.overflow;
    }
    return sine;
}

void abi_SetVelocityFromAngle(void)
{
    scale_exit_shim(SetVelocityFromAngle(cpu.a, cpu.x));
}

/* Negates the 16-bit value at the two bytes, low byte first, the carry running into the high byte. */
static void negate_temp_word(bool y_axis)
{
    uint8_t value;
    Sum sum;

    if (!y_axis) {
        value = RD8(ZP->temp00);
        sum = add_bytes((uint8_t)(value ^ 0xFF), 0, true);
        WR8(ZP->temp00, sum.value);
        value = RD8(ZP->temp01);
        sum = add_bytes((uint8_t)(value ^ 0xFF), 0, sum.carry);
        WR8(ZP->temp01, sum.value);
    } else {
        value = RD8(ZP->temp02);
        sum = add_bytes((uint8_t)(value ^ 0xFF), 0, true);
        WR8(ZP->temp02, sum.value);
        value = RD8(ZP->temp03);
        sum = add_bytes((uint8_t)(value ^ 0xFF), 0, sum.carry);
        WR8(ZP->temp03, sum.value);
    }
}

/*
 * @routine 0046:0B8 AngleOfFirstQuadrant
 * @run 69:58B8
 * WHAT IT DOES: The angle of a vector whose two axes (16-bit, zTemp00-01 across and zTemp02-03
 * up) are both positive, within the first quarter turn.
 * HOW: Both axes shift down together until the larger fits in a byte, then until both fit four
 * bits (the counts kept in A), and the two nibbles index a table of angles at overlay address
 * $58E7 (Y first, X in the low nibble).
 * WAITS: no.
 * Entry: zTemp00-03 the axes, D clear. Exit: A the angle, N and Z from it, C 0, V 1 (the
 * BIT #$F0 leaves V set), Y the table index; zTemp00-03 shifted down.
 */
AngleExit AngleOfFirstQuadrant(void)
{
    AngleExit result = { 0, 0, false, true };
    uint8_t high, counter, low;
    bool carry;

    high = RD8(ZP->temp01);
    counter = (uint8_t)(high | RD8(ZP->temp03));
    if (counter != 0) {
        do {
            carry = LSR8(ZP->temp01);
            ROR8(ZP->temp00, carry);
            carry = LSR8(ZP->temp03);
            ROR8(ZP->temp02, carry);
            counter = (uint8_t)(counter >> 1);
        } while (counter != 0);
    }
    low = RD8(ZP->temp00);
    counter = (uint8_t)(low | RD8(ZP->temp02));
    while ((counter & 0xF0) != 0) {
        LSR8(ZP->temp00);
        LSR8(ZP->temp02);
        counter = (uint8_t)(counter >> 1);
    }
    low = RD8(ZP->temp02);
    result.y = (uint8_t)((uint8_t)(low << 4) | RD8(ZP->temp00));
    result.a = rd8((uint16_t)(AtanTable + result.y));
    return result;
}

/* A the angle, N and Z from it, C and V as the last op left them, Y the table index when the core ran. */
void angle_exit_shim(AngleExit result)
{
    cpu.a = result.a;
    abi_nz(result.a);
    abi_flag(P_C, result.c);
    abi_flag(P_V, result.v);
    cpu.y = result.y;
}

void abi_AngleOfFirstQuadrant(void)
{
    angle_exit_shim(AngleOfFirstQuadrant());
}

/*
 * @routine 0046:04F AngleFromDelta
 * @run 69:584F
 * WHAT IT DOES: The angle of a vector from its signed 16-bit axes (zTemp00-01 across, zTemp02-03
 * up): both are made positive, AngleOfFirstQuadrant gives the angle in the first quarter turn, and
 * the signs fold it into its quarter.
 * HOW: Tests the signs of the two high bytes; the negative axes are negated in place; the
 * folds are: across negative, up positive: $81 minus the angle; both negative: the angle plus
 * $80; across positive, up negative: 2 minus the angle (in bytes, $FF minus it plus 1 plus 1).
 * WAITS: no.
 * Entry: zTemp00-03 the axes, D clear. Exit: A the angle, N and Z from it, C and V as the last
 * add or subtract (or the first quarter's: C 0, V 1) left them, Y the table index.
 */
AngleExit AngleFromDelta(void)
{
    uint8_t y_high = RD8(ZP->temp03);
    uint8_t x_high;
    AngleExit core;
    Sum sum;

    if ((y_high & 0x80) == 0) {
        x_high = RD8(ZP->temp01);
        if ((x_high & 0x80) == 0) {
            return AngleOfFirstQuadrant();
        }
        negate_temp_word(false);
        jsr_push(0x586C);
        core = AngleOfFirstQuadrant();
        jsr_pop();
        sum = add_bytes(core.a, 0x7F, true);
        core.v = sum.overflow;
        core.c = sum.carry;
        core.a = (uint8_t)(((uint8_t)(core.a - 0x80) ^ 0xFF) + 2);
        return core;
    }
    x_high = RD8(ZP->temp01);
    if ((x_high & 0x80) == 0) {
        negate_temp_word(true);
        jsr_push(0x58B3);
        core = AngleOfFirstQuadrant();
        jsr_pop();
        core.a = (uint8_t)((core.a ^ 0xFF) + 2);
        return core;
    }
    negate_temp_word(false);
    negate_temp_word(true);
    jsr_push(0x589C);
    core = AngleOfFirstQuadrant();
    jsr_pop();
    sum = add_bytes(core.a, 0x80, false);
    core.a = sum.value;
    core.c = sum.carry;
    core.v = sum.overflow;
    return core;
}

void abi_AngleFromDelta(void)
{
    angle_exit_shim(AngleFromDelta());
}

/*
 * @routine 0046:02D AngleToPlayer
 * @run 69:582D
 * WHAT IT DOES: The angle from an object to the player: the player's position minus the object's,
 * each axis a 16-bit subtract, then AngleFromDelta.
 * HOW: Four bytes subtracted with the carries running on; it runs into AngleFromDelta.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as AngleFromDelta. Clobbers zTemp00-03.
 */
AngleExit AngleToPlayer(uint8_t object)
{
    uint8_t value, other;
    Sum sum;

    value = RD8(OBJ->x_lo[0]);
    other = RD8(OBJ->x_lo[object]);
    sum = add_bytes(value, (uint8_t)~other, true);
    WR8(ZP->temp00, sum.value);
    value = RD8(OBJ->x_hi[0]);
    other = RD8(OBJ->x_hi[object]);
    sum = add_bytes(value, (uint8_t)~other, sum.carry);
    WR8(ZP->temp01, sum.value);
    value = RD8(OBJ->y_lo[0]);
    other = RD8(OBJ->y_lo[object]);
    sum = add_bytes(value, (uint8_t)~other, true);
    WR8(ZP->temp02, sum.value);
    value = RD8(OBJ->y_hi[0]);
    other = RD8(OBJ->y_hi[object]);
    sum = add_bytes(value, (uint8_t)~other, sum.carry);
    WR8(ZP->temp03, sum.value);
    return AngleFromDelta();
}

void abi_AngleToPlayer(void)
{
    angle_exit_shim(AngleToPlayer(cpu.x));
}

/*
 * @routine 0045:7CF AngleToPoint
 * @run 69:57CF
 * WHAT IT DOES: The angle from an object to a point: the point's coordinates (zTemp08-0B) minus
 * the object's, each axis a 16-bit subtract, then AngleFromDelta.
 * HOW: As AngleToPlayer, with the point in place of the player; a branch into AngleFromDelta.
 * WAITS: no.
 * Entry: X object, zTemp08-0B the point, D clear. Exit: as AngleFromDelta. Clobbers zTemp00-03.
 */
AngleExit AngleToPoint(uint8_t object)
{
    uint8_t value, other;
    Sum sum;

    value = RD8(ZP->temp08);
    other = RD8(OBJ->x_lo[object]);
    sum = add_bytes(value, (uint8_t)~other, true);
    WR8(ZP->temp00, sum.value);
    value = RD8(ZP->temp09);
    other = RD8(OBJ->x_hi[object]);
    sum = add_bytes(value, (uint8_t)~other, sum.carry);
    WR8(ZP->temp01, sum.value);
    value = RD8(ZP->temp0_a);
    other = RD8(OBJ->y_lo[object]);
    sum = add_bytes(value, (uint8_t)~other, true);
    WR8(ZP->temp02, sum.value);
    value = RD8(ZP->temp0_b);
    other = RD8(OBJ->y_hi[object]);
    sum = add_bytes(value, (uint8_t)~other, sum.carry);
    WR8(ZP->temp03, sum.value);
    return AngleFromDelta();
}

void abi_AngleToPoint(void)
{
    angle_exit_shim(AngleToPoint(cpu.x));
}

/*
 * @routine 0045:7EF AngleToPlayerOffset
 * @run 69:57EF
 * WHAT IT DOES: The angle from an object to a point offset from the player: the player's position
 * minus the object's, plus the offset in zTemp08-0B, each axis 16 bits, then AngleFromDelta.
 * HOW: The four subtracts as AngleToPlayer, then the four adds of the offset; a branch into
 * AngleFromDelta.
 * WAITS: no.
 * Entry: X object, zTemp08-0B the offset, D clear. Exit: as AngleFromDelta. Clobbers zTemp00-03.
 */
AngleExit AngleToPlayerOffset(uint8_t object)
{
    uint8_t value, other;
    Sum sum;

    value = RD8(OBJ->x_lo[0]);
    other = RD8(OBJ->x_lo[object]);
    sum = add_bytes(value, (uint8_t)~other, true);
    WR8(ZP->temp00, sum.value);
    value = RD8(OBJ->x_hi[0]);
    other = RD8(OBJ->x_hi[object]);
    sum = add_bytes(value, (uint8_t)~other, sum.carry);
    WR8(ZP->temp01, sum.value);
    value = RD8(OBJ->y_lo[0]);
    other = RD8(OBJ->y_lo[object]);
    sum = add_bytes(value, (uint8_t)~other, true);
    WR8(ZP->temp02, sum.value);
    value = RD8(OBJ->y_hi[0]);
    other = RD8(OBJ->y_hi[object]);
    sum = add_bytes(value, (uint8_t)~other, sum.carry);
    WR8(ZP->temp03, sum.value);
    value = RD8(ZP->temp00);
    other = RD8(ZP->temp08);
    sum = add_bytes(value, other, false);
    WR8(ZP->temp00, sum.value);
    value = RD8(ZP->temp01);
    other = RD8(ZP->temp09);
    sum = add_bytes(value, other, sum.carry);
    WR8(ZP->temp01, sum.value);
    value = RD8(ZP->temp02);
    other = RD8(ZP->temp0_a);
    sum = add_bytes(value, other, false);
    WR8(ZP->temp02, sum.value);
    value = RD8(ZP->temp03);
    other = RD8(ZP->temp0_b);
    sum = add_bytes(value, other, sum.carry);
    WR8(ZP->temp03, sum.value);
    return AngleFromDelta();
}

void abi_AngleToPlayerOffset(void)
{
    angle_exit_shim(AngleToPlayerOffset(cpu.x));
}

/*
 * @routine 0046:2D2 TurnAngleToward
 * @run 69:5AD2
 * WHAT IT DOES: Turns an angle one step toward a target, the short way round.
 * HOW: Equal angles stay. Otherwise compares them in the half of the circle the current angle
 * lies in, a half turn deciding which way is shorter, and adds or subtracts the step. A step
 * can overshoot the target: nothing here clamps it.
 * WAITS: no.
 * Entry: zTemp00 step, zTemp01 current, zTemp02 target, D clear. Exit: A the new angle, N and Z
 * from it, C and V from the step; on equal angles A current, N 0, Z 1, C 1, V kept. X and Y
 * kept.
 */
TurnAngleResult TurnAngleToward(void)
{
    TurnAngleResult result = { 0, true, { 0, false, false } };
    uint8_t now, target, shifted, step;
    bool down;

    now = RD8(ZP->temp01);
    target = RD8(ZP->temp02);
    result.angle = now;
    if (now == target) {
        return result;
    }
    result.equal = false;
    now = RD8(ZP->temp01);
    if ((now & 0x80) != 0) {
        target = RD8(ZP->temp02);
        if (now < target) {
            down = false;
        } else {
            shifted = (uint8_t)(now - 0x80);
            target = RD8(ZP->temp02);
            down = shifted < target;
        }
    } else {
        target = RD8(ZP->temp02);
        if (now >= target) {
            down = true;
        } else {
            shifted = (uint8_t)(now + 0x80);
            target = RD8(ZP->temp02);
            down = shifted < target;
        }
    }
    now = RD8(ZP->temp01);
    step = RD8(ZP->temp00);
    result.last = down ? add_bytes(now, (uint8_t)~step, true) : add_bytes(now, step, false);
    return result;
}

/* Equal: A the angle, N 0, Z 1, C 1. Else A the new angle, N and Z from it, C and V from the step. */
void turn_angle_exit_shim(TurnAngleResult result)
{
    if (result.equal) {
        cpu.a = result.angle;
        abi_nz(0);
        abi_flag(P_C, true);
        return;
    }
    cpu.a = result.last.value;
    abi_nz(result.last.value);
    abi_flag(P_C, result.last.carry);
    abi_flag(P_V, result.last.overflow);
}

void abi_TurnAngleToward(void)
{
    turn_angle_exit_shim(TurnAngleToward());
}

/*
 * @routine 0046:29A NextRandom
 * @run 69:5A9A
 * WHAT IT DOES: Steps the game's random numbers and returns the next byte. The
 * frame interrupt also steps it once a frame, so it depends on time as well as
 * on calls.
 * HOW: A 32-bit counter generator: the state becomes state * $01010101 + 1. Each
 * byte is added into those above it, the carry running on between the adds, top byte
 * first, then the 32-bit increment, and the top byte is read again as the answer.
 * WAITS: no.
 * Entry: nothing, D clear. Exit: A zRandom3, N and Z from it, C and V from the last add (the
 * top byte plus zRandom0 with the carry of the one before). X and Y kept. The increments
 * past zRandom0 (when it wraps) ran in no script: the draft stops there, so liftcmp does not
 * judge them; they wait for coverage (PROCEDURE section 6).
 */
RandomResult NextRandom(void)
{
    RandomResult result;
    Sum sum;
    uint8_t value, other;

    value = RD8(ZP->random3);
    other = RD8(ZP->random2);
    sum = add_bytes(value, other, false);
    WR8(ZP->random3, sum.value);
    value = RD8(ZP->random2);
    other = RD8(ZP->random1);
    sum = add_bytes(value, other, false);
    WR8(ZP->random2, sum.value);
    value = RD8(ZP->random3);
    other = RD8(ZP->random1);
    sum = add_bytes(value, other, sum.carry);
    WR8(ZP->random3, sum.value);
    value = RD8(ZP->random1);
    other = RD8(ZP->random0);
    sum = add_bytes(value, other, false);
    WR8(ZP->random1, sum.value);
    value = RD8(ZP->random2);
    other = RD8(ZP->random0);
    sum = add_bytes(value, other, sum.carry);
    WR8(ZP->random2, sum.value);
    value = RD8(ZP->random3);
    other = RD8(ZP->random0);
    sum = add_bytes(value, other, sum.carry);
    WR8(ZP->random3, sum.value);
    if (INC8(ZP->random0) == 0) {
        if (INC8(ZP->random1) == 0) {
            if (INC8(ZP->random2) == 0) {
                INC8(ZP->random3);
            }
        }
    }
    result.value = RD8(ZP->random3);
    result.carry = sum.carry;
    result.overflow = sum.overflow;
    return result;
}

void random_exit_shim(RandomResult result)
{
    cpu.a = result.value;
    abi_nz(result.value);
    abi_flag(P_C, result.carry);
    abi_flag(P_V, result.overflow);
}

void abi_NextRandom(void)
{
    random_exit_shim(NextRandom());
}
