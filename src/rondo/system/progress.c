/* The score and the events the save remembers. */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"

enum { SCORE_DIGIT_MAX = 0x99, SCORE_MARK_BITS = 0xA0 };

/* Two packed decimal digits and a carry, as the HuC6280 adds them in decimal mode. */
static uint8_t add_decimal(uint8_t a, uint8_t b, bool *carry)
{
    unsigned sum = (a & 0x0Fu) + (b & 0x0Fu) + (*carry ? 1u : 0u);

    if (sum >= 0x0A) {
        sum += 0x06;
    }
    sum += (a & 0xF0u) + (b & 0xF0u);
    if (sum >= 0xA0) {
        sum += 0x60;
    }
    *carry = (sum & 0xFF00u) != 0;
    return (uint8_t)sum;
}

/*
 * The sum from the middle byte on, in decimal mode: the middle score byte is read again, added
 * with the carry, then the high byte. Past 999999 all three bytes become $99. The answer is the
 * bits the mark watches that differ between the old middle byte (zTemp00) and the new.
 */
static uint8_t add_points_tail(uint8_t middle_add, bool carry)
{
    uint8_t value, old, new_middle;

    value = RD8(ZP->score.mid);
    value = add_decimal(middle_add, value, &carry);
    WR8(ZP->score.mid, value);
    value = RD8(ZP->score.hi);
    value = add_decimal(0, value, &carry);
    WR8(ZP->score.hi, value);
    if (carry) {
        WR8(ZP->score.lo, SCORE_DIGIT_MAX);
        WR8(ZP->score.mid, SCORE_DIGIT_MAX);
        WR8(ZP->score.hi, SCORE_DIGIT_MAX);
    }
    old = RD8(ZP->temp00);
    new_middle = RD8(ZP->score.mid);
    return (uint8_t)((old ^ new_middle) & SCORE_MARK_BITS);
}

/* The bits are the answer in A; C says whether there were any. BCD adds leave V alone. */
void score_exit_shim(uint8_t changed)
{
    cpu.a = changed;
    abi_nz(changed);
    abi_flag(P_C, changed != 0);
    abi_flag(P_D, false);
}

/*
 * @routine 0045:597 AddScoreByte
 * @run 69:5597
 * WHAT IT DOES: Adds up to 99 points to the score, which stops at 999999, and
 * says whether that crossed the mark the middle digits watch for.
 * HOW: A decimal sum through zScore0-2, old middle byte kept in zTemp00. The
 * answer is whether bits 7 and 5 of that byte differ afterward.
 * WAITS: no.
 * Entry: A points in BCD, D clear. Exit: C the answer, A those differing bits, N and Z
 * from them, D 0. V, X and Y kept.
 */
uint8_t AddScoreByte(uint8_t points)
{
    uint8_t low, old_middle;
    bool carry = false;

    low = RD8(ZP->score.lo);
    low = add_decimal(points, low, &carry);
    WR8(ZP->score.lo, low);
    old_middle = RD8(ZP->score.mid);
    WR8(ZP->temp00, old_middle);
    return add_points_tail(0, carry);
}

void abi_AddScoreByte(void)
{
    score_exit_shim(AddScoreByte(cpu.a));
}

/*
 * @routine 0045:5A4 AddScoreWord
 * @run 69:55A4
 * WHAT IT DOES: Adds up to 9999 points to the score, as AddScoreByte does.
 * HOW: The same sum with a middle byte to add.
 * WAITS: no.
 * Entry: zTemp00-01 points in BCD, D clear. Exit: as AddScoreByte. Clobbers zTemp00. The arm
 * where the score passes 999999 ran in no script: the draft stops there, so liftcmp judges only
 * the sums that stay under it; AddScoreByte has no draft (no script enters it at its own start)
 * and shares the tail, so it is NOT CHECKED.
 */
uint8_t AddScoreWord(void)
{
    uint8_t points, low, old_middle, middle_add;
    bool carry = false;

    points = RD8(ZP->temp00);
    low = RD8(ZP->score.lo);
    low = add_decimal(points, low, &carry);
    WR8(ZP->score.lo, low);
    old_middle = RD8(ZP->score.mid);
    WR8(ZP->temp00, old_middle);
    middle_add = RD8(ZP->temp01);
    return add_points_tail(middle_add, carry);
}

void abi_AddScoreWord(void)
{
    score_exit_shim(AddScoreWord());
}

/*
 * @routine 0046:303 BitOfIndex
 * @run 69:5B03
 * WHAT IT DOES: Turns an index into a bit set into its byte and mask.
 * HOW: Keeps X and the index on the stack; the byte is the index over 8; the mask a one
 * shifted left by the low three bits, one shift at a time.
 * WAITS: no.
 * Entry: A index, X (pushed and pulled back). Exit: A mask, Y byte, C 1 for bit 0 else 0, X kept
 * with N and Z from it. V kept.
 */
uint8_t BitOfIndex(uint8_t index, uint8_t x, uint8_t *byte)
{
    uint8_t count, mask = 1;

    push8(x);
    push8(index);
    *byte = (uint8_t)(index >> 3);
    index = pull8();
    count = (uint8_t)(index & 7);
    while (count != 0) {
        mask = (uint8_t)(mask << 1);
        count--;
    }
    (void)pull8();
    return mask;
}

/* A the mask, Y the byte, C 1 when the index's low three bits are zero, N and Z from X. */
void bit_of_index_exit_shim(uint8_t index, uint8_t mask, uint8_t byte)
{
    cpu.a = mask;
    cpu.y = byte;
    abi_flag(P_C, (index & 7) == 0);
    abi_nz(cpu.x);
}

void abi_BitOfIndex(void)
{
    uint8_t index = cpu.a;
    uint8_t byte;
    uint8_t mask = BitOfIndex(index, cpu.x, &byte);

    bit_of_index_exit_shim(index, mask, byte);
}

/*
 * @routine 0046:323 RecordEvent
 * @run 69:5B23
 * WHAT IT DOES: Marks an event as having happened, in the bit set the save
 * keeps: an object so marked does not come back.
 * HOW: BitOfIndex, then ORs the mask into its byte of EventFlags.
 * WAITS: no.
 * Entry: A event, X (kept). Exit: A the byte stored, Y its index, N and Z from it, C as
 * BitOfIndex. X and V kept.
 */
uint8_t RecordEvent(uint8_t event, uint8_t x, uint8_t *byte)
{
    uint8_t mask, flags, merged;

    jsr_push(0x5B26);
    mask = BitOfIndex(event, x, byte);
    jsr_pop();
    flags = RD8(LEVEL->event_flags[*byte]);
    merged = (uint8_t)(mask | flags);
    WR8(LEVEL->event_flags[*byte], merged);
    return merged;
}

/* A the byte stored, Y the byte's index, N and Z from it, C as BitOfIndex's. */
void record_event_exit_shim(uint8_t event, uint8_t merged, uint8_t byte)
{
    cpu.a = merged;
    cpu.y = byte;
    abi_nz(merged);
    abi_flag(P_C, (event & 7) == 0);
}

void abi_RecordEvent(void)
{
    uint8_t event = cpu.a;
    uint8_t byte;
    uint8_t merged = RecordEvent(event, cpu.x, &byte);

    record_event_exit_shim(event, merged, byte);
}

/*
 * @routine 0046:319 RecordObjectEvent
 * @run 69:5B19
 * WHAT IT DOES: If an object is one the save remembers, marks its event.
 * HOW: Bit 7 of the object's flags says it is; its event field says which; the rest is
 * RecordEvent, which it falls into.
 * WAITS: no.
 * Entry: X object. Exit: unmarked, A 0, N 0, Z 1, the rest kept; else as RecordEvent.
 */
bool RecordObjectEvent(uint8_t object, uint8_t *stored, uint8_t *event, uint8_t *byte)
{
    uint8_t flags, bit;

    flags = RD8(OBJ->flags[object]);
    bit = (uint8_t)(flags & 0x80);
    if (bit == 0) {
        return false;
    }
    *event = RD8(OBJ->event_id[object]);
    *stored = RecordEvent(*event, object, byte);
    return true;
}

void abi_RecordObjectEvent(void)
{
    uint8_t stored;
    uint8_t event;
    uint8_t byte;

    if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
        cpu.a = 0;
        abi_nz(0);
        return;
    }
    record_event_exit_shim(event, stored, byte);
}
