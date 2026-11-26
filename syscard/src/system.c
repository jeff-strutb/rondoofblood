/* The BIOS's system entries. */
#include "card.h"
#include "card_access.h"
#include "syscard.h"

enum {
    SIGNATURE_A = 0xAA, SIGNATURE_B = 0x55, SUPER_RAM_FLAG = 0x80,
    VECTOR_SOFT_RESET = 6
};

/* Both bytes of a signature must read true; records the last compare made. */
static bool signature_found(uint16_t port_a, uint16_t port_b, MemOpen *open)
{
    open->last_read = rd8(port_a);
    open->expected = SIGNATURE_A;
    if (open->last_read != SIGNATURE_A)
        return false;
    open->last_read = rd8(port_b);
    open->expected = SIGNATURE_B;
    return open->last_read == SIGNATURE_B;
}

/*
 * @card 00:FE92 ex_memopen
 * WHAT IT DOES: Finds the card's extra RAM, and the bank it starts at.
 * HOW: Looks for a signature at $18C5, then at $18C1, whose bank it marks with bit 7.
 * Entry: nothing. Exit, found: X the bank, A the card's byte at $FFF4, N and Z from it, C
 * clear. Exit, not found: X 0, C set, A the byte last read, N and Z from its compare. Y, V
 * kept.
 */
MemOpen ex_memopen(void)
{
    MemOpen open = { false, 0, 0, 0 };

    if (signature_found(ArcadeRamSigA, ArcadeRamSigB, &open)) {
        open.present = true;
        open.bank = rd8(ArcadeRamBank);
    } else if (signature_found(SuperRamSigA, SuperRamSigB, &open)) {
        open.present = true;
        open.bank = rd8(SuperRamBank) | SUPER_RAM_FLAG;
    }
    return open;
}

void abi_ex_memopen(void)
{
    MemOpen open = ex_memopen();

    cpu.x = open.bank;
    abi_flag(P_C, !open.present);
    if (open.present) {
        cpu.a = rd8(CardVersionByte);
        abi_nz(cpu.a);
    } else {
        cpu.a = open.last_read;
        abi_nz((uint8_t)(open.last_read - open.expected));
    }
}

/*
 * @card 00:F034 ex_setvec
 * WHAT IT DOES: Sets an interrupt handler the card calls.
 * HOW: The flags are pushed and interrupts held off; a word goes in the table at $2200 by which, through
 * A, Y and X pushed and pulled; which 6 is the soft reset's, kept apart; the flags are pulled.
 * WAITS: no.
 * Entry: A which, X and Y the address; the flags (pushed). Exit: flags as pushed, A and Y kept; X the
 * table's offset of the high byte unless which is 6.
 */
SetVec ex_setvec(uint8_t which, uint8_t low, uint8_t high, uint8_t flags)
{
    SetVec out = { 0, false, 0 };
    uint8_t offset;
    uint8_t value;

    push8(php_byte(flags));
    sei();
    if (which == VECTOR_SOFT_RESET) {
        WR8(CRAM->soft_reset.lo, low);
        WR8(CRAM->soft_reset.hi, high);
    } else {
        push8(which);
        push8(high);
        push8(low);
        offset = (uint8_t)(which << 1);
        value = pull8();
        WR8_IDX(ADDR_OF8(CRAM->irq_vectors), offset, value);
        offset++;
        value = pull8();
        WR8_IDX(ADDR_OF8(CRAM->irq_vectors), offset, value);
        (void)pull8();
        out.sets_x = true;
        out.x = offset;
    }
    out.flags = pull8();
    return out;
}

void abi_ex_setvec(void)
{
    SetVec out = ex_setvec(cpu.a, cpu.x, cpu.y, cpu.p);

    cpu.p = out.flags;
    if (out.sets_x) {
        cpu.x = out.x;
    }
}

/*
 * @card 00:F02D ex_getver
 * WHAT IT DOES: Gives the card's version: 3.00 on this card.
 * HOW: Two bytes of the ROM.
 * Entry: nothing. Exit: X the major version, Y the minor, N and Z from Y. A, C, V kept.
 */
CardVersion ex_getver(void)
{
    CardVersion version;

    version.major = rd8(CardVersionMajor);
    version.minor = rd8(CardVersionMinor);
    return version;
}

void abi_ex_getver(void)
{
    CardVersion version = ex_getver();

    cpu.x = version.major;
    cpu.y = version.minor;
    abi_nz(version.minor);
}

enum { BOOT_RAM_FIRST_MPR = 2, BOOT_RAM_LAST_MPR = 6 };

/*
 * @card 00:E13A map_boot_ram
 * WHAT IT DOES: Maps the five CD RAM banks the boot program runs in at $4000-$DFFF.
 * HOW: The bank in CardBootRamBank to MPR2 and the four after it to MPR3 to MPR6.
 * Entry: nothing. Exit: A the last bank mapped, N and Z from it. X, Y, C, V kept. Gives the last
 * bank.
 */
uint8_t map_boot_ram(void)
{
    uint8_t bank = rd8(CardBootRamBank);

    for (unsigned mpr = BOOT_RAM_FIRST_MPR; mpr <= BOOT_RAM_LAST_MPR; mpr++, bank++)
        tam((uint8_t)(1u << mpr), bank);
    return (uint8_t)(bank - 1);
}

void abi_map_boot_ram(void)
{
    cpu.a = map_boot_ram();
    abi_nz(cpu.a);
}

enum {
    SOFT_RESET_ENTRY = 0xE000, DEFAULT_IRQ_HANDLER = 0xE8E2, IRQ_VECTOR_COUNT = 6,
    RESET_MARK = 0x80
};

/*
 * @card 01:CB30 init_card_vectors
 * WHAT IT DOES: Sets the card's vectors at boot: the soft reset to $E000, and all six interrupt
 * handlers to the card's default at $E8E2.
 * HOW: Writes the reset vector and CardResetMark, then the first handler, and a block move
 * repeats its two bytes over the other five.
 * WAITS: no.
 * Entry: nothing; X, Y (pushed by the block move; A is the default handler's high byte then). Exit: A $E8,
 * N 1, Z 0. X, Y, C, V kept.
 */
CardExit init_card_vectors(uint8_t x, uint8_t y)
{
    CardExit e = { 0 };

    WR8(CRAM->soft_reset.lo, (uint8_t)SOFT_RESET_ENTRY);
    WR8(CRAM->soft_reset.hi, (uint8_t)(SOFT_RESET_ENTRY >> 8));
    WR8(CZP->reset_mark, RESET_MARK);
    WR8(CZP->zp_e6, 0);
    WR8_IDX(ADDR_OF8(CRAM->irq_vectors), 0, (uint8_t)DEFAULT_IRQ_HANDLER);
    WR8_IDX(ADDR_OF8(CRAM->irq_vectors), 1, (uint8_t)(DEFAULT_IRQ_HANDLER >> 8));
    push8(y);
    push8((uint8_t)(DEFAULT_IRQ_HANDLER >> 8));
    push8(x);
    for (unsigned i = 0; i < 10; i++) {
        uint8_t byte = RD8_IDX(ADDR_OF8(CRAM->irq_vectors), i & 1);

        WR8_IDX(ADDR_OF8(CRAM->irq_vectors), 2 + i, byte);
    }
    (void)pull8();
    (void)pull8();
    (void)pull8();
    WR8(CZP->irq_flags, 0);
    e.a = e.nz = (uint8_t)(DEFAULT_IRQ_HANDLER >> 8);
    e.set_a = e.set_nz = true;
    return e;
}

void abi_init_card_vectors(void)
{
    card_exit_apply(init_card_vectors(cpu.x, cpu.y));
}

enum { RND_FEEDBACK_BIT = 6 };

/*
 * @card 00:E67E ex_rnd
 * WHAT IT DOES: Gives a pseudo-random byte from the three bytes of state in CardRndSeed.
 * HOW: The state's low byte is XORed into its middle byte, and its high byte XORed with it
 * becomes the new low byte; those two then shift left one as a word, taking in bit 6 of the old
 * middle byte XOR bit 0 of the old low byte. The high byte never changes. The byte given is the
 * new low and middle bytes XORed.
 * Entry: CardRndSeed. Exit: A the byte, N and Z from it, C the bit shifted out of the middle
 * byte. X, Y, V kept.
 */
RandomByte ex_rnd(void)
{
    uint8_t low = rd8(CardRndSeed);
    uint8_t middle = rd8((uint16_t)(CardRndSeed + 1));
    uint8_t high = rd8((uint16_t)(CardRndSeed + 2));
    uint8_t feedback = (uint8_t)(((middle >> RND_FEEDBACK_BIT) ^ low) & 1);
    uint8_t mixed_middle = (uint8_t)(middle ^ low);
    uint8_t mixed_low = (uint8_t)(high ^ low);
    RandomByte random;

    wr8((uint16_t)(CardRndSeed + 1), mixed_middle);
    wr8(CardRndSeed, mixed_low);
    low = (uint8_t)((mixed_low << 1) | feedback);
    wr8(CardRndSeed, low);
    middle = (uint8_t)((mixed_middle << 1) | (mixed_low >> 7));
    wr8((uint16_t)(CardRndSeed + 1), middle);
    random.value = (uint8_t)(middle ^ low);
    random.carry = (mixed_middle & 0x80) != 0;
    return random;
}

void abi_ex_rnd(void)
{
    RandomByte random = ex_rnd();

    cpu.a = random.value;
    abi_nz(random.value);
    abi_flag(P_C, random.carry);
}
