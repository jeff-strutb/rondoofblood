/* The BIOS's CD entries. */
#include "card.h"
#include "syscard.h"

enum {
    CD_FADE_BITS = 0x0F, CD_COMMAND_BYTES = 10, CD_RESULT_BYTES = 10,
    CD_STATUS_SUBCODE = 0x10, CD_STATUS_PCM_MORE = 0x02
};

/*
 * @card 00:F0EE clear_cd_command
 * WHAT IT DOES: Clears the CD command being built, past its first byte.
 * HOW: Zeroes bytes 1 to 9.
 * Entry: nothing. Exit: N and Z from X. A, X, Y, C, V kept.
 */
void clear_cd_command(void)
{
    for (unsigned i = 1; i < CD_COMMAND_BYTES; i++)
        wr8((uint16_t)(CardCdCommand + i), 0);
}

void abi_clear_cd_command(void)
{
    clear_cd_command();
    abi_nz(cpu.x);
}

/*
 * @card 00:F0F9 clear_cd_result
 * WHAT IT DOES: Clears what the drive last answered.
 * HOW: Zeroes all ten bytes.
 * Entry: nothing. Exit: N and Z from X. A, X, Y, C, V kept.
 */
void clear_cd_result(void)
{
    for (unsigned i = 0; i < CD_RESULT_BYTES; i++)
        wr8((uint16_t)(CardCdResult + i), 0);
}

void abi_clear_cd_result(void)
{
    clear_cd_result();
    abi_nz(cpu.x);
}

/* A disc address in _CL, _CH, _DL, high byte first; the table holds them the same way. */
static uint32_t card_disc_address(void)
{
    uint32_t address = (uint32_t)rd8(CardCl) << 16;

    address |= (uint32_t)rd8(CardCh) << 8;
    return address | rd8(CardDl);
}

static uint8_t cd_base_offset(void)
{
    return (uint8_t)(rd8(CardCdBaseIndex) * 3);
}

static uint32_t cd_base_at(uint8_t offset)
{
    uint32_t base = (uint32_t)rd8((uint16_t)(CardCdBases + offset)) << 16;

    base |= (uint32_t)rd8((uint16_t)(CardCdBases + 1 + offset)) << 8;
    return base | rd8((uint16_t)(CardCdBases + 2 + offset));
}

/*
 * @card 00:F104 add_cd_base
 * WHAT IT DOES: Adds the base the caller chose to a disc address.
 * HOW: The base table, three bytes an entry, by CardCdBaseIndex; a 24-bit add into _CL, _CH
 * and _DL.
 * Entry: _CL, _CH, _DL. Exit: A the sum's top byte, N, Z, C, V from its add, Y the table's
 * offset. X kept.
 */
void add_cd_base(void)
{
    uint32_t sum = card_disc_address() + cd_base_at(cd_base_offset());

    wr8(CardDl, (uint8_t)sum);
    wr8(CardCh, (uint8_t)(sum >> 8));
    wr8(CardCl, (uint8_t)(sum >> 16));
}

/* The top byte's add, with what carried out of the two below it. */
static void add_cd_base_exit_shim(uint32_t address, uint32_t base, uint8_t offset)
{
    uint8_t a = (uint8_t)(address >> 16);
    uint8_t b = (uint8_t)(base >> 16);
    unsigned carry = ((address & 0xFFFF) + (base & 0xFFFF)) >> 16;
    unsigned sum = (unsigned)a + b + carry;

    cpu.a = (uint8_t)sum;
    cpu.y = offset;
    abi_nz((uint8_t)sum);
    abi_flag(P_C, sum > 0xFF);
    abi_flag(P_V, (~(a ^ b) & (a ^ sum) & 0x80) != 0);
}

void abi_add_cd_base(void)
{
    uint8_t offset = cd_base_offset();
    uint32_t address = card_disc_address();
    uint32_t base = cd_base_at(offset);

    add_cd_base();
    add_cd_base_exit_shim(address, base, offset);
}

/*
 * @card 00:F309 save_card_args
 * WHAT IT DOES: Keeps _AX to _DX across a command that uses them.
 * HOW: Copies the eight bytes to CardSavedArgs.
 * Entry: nothing. Exit: N and Z from X. A, X, Y, C, V kept.
 */
void save_card_args(void)
{
    for (unsigned i = 0; i < CARD_ARG_BYTES; i++)
        wr8((uint16_t)(CardSavedArgs + i), rd8((uint16_t)(CardArgs + i)));
}

void abi_save_card_args(void)
{
    save_card_args();
    abi_nz(cpu.x);
}

/*
 * @card 00:F318 restore_card_args
 * WHAT IT DOES: Puts back what save_card_args kept.
 * HOW: Copies the eight bytes back.
 * Entry: nothing. Exit: N and Z from X. A, X, Y, C, V kept.
 */
void restore_card_args(void)
{
    for (unsigned i = 0; i < CARD_ARG_BYTES; i++)
        wr8((uint16_t)(CardArgs + i), rd8((uint16_t)(CardSavedArgs + i)));
}

void abi_restore_card_args(void)
{
    restore_card_args();
    abi_nz(cpu.x);
}

/*
 * @card 00:F327 copy_args_to_command
 * WHAT IT DOES: Puts a three-byte argument into the CD command being built.
 * HOW: Three bytes from _AL plus an offset to the command plus another.
 * Entry: X the argument's offset, Y the command's. Exit: A the last byte, N and Z from it. X,
 * Y, C, V kept.
 */
uint8_t copy_args_to_command(uint8_t from, uint8_t to)
{
    uint8_t byte = 0;

    for (unsigned i = 0; i < 3; i++) {
        /* A zero-page index wraps inside zero page. */
        byte = rd8((uint16_t)(0x2000 + (uint8_t)(0xF8 + from + i)));
        wr8((uint16_t)(CardCdCommand + to + i), byte);
    }
    return byte;
}

void abi_copy_args_to_command(void)
{
    uint8_t last = copy_args_to_command(cpu.x, cpu.y);

    cpu.a = last;
    abi_nz(last);
}

/*
 * @card 00:F337 write_vram_word_pending
 * WHAT IT DOES: Writes _BX to the VDC's data port, and marks that it was written.
 * HOW: CardVramPending 1, then the two bytes.
 * Entry: _BX. Exit: A _BH, N and Z from it. X, Y, C, V kept.
 */
uint8_t write_vram_word_pending(void)
{
    uint8_t low = rd8(CardBl);
    uint8_t high = rd8((uint16_t)(CardBl + 1));

    wr8(CardVramPending, 1);
    wr8(VdcDataLo, low);
    wr8(VdcDataHi, high);
    return high;
}

void abi_write_vram_word_pending(void)
{
    uint8_t high = write_vram_word_pending();

    cpu.a = high;
    abi_nz(high);
}

/*
 * @card 00:F354 cd_subrd
 * WHAT IT DOES: Reads a subcode byte, if one is waiting.
 * HOW: Bit 4 of the status port; the byte comes from $1807 and is kept in CardSubcodeByte.
 * Entry: nothing. Exit, one read: A the byte, N and Z from it, C clear, V bit 6 of the status.
 * Exit, none: C set, Z set, N and V bits 7 and 6 of the status, A kept. X, Y kept.
 */
CdSubcodeRead cd_subrd(void)
{
    CdSubcodeRead read = { false, 0, 0 };

    read.status = rd8(CdIrqStatus);
    if (read.status & CD_STATUS_SUBCODE) {
        read.byte = rd8(CdSubcode);
        wr8(CardSubcodeByte, read.byte);
        read.ready = true;
    }
    return read;
}

void abi_cd_subrd(void)
{
    CdSubcodeRead read = cd_subrd();

    abi_flag(P_V, (read.status & 0x40) != 0);
    abi_flag(P_C, !read.ready);
    if (read.ready) {
        cpu.a = read.byte;
        abi_nz(read.byte);
    } else {
        abi_flag(P_N, (read.status & 0x80) != 0);
        abi_flag(P_Z, true);
    }
}

/*
 * @card 00:F364 cd_pcmrd
 * WHAT IT DOES: Latches a CD audio sample and reads it.
 * HOW: A write to the latch port, a pause for it that the C does not state, then its two
 * bytes and the status port's bit 1.
 * Entry: A what goes to the latch. Exit: X the low byte, Y the high, A the bit, N 0, Z from
 * it, C clear. V kept.
 */
CdPcmRead cd_pcmrd(uint8_t latch)
{
    CdPcmRead read;

    wr8(CdPcmLatch, latch);
    read.low = rd8(CdPcmLatch);
    read.high = rd8(CdPcmHigh);
    read.more = (rd8(CdIrqStatus) & CD_STATUS_PCM_MORE) >> 1;
    return read;
}

void abi_cd_pcmrd(void)
{
    CdPcmRead read = cd_pcmrd(cpu.a);

    cpu.x = read.low;
    cpu.y = read.high;
    cpu.a = read.more;
    abi_nz(read.more);
    abi_flag(P_C, false);
}

/*
 * @card 00:F379 cd_fade
 * WHAT IT DOES: Starts or stops the CD and ADPCM volume fade.
 * HOW: The low nibble of the type goes to the fade port.
 * Entry: A type. Exit: A the nibble, N and Z from it. X, Y, C, V kept.
 */
void cd_fade(uint8_t type)
{
    wr8(CdFade, type & CD_FADE_BITS);
}

void abi_cd_fade(void)
{
    uint8_t nibble = cpu.a & CD_FADE_BITS;

    cd_fade(cpu.a);
    cpu.a = nibble;
    abi_nz(nibble);
}
