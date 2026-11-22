/*
 * The shared object pool's state helpers: what the stage overlays' object handlers call at the
 * head of a state, before they jump through the object's state table (the byte OBJ->f27a8 is the
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
 * @run 69:A46A
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

    state = RD8(OBJ->f27a8[slot]);
    result.value = state;
    if (state == 0) {
        result.end = HURT_NOT_ACTIVE;
        return result;
    }
    if (state >= HIT_LIMIT) {
        result.end = HURT_STATE_PAST;
        return result;
    }
    hit = RD8(OBJ->f33b0[slot]);
    result.value = hit;
    if (hit == 0) {
        result.end = HURT_NOT_HIT;
        return result;
    }
    if (hit >= HIT_LIMIT) {
        result.end = HURT_HIT_PAST;
        return result;
    }
    WR8(OBJ->f27a8[slot], HIT_STATE);
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
 * @run 69:A74E
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

    WR8(OBJ->f2b18[slot], 0);
    timer = RD8(OBJ->f3a90[slot]);
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
    WR8(OBJ->f2b18[slot], BLINK_FLAG);
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
 * @run 69:A482
 * WHAT IT DOES: Starts a newly placed object: loads its record, copies slot 0's byte of the
 * OBJ_2FE8 column into the object's, sets two of its counters and its animation delay to 4, and
 * its timer to $60.
 * HOW: Calls LoadObjectRecord through its vector (which keeps X), then stores each byte.
 * WAITS: no.
 * Entry: X object. Exit: A $60 and flags from that load; X kept; Y as LoadObjectRecord left it.
 */
void InitObjectFromRecord(uint8_t slot)
{
    uint8_t shared;

    cpu.x = slot;
    if (call_frame(0xA485, abi_VecLoadObjectRecord)) {
        return;
    }
    shared = RD8(OBJ->f2fe8[0]);
    WR8(OBJ->f2fe8[slot], shared);
    WR8(OBJ->f3988[slot], 4);
    WR8(OBJ->f3a38[slot], 4);
    WR8(OBJ->f3510[slot], 4);
    WR8(OBJ->f34b8[slot], 0x60);
}

void abi_InitObjectFromRecord(void)
{
    InitObjectFromRecord(cpu.x);
    cpu.a = 0x60;
    abi_nz(0x60);
}
