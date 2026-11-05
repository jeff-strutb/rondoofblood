/* The score and the events the save remembers. */
#include "machine.h"
#include "ram.h"

/* A score sum finishing from the middle byte on. */
static Alu finish_score(uint8_t add_middle, bool carry)
{
    uint8_t byte;
    uint8_t old_middle;
    Alu r = { 0 };

    byte = rd8(zScore1);
    r = adc8_bcd(add_middle, byte, carry);
    wr8(zScore1, r.a);
    byte = rd8(zScore2);
    r = adc8_bcd(0, byte, r.c);
    wr8(zScore2, r.a);
    if (r.c) {
        wr8(zScore0, 0x99);
        wr8(zScore1, 0x99);
        wr8(zScore2, 0x99);
    }
    old_middle = rd8(zTemp00);
    byte = rd8(zScore1);
    r.a = (old_middle ^ byte) & 0xA0;
    r.nz = r.a;
    r.c = r.a != 0;
    return r;
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
Alu AddScoreByte(uint8_t points)
{
    uint8_t byte = rd8(zScore0);
    Alu r = adc8_bcd(points, byte, false);

    wr8(zScore0, r.a);
    byte = rd8(zScore1);
    wr8(zTemp00, byte);
    return finish_score(0, r.c);
}

void abi_AddScoreByte(void)
{
    abi_alu(AddScoreByte(cpu.a));
    abi_flag(P_D, false);
}

/*
 * @routine 0045:5A4 AddScoreWord
 * @run 69:55A4
 * WHAT IT DOES: Adds up to 9999 points to the score, as AddScoreByte does.
 * HOW: The same sum with a middle byte to add.
 * Entry: zTemp00-01 points in BCD. Exit: as AddScoreByte. Clobbers zTemp00.
 */
Alu AddScoreWord(void)
{
    uint8_t points = rd8(zTemp00);
    uint8_t byte = rd8(zScore0);
    Alu r = adc8_bcd(points, byte, false);

    wr8(zScore0, r.a);
    byte = rd8(zScore1);
    wr8(zTemp00, byte);
    points = rd8(zTemp01);
    return finish_score(points, r.c);
}

void abi_AddScoreWord(void)
{
    abi_alu(AddScoreWord());
    abi_flag(P_D, false);
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
