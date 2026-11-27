/* Hitboxes: where each of the first sixteen objects' hitbox centres lie. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "stage_map.h"

enum { HITBOX_SLOTS = 0x10 };

/*
 * @routine 024F:413 ComputeHitboxPositions
 * @run 6B:9413
 * WHAT IT DOES: Works out where the hitbox centre of each of the first sixteen objects lies:
 * its position plus the hitbox's offset, the X offset turned round when the object faces
 * left. An object without a hitbox is passed over and its entry left as it was.
 * HOW: Per object, the X offset (negated when facing left) is sign extended and added to X
 * low and high, to HITBOX x_lo and, in the high byte, hi; then the Y offset the same way, to
 * HITBOX y_lo, its sign extended high byte added to Y high and ORed into hi.
 * WAITS: no.
 * Entry: nothing. Exit: A the last value the loop had in A, X $10, N 0, Z 1, C 1. Y kept.
 */
/* The loop; v_seen and v are the last add's overflow, which the original leaves in V. */
static uint8_t hitbox_loop(bool *v_seen, bool *v)
{
    uint8_t a = 0;
    uint8_t slot;

    for (slot = 0; slot < HITBOX_SLOTS; slot++) {
        uint8_t offset;
        uint8_t sign;
        Sum sum;
        uint8_t byte;

        a = RD8(OBJ->hitbox_on[slot]);
        if (a == 0) {
            continue;
        }
        a = RD8(OBJ->hitbox_dx[slot]);
        WR8(ZP->temp01, a);
        byte = RD8(OBJ->facing_left[slot]);
        if (byte != 0) {
            a = RD8(ZP->temp01);
            a = (uint8_t)(~a + 1);
            WR8(ZP->temp01, a);
        }
        WR8(ZP->temp00, 0);
        offset = RD8(ZP->temp01);
        if (offset & 0x80) {
            DEC8(ZP->temp00);
        }
        byte = RD8(OBJ->x_lo[slot]);
        sum = add_bytes(offset, byte, false);
        WR8(HITBOX->x_lo[slot], sum.value);
        byte = RD8(OBJ->x_hi[slot]);
        sign = RD8(ZP->temp00);
        sum = add_bytes(byte, sign, sum.carry);
        WR8(HITBOX->hi[slot], sum.value);

        WR8(ZP->temp00, 0);
        offset = RD8(OBJ->hitbox_dy[slot]);
        if (offset & 0x80) {
            DEC8(ZP->temp00);
        }
        byte = RD8(OBJ->y_lo[slot]);
        offset = RD8(OBJ->hitbox_dy[slot]);
        sum = add_bytes(byte, offset, false);
        WR8(HITBOX->y_lo[slot], sum.value);
        byte = RD8(OBJ->y_hi[slot]);
        sign = RD8(ZP->temp00);
        sum = add_bytes(byte, sign, sum.carry);
        byte = RD8(HITBOX->hi[slot]);
        *v_seen = true;
        *v = sum.overflow;
        a = (uint8_t)(sum.value | byte);
        WR8(HITBOX->hi[slot], a);
    }
    return a;
}

HitboxEnd ComputeHitboxPositions(void)
{
    HitboxEnd end;

    end.v_seen = false;
    end.v = false;
    end.a = hitbox_loop(&end.v_seen, &end.v);
    return end;
}

static void hitbox_positions_exit_shim(HitboxEnd end)
{
    cpu.a = end.a;
    if (end.v_seen) {
        abi_flag(P_V, end.v);
    }
    cpu.x = HITBOX_SLOTS;
    abi_flag(P_N, false);
    abi_flag(P_Z, true);
    abi_flag(P_C, true);
}

void abi_ComputeHitboxPositions(void)
{
    hitbox_positions_exit_shim(ComputeHitboxPositions());
}

/*
 * @routine 05BF:725 ComputeHitboxPositionsLong
 * @run 6B:9725
 * WHAT IT DOES: ComputeHitboxPositions in 05BF's group of stage overlays: the hitbox centres of the first sixteen objects.
 * HOW: The same instructions as ComputeHitboxPositions.
 * WAITS: no.
 * Entry: nothing. Exit: as ComputeHitboxPositions.
 */
HitboxEnd ComputeHitboxPositionsLong(void)
{
    HitboxEnd end;

    end.v_seen = false;
    end.v = false;
    end.a = hitbox_loop(&end.v_seen, &end.v);
    return end;
}

void abi_ComputeHitboxPositionsLong(void)
{
    hitbox_positions_exit_shim(ComputeHitboxPositionsLong());
}
