/* Background layers: the scroll step each layer moves by in a frame. */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "layers.h"

/* The exit of a routine that ends in a ROR of the low byte: A, N and Z from it, C the bit shifted out. */
static void halved_exit_shim(ScrollStepHalved halved)
{
    cpu.a = halved.low;
    abi_nz(halved.low);
    abi_flag(P_C, halved.carry);
}

/*
 * @routine 0515:132 HalveScrollStep
 * @run 6F:A132
 * WHAT IT DOES: Sets one entry of this frame's X scroll step to half another's, so a layer
 * behind the first moves at half its speed.
 * HOW: A signed 16-bit shift right: the high byte keeps its sign (the first read of it
 * only gives the sign, the second is shifted), and the bit it loses goes into the low byte's
 * top. The high byte is written before the low byte is read.
 * WAITS: no.
 * Entry: X the entry to halve, Y the entry that gets the result. Exit: A the low byte written, N
 * and Z from it, C the low bit shifted out. X and Y kept.
 * Names: ScrollStepXLo and ScrollStepXHi are indexed here by a sum of the field and the entry,
 * as the assembly does, until the owner of the records adds the two columns.
 */
ScrollStepHalved HalveScrollStep(uint8_t from, uint8_t to)
{
    ScrollStepHalved halved;
    uint8_t sign_byte, high, low;
    uint8_t sign;

    sign_byte = rd8((uint16_t)(ScrollStepXHi + from));
    sign = sign_byte >> 7;
    high = rd8((uint16_t)(ScrollStepXHi + from));
    wr8((uint16_t)(ScrollStepXHi + to), (uint8_t)((high >> 1) | (sign << 7)));
    low = rd8((uint16_t)(ScrollStepXLo + from));
    halved.low = (uint8_t)((low >> 1) | ((high & 1) << 7));
    wr8((uint16_t)(ScrollStepXLo + to), halved.low);
    halved.carry = (low & 1) != 0;
    return halved;
}

void abi_HalveScrollStep(void)
{
    halved_exit_shim(HalveScrollStep(cpu.x, cpu.y));
}

/*
 * @routine 07D5:131 HalveScrollStepFromFirst
 * @run 6F:A131
 * WHAT IT DOES: Sets entry Y of this frame's X scroll step to half of entry 0's.
 * HOW: Entry 0 into HalveScrollStep. The one byte is a CLX that runs on into it.
 * WAITS: no.
 * Entry: Y the entry that gets the result. Exit: as HalveScrollStep, X 0.
 */
ScrollStepHalved HalveScrollStepFromFirst(uint8_t to)
{
    return HalveScrollStep(0, to);
}

void abi_HalveScrollStepFromFirst(void)
{
    cpu.x = 0;
    halved_exit_shim(HalveScrollStepFromFirst(cpu.y));
}

/*
 * @routine 0515:12F HalveScrollStepFirstToSecond
 * @run 6F:A12F
 * WHAT IT DOES: Sets entry 1 of this frame's X scroll step to half of entry 0's.
 * HOW: Entry 0 to entry 1 through HalveScrollStep. The three bytes load Y and clear X, then
 * run on into it.
 * WAITS: no.
 * Entry: nothing. Exit: as HalveScrollStep, X 0 and Y 1.
 */
ScrollStepHalved HalveScrollStepFirstToSecond(void)
{
    return HalveScrollStep(0, 1);
}

void abi_HalveScrollStepFirstToSecond(void)
{
    cpu.y = 1;
    cpu.x = 0;
    halved_exit_shim(HalveScrollStepFirstToSecond());
}
