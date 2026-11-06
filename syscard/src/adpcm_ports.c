/* The ADPCM unit's address and data: strobes on its control port. */
#include "card.h"
#include "syscard.h"

enum {
    AD_STROBE_READ = 0x08, AD_STROBE_WRITE_SET = 0x03, AD_STROBE_WRITE_ONE = 0x01,
    AD_STROBE_WRITE_TWO = 0x02, AD_STROBE_LENGTH = 0x10, AD_STATUS_READING = 0x80
};

static void set_control_bits(uint8_t bits)
{
    wr8(AdpcmControl, rd8(AdpcmControl) | bits);
}

/* Clears bits, and gives what the port held. */
static uint8_t clear_control_bits(uint8_t bits)
{
    uint8_t control = rd8(AdpcmControl);

    wr8(AdpcmControl, (uint8_t)(control & ~bits));
    return control;
}

/* A clearing strobe's flags: A the bits, Z if nothing else was set, N and V from the port. */
static void strobe_exit_shim(uint8_t bits, uint8_t control)
{
    cpu.a = bits;
    abi_flag(P_Z, (control & ~bits) == 0);
    abi_flag(P_N, (control & 0x80) != 0);
    abi_flag(P_V, (control & 0x40) != 0);
}

/*
 * @card 00:F729 ad_set_address
 * WHAT IT DOES: Gives the ADPCM unit an address.
 * HOW: Two bytes to its address ports.
 * Entry: X low, Y high. Exit: all registers and flags kept.
 */
void ad_set_address(uint8_t low, uint8_t high)
{
    wr8(AdpcmAddressLo, low);
    wr8(AdpcmAddressHi, high);
}

void abi_ad_set_address(void)
{
    ad_set_address(cpu.x, cpu.y);
}

/*
 * @card 00:F6FF ad_latch_read_address
 * WHAT IT DOES: Makes the address just given the one the ADPCM unit reads from.
 * HOW: A strobe around a dummy read of the data port, with a pause the C does not state.
 * Entry: nothing. Exit: A $08, Z if nothing else was set in the port, N and V bits 7 and 6 of
 * what it held. X, Y, C kept.
 */
uint8_t ad_latch_read_address(void)
{
    set_control_bits(AD_STROBE_READ);
    rd8(AdpcmData);
    return clear_control_bits(AD_STROBE_READ);
}

void abi_ad_latch_read_address(void)
{
    strobe_exit_shim(AD_STROBE_READ, ad_latch_read_address());
}

/*
 * @card 00:F710 ad_latch_write_address
 * WHAT IT DOES: Makes the address just given the one the ADPCM unit writes to.
 * HOW: Sets two strobes and clears them in turn.
 * Entry: nothing. Exit: A $02, Z if nothing else was set in the port, N and V bits 7 and 6 of
 * what it held. X, Y, C kept.
 */
uint8_t ad_latch_write_address(void)
{
    set_control_bits(AD_STROBE_WRITE_SET);
    clear_control_bits(AD_STROBE_WRITE_ONE);
    return clear_control_bits(AD_STROBE_WRITE_TWO);
}

void abi_ad_latch_write_address(void)
{
    strobe_exit_shim(AD_STROBE_WRITE_TWO, ad_latch_write_address());
}

/*
 * @card 00:F71E ad_latch_length
 * WHAT IT DOES: Makes the address just given the length the ADPCM unit plays.
 * HOW: A strobe.
 * Entry: nothing. Exit: A $10, Z if nothing else was set in the port, N and V bits 7 and 6 of
 * what it held. X, Y, C kept.
 */
uint8_t ad_latch_length(void)
{
    set_control_bits(AD_STROBE_LENGTH);
    return clear_control_bits(AD_STROBE_LENGTH);
}

void abi_ad_latch_length(void)
{
    strobe_exit_shim(AD_STROBE_LENGTH, ad_latch_length());
}

/*
 * @card 00:F3FE ad_read_byte
 * WHAT IT DOES: Reads the next byte of ADPCM memory.
 * HOW: Waits for the unit to finish fetching, then its data port.
 * Entry: nothing. Exit: A the byte, N and Z from it. X, Y, C, V kept.
 */
uint8_t ad_read_byte(void)
{
    while (rd8(AdpcmStatus) & AD_STATUS_READING)
        ;
    return rd8(AdpcmData);
}

void abi_ad_read_byte(void)
{
    uint8_t byte = ad_read_byte();

    cpu.a = byte;
    abi_nz(byte);
}
