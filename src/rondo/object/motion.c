/* Object movement: velocity added to position, velocity turned round. */
#include "machine.h"
#include "ram.h"

/* An 8.8 velocity, sign extended through zTemp10, added to a 16.8 position. */
static Alu apply_axis(uint16_t velocity_hi, uint16_t velocity_lo, uint16_t frac,
                      uint16_t low, uint16_t high)
{
    uint8_t a;
    uint8_t b;
    Alu r = { 0 };

    wr8(zTemp10, 0);
    a = rd8(velocity_hi);
    if (a & 0x80) {
        a = rd8(zTemp10);
        wr8(zTemp10, (uint8_t)(a - 1));
    }
    a = rd8(velocity_lo);
    b = rd8(frac);
    r = adc8(a, b, false);
    wr8(frac, r.a);
    a = rd8(velocity_hi);
    b = rd8(low);
    r = adc8(a, b, r.c);
    wr8(low, r.a);
    a = rd8(zTemp10);
    b = rd8(high);
    r = adc8(a, b, r.c);
    wr8(high, r.a);
    return r;
}

/*
 * @routine 0045:61B ApplyVelocityX
 * @run 69:561B
 * WHAT IT DOES: Moves an object along X by its X velocity.
 * HOW: See apply_axis. The bytes repeat the head of ApplyVelocity, as its own
 * entry that returns.
 * Entry: X object. Exit: A the position's high byte, N, Z, C and V from its add.
 * X and Y kept. Clobbers zTemp10.
 */
Alu ApplyVelocityX(uint8_t object)
{
    return apply_axis((uint16_t)(OBJ_VX_HI + object), (uint16_t)(OBJ_VX_LO + object),
                      (uint16_t)(OBJ_X_FRAC + object), (uint16_t)(OBJ_X_LO + object),
                      (uint16_t)(OBJ_X_HI + object));
}

void abi_ApplyVelocityX(void)
{
    abi_alu(ApplyVelocityX(cpu.x));
}

/*
 * @routine 0045:5F6 ApplyVelocityY
 * @run 69:55F6
 * WHAT IT DOES: Moves an object along Y by its Y velocity.
 * HOW: See apply_axis.
 * Entry: X object. Exit: as ApplyVelocityX. Clobbers zTemp10.
 */
Alu ApplyVelocityY(uint8_t object)
{
    return apply_axis((uint16_t)(OBJ_VY_HI + object), (uint16_t)(OBJ_VY_LO + object),
                      (uint16_t)(OBJ_Y_FRAC + object), (uint16_t)(OBJ_Y_LO + object),
                      (uint16_t)(OBJ_Y_HI + object));
}

void abi_ApplyVelocityY(void)
{
    abi_alu(ApplyVelocityY(cpu.x));
}

/*
 * @routine 0045:5D2 ApplyVelocity
 * @run 69:55D2
 * WHAT IT DOES: Moves an object by its velocity on both axes.
 * HOW: X, then runs into ApplyVelocityY.
 * Entry: X object. Exit: as ApplyVelocityY. Clobbers zTemp10.
 */
Alu ApplyVelocity(uint8_t object)
{
    ApplyVelocityX(object);
    return ApplyVelocityY(object);
}

void abi_ApplyVelocity(void)
{
    abi_alu(ApplyVelocity(cpu.x));
}

static Alu negate_velocity(uint16_t lo, uint16_t hi)
{
    Alu r = adc8((uint8_t)~rd8(lo), 1, false);

    wr8(lo, r.a);
    r = adc8((uint8_t)~rd8(hi), 0, r.c);
    wr8(hi, r.a);
    return r;
}

/*
 * @routine 0046:32D NegateVelocityX
 * @run 69:5B2D
 * WHAT IT DOES: Reverses an object's X velocity.
 * HOW: Two's complement of the word, low byte first.
 * Entry: X object. Exit: A the high byte, N, Z, C and V from its add. X and Y kept.
 */
Alu NegateVelocityX(uint8_t object)
{
    return negate_velocity((uint16_t)(OBJ_VX_LO + object), (uint16_t)(OBJ_VX_HI + object));
}

void abi_NegateVelocityX(void)
{
    abi_alu(NegateVelocityX(cpu.x));
}

/*
 * @routine 0046:343 NegateVelocityY
 * @run 69:5B43
 * WHAT IT DOES: Reverses an object's Y velocity.
 * HOW: Two's complement of the word, low byte first.
 * Entry: X object. Exit: as NegateVelocityX.
 */
Alu NegateVelocityY(uint8_t object)
{
    return negate_velocity((uint16_t)(OBJ_VY_LO + object), (uint16_t)(OBJ_VY_HI + object));
}

void abi_NegateVelocityY(void)
{
    abi_alu(NegateVelocityY(cpu.x));
}
