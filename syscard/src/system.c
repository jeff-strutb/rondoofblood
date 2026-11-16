/* The BIOS's system entries. */
#include "card.h"
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
 * HOW: A word in the table at $2200 by which; which 6 is the soft reset's, kept apart.
 * Entry: A which, X and Y the address. Exit: flags as they were, A and Y kept; X the
 * table's offset of the high byte unless which is 6.
 */
void ex_setvec(uint8_t which, uint8_t low, uint8_t high)
{
    uint8_t offset = (uint8_t)(which << 1);

    if (which == VECTOR_SOFT_RESET) {
        wr8(CardSoftResetLo, low);
        wr8(CardSoftResetHi, high);
        return;
    }
    wr8((uint16_t)(CardIrqVectors + offset), low);
    wr8((uint16_t)(CardIrqVectors + (uint8_t)(offset + 1)), high);
}

void abi_ex_setvec(void)
{
    ex_setvec(cpu.a, cpu.x, cpu.y);
    if (cpu.a != VECTOR_SOFT_RESET)
        cpu.x = (uint8_t)((uint8_t)(cpu.a << 1) + 1);
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
 * Entry: nothing. Exit: A $E8, N 1, Z 0. X, Y, C, V kept.
 */
void init_card_vectors(void)
{
    wr8(CardSoftResetLo, (uint8_t)SOFT_RESET_ENTRY);
    wr8(CardSoftResetHi, (uint8_t)(SOFT_RESET_ENTRY >> 8));
    wr8(CardResetMark, RESET_MARK);
    wr8(CardZpE6, 0);
    for (unsigned i = 0; i < IRQ_VECTOR_COUNT; i++) {
        wr8((uint16_t)(CardIrqVectors + 2 * i), (uint8_t)DEFAULT_IRQ_HANDLER);
        wr8((uint16_t)(CardIrqVectors + 2 * i + 1), (uint8_t)(DEFAULT_IRQ_HANDLER >> 8));
    }
    wr8(CardIrqFlags, 0);
}

void abi_init_card_vectors(void)
{
    init_card_vectors();
    cpu.a = (uint8_t)(DEFAULT_IRQ_HANDLER >> 8);
    abi_nz(cpu.a);
}
