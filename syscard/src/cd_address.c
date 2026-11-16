/*
 * Disc addresses: a position as minutes, seconds and frames made into a sector number. The
 * routines lie in bank 1 and run at $C000. A sector number is three bytes, high first: the sum
 * in _AL, _AH, _BL, the term being added in CardPointerLo, CardPointerHi, CardCopyLo.
 */
#include "card.h"
#include "syscard.h"

enum {
    FRAMES_PER_SECOND = 75, SECONDS_PER_MINUTE = 60, LEAD_IN_FRAMES = 150,
    LBA_MASK = 0xFFFFFF, LBA_TOP_BIT = 0x800000, MSF_BYTES = 3
};

static uint32_t read_three(uint16_t high)
{
    uint8_t top = rd8(high);
    uint8_t middle = rd8((uint16_t)(high + 1));

    return ((uint32_t)top << 16) | ((uint32_t)middle << 8) | rd8((uint16_t)(high + 2));
}

static void write_three(uint16_t high, uint32_t value)
{
    wr8(high, (uint8_t)(value >> 16));
    wr8((uint16_t)(high + 1), (uint8_t)(value >> 8));
    wr8((uint16_t)(high + 2), (uint8_t)value);
}

/* The exit of an add: A the sum's high byte; N, Z, C and V from adding the high bytes. */
void abi_lba_add_exit_shim(LbaAdd add)
{
    uint32_t sum = (add.before + add.term) & LBA_MASK;
    bool carry_in = (add.before & 0xFFFF) + (add.term & 0xFFFF) > 0xFFFF;
    int top = (int8_t)(add.before >> 16) + (int8_t)(add.term >> 16) + (carry_in ? 1 : 0);

    cpu.a = (uint8_t)(sum >> 16);
    abi_nz(cpu.a);
    abi_flag(P_C, add.before + add.term > LBA_MASK);
    abi_flag(P_V, top < -128 || top > 127);
}

/*
 * @card 01:CC97 lba_load_term
 * WHAT IT DOES: Makes a byte the term to add.
 * HOW: The byte to the term's low byte, its upper two bytes cleared.
 * Entry: A. Exit: all registers and flags kept.
 */
void lba_load_term(uint8_t value)
{
    write_three(CardPointerLo, value);
}

void abi_lba_load_term(void)
{
    lba_load_term(cpu.a);
}

/*
 * @card 01:CC7A lba_double_term
 * WHAT IT DOES: Doubles the term.
 * HOW: A shift left through its three bytes.
 * Entry: the term. Exit: C its old top bit, N its new top bit, Z if its new high byte is zero.
 * A, X, Y, V kept. Gives the old top bit.
 */
bool lba_double_term(void)
{
    uint32_t term = read_three(CardPointerLo);

    write_three(CardPointerLo, (term << 1) & LBA_MASK);
    return (term & LBA_TOP_BIT) != 0;
}

void abi_lba_double_term(void)
{
    bool out = lba_double_term();
    uint8_t high = rd8(CardPointerLo);

    abi_flag(P_C, out);
    abi_nz(high);
}

/*
 * @card 01:CC83 lba_add_term
 * WHAT IT DOES: Adds the term to the sum.
 * HOW: A three-byte add, low byte first.
 * Entry: the sum, the term. Exit: A the sum's high byte, N, Z, C and V from adding the high
 * bytes. X, Y kept.
 */
LbaAdd lba_add_term(void)
{
    LbaAdd add;

    add.before = read_three(CardAl);
    add.term = read_three(CardPointerLo);
    write_three(CardAl, (add.before + add.term) & LBA_MASK);
    return add;
}

void abi_lba_add_term(void)
{
    abi_lba_add_exit_shim(lba_add_term());
}

/*
 * @card 01:CC81 lba_double_and_add
 * WHAT IT DOES: Doubles the term, then adds it to the sum.
 * HOW: lba_double_term, then lba_add_term.
 * Entry: the sum, the term. Exit: as lba_add_term.
 */
LbaAdd lba_double_and_add(void)
{
    lba_double_term();
    return lba_add_term();
}

void abi_lba_double_and_add(void)
{
    abi_lba_add_exit_shim(lba_double_and_add());
}

/*
 * @card 01:CC17 msf_to_lba
 * WHAT IT DOES: Makes the disc position in CardCdResult, as BCD minutes, seconds and frames,
 * into a sector number in _AL, _AH, _BL, high first.
 * HOW: Each byte goes to binary in place (ex_bcdbin, which leaves a bad byte as it was); then
 * ((minutes times 60 plus seconds) times 75 plus frames) less 150, the frames before sector 0,
 * by shifts and adds of the term in CardPointerLo, in 24 bits.
 * Entry: CardCdResult. Exit: A the high byte, N, Z, C and V from adding the last term's high
 * byte. X, Y kept. The term is left as the frames less 150.
 */
LbaAdd msf_to_lba(void)
{
    uint8_t msf[MSF_BYTES];
    uint32_t seconds;
    LbaAdd add;

    for (unsigned i = 0; i < MSF_BYTES; i++) {
        msf[i] = ex_bcdbin(rd8((uint16_t)(CardCdResult + i))).value;
        wr8((uint16_t)(CardCdResult + i), msf[i]);
    }
    seconds = (uint32_t)msf[0] * SECONDS_PER_MINUTE + msf[1];
    add.before = (seconds * FRAMES_PER_SECOND) & LBA_MASK;
    add.term = ((uint32_t)msf[2] - LEAD_IN_FRAMES) & LBA_MASK;
    write_three(CardPointerLo, add.term);
    write_three(CardAl, (add.before + add.term) & LBA_MASK);
    return add;
}

void abi_msf_to_lba(void)
{
    abi_lba_add_exit_shim(msf_to_lba());
}
