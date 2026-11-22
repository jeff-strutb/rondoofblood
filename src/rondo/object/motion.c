/* Object movement: velocity added to position, velocity turned round. */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"

/* One axis's fields in the object table: the column of each, kept as a base so one body serves X and Y. */
typedef struct {
    uint16_t velocity_lo, velocity_hi, frac, low, high;
} AxisFields;

static AxisFields x_fields(void)
{
    AxisFields f = { COL(OBJ->vx_lo), COL(OBJ->vx_hi), COL(OBJ->x_frac), COL(OBJ->x_lo), COL(OBJ->x_hi) };

    return f;
}

static AxisFields y_fields(void)
{
    AxisFields f = { COL(OBJ->vy_lo), COL(OBJ->vy_hi), COL(OBJ->y_frac), COL(OBJ->y_lo), COL(OBJ->y_hi) };

    return f;
}

/*
 * An 8.8 velocity added to a 16.8 position, access for access as the assembly makes them: the
 * sign byte is stored first, the velocity's high byte is read twice (once for the sign, once
 * for the add), and each position byte is read, added and stored before the next. zTemp10 is
 * left as the velocity's sign.
 */
static AxisMove apply_axis(AxisFields f, uint8_t object)
{
    AxisMove move;
    uint8_t high_for_sign, low, high, frac, mid, top, sign;
    unsigned sum;
    uint8_t carry;

    WR8(ZP->temp10, 0);
    high_for_sign = RD8_IDX(f.velocity_hi, object);
    if (high_for_sign & 0x80) {
        DEC8(ZP->temp10);
    }
    low = RD8_IDX(f.velocity_lo, object);
    frac = RD8_IDX(f.frac, object);
    sum = (unsigned)low + frac;
    carry = sum > 0xFF;
    WR8_IDX(f.frac, object, (uint8_t)sum);
    high = RD8_IDX(f.velocity_hi, object);
    mid = RD8_IDX(f.low, object);
    sum = (unsigned)high + mid + carry;
    carry = sum > 0xFF;
    WR8_IDX(f.low, object, (uint8_t)sum);
    sign = RD8(ZP->temp10);
    top = RD8_IDX(f.high, object);
    sum = (unsigned)sign + top + carry;
    WR8_IDX(f.high, object, (uint8_t)sum);
    move.velocity = (int16_t)(low | (high << 8));
    move.before = frac | ((uint32_t)mid << 8) | ((uint32_t)top << 16);
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
 * WAITS: no.
 * Entry: X object. Exit: A the position's high byte, N, Z, C and V from its add.
 * X and Y kept. Clobbers zTemp10.
 */
AxisMove ApplyVelocityX(uint8_t object)
{
    return apply_axis(x_fields(), object);
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
 * WAITS: no.
 * Entry: X object. Exit: as ApplyVelocityX. Clobbers zTemp10.
 */
AxisMove ApplyVelocityY(uint8_t object)
{
    return apply_axis(y_fields(), object);
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
 * WAITS: no.
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

/* A velocity word replaced by its negative: the low byte's add first, its carry into the high byte's. */
static Sum negate_velocity(uint16_t lo_column, uint16_t hi_column, uint8_t object)
{
    uint8_t value;
    Sum sum;

    value = RD8_IDX(lo_column, object);
    sum = add_bytes((uint8_t)(value ^ 0xFF), 0, true);
    WR8_IDX(lo_column, object, sum.value);
    value = RD8_IDX(hi_column, object);
    sum = add_bytes((uint8_t)(value ^ 0xFF), 0, sum.carry);
    WR8_IDX(hi_column, object, sum.value);
    return sum;
}

/* A the new high byte, N and Z from it, C and V from its add. */
void negate_exit_shim(Sum last)
{
    cpu.a = last.value;
    abi_nz(last.value);
    abi_flag(P_C, last.carry);
    abi_flag(P_V, last.overflow);
}

/*
 * @routine 0046:32D NegateVelocityX
 * @run 69:5B2D
 * WHAT IT DOES: Reverses an object's X velocity.
 * HOW: Two's complement of the word, low byte first, the carry carried into the high byte.
 * WAITS: no.
 * Entry: X object, D clear. Exit: A the high byte, N, Z, C and V from its add. X and Y kept.
 */
Sum NegateVelocityX(uint8_t object)
{
    return negate_velocity(COL(OBJ->vx_lo), COL(OBJ->vx_hi), object);
}

void abi_NegateVelocityX(void)
{
    negate_exit_shim(NegateVelocityX(cpu.x));
}

/*
 * @routine 0046:343 NegateVelocityY
 * @run 69:5B43
 * WHAT IT DOES: Reverses an object's Y velocity.
 * HOW: Two's complement of the word, low byte first, the carry carried into the high byte.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as NegateVelocityX.
 */
Sum NegateVelocityY(uint8_t object)
{
    return negate_velocity(COL(OBJ->vy_lo), COL(OBJ->vy_hi), object);
}

void abi_NegateVelocityY(void)
{
    negate_exit_shim(NegateVelocityY(cpu.x));
}
