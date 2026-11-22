/*
 * The shared object pool's state helpers: what the stage overlays' object handlers call at the
 * head of a state, before they jump through the object's state table (the byte OBJ->state is the
 * state, an index into a table of handlers).
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"

enum { HIT_STATE = 5, HIT_LIMIT = 5, BLINK_LIMIT = 0x18, BLINK_FLAG = 0x86, BLINK_WAVE_MASK = 0x02 };

/*
 * @routine 0519:46A EnterHurtStateOnHit
 * @run 70:A46A
 * WHAT IT DOES: Sends an object that is in one of its first four states and has taken a hit
 * of 1 to 4 into state 5, the state its handler table uses for being hurt.
 * HOW: Reads the state and the hit byte, each once, and writes the state only when both are
 * from 1 to 4.
 * WAITS: no.
 * Entry: X object. Exit: A and flags as the last compare or load left them (see the wrapper);
 * X and Y kept.
 */
HurtCheck EnterHurtStateOnHit(uint8_t slot)
{
    HurtCheck result;
    uint8_t state, hit;

    state = RD8(OBJ->state[slot]);
    result.value = state;
    if (state == 0) {
        result.end = HURT_NOT_ACTIVE;
        return result;
    }
    if (state >= HIT_LIMIT) {
        result.end = HURT_STATE_PAST;
        return result;
    }
    hit = RD8(OBJ->hit_taken[slot]);
    result.value = hit;
    if (hit == 0) {
        result.end = HURT_NOT_HIT;
        return result;
    }
    if (hit >= HIT_LIMIT) {
        result.end = HURT_HIT_PAST;
        return result;
    }
    WR8(OBJ->state[slot], HIT_STATE);
    result.end = HURT_ENTERED;
    return result;
}

void abi_EnterHurtStateOnHit(void)
{
    HurtCheck result = EnterHurtStateOnHit(cpu.x);

    switch (result.end) {
    case HURT_NOT_ACTIVE:
        cpu.a = 0;
        abi_nz(0);
        break;
    case HURT_NOT_HIT:
        cpu.a = 0;
        abi_nz(0);
        abi_flag(P_C, false);  /* the state's compare against 5 left C clear */
        break;
    case HURT_STATE_PAST:
    case HURT_HIT_PAST:
        cpu.a = result.value;
        abi_nz((uint8_t)(result.value - HIT_LIMIT));
        abi_flag(P_C, true);
        break;
    case HURT_ENTERED:
        cpu.a = HIT_STATE;
        abi_nz(HIT_STATE);
        abi_flag(P_C, false);
        break;
    }
}

/*
 * @routine 0519:74E SetBlinkFlash
 * @run 70:A74E
 * WHAT IT DOES: Sets an object's flash flag (the palette flash its sprite shows) to $86 on
 * alternate beats of the wave counter once its timer has reached $18, and to 0 otherwise.
 * HOW: Clears the flag first, then reads the timer and, if it is $18 or more and bit 1 of the
 * wave counter is clear, writes $86.
 * WAITS: no.
 * Entry: X object. Exit: A and flags as the last compare, AND or load left them (see the
 * wrapper); X and Y kept.
 */
BlinkCheck SetBlinkFlash(uint8_t slot)
{
    BlinkCheck result;
    uint8_t timer, wave;

    WR8(OBJ->flash_flags[slot], 0);
    timer = RD8(OBJ->timer[slot]);
    result.value = timer;
    if (timer < BLINK_LIMIT) {
        result.end = BLINK_TIMER_LOW;
        return result;
    }
    wave = RD8(ZP->wave_counter);
    result.value = (uint8_t)(wave & BLINK_WAVE_MASK);
    if (result.value != 0) {
        result.end = BLINK_WAVE_BIT;
        return result;
    }
    WR8(OBJ->flash_flags[slot], BLINK_FLAG);
    result.end = BLINK_SET;
    return result;
}

void abi_SetBlinkFlash(void)
{
    BlinkCheck result = SetBlinkFlash(cpu.x);

    switch (result.end) {
    case BLINK_TIMER_LOW:
        cpu.a = result.value;
        abi_nz((uint8_t)(result.value - BLINK_LIMIT));
        abi_flag(P_C, false);
        break;
    case BLINK_WAVE_BIT:
        cpu.a = result.value;
        abi_nz(result.value);
        abi_flag(P_C, true);
        break;
    case BLINK_SET:
        cpu.a = BLINK_FLAG;
        abi_nz(BLINK_FLAG);
        abi_flag(P_C, true);
        break;
    }
}

/*
 * @routine 0519:482 InitObjectFromRecord
 * @run 70:A482
 * WHAT IT DOES: Starts a newly placed object: loads its record, copies slot 0's byte of the
 * OBJ_2FE8 column into the object's, sets two of its counters and its animation delay to 4, and
 * its timer to $60.
 * HOW: Calls LoadObjectRecord through its vector (which keeps X), then stores each byte.
 * WAITS: no.
 * Entry: X object. Exit: A $60, N and Z from it, C as LoadObjectRecord left it; Y 11; X kept. D clear.
 */
bool InitObjectFromRecord(uint8_t slot)
{
    uint8_t shared, nz;
    bool carry;

    jsr_push(0xA485);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    shared = RD8(OBJ->f2fe8[0]);
    WR8(OBJ->f2fe8[slot], shared);
    WR8(OBJ->hitbox_half_w[slot], 4);
    WR8(OBJ->hitbox_half_h[slot], 4);
    WR8(OBJ->f3510[slot], 4);
    WR8(OBJ->life_timer[slot], 0x60);
    return carry;
}

void abi_InitObjectFromRecord(void)
{
    bool carry = InitObjectFromRecord(cpu.x);

    cpu.a = 0x60;
    cpu.y = 11;
    abi_nz(0x60);
    abi_flag(P_C, carry);
}

/*
 * @routine 0519:4BD FallAndBounce
 * @run 70:A4BD
 * WHAT IT DOES: Moves an object by its velocity and asks the ground under it; on floor the
 * object enters its next state. Unless the ground is deadly, the object's X speed is set
 * from its angle (the full cosine) and its Y speed to $80, and its angle is turned on by
 * $0280.
 * HOW: Calls ApplyVelocity and ProbeTerrain (floor, height 0), then ScaleByCosine with a
 * multiplier of 256; the angle is the two bytes OBJ_35C0 (low) and OBJ_3568 (high).
 * WAITS: no.
 * Entry: X object. Exit: A and flags from the last AND or add (see the wrapper); X kept; Y as
 * ScaleByCosine left it. D clear.
 */
FallResult FallAndBounce(uint8_t slot)
{
    FallResult result = { FALL_DEADLY_GROUND, 0, 0, false, false };
    uint8_t answer, deadly, angle_low, angle_high, product_hi, product_mid;
    Sum sum;

    cpu.x = slot;
    if (call_frame(0xA4C0, abi_VecApplyVelocity)) {
        return result;
    }
    WR8(ZP->product.mid, 0);
    cpu.a = 1;
    if (call_frame(0xA4C9, abi_VecProbeTerrain)) {
        return result;
    }
    answer = (uint8_t)(cpu.a & 0x01);
    if (answer != 0) {
        INC8(OBJ->state[slot]);
    }
    deadly = (uint8_t)(RD8(ZP->product.lo) & 0x10);
    result.probe_bit4 = deadly;
    if (deadly != 0) {
        return result;
    }
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 1);
    cpu.a = RD8(OBJ->angle_hi[slot]);
    if (call_frame(0xA4E4, abi_VecScaleByCosine)) {
        return result;
    }
    product_hi = RD8(ZP->product.hi);
    WR8(OBJ->vx_hi[slot], product_hi);
    product_mid = RD8(ZP->product.mid);
    WR8(OBJ->vx_lo[slot], product_mid);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0x80);
    angle_low = RD8(OBJ->angle_lo[slot]);
    sum = add_bytes(angle_low, 0x80, false);
    WR8(OBJ->angle_lo[slot], sum.value);
    angle_high = RD8(OBJ->angle_hi[slot]);
    sum = add_bytes(angle_high, 0x02, sum.carry);
    WR8(OBJ->angle_hi[slot], sum.value);
    result.end = FALL_BOUNCED;
    result.angle_high = sum.value;
    result.carry = sum.carry;
    result.overflow = sum.overflow;
    return result;
}

void abi_FallAndBounce(void)
{
    FallResult result = FallAndBounce(cpu.x);

    if (result.end == FALL_DEADLY_GROUND) {
        cpu.a = result.probe_bit4;
        abi_nz(result.probe_bit4);
    } else {
        cpu.a = result.angle_high;
        abi_nz(result.angle_high);
        abi_flag(P_C, result.carry);
        abi_flag(P_V, result.overflow);
    }
}

/*
 * @routine 051A:055 BitMaskFromIndex
 * @run 70:A855
 * WHAT IT DOES: Gives the byte with only bit Y set (1 shifted left Y times; 0 if Y is 8 or more).
 * HOW: Shifts a 1 left, counting Y down to 0.
 * WAITS: no.
 * Entry: Y the bit number. Exit: A the mask; Y 0; Z and C set, N clear (the last compare).
 */
uint8_t BitMaskFromIndex(uint8_t count)
{
    uint8_t mask = 1;

    while (count != 0) {
        mask = (uint8_t)(mask << 1);
        count--;
    }
    return mask;
}

void abi_BitMaskFromIndex(void)
{
    cpu.a = BitMaskFromIndex(cpu.y);
    cpu.y = 0;
    abi_nz(0);
    abi_flag(P_C, true);
}
