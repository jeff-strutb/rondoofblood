/* Object movement: velocity added to position, velocity turned round. */
#include "machine.h"
#include "ram.h"
#include "routines.h"

/* One object's fields on one axis, as addresses. */
typedef struct {
    uint16_t velocity_lo, velocity_hi, frac, low, high;
} AxisFields;

static AxisFields x_fields(uint8_t object)
{
    AxisFields f = { (uint16_t)(OBJ_VX_LO + object), (uint16_t)(OBJ_VX_HI + object),
                     (uint16_t)(OBJ_X_FRAC + object), (uint16_t)(OBJ_X_LO + object),
                     (uint16_t)(OBJ_X_HI + object) };

    return f;
}

static AxisFields y_fields(uint8_t object)
{
    AxisFields f = { (uint16_t)(OBJ_VY_LO + object), (uint16_t)(OBJ_VY_HI + object),
                     (uint16_t)(OBJ_Y_FRAC + object), (uint16_t)(OBJ_Y_LO + object),
                     (uint16_t)(OBJ_Y_HI + object) };

    return f;
}

/* An 8.8 velocity added to a 16.8 position. zTemp10 is left as the velocity's sign. */
static AxisMove apply_axis(AxisFields f)
{
    AxisMove move;
    uint32_t after;

    move.velocity = (int16_t)(rd8(f.velocity_lo) | (rd8(f.velocity_hi) << 8));
    move.before = rd8(f.frac) | ((uint32_t)rd8(f.low) << 8) | ((uint32_t)rd8(f.high) << 16);
    after = move.before + (uint32_t)(int32_t)move.velocity;
    wr8(zTemp10, move.velocity < 0 ? 0xFF : 0x00);
    wr8(f.frac, (uint8_t)after);
    wr8(f.low, (uint8_t)(after >> 8));
    wr8(f.high, (uint8_t)(after >> 16));
    return move;
}

/* What the assembly's last add leaves in C and V, and in A: the position's high byte. */
uint8_t axis_move_flags_shim(AxisMove move)
{
    uint32_t added = (uint32_t)(int32_t)move.velocity & 0xFFFFFF;
    uint32_t sum = move.before + added;
    uint8_t sign = (uint8_t)(added >> 16);
    uint8_t high = (uint8_t)(move.before >> 16);
    uint8_t result = (uint8_t)(sum >> 16);

    abi_flag(P_C, sum > 0xFFFFFF);
    abi_flag(P_V, (~(sign ^ high) & (sign ^ result) & 0x80) != 0);
    return result;
}

static void axis_move_exit_shim(AxisMove move)
{
    cpu.a = axis_move_flags_shim(move);
    abi_nz(cpu.a);
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
AxisMove ApplyVelocityX(uint8_t object)
{
    return apply_axis(x_fields(object));
}

void abi_ApplyVelocityX(void)
{
    axis_move_exit_shim(ApplyVelocityX(cpu.x));
}

/*
 * @routine 0045:5F6 ApplyVelocityY
 * @run 69:55F6
 * WHAT IT DOES: Moves an object along Y by its Y velocity.
 * HOW: See apply_axis.
 * Entry: X object. Exit: as ApplyVelocityX. Clobbers zTemp10.
 */
AxisMove ApplyVelocityY(uint8_t object)
{
    return apply_axis(y_fields(object));
}

void abi_ApplyVelocityY(void)
{
    axis_move_exit_shim(ApplyVelocityY(cpu.x));
}

/*
 * @routine 0045:5D2 ApplyVelocity
 * @run 69:55D2
 * WHAT IT DOES: Moves an object by its velocity on both axes.
 * HOW: X, then runs into ApplyVelocityY.
 * Entry: X object. Exit: as ApplyVelocityY. Clobbers zTemp10.
 */
AxisMove ApplyVelocity(uint8_t object)
{
    ApplyVelocityX(object);
    return ApplyVelocityY(object);
}

void abi_ApplyVelocity(void)
{
    axis_move_exit_shim(ApplyVelocity(cpu.x));
}

/* A velocity word replaced by its negative, which is returned. */
static uint16_t negate_velocity(uint16_t lo, uint16_t hi)
{
    uint16_t word = (uint16_t)(rd8(lo) | (rd8(hi) << 8));
    uint16_t negated = (uint16_t)(0u - word);

    wr8(lo, (uint8_t)negated);
    wr8(hi, (uint8_t)(negated >> 8));
    return negated;
}

/*
 * The high byte's add leaves A and the flags. Its carry comes only from zero, and it
 * overflows only on $8000: the two words that are their own negatives.
 */
static void negate_exit_shim(uint16_t negated)
{
    cpu.a = (uint8_t)(negated >> 8);
    abi_nz(cpu.a);
    abi_flag(P_C, negated == 0);
    abi_flag(P_V, negated == 0x8000);
}

/*
 * @routine 0046:32D NegateVelocityX
 * @run 69:5B2D
 * WHAT IT DOES: Reverses an object's X velocity.
 * HOW: Two's complement of the word, low byte first.
 * Entry: X object. Exit: A the high byte, N, Z, C and V from its add. X and Y kept.
 */
uint16_t NegateVelocityX(uint8_t object)
{
    return negate_velocity((uint16_t)(OBJ_VX_LO + object), (uint16_t)(OBJ_VX_HI + object));
}

void abi_NegateVelocityX(void)
{
    negate_exit_shim(NegateVelocityX(cpu.x));
}

/*
 * @routine 0046:343 NegateVelocityY
 * @run 69:5B43
 * WHAT IT DOES: Reverses an object's Y velocity.
 * HOW: Two's complement of the word, low byte first.
 * Entry: X object. Exit: as NegateVelocityX.
 */
uint16_t NegateVelocityY(uint8_t object)
{
    return negate_velocity((uint16_t)(OBJ_VY_LO + object), (uint16_t)(OBJ_VY_HI + object));
}

void abi_NegateVelocityY(void)
{
    negate_exit_shim(NegateVelocityY(cpu.x));
}
