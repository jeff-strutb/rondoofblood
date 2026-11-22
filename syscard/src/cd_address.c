/*
 * Disc addresses: a position as minutes, seconds and frames made into a sector number. The
 * routines lie in bank 1 and run at $C000. A sector number is three bytes, high first: the sum
 * in _AL, _AH, _BL, the term being added in CardPointerLo, CardPointerHi, CardCopyLo.
 */
#include "card.h"
#include "card_access.h"
#include "syscard.h"

enum { LEAD_IN_FRAMES = 0x96, RESULT_BYTES = 3 };


/*
 * @card 01:CC97 lba_load_term
 * WHAT IT DOES: Makes a byte the term to add.
 * HOW: The byte to the term's low byte, its upper two bytes cleared.
 * WAITS: no.
 * Entry: A. Exit: all registers and flags kept.
 */
void lba_load_term(uint8_t value)
{
    WR8(CZP->copy.lo, value);
    WR8(CZP->pointer.hi, 0);
    WR8(CZP->pointer.lo, 0);
}

void abi_lba_load_term(void)
{
    lba_load_term(cpu.a);
}

/*
 * @card 01:CC7A lba_double_term
 * WHAT IT DOES: Doubles the term.
 * HOW: A shift left through its three bytes.
 * WAITS: no.
 * Entry: the term. Exit: C its old top bit, N its new top bit, Z if its new high byte is zero.
 * A, X, Y, V kept.
 */
CardExit lba_double_term(void)
{
    CardExit e = { 0 };
    uint8_t value;
    bool carry;
    uint8_t result;

    value = RD8(CZP->copy.lo);
    carry = (value & 0x80) != 0;
    WR8(CZP->copy.lo, (uint8_t)(value << 1));
    value = RD8(CZP->pointer.hi);
    result = (uint8_t)((value << 1) | (carry ? 1 : 0));
    carry = (value & 0x80) != 0;
    WR8(CZP->pointer.hi, result);
    value = RD8(CZP->pointer.lo);
    result = (uint8_t)((value << 1) | (carry ? 1 : 0));
    carry = (value & 0x80) != 0;
    WR8(CZP->pointer.lo, result);
    e.nz = result;
    e.c = carry;
    e.set_nz = e.set_c = true;
    return e;
}

void abi_lba_double_term(void)
{
    card_exit_apply(lba_double_term());
}

/*
 * @card 01:CC83 lba_add_term
 * WHAT IT DOES: Adds the term to the sum.
 * HOW: A three-byte add, low byte first.
 * WAITS: no.
 * Entry: the sum, the term. Exit: A the sum's high byte, N, Z, C and V from adding the high
 * bytes. X, Y kept.
 */
CardExit lba_add_term(void)
{
    CardExit e = { 0 };
    Sum low;
    Sum middle;
    Sum high;
    uint8_t value;
    uint8_t term;

    value = RD8(CZP->b.lo);
    term = RD8(CZP->copy.lo);
    low = add8(value, term, false);
    WR8(CZP->b.lo, low.value);
    value = RD8(CZP->a.hi);
    term = RD8(CZP->pointer.hi);
    middle = add8(value, term, low.carry);
    WR8(CZP->a.hi, middle.value);
    value = RD8(CZP->a.lo);
    term = RD8(CZP->pointer.lo);
    high = add8(value, term, middle.carry);
    WR8(CZP->a.lo, high.value);
    e.a = e.nz = high.value;
    e.c = high.carry;
    e.v = high.overflow;
    e.set_a = e.set_nz = e.set_c = e.set_v = true;
    return e;
}

void abi_lba_add_term(void)
{
    card_exit_apply(lba_add_term());
}

/*
 * @card 01:CC81 lba_double_and_add
 * WHAT IT DOES: Doubles the term, then adds it to the sum.
 * HOW: lba_double_term, then lba_add_term, which it runs into.
 * WAITS: no.
 * Entry: the sum, the term. Exit: as lba_add_term.
 */
CardExit lba_double_and_add(void)
{
    jsr_push(0xCC83);
    (void)lba_double_term();
    jsr_pop();
    return lba_add_term();
}

void abi_lba_double_and_add(void)
{
    card_exit_apply(lba_double_and_add());
}

/* A call of lba_double_term or lba_double_and_add: the frame is the one the assembly's BSR makes. */
static void lba_double(uint16_t ret)
{
    jsr_push(ret);
    (void)lba_double_term();
    jsr_pop();
}

static CardExit lba_double_add(uint16_t ret)
{
    CardExit e;

    jsr_push(ret);
    e = lba_double_and_add();
    jsr_pop();
    return e;
}

/*
 * @card 01:CC17 msf_to_lba
 * WHAT IT DOES: Makes the disc position in CardCdResult, as BCD minutes, seconds and frames,
 * into a sector number in _AL, _AH, _BL, high first.
 * HOW: Each byte goes to binary in place (ex_bcdbin, which leaves a bad byte as it was); then
 * ((minutes times 60 plus seconds) times 75 plus frames) less 150, the frames before sector 0,
 * by shifts and adds of the term in CardPointerLo, in 24 bits.
 * WAITS: no.
 * Entry: CardCdResult; X (pushed by ex_bcdbin). Exit: A the high byte, N, Z, C and V from adding the
 * last term's high byte. X, Y kept. The term is left as the frames less 150.
 */
CardExit msf_to_lba(uint8_t x)
{
    CardExit e;
    BcdConvert converted;
    Sum low;
    Sum middle;
    uint8_t value;
    uint16_t rets[3] = { 0xCC1D, 0xCC26, 0xCC2F };

    for (unsigned i = 0; i < RESULT_BYTES; i++) {
        value = RD8_IDX(COL(CRAM->cd_result), i);
        jsr_push(rets[i]);
        converted = ex_bcdbin(value, x);
        jsr_pop();
        WR8_IDX(COL(CRAM->cd_result), i, converted.value);
    }
    WR8(CZP->a.lo, 0);
    WR8(CZP->a.hi, 0);
    WR8(CZP->b.lo, 0);
    value = RD8_IDX(COL(CRAM->cd_result), 0);
    jsr_push(0xCC3D);
    lba_load_term(value);
    jsr_pop();
    lba_double(0xCC3F);
    (void)lba_double_add(0xCC41);
    (void)lba_double_add(0xCC43);
    (void)lba_double_add(0xCC45);
    (void)lba_double_add(0xCC47);
    value = RD8_IDX(COL(CRAM->cd_result), 1);
    jsr_push(0xCC4C);
    lba_load_term(value);
    jsr_pop();
    jsr_push(0xCC4E);
    (void)lba_add_term();
    jsr_pop();
    value = RD8(CZP->a.lo);
    WR8(CZP->pointer.lo, value);
    value = RD8(CZP->a.hi);
    WR8(CZP->pointer.hi, value);
    value = RD8(CZP->b.lo);
    WR8(CZP->copy.lo, value);
    (void)lba_double_add(0xCC5C);
    lba_double(0xCC5E);
    (void)lba_double_add(0xCC60);
    lba_double(0xCC62);
    lba_double(0xCC64);
    (void)lba_double_add(0xCC66);
    value = RD8_IDX(COL(CRAM->cd_result), 2);
    low = sub8(value, LEAD_IN_FRAMES, true);
    WR8(CZP->copy.lo, low.value);
    middle = sub8(0, 0, low.carry);
    WR8(CZP->pointer.hi, middle.value);
    middle = sub8(0, 0, middle.carry);
    WR8(CZP->pointer.lo, middle.value);
    e = lba_add_term();
    return e;
}

void abi_msf_to_lba(void)
{
    card_exit_apply(msf_to_lba(cpu.x));
}
