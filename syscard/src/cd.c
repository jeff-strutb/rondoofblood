/*
 * The BIOS's CD entries: the command and result buffers, the saved arguments, the base added to a disc address.
 *
 * Bytes named by address, not yet by field: the command and result buffers, the saved arguments, the base
 * table; their array rows are asked of the typed access session. The rest is typed.
 */
#include "card.h"
#include "card_access.h"
#include "syscard.h"

enum {
    CD_FADE_BITS = 0x0F, CD_COMMAND_BYTES = 10, CD_RESULT_BYTES = 10,
    CD_STATUS_SUBCODE = 0x10, CD_STATUS_PCM_MORE = 0x02, ARG_LAST = 7, PCM_PAUSE = 0x1B,
    VRAM_PENDING = 1, COPY_BYTES = 3, BASE_STRIDE_PLUS = 2
};

#define ARG(i) ((uint16_t)(0x2000 + (uint8_t)(0xF8 + (i))))

/*
 * @card 00:F0EE clear_cd_command
 * WHAT IT DOES: Clears the CD command being built, past its first byte.
 * HOW: X is pushed, bytes 9 down to 1 are zeroed, X is pulled.
 * WAITS: no.
 * Entry: X (only kept: it goes on the stack and comes back). Exit: N and Z from X. A, X, Y, C, V kept.
 */
void clear_cd_command(uint8_t x)
{
    push8(x);
    for (unsigned i = CD_COMMAND_BYTES - 1; i != 0; i--) {
        WR8_IDX(COL(CRAM->cd_command), i, 0);
    }
    (void)pull8();
}

void abi_clear_cd_command(void)
{
    clear_cd_command(cpu.x);
    abi_nz(cpu.x);
}

/*
 * @card 00:F0F9 clear_cd_result
 * WHAT IT DOES: Clears what the drive last answered.
 * HOW: X is pushed, all ten bytes are zeroed from the last, X is pulled.
 * WAITS: no.
 * Entry: X (only kept). Exit: N and Z from X. A, X, Y, C, V kept.
 */
void clear_cd_result(uint8_t x)
{
    push8(x);
    for (int i = CD_RESULT_BYTES - 1; i >= 0; i--) {
        WR8_IDX(COL(CRAM->cd_result), (unsigned)i, 0);
    }
    (void)pull8();
}

void abi_clear_cd_result(void)
{
    clear_cd_result(cpu.x);
    abi_nz(cpu.x);
}

/*
 * @card 00:F104 add_cd_base
 * WHAT IT DOES: Adds the base the caller chose to a disc address.
 * HOW: The base table, three bytes an entry, by CardCdBaseIndex; a 24-bit add into _CL, _CH
 * and _DL.
 * WAITS: no.
 * Entry: _CL, _CH, _DL. Exit: A the sum's top byte, N, Z, C, V from its add, Y the table's
 * offset. X kept.
 */
CardExit add_cd_base(void)
{
    CardExit e = { 0 };
    Sum low;
    Sum middle;
    Sum high;
    uint8_t index = RD8(CRAM->cd_base_index);
    uint8_t twice = (uint8_t)(index << 1);
    uint8_t again = RD8(CRAM->cd_base_index);
    uint8_t y = add8(twice, again, false).value;
    uint8_t value;
    uint8_t base;

    value = RD8(CZP->d.lo);
    base = RD8_IDX(COL(CRAM->cd_bases), 2u + y);
    low = add8(value, base, false);
    WR8(CZP->d.lo, low.value);
    value = RD8(CZP->c.hi);
    base = RD8_IDX(COL(CRAM->cd_bases), 1u + y);
    middle = add8(value, base, low.carry);
    WR8(CZP->c.hi, middle.value);
    value = RD8(CZP->c.lo);
    base = RD8_IDX(COL(CRAM->cd_bases), y);
    high = add8(value, base, middle.carry);
    WR8(CZP->c.lo, high.value);
    e.a = e.nz = high.value;
    e.y = y;
    e.c = high.carry;
    e.v = high.overflow;
    e.set_a = e.set_nz = e.set_y = e.set_c = e.set_v = true;
    return e;
}

void abi_add_cd_base(void)
{
    card_exit_apply(add_cd_base());
}

/*
 * @card 00:F309 save_card_args
 * WHAT IT DOES: Keeps _AX to _DX across a command that uses them.
 * HOW: X and A are pushed, the eight bytes go to CardSavedArgs from the last, A and X are pulled.
 * WAITS: no.
 * Entry: A and X (only kept). Exit: N and Z from X. A, X, Y, C, V kept.
 */
void save_card_args(uint8_t a, uint8_t x)
{
    push8(x);
    push8(a);
    for (int i = ARG_LAST; i >= 0; i--) {
        uint8_t value = RD8_IDX(COL(CZP->args), i);

        WR8_IDX(COL(CRAM->saved_args), (unsigned)i, value);
    }
    (void)pull8();
    (void)pull8();
}

void abi_save_card_args(void)
{
    save_card_args(cpu.a, cpu.x);
    abi_nz(cpu.x);
}

/*
 * @card 00:F318 restore_card_args
 * WHAT IT DOES: Puts back what save_card_args kept.
 * HOW: As save_card_args, the eight bytes the other way.
 * WAITS: no.
 * Entry: A and X (only kept). Exit: N and Z from X. A, X, Y, C, V kept.
 */
void restore_card_args(uint8_t a, uint8_t x)
{
    push8(x);
    push8(a);
    for (int i = ARG_LAST; i >= 0; i--) {
        uint8_t value = RD8_IDX(COL(CRAM->saved_args), (unsigned)i);

        WR8_IDX(COL(CZP->args), i, value);
    }
    (void)pull8();
    (void)pull8();
}

void abi_restore_card_args(void)
{
    restore_card_args(cpu.a, cpu.x);
    abi_nz(cpu.x);
}

/*
 * @card 00:F327 copy_args_to_command
 * WHAT IT DOES: Puts a three-byte argument into the CD command being built.
 * HOW: Three bytes from _AL plus an offset to the command plus another.
 * WAITS: no.
 * Entry: X the argument's offset, Y the command's. Exit: A the last byte, N and Z from it. X,
 * Y, C, V kept.
 */
uint8_t copy_args_to_command(uint8_t x, uint8_t y)
{
    uint8_t value = 0;

    for (unsigned i = 0; i < COPY_BYTES; i++) {
        value = rd8(ARG(x + i));    /* a zero page index wraps inside the page: no typed column wraps */
        WR8_IDX(COL(CRAM->cd_command), (unsigned)y + i, value);
    }
    return value;
}

void abi_copy_args_to_command(void)
{
    cpu.a = copy_args_to_command(cpu.x, cpu.y);
    abi_nz(cpu.a);
}

/*
 * @card 00:F337 write_vram_word_pending
 * WHAT IT DOES: Writes _BX to the VDC's data port, and marks that it was written.
 * HOW: CardVramPending 1, then the two bytes, each read just before it is written.
 * WAITS: no.
 * Entry: _BX. Exit: A _BH, N and Z from it. X, Y, C, V kept.
 */
uint8_t write_vram_word_pending(void)
{
    uint8_t low;
    uint8_t high;

    WR8(CRAM->vram_pending, VRAM_PENDING);
    low = RD8(CZP->b.lo);
    WR8(CVDC->data_lo, low);
    high = RD8(CZP->b.hi);
    WR8(CVDC->data_hi, high);
    return high;
}

void abi_write_vram_word_pending(void)
{
    cpu.a = write_vram_word_pending();
    abi_nz(cpu.a);
}

/*
 * @card 00:F354 cd_subrd
 * WHAT IT DOES: Reads a subcode byte, if one is waiting.
 * HOW: Bit 4 of the status port; the byte comes from $1807 and is kept in CardSubcodeByte.
 * WAITS: no.
 * Entry: nothing. Exit, one read: A the byte, N and Z from it, C clear, V bit 6 of the status.
 * Exit, none: C set, Z set, N and V bits 7 and 6 of the status, A kept. X, Y kept.
 */
CdSubcodeRead cd_subrd(void)
{
    CdSubcodeRead read = { false, 0, 0 };

    read.status = RD8(CCDU->irq_status);
    if (read.status & CD_STATUS_SUBCODE) {
        read.byte = RD8(CCDU->subcode);
        WR8(CRAM->subcode_byte, read.byte);
        read.ready = true;
    }
    return read;
}

/* cd_subrd's exit, for its wrapper and for a caller's that ends in it. */
void cd_subrd_exit_shim(CdSubcodeRead read)
{
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

void abi_cd_subrd(void)
{
    cd_subrd_exit_shim(cd_subrd());
}

/*
 * @card 01:CB54 reset_cd_unit
 * WHAT IT DOES: Puts the CD unit in a known state at boot: reset, its interrupts off, no fade,
 * ADPCM reset, and any waiting subcode byte taken.
 * HOW: cd_reset, then the interrupt enables and the fade port cleared, ad_reset, cd_subrd.
 * WAITS: no.
 * Entry: nothing. Exit: X 0, Y 0; A, N, Z, C and V as cd_subrd leaves them, A $6F if no byte.
 * Not run by any script and with no draft: no unit call reaches it; its calls are framed as the assembly makes them.
 */
CdSubcodeRead reset_cd_unit(void)
{
    CdSubcodeRead read;

    jsr_push(0xCB57);
    (void)cd_reset();
    jsr_pop();
    WR8(CCDU->irq_control, 0);
    WR8(CCDU->fade, 0);
    jsr_push(0xCB60);
    (void)ad_reset();
    jsr_pop();
    jsr_push(0xCB63);
    read = cd_subrd();
    jsr_pop();
    return read;
}

void abi_reset_cd_unit(void)
{
    CdSubcodeRead read = reset_cd_unit();

    cpu.x = 0;
    cpu.y = 0;
    cpu.a = 0x6F;
    cd_subrd_exit_shim(read);
}

/*
 * @card 00:F364 cd_pcmrd
 * WHAT IT DOES: Latches a CD audio sample and reads it.
 * HOW: A write to the latch port, a pause for it that the C does not state, then its two
 * bytes and the status port's bit 1.
 * WAITS: no.
 * Entry: A what goes to the latch. Exit: X the low byte, Y the high, A the bit, N 0, Z from
 * it, C clear. V kept.
 * Not run by any script.
 */
CdPcmRead cd_pcmrd(uint8_t latch)
{
    CdPcmRead read;
    uint8_t status;

    WR8(CCDU->pcm_latch, latch);
    read.low = RD8(CCDU->pcm_latch);
    read.high = RD8(CCDU->pcm_high);
    status = RD8(CCDU->irq_status);
    read.more = (uint8_t)((status & CD_STATUS_PCM_MORE) >> 1);
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
 * WAITS: no.
 * Entry: A type. Exit: A the nibble, N and Z from it. X, Y, C, V kept.
 */
uint8_t cd_fade(uint8_t type)
{
    uint8_t nibble = (uint8_t)(type & CD_FADE_BITS);

    WR8(CCDU->fade, nibble);
    return nibble;
}

void abi_cd_fade(void)
{
    cd_fade_exit_shim(cd_fade(cpu.a));
}

/* cd_fade's exit, for its wrapper and for a caller's that ends in it: the nibble it returned. */
void cd_fade_exit_shim(uint8_t nibble)
{
    cpu.a = nibble;
    abi_nz(nibble);
}
