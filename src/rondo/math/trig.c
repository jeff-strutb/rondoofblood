/*
 * Angles, sines, the multiply under them, and the game's random numbers.
 *
 * The bodies work in whole words and leave the original's results in the original's RAM.
 * What the assembly leaves in A and the flags is worked out from those values in the
 * *_exit_shim functions, for the abi_ wrappers, and nowhere else.
 */
#include "machine.h"
#include "ram.h"

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

static void put_product(uint32_t product)
{
    wr8(zProduct0, (uint8_t)product);
    wr8(zProduct1, (uint8_t)(product >> 8));
    wr8(zProduct2, (uint8_t)(product >> 16));
}

/*
 * What the assembly leaves after the multiply. The last add was the highest multiplier bit
 * set, so its operands, and V, follow from the value, the word and the product. C is the
 * top bit the last round shifted out of the copy, bit 15 of the value. N and Z are the
 * round counter's, at zero.
 */
static Alu multiply_exit_shim(uint16_t value, uint16_t word, uint32_t product)
{
    Alu exit = alu_none();
    unsigned bits = word & MULTIPLIER_BITS;

    exit.a = (uint8_t)(value >> 8);
    exit.nz = 0;
    exit.c = (value & 0x8000) != 0;
    exit.c_kept = false;
    if (bits != 0) {
        unsigned top = MULTIPLY_ROUNDS - 1;
        uint32_t added;
        uint32_t before;

        while (!(bits & (1u << top)))
            top--;
        added = (uint32_t)value << top;
        before = (product - added) & PRODUCT_MASK;
        exit.a = (uint8_t)(product >> 16);
        exit.v = (~(before ^ added) & (before ^ product) & PRODUCT_TOP) != 0;
        exit.v_set = true;
    }
    return exit;
}

/*
 * @routine 0045:69A MultiplyBySineWord
 * @run 69:569A
 * WHAT IT DOES: Multiplies the 16-bit value in zTemp00 by the 9-bit word in
 * zMultiplierLo, for a 24-bit product in zProduct0-2; the word is a sine, so
 * the product is the value scaled by that sine as 8.8.
 * HOW: Shift and add, nine rounds: the multiplier shifts right through RAM, a
 * 24-bit copy of the value in zTemp10-12 shifts left, and each bit shifted out
 * set adds the copy to the product.
 * Entry: zTemp00-01 value, zMultiplierLo-Hi word. Exit: A the last byte
 * loaded (the top of the product if any bit was set, else zTemp01), Y 0, N 0,
 * Z 1, C the top bit shifted out of zTemp12, V from the last add if any.
 * X kept. Clobbers zTemp10-12, zMultiplierLo-Hi.
 */
uint32_t MultiplyBySineWord(void)
{
    uint32_t shifted = trig_word(zTemp00, zTemp01);
    uint16_t multiplier = trig_word(zMultiplierLo, zMultiplierHi);
    uint32_t product = 0;

    for (int round = 0; round < MULTIPLY_ROUNDS; round++) {
        if (multiplier & 1)
            product += shifted;
        multiplier >>= 1;
        shifted <<= 1;
    }
    product &= PRODUCT_MASK;
    put_product(product);
    wr8(zTemp10, (uint8_t)shifted);
    wr8(zTemp11, (uint8_t)(shifted >> 8));
    wr8(zTemp12, (uint8_t)(shifted >> 16));
    trig_put_word(zMultiplierLo, zMultiplierHi, multiplier);
    return product;
}

void abi_MultiplyBySineWord(void)
{
    uint16_t value = trig_word(zTemp00, zTemp01);
    uint16_t word = trig_word(zMultiplierLo, zMultiplierHi);

    abi_alu(multiply_exit_shim(value, word, MultiplyBySineWord()));
    cpu.y = 0;
}

/* The half wave's word for an angle: the angle's sign is not part of the index. */
static uint16_t sine_word(uint8_t angle)
{
    uint8_t index = (uint8_t)(angle << 1);

    return trig_word((uint16_t)(SineTable + index), (uint16_t)(SineTable + 1 + index));
}

/*
 * @routine 0045:660 ScaleBySine
 * @run 69:5660
 * WHAT IT DOES: Scales the value in zTemp00 by the sine of an angle: 256 steps
 * to the turn, angle 64 the peak, angles from 128 up negative.
 * HOW: Doubles the angle for a word index: bit 7 goes out as the sign, the
 * rest indexes the half wave in SineTable. Multiplies, and for the negative
 * half takes the two's complement of the 24-bit product.
 * Entry: A angle, zTemp00-01 value. Exit: zProduct0-2 result; A, N, Z, C and V
 * from the multiply, or from the complement's last add when negative; Y 0.
 * X kept. Clobbers zTemp10-12, zMultiplierLo-Hi.
 */
uint32_t ScaleBySine(uint8_t angle)
{
    uint32_t product;

    trig_put_word(zMultiplierLo, zMultiplierHi, sine_word(angle));
    product = MultiplyBySineWord();
    if (angle & ANGLE_NEGATIVE) {
        product = (0u - product) & PRODUCT_MASK;
        put_product(product);
    }
    return product;
}

/* The complement's last add carries only out of zero, and overflows only on $800000. */
static Alu scale_exit_shim(uint8_t angle, uint16_t value, uint32_t result)
{
    bool negative = (angle & ANGLE_NEGATIVE) != 0;
    uint32_t magnitude = negative ? (0u - result) & PRODUCT_MASK : result;
    Alu exit = multiply_exit_shim(value, sine_word(angle), magnitude);

    if (negative) {
        exit.a = (uint8_t)(result >> 16);
        exit.nz = exit.a;
        exit.c = magnitude == 0;
        exit.v = magnitude == PRODUCT_TOP;
        exit.v_set = true;
    }
    return exit;
}

void abi_ScaleBySine(void)
{
    uint8_t angle = cpu.a;
    uint16_t value = trig_word(zTemp00, zTemp01);

    abi_alu(scale_exit_shim(angle, value, ScaleBySine(angle)));
    cpu.y = 0;
}

/*
 * @routine 0045:65D ScaleByCosine
 * @run 69:565D
 * WHAT IT DOES: Scales the value in zTemp00 by the cosine of an angle.
 * HOW: A quarter turn added to the angle, then ScaleBySine, which it runs into.
 * Entry: A angle, zTemp00-01 value. Exit: as ScaleBySine; V from the angle's
 * add when no later add set it.
 */
uint32_t ScaleByCosine(uint8_t angle)
{
    return ScaleBySine((uint8_t)(angle + QUARTER_TURN));
}

/* Adding $40 overflows only from $40-$7F. */
static Alu cosine_exit_shim(uint8_t angle, uint16_t value, uint32_t result)
{
    Alu exit = scale_exit_shim((uint8_t)(angle + QUARTER_TURN), value, result);

    if (!exit.v_set) {
        exit.v = (angle & 0xC0) == QUARTER_TURN;
        exit.v_set = true;
    }
    return exit;
}

void abi_ScaleByCosine(void)
{
    uint8_t angle = cpu.a;
    uint16_t value = trig_word(zTemp00, zTemp01);

    abi_alu(cosine_exit_shim(angle, value, ScaleByCosine(angle)));
    cpu.y = 0;
}

/*
 * @routine 0045:640 SetVelocityFromAngle
 * @run 69:5640
 * WHAT IT DOES: Points an object along an angle at a given speed: its X
 * velocity becomes speed times cosine, its Y velocity speed times sine.
 * HOW: ScaleByCosine and ScaleBySine in turn, taking the top two bytes of each
 * product as an 8.8 velocity.
 * Entry: A angle, X object, zTemp00-01 speed. Exit: A the Y velocity's low
 * byte, N and Z from it, C and V from the sine, Y 0. X kept. Clobbers zTemp10-12,
 * zProduct0-2, zMultiplierLo-Hi.
 */
ScaledVector SetVelocityFromAngle(uint8_t angle, uint8_t object)
{
    ScaledVector scaled;

    scaled.x = ScaleByCosine(angle);
    wr8((uint16_t)(OBJ_VX_HI + object), (uint8_t)(scaled.x >> 16));
    wr8((uint16_t)(OBJ_VX_LO + object), (uint8_t)(scaled.x >> 8));
    scaled.y = ScaleBySine(angle);
    wr8((uint16_t)(OBJ_VY_HI + object), (uint8_t)(scaled.y >> 16));
    wr8((uint16_t)(OBJ_VY_LO + object), (uint8_t)(scaled.y >> 8));
    return scaled;
}

/* The cosine's V stands when the sine set none. */
static Alu velocity_exit_shim(uint8_t angle, uint16_t speed, ScaledVector scaled)
{
    Alu exit = scale_exit_shim(angle, speed, scaled.y);

    if (!exit.v_set)
        exit.v = cosine_exit_shim(angle, speed, scaled.x).v;
    exit.v_set = true;
    exit.a = (uint8_t)(scaled.y >> 8);
    exit.nz = exit.a;
    return exit;
}

void abi_SetVelocityFromAngle(void)
{
    uint8_t angle = cpu.a;
    uint16_t speed = trig_word(zTemp00, zTemp01);

    abi_alu(velocity_exit_shim(angle, speed, SetVelocityFromAngle(angle, cpu.x)));
    cpu.y = 0;
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
 * HOW: A 32-bit counter generator: the state becomes state * $01010101 + 1,
 * each byte added into those above it, top byte first, then the 32-bit
 * increment. The top byte is the result.
 * Entry: nothing. Exit: A zRandom3, N and Z from it, C and V from the last add.
 * X and Y kept.
 */
uint8_t NextRandom(void)
{
    uint32_t state = (uint32_t)rd8(zRandom0) | ((uint32_t)rd8(zRandom1) << 8) |
                     ((uint32_t)rd8(zRandom2) << 16) | ((uint32_t)rd8(zRandom3) << 24);

    state = state * 0x01010101u + 1u;
    wr8(zRandom0, (uint8_t)state);
    wr8(zRandom1, (uint8_t)(state >> 8));
    wr8(zRandom2, (uint8_t)(state >> 16));
    wr8(zRandom3, (uint8_t)(state >> 24));
    return (uint8_t)(state >> 24);
}

/* C and V are the top byte's last add, as the assembly's order of adds leaves them. */
static Alu random_exit_shim(uint8_t state0, uint8_t state1, uint8_t state2, uint8_t state3,
                            uint8_t result)
{
    uint8_t top = (uint8_t)(state3 + state2);
    Alu middle = adc8(state2, state1, false);
    Alu low;
    Alu exit;

    top = adc8(top, state1, middle.c).a;
    low = adc8(state1, state0, false);
    middle = adc8(middle.a, state0, low.c);
    exit = adc8(top, state0, middle.c);
    exit.a = result;
    exit.nz = result;
    return exit;
}

void abi_NextRandom(void)
{
    uint8_t state0 = rd8(zRandom0);
    uint8_t state1 = rd8(zRandom1);
    uint8_t state2 = rd8(zRandom2);
    uint8_t state3 = rd8(zRandom3);

    abi_alu(random_exit_shim(state0, state1, state2, state3, NextRandom()));
}
