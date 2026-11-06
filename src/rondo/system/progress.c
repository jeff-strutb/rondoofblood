/* The score and the events the save remembers. */
#include "machine.h"
#include "ram.h"

enum { SCORE_DIGIT_MAX = 0x99, SCORE_MARK_BITS = 0xA0 };

/* Two packed decimal digits and a carry, as the HuC6280 adds them in decimal mode. */
static uint8_t add_decimal(uint8_t a, uint8_t b, bool *carry)
{
    unsigned sum = (a & 0x0Fu) + (b & 0x0Fu) + (*carry ? 1u : 0u);

    if (sum >= 0x0A)
        sum += 0x06;
    sum += (a & 0xF0u) + (b & 0xF0u);
    if (sum >= 0xA0)
        sum += 0x60;
    *carry = (sum & 0xFF00u) != 0;
    return (uint8_t)sum;
}

/*
 * Adds points in packed decimal to the score, which stops at 999999. Gives the bits of the
 * middle byte the mark watches that changed. The old middle byte stays in zTemp00.
 */
static uint8_t add_points(uint8_t low, uint8_t middle)
{
    uint8_t old_middle = rd8(zScore1);
    uint8_t new_middle;
    uint8_t new_low;
    uint8_t new_high;
    bool carry = false;

    wr8(zTemp00, old_middle);
    new_low = add_decimal(low, rd8(zScore0), &carry);
    new_middle = add_decimal(middle, old_middle, &carry);
    new_high = add_decimal(0, rd8(zScore2), &carry);
    if (carry) {
        new_low = SCORE_DIGIT_MAX;
        new_middle = SCORE_DIGIT_MAX;
        new_high = SCORE_DIGIT_MAX;
    }
    wr8(zScore0, new_low);
    wr8(zScore1, new_middle);
    wr8(zScore2, new_high);
    return (old_middle ^ new_middle) & SCORE_MARK_BITS;
}

/* The bits are the answer in A; C says whether there were any. BCD adds leave V alone. */
static void abi_score_exit(uint8_t changed)
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
 * Entry: A points in BCD. Exit: C the answer, A those differing bits, N and Z
 * from them, D 0. V, X and Y kept.
 */
uint8_t AddScoreByte(uint8_t points)
{
    return add_points(points, 0);
}

void abi_AddScoreByte(void)
{
    abi_score_exit(AddScoreByte(cpu.a));
}

/*
 * @routine 0045:5A4 AddScoreWord
 * @run 69:55A4
 * WHAT IT DOES: Adds up to 9999 points to the score, as AddScoreByte does.
 * HOW: The same sum with a middle byte to add.
 * Entry: zTemp00-01 points in BCD. Exit: as AddScoreByte. Clobbers zTemp00.
 */
uint8_t AddScoreWord(void)
{
    uint8_t low = rd8(zTemp00);
    uint8_t middle = rd8(zTemp01);

    return add_points(low, middle);
}

void abi_AddScoreWord(void)
{
    abi_score_exit(AddScoreWord());
}

/*
 * @routine 0046:303 BitOfIndex
 * @run 69:5B03
 * WHAT IT DOES: Turns an index into a bit set into its byte and mask.
 * HOW: The byte is the index over 8; the mask a one shifted by the low bits.
 * Entry: A index. Exit: A mask, Y byte, C 1 for bit 0 else 0, X kept with N and
 * Z from it. V kept.
 */
uint8_t BitOfIndex(uint8_t index, uint8_t *byte)
{
    *byte = index >> 3;
    return (uint8_t)(1u << (index & 7));
}

void abi_BitOfIndex(void)
{
    uint8_t index = cpu.a;
    uint8_t byte;

    cpu.a = BitOfIndex(index, &byte);
    cpu.y = byte;
    abi_flag(P_C, (index & 7) == 0);
    abi_nz(cpu.x);
}

/*
 * @routine 0046:323 RecordEvent
 * @run 69:5B23
 * WHAT IT DOES: Marks an event as having happened, in the bit set the save
 * keeps: an object so marked does not come back.
 * HOW: BitOfIndex, then ORs the mask into its byte of EventFlags.
 * Entry: A event. Exit: A the byte stored, Y its index, N and Z from it, C as
 * BitOfIndex. X and V kept.
 */
uint8_t RecordEvent(uint8_t event, uint8_t *byte)
{
    uint8_t mask = BitOfIndex(event, byte);
    uint8_t merged = (uint8_t)(mask | rd8((uint16_t)(EventFlags + *byte)));

    wr8((uint16_t)(EventFlags + *byte), merged);
    return merged;
}

void abi_RecordEvent(void)
{
    uint8_t event = cpu.a;
    uint8_t byte;

    cpu.a = RecordEvent(event, &byte);
    cpu.y = byte;
    abi_flag(P_C, (event & 7) == 0);
    abi_nz(cpu.a);
}

/*
 * @routine 0046:319 RecordObjectEvent
 * @run 69:5B19
 * WHAT IT DOES: If an object is one the save remembers, marks its event.
 * HOW: Bit 7 of the object's flags says it is; its event field says which.
 * Entry: X object. Exit: unmarked, A 0, N 0, Z 1, the rest kept; else as
 * RecordEvent.
 */
bool RecordObjectEvent(uint8_t object, uint8_t *stored, uint8_t *event, uint8_t *byte)
{
    uint8_t flags = rd8((uint16_t)(OBJ_FLAGS_2F38 + object));

    if (!(flags & 0x80))
        return false;
    *event = rd8((uint16_t)(OBJ_EVENT_ID + object));
    *stored = RecordEvent(*event, byte);
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
    cpu.a = stored;
    cpu.y = byte;
    abi_flag(P_C, (event & 7) == 0);
    abi_nz(stored);
}
