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

/* The two velocities' 24-bit products, the top two bytes of each being the velocity. */
typedef struct {
    uint32_t x;
    uint32_t y;
} ScaledVector;

/* How the first quadrant's angle folds into the vector's real quadrant. */
typedef enum { FOLD_NONE, FOLD_MIRROR, FOLD_HALF_TURN, FOLD_NEGATE } AngleFold;

typedef struct {
    uint8_t angle;
    uint8_t index;
    uint8_t first_quadrant;
    AngleFold fold;
} AngleResult;

static uint16_t trig_word(uint16_t lo, uint16_t hi)
{
    uint8_t low = rd8(lo);

    return (uint16_t)(low | (rd8(hi) << 8));
}

static void trig_put_word(uint16_t lo, uint16_t hi, uint16_t value)
{
    wr8(lo, (uint8_t)value);
    wr8(hi, (uint8_t)(value >> 8));
}

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

/*
 * The direction of a vector. Both axes are made positive and shifted down together until
 * each fits four bits, which index the first quadrant's angles; the angle is then folded
 * by the signs. What is left of the axes stays in zTemp00-03.
 */
static AngleResult angle_of_vector(uint16_t dx, uint16_t dy)
{
    bool x_negative = (dx & 0x8000) != 0;
    bool y_negative = (dy & 0x8000) != 0;
    AngleResult result;

    if (x_negative)
        dx = (uint16_t)(0u - dx);
    if (y_negative)
        dy = (uint16_t)(0u - dy);
    while ((dx | dy) > ATAN_AXIS_MAX) {
        dx >>= 1;
        dy >>= 1;
    }
    trig_put_word(zTemp00, zTemp01, dx);
    trig_put_word(zTemp02, zTemp03, dy);
    result.index = (uint8_t)((dy << 4) | dx);
    result.first_quadrant = rd8((uint16_t)(AtanTable + result.index));
    if (x_negative && y_negative) {
        result.fold = FOLD_HALF_TURN;
        result.angle = (uint8_t)(result.first_quadrant + HALF_TURN);
    } else if (x_negative) {
        result.fold = FOLD_MIRROR;
        result.angle = (uint8_t)(HALF_TURN + 1 - result.first_quadrant);
    } else if (y_negative) {
        result.fold = FOLD_NEGATE;
        result.angle = (uint8_t)(1 - result.first_quadrant);
    } else {
        result.fold = FOLD_NONE;
        result.angle = result.first_quadrant;
    }
    return result;
}

/*
 * The table read leaves C clear, the axes' last shift having moved out a zero, and V set,
 * by the BIT #$F0 on every path. The mirror's SBC #$80 and the half turn's ADC #$80 then
 * set both from the first quadrant's angle.
 */
static Alu angle_exit_shim(AngleResult result)
{
    Alu exit = { 0 };
    bool high = result.first_quadrant >= 0x80;

    exit.a = result.angle;
    exit.nz = result.angle;
    exit.v_set = true;
    switch (result.fold) {
    case FOLD_NONE:
    case FOLD_NEGATE:
        exit.c = false;
        exit.v = true;
        break;
    case FOLD_MIRROR:
        exit.c = high;
        exit.v = !high;
        break;
    case FOLD_HALF_TURN:
        exit.c = high;
        exit.v = high;
        break;
    }
    return exit;
}

static void abi_angle(AngleResult result)
{
    abi_alu(angle_exit_shim(result));
    cpu.y = result.index;
}

/*
 * @routine 0046:04F AngleFromDelta
 * @run 69:584F
 * WHAT IT DOES: Gives the direction of a 16-bit vector as an angle, 256 steps
 * to the turn.
 * HOW: By the signs of Y then X, makes both axes positive in RAM, reads the
 * first-quadrant angle and folds it into its real quadrant: X negative and Y
 * positive mirrors about a quarter turn, both negative adds a half turn, Y
 * negative alone negates.
 * WAITS: no.
 * Entry: zTemp00-01 delta X, zTemp02-03 delta Y. Exit: A the angle, Y the
 * table index, N and Z from the last step, C and V as it left them. X kept.
 * Clobbers zTemp00-03.
 */
AngleResult AngleFromDelta(void)
{
    uint16_t dx = trig_word(zTemp00, zTemp01);
    uint16_t dy = trig_word(zTemp02, zTemp03);

    return angle_of_vector(dx, dy);
}

void abi_AngleFromDelta(void)
{
    abi_angle(AngleFromDelta());
}

static uint16_t trig_object_x(uint8_t object)
{
    return trig_word((uint16_t)(OBJ_X_LO + object), (uint16_t)(OBJ_X_HI + object));
}

static uint16_t trig_object_y(uint8_t object)
{
    return trig_word((uint16_t)(OBJ_Y_LO + object), (uint16_t)(OBJ_Y_HI + object));
}

/*
 * @routine 0045:7CF AngleToPoint
 * @run 69:57CF
 * WHAT IT DOES: Gives the direction from an object to a point.
 * HOW: Subtracts the object's position from the point, then AngleFromDelta.
 * WAITS: no.
 * Entry: X object, zTemp08-09 point X, zTemp0A-0B point Y. Exit: as
 * AngleFromDelta. Clobbers zTemp00-03.
 */
AngleResult AngleToPoint(uint8_t object)
{
    uint16_t dx = (uint16_t)(trig_word(zTemp08, zTemp09) - trig_object_x(object));
    uint16_t dy = (uint16_t)(trig_word(zTemp0A, zTemp0B) - trig_object_y(object));

    return angle_of_vector(dx, dy);
}

void abi_AngleToPoint(void)
{
    abi_angle(AngleToPoint(cpu.x));
}

/*
 * @routine 0045:7EF AngleToPlayerOffset
 * @run 69:57EF
 * WHAT IT DOES: Gives the direction from an object to a point offset from the
 * player, object 0.
 * HOW: Subtracts the object's position from the player's, adds the offset, then
 * AngleFromDelta.
 * WAITS: no.
 * Entry: X object, zTemp08-09 offset X, zTemp0A-0B offset Y. Exit: as
 * AngleFromDelta. Clobbers zTemp00-03.
 */
AngleResult AngleToPlayerOffset(uint8_t object)
{
    uint16_t dx = (uint16_t)(trig_object_x(0) - trig_object_x(object) + trig_word(zTemp08, zTemp09));
    uint16_t dy = (uint16_t)(trig_object_y(0) - trig_object_y(object) + trig_word(zTemp0A, zTemp0B));

    return angle_of_vector(dx, dy);
}

void abi_AngleToPlayerOffset(void)
{
    abi_angle(AngleToPlayerOffset(cpu.x));
}

/*
 * @routine 0046:02D AngleToPlayer
 * @run 69:582D
 * WHAT IT DOES: Gives the direction from an object to the player, object 0.
 * HOW: Subtracts the object's position from the player's, then AngleFromDelta.
 * WAITS: no.
 * Entry: X object. Exit: as AngleFromDelta. Clobbers zTemp00-03.
 */
AngleResult AngleToPlayer(uint8_t object)
{
    uint16_t dx = (uint16_t)(trig_object_x(0) - trig_object_x(object));
    uint16_t dy = (uint16_t)(trig_object_y(0) - trig_object_y(object));

    return angle_of_vector(dx, dy);
}

void abi_AngleToPlayer(void)
{
    abi_angle(AngleToPlayer(cpu.x));
}

/* The short way round is downward when the target is less than a half turn below. */
static bool turn_goes_down(uint8_t now, uint8_t target)
{
    return (uint8_t)(now - target) < HALF_TURN;
}

/*
 * @routine 0046:2D2 TurnAngleToward
 * @run 69:5AD2
 * WHAT IT DOES: Turns an angle one step toward a target, the short way round.
 * HOW: Equal angles stay. Otherwise compares them in the half of the circle
 * the current angle lies in, a half turn deciding which way is shorter, and
 * adds or subtracts the step. A step can overshoot the target: nothing here
 * clamps it.
 * WAITS: no.
 * Entry: zTemp00 step, zTemp01 current, zTemp02 target. Exit: A the new angle,
 * N and Z from it, C and V from the step; on equal angles A current, N 0, Z 1,
 * C 1, V kept. X and Y kept.
 */
uint8_t TurnAngleToward(void)
{
    uint8_t step = rd8(zTemp00);
    uint8_t now = rd8(zTemp01);
    uint8_t target = rd8(zTemp02);

    if (now == target)
        return now;
    return (uint8_t)(turn_goes_down(now, target) ? now - step : now + step);
}

static Alu turn_exit_shim(uint8_t step, uint8_t now, uint8_t target)
{
    Alu exit = alu_none();

    if (now == target) {
        exit.a = now;
        exit.nz = 0;
        exit.c = true;
        exit.c_kept = false;
        return exit;
    }
    return turn_goes_down(now, target) ? sbc8(now, step, true) : adc8(now, step, false);
}

void abi_TurnAngleToward(void)
{
    uint8_t step = rd8(zTemp00);
    uint8_t now = rd8(zTemp01);
    uint8_t target = rd8(zTemp02);
    Alu exit = turn_exit_shim(step, now, target);

    exit.a = TurnAngleToward();
    abi_alu(exit);
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
