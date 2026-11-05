/* Angles, sines, the multiply under them, and the game's random numbers. */
#include "machine.h"
#include "ram.h"

/* Two's complement of a word in RAM, low byte then high. */
static Alu negate16(uint16_t lo, uint16_t hi)
{
    Alu r = adc8((uint8_t)~rd8(lo), 1, false);

    wr8(lo, r.a);
    r = adc8((uint8_t)~rd8(hi), 0, r.c);
    wr8(hi, r.a);
    return r;
}

/* Keeps the V an earlier step set when the later one set none. */
static Alu keep_v(Alu later, Alu earlier)
{
    if (!later.v_set && earlier.v_set) {
        later.v = earlier.v;
        later.v_set = true;
    }
    return later;
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
Alu MultiplyBySineWord(void)
{
    Alu r = { 0 };
    uint8_t a;
    bool out;

    wr8(zProduct0, 0);
    wr8(zProduct1, 0);
    wr8(zProduct2, 0);
    a = rd8(zTemp00);
    wr8(zTemp10, a);
    a = rd8(zTemp01);
    wr8(zTemp11, a);
    wr8(zTemp12, 0);
    r.a = a;
    r.c = false;
    r.v = false;
    r.v_set = false;
    for (int round = 0; round < 9; round++) {
        out = lsr_mem(zMultiplierHi);
        out = ror_mem(zMultiplierLo, out);
        if (out) {
            uint8_t sum = rd8(zProduct0);
            uint8_t add = rd8(zTemp10);

            r = adc8(sum, add, false);
            wr8(zProduct0, r.a);
            sum = rd8(zProduct1);
            add = rd8(zTemp11);
            r = adc8(sum, add, r.c);
            wr8(zProduct1, r.a);
            sum = rd8(zProduct2);
            add = rd8(zTemp12);
            r = adc8(sum, add, r.c);
            wr8(zProduct2, r.a);
        }
        out = asl_mem(zTemp10);
        out = rol_mem(zTemp11, out);
        r.c = rol_mem(zTemp12, out);
    }
    r.nz = 0;
    return r;
}

void abi_MultiplyBySineWord(void)
{
    abi_alu(MultiplyBySineWord());
    cpu.y = 0;
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
Alu ScaleBySine(uint8_t angle)
{
    uint8_t index = (uint8_t)(angle << 1);
    uint8_t word;
    Alu r = { 0 };

    word = rd8((uint16_t)(SineTable + index));
    wr8(zMultiplierLo, word);
    word = rd8((uint16_t)(SineTable + 1 + index));
    wr8(zMultiplierHi, word);
    r = MultiplyBySineWord();
    if (angle & 0x80) {
        r = adc8((uint8_t)~rd8(zProduct0), 1, false);
        wr8(zProduct0, r.a);
        r = adc8((uint8_t)~rd8(zProduct1), 0, r.c);
        wr8(zProduct1, r.a);
        r = adc8((uint8_t)~rd8(zProduct2), 0, r.c);
        wr8(zProduct2, r.a);
    }
    return r;
}

void abi_ScaleBySine(void)
{
    abi_alu(ScaleBySine(cpu.a));
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
Alu ScaleByCosine(uint8_t angle)
{
    Alu turned = adc8(angle, 0x40, false);

    return keep_v(ScaleBySine(turned.a), turned);
}

void abi_ScaleByCosine(void)
{
    abi_alu(ScaleByCosine(cpu.a));
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
Alu SetVelocityFromAngle(uint8_t angle, uint8_t object)
{
    Alu first = ScaleByCosine(angle);
    Alu r = { 0 };
    uint8_t byte;

    byte = rd8(zProduct2);
    wr8((uint16_t)(OBJ_VX_HI + object), byte);
    byte = rd8(zProduct1);
    wr8((uint16_t)(OBJ_VX_LO + object), byte);
    r = keep_v(ScaleBySine(angle), first);
    byte = rd8(zProduct2);
    wr8((uint16_t)(OBJ_VY_HI + object), byte);
    byte = rd8(zProduct1);
    wr8((uint16_t)(OBJ_VY_LO + object), byte);
    r.a = byte;
    r.nz = byte;
    return r;
}

void abi_SetVelocityFromAngle(void)
{
    abi_alu(SetVelocityFromAngle(cpu.a, cpu.x));
    cpu.y = 0;
}

/*
 * The first quadrant: the angle of a positive delta, read from AtanTable by
 * the top four significant bits of each axis. Returns the table index too.
 */
static Alu first_quadrant_angle(uint8_t *index)
{
    uint8_t high = rd8(zTemp01);
    uint8_t low;
    uint8_t other;
    uint8_t y;
    Alu r = { 0 };

    other = rd8(zTemp03);
    high |= other;
    while (high != 0) {
        bool out = lsr_mem(zTemp01);

        ror_mem(zTemp00, out);
        out = lsr_mem(zTemp03);
        ror_mem(zTemp02, out);
        high >>= 1;
    }
    low = rd8(zTemp00);
    other = rd8(zTemp02);
    low |= other;
    while (low & 0xF0) {
        lsr_mem(zTemp00);
        lsr_mem(zTemp02);
        low >>= 1;
    }
    y = rd8(zTemp02);
    r.c = (y & 0x10) != 0;
    y = (uint8_t)(y << 4);
    other = rd8(zTemp00);
    y |= other;
    *index = y;
    r.a = rd8((uint16_t)(AtanTable + y));
    r.nz = r.a;
    r.v = true;    /* BIT #$F0 sets it, on every path */
    r.v_set = true;
    return r;
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
Alu AngleFromDelta(uint8_t *index)
{
    uint8_t y_high = rd8(zTemp03);
    uint8_t x_high = rd8(zTemp01);
    Alu r = { 0 };

    if (!(y_high & 0x80)) {
        if (!(x_high & 0x80))
            return first_quadrant_angle(index);
        negate16(zTemp00, zTemp01);
        r = first_quadrant_angle(index);
        r = sbc8(r.a, 0x80, true);
        r.a = (uint8_t)(~r.a + 2);
        r.nz = r.a;
        return r;
    }
    if (x_high & 0x80) {
        negate16(zTemp00, zTemp01);
        negate16(zTemp02, zTemp03);
        r = first_quadrant_angle(index);
        return adc8(r.a, 0x80, false);
    }
    negate16(zTemp02, zTemp03);
    r = first_quadrant_angle(index);
    r.a = (uint8_t)(~r.a + 2);
    r.nz = r.a;
    return r;
}

void abi_AngleFromDelta(void)
{
    uint8_t index = cpu.y;

    abi_alu(AngleFromDelta(&index));
    cpu.y = index;
}

/* One axis of a vector: the word at to_lo and to_hi less an object's. */
static void store_axis_delta(uint16_t to_lo, uint16_t to_hi, uint16_t field_lo,
                             uint16_t field_hi, uint16_t out_lo, uint16_t out_hi)
{
    uint8_t minuend = rd8(to_lo);
    uint8_t subtrahend = rd8(field_lo);
    Alu r = sbc8(minuend, subtrahend, true);

    wr8(out_lo, r.a);
    minuend = rd8(to_hi);
    subtrahend = rd8(field_hi);
    r = sbc8(minuend, subtrahend, r.c);
    wr8(out_hi, r.a);
}

/*
 * @routine 0045:7CF AngleToPoint
 * @run 69:57CF
 * WHAT IT DOES: Gives the direction from an object to a point.
 * HOW: Subtracts the object's position from the point, then AngleFromDelta.
 * Entry: X object, zTemp08-09 point X, zTemp0A-0B point Y. Exit: as
 * AngleFromDelta. Clobbers zTemp00-03.
 */
Alu AngleToPoint(uint8_t object, uint8_t *index)
{
    store_axis_delta(zTemp08, zTemp09, (uint16_t)(OBJ_X_LO + object),
                     (uint16_t)(OBJ_X_HI + object), zTemp00, zTemp01);
    store_axis_delta(zTemp0A, zTemp0B, (uint16_t)(OBJ_Y_LO + object),
                     (uint16_t)(OBJ_Y_HI + object), zTemp02, zTemp03);
    return AngleFromDelta(index);
}

void abi_AngleToPoint(void)
{
    uint8_t index = cpu.y;

    abi_alu(AngleToPoint(cpu.x, &index));
    cpu.y = index;
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
Alu AngleToPlayerOffset(uint8_t object, uint8_t *index)
{
    uint8_t a;
    uint8_t b;
    Alu r = { 0 };

    store_axis_delta(OBJ_X_LO, OBJ_X_HI, (uint16_t)(OBJ_X_LO + object),
                     (uint16_t)(OBJ_X_HI + object), zTemp00, zTemp01);
    store_axis_delta(OBJ_Y_LO, OBJ_Y_HI, (uint16_t)(OBJ_Y_LO + object),
                     (uint16_t)(OBJ_Y_HI + object), zTemp02, zTemp03);
    a = rd8(zTemp00);
    b = rd8(zTemp08);
    r = adc8(a, b, false);
    wr8(zTemp00, r.a);
    a = rd8(zTemp01);
    b = rd8(zTemp09);
    r = adc8(a, b, r.c);
    wr8(zTemp01, r.a);
    a = rd8(zTemp02);
    b = rd8(zTemp0A);
    r = adc8(a, b, false);
    wr8(zTemp02, r.a);
    a = rd8(zTemp03);
    b = rd8(zTemp0B);
    r = adc8(a, b, r.c);
    wr8(zTemp03, r.a);
    return AngleFromDelta(index);
}

void abi_AngleToPlayerOffset(void)
{
    uint8_t index = cpu.y;

    abi_alu(AngleToPlayerOffset(cpu.x, &index));
    cpu.y = index;
}

/*
 * @routine 0046:02D AngleToPlayer
 * @run 69:582D
 * WHAT IT DOES: Gives the direction from an object to the player, object 0.
 * HOW: Subtracts the object's position from the player's, then AngleFromDelta.
 * Entry: X object. Exit: as AngleFromDelta. Clobbers zTemp00-03.
 */
Alu AngleToPlayer(uint8_t object, uint8_t *index)
{
    store_axis_delta(OBJ_X_LO, OBJ_X_HI, (uint16_t)(OBJ_X_LO + object),
                     (uint16_t)(OBJ_X_HI + object), zTemp00, zTemp01);
    store_axis_delta(OBJ_Y_LO, OBJ_Y_HI, (uint16_t)(OBJ_Y_LO + object),
                     (uint16_t)(OBJ_Y_HI + object), zTemp02, zTemp03);
    return AngleFromDelta(index);
}

void abi_AngleToPlayer(void)
{
    uint8_t index = cpu.y;

    abi_alu(AngleToPlayer(cpu.x, &index));
    cpu.y = index;
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
Alu TurnAngleToward(void)
{
    uint8_t now = rd8(zTemp01);
    uint8_t target = rd8(zTemp02);
    bool subtract;
    uint8_t step;
    Alu r = { 0 };

    if (now == target) {
        r.a = now;
        r.nz = 0;
        r.c = true;
        r.v = false;
        r.v_set = false;
        return r;
    }
    now = rd8(zTemp01);
    if (!(now & 0x80)) {
        target = rd8(zTemp02);
        if (now >= target) {
            subtract = true;
        } else {
            target = rd8(zTemp02);
            subtract = (uint8_t)(now + 0x80) < target;
        }
    } else {
        target = rd8(zTemp02);
        if (now < target) {
            subtract = false;
        } else {
            target = rd8(zTemp02);
            subtract = (uint8_t)(now - 0x80) < target;
        }
    }
    now = rd8(zTemp01);
    step = rd8(zTemp00);
    return subtract ? sbc8(now, step, true) : adc8(now, step, false);
}

void abi_TurnAngleToward(void)
{
    abi_alu(TurnAngleToward());
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
Alu NextRandom(void)
{
    uint8_t a;
    uint8_t b;
    Alu r = { 0 };

    a = rd8(zRandom3);
    b = rd8(zRandom2);
    r = adc8(a, b, false);
    wr8(zRandom3, r.a);
    a = rd8(zRandom2);
    b = rd8(zRandom1);
    r = adc8(a, b, false);
    wr8(zRandom2, r.a);
    a = rd8(zRandom3);
    b = rd8(zRandom1);
    r = adc8(a, b, r.c);
    wr8(zRandom3, r.a);
    a = rd8(zRandom1);
    b = rd8(zRandom0);
    r = adc8(a, b, false);
    wr8(zRandom1, r.a);
    a = rd8(zRandom2);
    b = rd8(zRandom0);
    r = adc8(a, b, r.c);
    wr8(zRandom2, r.a);
    a = rd8(zRandom3);
    b = rd8(zRandom0);
    r = adc8(a, b, r.c);
    wr8(zRandom3, r.a);
    a = (uint8_t)(rd8(zRandom0) + 1);
    wr8(zRandom0, a);
    if (a == 0) {
        a = (uint8_t)(rd8(zRandom1) + 1);
        wr8(zRandom1, a);
        if (a == 0) {
            a = (uint8_t)(rd8(zRandom2) + 1);
            wr8(zRandom2, a);
            if (a == 0) {
                a = (uint8_t)(rd8(zRandom3) + 1);
                wr8(zRandom3, a);
            }
        }
    }
    r.a = rd8(zRandom3);
    r.nz = r.a;
    return r;
}

void abi_NextRandom(void)
{
    abi_alu(NextRandom());
}
