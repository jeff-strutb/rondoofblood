/* The BIOS's ADPCM entries. */
#include "card.h"
#include "card_access.h"
#include "syscard.h"

enum {
    AD_IRQ_BITS = 0x0C, AD_CONTROL_PLAY_BITS = 0x60,
    AD_STATUS_BUSY = 0x01, CD_IRQ_ADPCM_HALF = 0x04,
    AD_CONTROL_PLAYING = 0x20, AD_STATUS_PLAYING = 0x08
};

/*
 * @card 00:F6C1 ad_stop
 * WHAT IT DOES: Stops ADPCM playback and forgets what was playing.
 * HOW: Clears its interrupt enables and its control bits, then the card's play state.
 * Entry: nothing. Exit: A $60; N and V bits 7 and 6 of the control port as it read, Z if
 * nothing else was set in it. X, Y, C kept.
 */
uint8_t ad_stop(void)
{
    uint8_t control;

    wr8(CdIrqControl, (uint8_t)(rd8(CdIrqControl) & ~AD_IRQ_BITS));
    control = rd8(AdpcmControl);
    wr8(AdpcmControl, (uint8_t)(control & ~AD_CONTROL_PLAY_BITS));
    wr8(CardAdpcmActive, 0);
    wr8(CardAdpcmStep, 0);
    wr8(CardAdpcmSectorsLeft, 0);
    wr8((uint16_t)(CardAdpcmSectorsLeft + 1), 0);
    wr8((uint16_t)(CardAdpcmSectorsLeft + 2), 0);
    return control;
}

void abi_ad_stop(void)
{
    ad_stop_exit_shim(ad_stop());
}

/* ad_stop's exit, for its wrapper and for a caller's that ends in it. */
void ad_stop_exit_shim(uint8_t control)
{
    cpu.a = AD_CONTROL_PLAY_BITS;
    abi_flag(P_N, (control & 0x80) != 0);
    abi_flag(P_V, (control & 0x40) != 0);
    abi_flag(P_Z, (control & ~AD_CONTROL_PLAY_BITS) == 0);
}

/*
 * @card 00:F6DB ad_stat
 * WHAT IT DOES: Says whether ADPCM is busy and whether it is playing.
 * HOW: Each answer is one port's bit, or if clear another's.
 * Entry: nothing. Exit: X busy, A playing, N and Z from A. Y, C, V kept.
 */
AdStat ad_stat(void)
{
    AdStat stat;

    stat.busy = rd8(AdpcmStatus) & AD_STATUS_BUSY;
    if (stat.busy == 0)
        stat.busy = rd8(CdIrqStatus) & CD_IRQ_ADPCM_HALF;
    stat.playing = rd8(AdpcmControl) & AD_CONTROL_PLAYING;
    if (stat.playing == 0)
        stat.playing = rd8(AdpcmStatus) & AD_STATUS_PLAYING;
    return stat;
}

void abi_ad_stat(void)
{
    ad_stat_exit_shim(ad_stat());
}

/* ad_stat's exit, for its wrapper and for a caller's that ends in it. */
void ad_stat_exit_shim(AdStat stat)
{
    cpu.x = stat.busy;
    cpu.a = stat.playing;
    abi_nz(stat.playing);
}

enum {
    AD_CONTROL_RESET = 0x80, AD_RESET_IRQ_BITS = 0x6F, AD_IRQ_HALF_ENABLE = 0x08,
    AD_PLAY_REPEAT_BIT = 0x80, AD_PLAY_MODE_BITS = 0x7F, AD_RATE_LIMIT = 0x10
};

/*
 * @card 00:F37F ad_reset
 * WHAT IT DOES: Resets the ADPCM unit.
 * HOW: Pulses its reset bit, stops any transfer from the CD, turns off the CD unit's interrupt
 * enables but the bus acknowledge, and sets the playback rate to zero.
 * Entry: nothing. Exit: A $6F, N and V bits 7 and 6 of the interrupt enables as they were, Z if
 * nothing outside $6F was set in them. X, Y, C kept.
 */
uint8_t ad_reset(void)
{
    uint8_t enables;

    wr8(AdpcmControl, AD_CONTROL_RESET);
    wr8(AdpcmControl, 0);
    wr8(AdpcmDma, 0);
    enables = rd8(CdIrqControl);
    wr8(CdIrqControl, (uint8_t)(enables & ~AD_RESET_IRQ_BITS));
    wr8(AdpcmRate, 0);
    return enables;
}

void abi_ad_reset(void)
{
    uint8_t enables = ad_reset();

    cpu.a = AD_RESET_IRQ_BITS;
    abi_flag(P_N, (enables & 0x80) != 0);
    abi_flag(P_V, (enables & 0x40) != 0);
    abi_flag(P_Z, (enables & ~AD_RESET_IRQ_BITS) == 0);
}

static void latch_word(uint16_t at)
{
    ad_set_address(rd8(at), rd8((uint16_t)(at + 1)));
}

/*
 * @card 00:F5C6 ad_play
 * WHAT IT DOES: Plays ADPCM: _AX bytes from address _BX at rate _DH; bit 7 of _DL repeats it,
 * and any other bit of _DL plays what is already latched.
 * HOW: Does nothing while ADPCM plays. Keeps _BX, _AX and the rate, latches the address and the
 * length, sets the rate, then turns on the half-way interrupt and starts the unit.
 * Entry: _AX, _BX, _DL, _DH. Exit, busy: as ad_stat. Bad rate: A _DH, C set, N and Z from
 * comparing it with $10, X as ad_stat. Played: A 0, N 0, Z 0, V bit 6 of the interrupt enables
 * as they were; latched: X and Y _AX, C clear; played again: X as ad_stat. Y, C, V kept
 * otherwise.
 */
AdPlay ad_play(void)
{
    AdPlay play = { AD_PLAY_BUSY, { 0, 0 }, 0, 0, 0, 0 };
    uint8_t mode;

    play.stat = ad_stat();
    if (play.stat.playing != 0)
        return play;
    mode = rd8(CardDl);
    wr8(CardAdpcmActive, mode & AD_PLAY_REPEAT_BIT);
    if ((mode & AD_PLAY_MODE_BITS) == 0) {
        wr8(CardAdpcmAddress, rd8(CardBl));
        wr8((uint16_t)(CardAdpcmAddress + 1), rd8(CardBh));
        wr8(CardAdpcmLength, rd8(CardAl));
        wr8((uint16_t)(CardAdpcmLength + 1), rd8(CardAh));
        play.rate = rd8(CardDh);
        if (play.rate >= AD_RATE_LIMIT) {
            play.how = AD_PLAY_BAD_RATE;
            return play;
        }
        wr8(CardAdpcmRate, play.rate);
        latch_word(CardAdpcmAddress);
        ad_latch_read_address();
        latch_word(CardAdpcmLength);
        ad_latch_length();
        play.length_low = rd8(CardAdpcmLength);
        play.length_high = rd8((uint16_t)(CardAdpcmLength + 1));
        wr8(AdpcmRate, rd8(CardAdpcmRate));
        play.how = AD_PLAY_STARTED;
    } else {
        play.how = AD_PLAY_AGAIN;
    }
    play.irq_before = rd8(CdIrqControl);
    wr8(CdIrqControl, play.irq_before | AD_IRQ_HALF_ENABLE);
    wr8(AdpcmControl, AD_CONTROL_PLAY_BITS);
    return play;
}

void abi_ad_play(void)
{
    ad_play_exit_shim(ad_play());
}

/* ad_play's exit, for its wrapper and for a caller's that ends in it. */
void ad_play_exit_shim(AdPlay play)
{
    cpu.x = play.stat.busy;
    if (play.how == AD_PLAY_BUSY) {
        cpu.a = play.stat.playing;
        abi_nz(cpu.a);
        return;
    }
    if (play.how == AD_PLAY_BAD_RATE) {
        cpu.a = play.rate;
        abi_nz((uint8_t)(play.rate - AD_RATE_LIMIT));
        abi_flag(P_C, true);
        return;
    }
    if (play.how == AD_PLAY_STARTED) {
        cpu.x = play.length_low;
        cpu.y = play.length_high;
        abi_flag(P_C, false);
    }
    cpu.a = 0;
    abi_nz(AD_CONTROL_PLAY_BITS);
    abi_flag(P_V, (play.irq_before & 0x40) != 0);
}

enum {
    AD_READ_TO_MEMORY = 0x00, AD_READ_TO_VRAM = 0xFF, AD_READ_BANK_LIMIT = 0x07,
    AD_READ_REFUSED = 0x22, STUB_TMA = 0x43, STUB_TAM = 0x53, STUB_RTS = 0x60,
    BANK_PAGES = 0x20, BANK_WINDOW_SHIFT = 5, VDC_SELECT_MAWR = 0x00, VDC_SELECT_VWR = 0x02,
    VDC_DATA_PORT = 0x0002
};

/* Takes one from the 16-bit count in _AX, and says whether any is left. */
static bool count_down_ax(void)
{
    uint16_t left = (uint16_t)((rd8(CardAl) | (rd8(CardAh) << 8)) - 1);

    wr8(CardAl, (uint8_t)left);
    wr8(CardAh, (uint8_t)(left >> 8));
    return left != 0;
}

static void select_vdc(uint8_t reg)
{
    wr8(CardVdcReg, reg);
    wr8(VdcAddress, reg);
}

/* The MPR stub the card builds in RAM: TMA or TAM with the mask, then RTS. */
static void build_mpr_stub(uint8_t opcode, uint8_t mask)
{
    wr8(CardMprStub, opcode);
    wr8(CardMprStubMask, mask);
    wr8(CardMprStubReturn, STUB_RTS);
}

/*
 * @card 00:F6F5 aim_bx_at_vram_port
 * WHAT IT DOES: Makes _BX the VDC's write address, then points _BX at the VDC's data port, for a
 * copy that writes through [_BX].
 * HOW: write_vram_word_pending, with the address register selected; then _BX = $0002.
 * Entry: _BX, the VDC's address register selected. Exit: A 2, N 0, Z 0. X, Y, C, V kept.
 */
void aim_bx_at_vram_port(void)
{
    write_vram_word_pending();
    wr8(CardBl, (uint8_t)VDC_DATA_PORT);
    wr8(CardBh, (uint8_t)(VDC_DATA_PORT >> 8));
}

void abi_aim_bx_at_vram_port(void)
{
    aim_bx_at_vram_port();
    cpu.a = (uint8_t)VDC_DATA_PORT;
    abi_nz(cpu.a);
}

/*
 * @card 00:F407 ad_read
 * WHAT IT DOES: Reads _AX bytes of ADPCM memory from address _CX: to memory at _BX if _DH is 0,
 * to VRAM at _BX if _DH is $FF, or else to banks from _BL on, mapped through the MPR numbered _DH
 * (1 to 6). Any other _DH gives $22.
 * HOW: Waits until ADPCM is not playing, latches the address, drops the byte the unit had
 * fetched, then reads byte after byte. VRAM goes through the data port, its two halves in turn;
 * banks through a TMA or TAM stub it writes in RAM, 8 KB a bank, the MPR put back at the end.
 * Entry: _AX, _BX, _CX, _DH, and _BL for banks. Exit: A 0, or $22 refused. See abi_ad_read.
 */
AdRead ad_read(void)
{
    AdRead read = { 0 };

    do
        read.stat = ad_stat();
    while (read.stat.playing != 0);
    ad_set_address(rd8(CardCl), rd8(CardCh));
    read.latch = ad_latch_read_address();
    ad_read_byte();
    read.mode = rd8(CardDh);
    if (read.mode == AD_READ_TO_MEMORY) {
        do {
            uint16_t at = (uint16_t)(rd8(CardBl) | (rd8(CardBh) << 8));

            wr8(at, ad_read_byte());
            at++;
            wr8(CardBl, (uint8_t)at);
            wr8(CardBh, (uint8_t)(at >> 8));
        } while (count_down_ax());
        return read;
    }
    if (read.mode == AD_READ_TO_VRAM) {
        select_vdc(VDC_SELECT_MAWR);
        aim_bx_at_vram_port();
        select_vdc(VDC_SELECT_VWR);
        do {
            wr8(rd8(CardBl), ad_read_byte());
            wr8(CardBl, rd8(CardBl) ^ 1);
        } while (count_down_ax());
        wr8(CardVramPending, 0);
        return read;
    }
    ad_read_to_banks(&read);
    return read;
}

/*
 * @card 00:F46B ad_read_to_banks
 * WHAT IT DOES: ad_read's reading to banks: _AX bytes of ADPCM memory to bank _BL and the banks
 * after it, through the MPR numbered _DH (A on entry). A number of 7 or more is refused with $22.
 * HOW: A TMA stub written in RAM keeps the bank the MPR held; each bank is mapped with a TAM stub
 * and filled, 8 KB, at the MPR's window; the kept bank goes back at the end.
 * Entry: A _DH, the ADPCM address latched. Exit, refused: A $22, N 0, Z 0, C from comparing with
 * 7. Read: A 0, N and Z from the bank put back, C bit 3 of _DH, X the pages and Y the bytes left
 * of the last bank. V kept.
 */
void ad_read_to_banks(AdRead *read)
{
    if ((int8_t)(read->mode - AD_READ_BANK_LIMIT) >= 0) {
        read->refused = true;
        return;
    }
    read->mask = (uint8_t)(read->mode < 8 ? 1u << read->mode : 0);
    build_mpr_stub(STUB_TMA, read->mask);
    read->saved_bank = tma(read->mask);
    wr8(CardSavedMprBank, read->saved_bank);
    wr8(CardDl, rd8(CardBl));
    for (;;) {
        uint16_t at = (uint16_t)((uint8_t)(read->mode << BANK_WINDOW_SHIFT) << 8);
        bool more = true;

        wr8(CardBl, 0);
        wr8(CardBh, (uint8_t)(at >> 8));
        wr8(CardMprStub, STUB_TAM);
        tam(read->mask, rd8(CardDl));
        for (read->pages_left = BANK_PAGES; read->pages_left != 0 && more; ) {
            for (read->bytes_left = 0; ; ) {
                wr8(at, ad_read_byte());
                at++;
                wr8(CardBl, (uint8_t)at);
                wr8(CardBh, (uint8_t)(at >> 8));
                more = count_down_ax();
                if (!more)
                    break;
                read->bytes_left--;
                if (read->bytes_left == 0)
                    break;
            }
            if (more)
                read->pages_left--;
        }
        if (!more)
            break;
        wr8(CardDl, (uint8_t)(rd8(CardDl) + 1));
    }
    wr8(CardMprStub, STUB_TAM);
    tam(read->mask, rd8(CardSavedMprBank));
}

void abi_ad_read_to_banks(void)
{
    AdRead read = { 0 };

    read.mode = rd8(CardDh);
    ad_read_to_banks(&read);
    if (read.refused) {
        cpu.a = AD_READ_REFUSED;
        abi_nz(AD_READ_REFUSED);
        abi_flag(P_C, read.mode >= AD_READ_BANK_LIMIT);
        return;
    }
    cpu.a = 0;
    cpu.x = read.pages_left;
    cpu.y = read.bytes_left;
    abi_nz(read.saved_bank);
    abi_flag(P_C, (read.mode & 0x08) != 0);
}

/*
 * ad_read's exits: X _CL and Y _CH, V bit 6 of the control port as the latch read it. To memory
 * or VRAM: A 0, N 0, Z 1, C kept. Refused: A $22, N 0, Z 0, C from comparing _DH with 7. Banks:
 * A 0, N and Z from the bank put back, C bit 3 of _DH, X the pages and Y the bytes left of the
 * last bank.
 */
void abi_ad_read(void)
{
    uint8_t low = rd8(CardCl);
    uint8_t high = rd8(CardCh);
    AdRead read = ad_read();

    cpu.x = low;
    cpu.y = high;
    abi_flag(P_V, (read.latch & 0x40) != 0);
    if (read.refused) {
        cpu.a = AD_READ_REFUSED;
        abi_nz(AD_READ_REFUSED);
        abi_flag(P_C, read.mode >= AD_READ_BANK_LIMIT);
        return;
    }
    cpu.a = 0;
    if (read.mode == AD_READ_TO_MEMORY || read.mode == AD_READ_TO_VRAM) {
        abi_nz(0);
        return;
    }
    cpu.x = read.pages_left;
    cpu.y = read.bytes_left;
    abi_nz(read.saved_bank);
    abi_flag(P_C, (read.mode & 0x08) != 0);
}


enum {
    AD_DMA_BITS = 0x03, AD_ADDRESS_CARRY_ON = 0xFF, AD_STATUS_WRITING = 0x04,
    VDC_SELECT_MARR = 0x01
};

/* Waits for the unit to take the byte before; gives the status as the wait last read it. */
static uint8_t wait_for_adpcm_write(void)
{
    uint8_t status;

    do
        status = rd8(AdpcmStatus);
    while (status & AD_STATUS_WRITING);
    return status;
}

static uint16_t card_bx(void)
{
    uint8_t low = rd8(CardBl);

    return (uint16_t)(low | (rd8(CardBh) << 8));
}

static void set_card_bx(uint16_t value)
{
    wr8(CardBl, (uint8_t)value);
    wr8(CardBh, (uint8_t)(value >> 8));
}

/* One bank of ad_write's writing from banks: true while bytes are left for the next. */
static bool write_from_bank(AdWrite *write)
{
    uint16_t at = (uint16_t)((uint8_t)(write->mode << BANK_WINDOW_SHIFT) << 8);

    set_card_bx(at);
    wr8(CardMprStub, STUB_TAM);
    tam(write->mask, rd8(CardDl));
    write->pages_left = BANK_PAGES;
    write->bytes_left = 0;
    for (;;) {
        write->last_status = wait_for_adpcm_write();
        wr8(AdpcmData, rd8(at));
        at++;
        set_card_bx(at);
        if (!count_down_ax())
            return false;
        write->bytes_left--;
        if (write->bytes_left != 0)
            continue;
        write->pages_left--;
        if (write->pages_left == 0)
            return true;
    }
}

static void write_from_banks(AdWrite *write)
{
    if ((int8_t)(write->mode - AD_READ_BANK_LIMIT) >= 0) {
        write->refused = true;
        return;
    }
    write->mask = (uint8_t)(write->mode < 8 ? 1u << write->mode : 0);
    build_mpr_stub(STUB_TMA, write->mask);
    write->saved_bank = tma(write->mask);
    wr8(CardSavedMprBank, write->saved_bank);
    wr8(CardDl, rd8(CardBl));
    while (write_from_bank(write))
        wr8(CardDl, (uint8_t)(rd8(CardDl) + 1));
    wr8(CardMprStub, STUB_TAM);
    tam(write->mask, rd8(CardSavedMprBank));
}

/*
 * @card 00:F4D8 ad_write
 * WHAT IT DOES: Writes _AX bytes to ADPCM memory at address _CX, or where the last write left
 * off if _CX is $FFFF: from memory at _BX if _DH is 0, from VRAM at _BX if _DH is $FF, or else
 * from banks from _BL on, mapped through the MPR numbered _DH (1 to 6; $87 to $FE pass the same
 * test). _DH of 7 to $86 gives $22. Refused while a transfer from the CD runs.
 * HOW: Latches the address unless _CX is $FFFF, then byte after byte, waiting for the unit to
 * take each. VRAM comes through the VDC's read address and data port, its two halves in turn;
 * banks through a TMA or TAM stub it writes in RAM, 8 KB a bank, the MPR put back at the end.
 * Entry: _AX, _BX, _CX, _DH, and _BL for banks. Exit, a transfer running: A its bits, N 0, Z 0;
 * X, Y, C, V kept. Else, if _CX was latched, X _CL, Y _CH, C 0, V bit 6 of the control port as
 * the latch read it; if not, X, Y, V kept and C 1. Bad _DH: A $22, N 0, Z 0, C 1. Written: A 0,
 * V bit 6 of the status as the last wait read it; from memory N 0, Z 1; from VRAM N 0, Z 1, C 1;
 * from banks N and Z from the bank put back, C bit 3 of _DH, X the pages and Y the bytes left of
 * the last bank.
 */
AdWrite ad_write(void)
{
    AdWrite write = { 0 };

    write.busy = rd8(AdpcmDma) & AD_DMA_BITS;
    if (write.busy != 0)
        return write;
    write.address_low = rd8(CardCl);
    write.address_high = rd8(CardCh);
    write.addressed = (write.address_low & write.address_high) != AD_ADDRESS_CARRY_ON;
    if (write.addressed) {
        ad_set_address(write.address_low, write.address_high);
        write.latch = ad_latch_write_address();
    }
    write.mode = rd8(CardDh);
    if (write.mode == AD_READ_TO_MEMORY) {
        do {
            uint16_t at = card_bx();

            wr8(AdpcmData, rd8(at));
            write.last_status = wait_for_adpcm_write();
            set_card_bx((uint16_t)(at + 1));
        } while (count_down_ax());
        return write;
    }
    if (write.mode == AD_READ_TO_VRAM) {
        select_vdc(VDC_SELECT_MARR);
        aim_bx_at_vram_port();
        select_vdc(VDC_SELECT_VWR);
        do {
            write.last_status = wait_for_adpcm_write();
            wr8(AdpcmData, rd8(rd8(CardBl)));
            wr8(CardBl, rd8(CardBl) ^ 1);
        } while (count_down_ax());
        wr8(CardVramPending, 0);
        return write;
    }
    write_from_banks(&write);
    return write;
}

/* ad_write's exit, for its wrapper and for a caller's that ends in it. */
void ad_write_exit_shim(AdWrite write)
{
    if (write.busy != 0) {
        cpu.a = write.busy;
        abi_nz(write.busy);
        return;
    }
    if (write.addressed) {
        cpu.x = write.address_low;
        cpu.y = write.address_high;
        abi_flag(P_V, (write.latch & 0x40) != 0);
    }
    abi_flag(P_C, !write.addressed);
    if (write.refused) {
        cpu.a = AD_READ_REFUSED;
        abi_nz(AD_READ_REFUSED);
        abi_flag(P_C, write.mode >= AD_READ_BANK_LIMIT);
        return;
    }
    cpu.a = 0;
    abi_flag(P_V, (write.last_status & 0x40) != 0);
    if (write.mode == AD_READ_TO_MEMORY || write.mode == AD_READ_TO_VRAM) {
        abi_nz(0);
        if (write.mode == AD_READ_TO_VRAM)
            abi_flag(P_C, true);
        return;
    }
    cpu.x = write.pages_left;
    cpu.y = write.bytes_left;
    abi_nz(write.saved_bank);
    abi_flag(P_C, (write.mode & 0x08) != 0);
}

void abi_ad_write(void)
{
    ad_write_exit_shim(ad_write());
}

enum {
    AD_TRANS_COMMAND_READ = 0x08, AD_TRANS_FIRST_STEP = 1, CPLAY_STEP = 0x20, CPLAY_LENGTH_ALL = 0xFF,
    CPLAY_IRQ_BITS = 0x0C
};

/*
 * @card 00:F730 ad_trans_begin
 * WHAT IT DOES: Sets up a read of _AL sectors from the disc into ADPCM memory at _BX, from the
 * sector _CL, _CH, _DL (high first) plus the base CardCdBaseIndex chooses, for ad_trans_step to
 * carry out a step a call. Returns at once; refused while a transfer from the CD runs.
 * HOW: Builds command 8 in CardCdCommand without clearing it first (bytes 6 to 9 stay as they
 * were), keeps _BX, and sets ad_trans_step's first step.
 * WAITS: no.
 * Entry: _AL, _BX, _CL, _CH, _DL. Exit, refused: C 1, N and V bits 7 and 6 of the transfer port,
 * Z 0; A, X, Y kept. Set up: A 1, N 0, Z 0, C 0, V from the base's add, Y the base's offset; X
 * kept. _CL, _CH, _DL left as the sector plus the base.
 */
CardExit ad_trans_begin(void)
{
    CardExit e = { 0 };
    CardExit added;
    uint8_t held;
    uint8_t value;

    held = RD8(CCDU->adpcm_dma);
    if (held & AD_DMA_BITS) {
        e.c = true;
        e.n = (held & 0x80) != 0;
        e.v = (held & 0x40) != 0;
        e.z = false;
        e.set_c = e.set_n = e.set_v = e.set_z = true;
        return e;
    }
    WR8(CRAM->cd_command, AD_TRANS_COMMAND_READ);
    jsr_push(0xF741);
    added = add_cd_base();
    jsr_pop();
    value = RD8(CZP->c.lo);
    wr8((uint16_t)(CardCdCommand + 1), value);
    value = RD8(CZP->c.hi);
    wr8((uint16_t)(CardCdCommand + 2), value);
    value = RD8(CZP->d.lo);
    wr8((uint16_t)(CardCdCommand + 3), value);
    value = RD8(CZP->a.lo);
    wr8((uint16_t)(CardCdCommand + 4), value);
    wr8((uint16_t)(CardCdCommand + 5), 0);
    value = RD8(CZP->b.lo);
    WR8(CRAM->adpcm_address, value);
    value = RD8(CZP->b.hi);
    wr8((uint16_t)(CardAdpcmAddress + 1), value);
    WR8(CRAM->ad_trans_sent, 0);
    WR8(CRAM->ad_trans_step, AD_TRANS_FIRST_STEP);
    e.a = AD_TRANS_FIRST_STEP;
    e.nz = AD_TRANS_FIRST_STEP;
    e.y = added.y;
    e.c = false;
    e.v = added.v;
    e.set_a = e.set_nz = e.set_y = e.set_c = e.set_v = true;
    return e;
}

void abi_ad_trans_begin(void)
{
    card_exit_apply(ad_trans_begin());
}

/*
 * @card 00:F68C cplay_advance
 * WHAT IT DOES: Moves continuous ADPCM play on by CardAdpcmStep sectors: the next sector to read
 * on, the sectors left down.
 * HOW: A three-byte add to CardAdpcmNextSector (high first) and a three-byte subtract from
 * CardAdpcmSectorsLeft (low first).
 * WAITS: no.
 * Entry: CardAdpcmStep. Exit: A the high byte of what is left, N and Z from it, C clear if the
 * subtract went below zero, V from its high byte. X, Y kept.
 */
CardExit cplay_advance(void)
{
    CardExit e = { 0 };
    Sum low;
    Sum middle;
    Sum high;
    uint8_t value;
    uint8_t step;

    value = rd8((uint16_t)(CardAdpcmNextSector + 2));
    step = RD8(CRAM->adpcm_step);
    low = add8(value, step, false);
    wr8((uint16_t)(CardAdpcmNextSector + 2), low.value);
    value = rd8((uint16_t)(CardAdpcmNextSector + 1));
    middle = add8(value, 0, low.carry);
    wr8((uint16_t)(CardAdpcmNextSector + 1), middle.value);
    value = rd8(CardAdpcmNextSector);
    high = add8(value, 0, middle.carry);
    wr8(CardAdpcmNextSector, high.value);
    value = rd8(CardAdpcmSectorsLeft);
    step = RD8(CRAM->adpcm_step);
    low = sub8(value, step, true);
    wr8(CardAdpcmSectorsLeft, low.value);
    value = rd8((uint16_t)(CardAdpcmSectorsLeft + 1));
    middle = sub8(value, 0, low.carry);
    wr8((uint16_t)(CardAdpcmSectorsLeft + 1), middle.value);
    value = rd8((uint16_t)(CardAdpcmSectorsLeft + 2));
    high = sub8(value, 0, middle.carry);
    wr8((uint16_t)(CardAdpcmSectorsLeft + 2), high.value);
    e.a = e.nz = high.value;
    e.c = high.carry;
    e.v = high.overflow;
    e.set_a = e.set_nz = e.set_c = e.set_v = true;
    return e;
}

void abi_cplay_advance(void)
{
    card_exit_apply(cplay_advance());
}

/*
 * @card 00:F61F ad_cplay
 * WHAT IT DOES: Plays ADPCM straight from the disc: _AL, _AH, _BL sectors (low first) from the
 * sector _CL, _CH, _DL (high first, counted from the first disc base) at rate _DH.
 * HOW: Does nothing while ADPCM plays. Keeps the count and the first sector, resets the unit,
 * sets the rate, loads the first $20 sectors into ADPCM memory from 0 with ad_trans, moves the
 * counts on by them, makes the length all of memory, and starts the unit with its half-way and
 * end interrupts, which carry on the play.
 * WAITS: only through ad_trans.
 * Entry: _AX, _BL, _CL, _CH, _DL, _DH; A, X (pushed in ad_trans). Exit: busy, as ad_stat; bad rate, A _DH
 * and C set; a failed load, as ad_trans with C set; started, A 0, N 0, Z 0, X and Y as ad_trans left them.
 * _AL, _BX and _DH changed.
 */
CardExit ad_cplay(uint8_t a, uint8_t x)
{
    CardExit r = { 0 };
    AdStat stat;
    Sum low;
    Sum middle;
    Sum high;
    uint8_t value;
    uint8_t base;
    uint8_t rate;
    uint8_t held;

    r.a = a;
    r.x = x;
    jsr_push(0xF622);
    stat = ad_stat();
    jsr_pop();
    r.x = stat.busy;
    r.a = stat.playing;
    r.nz = stat.playing;
    r.set_x = r.set_a = r.set_nz = true;
    if (stat.playing != 0) {
        return r;
    }
    value = RD8(CZP->a.lo);
    wr8(CardAdpcmSectorsLeft, value);
    value = RD8(CZP->a.hi);
    wr8((uint16_t)(CardAdpcmSectorsLeft + 1), value);
    value = RD8(CZP->b.lo);
    wr8((uint16_t)(CardAdpcmSectorsLeft + 2), value);
    value = RD8(CZP->d.lo);
    base = rd8((uint16_t)(CardCdBases + 2));
    low = add8(value, base, false);
    wr8((uint16_t)(CardAdpcmNextSector + 2), low.value);
    value = RD8(CZP->c.hi);
    base = rd8((uint16_t)(CardCdBases + 1));
    middle = add8(value, base, low.carry);
    wr8((uint16_t)(CardAdpcmNextSector + 1), middle.value);
    value = RD8(CZP->c.lo);
    base = rd8(CardCdBases);
    high = add8(value, base, middle.carry);
    wr8(CardAdpcmNextSector, high.value);
    r.a = high.value;
    r.nz = high.value;
    r.c = high.carry;
    r.v = high.overflow;
    r.set_c = r.set_v = true;
    jsr_push(0xF64F);
    held = ad_reset();
    jsr_pop();
    r.a = 0x6F;
    r.n = (held & 0x80) != 0;
    r.v = (held & 0x40) != 0;
    r.z = (held & ~0x6F) == 0;
    r.set_n = r.set_z = true;
    r.set_nz = false;
    rate = RD8(CZP->d.hi);
    r.a = rate;
    r.nz = (uint8_t)(rate - AD_RATE_LIMIT);
    r.c = rate >= AD_RATE_LIMIT;
    r.set_nz = true;
    r.set_n = r.set_z = false;
    if (rate >= AD_RATE_LIMIT) {
        return r;
    }
    WR8(CCDU->adpcm_rate, rate);
    WR8(CZP->a.lo, CPLAY_STEP);
    WR8(CZP->d.hi, 0);
    WR8(CZP->b.lo, 0);
    WR8(CZP->b.hi, 0);
    jsr_push(0xF667);
    r = card_exit_over(r, ad_trans(0, r.x));
    jsr_pop();
    r.nz = r.a;
    r.c = true;
    r.set_nz = r.set_c = true;
    if (r.a != 0) {
        return r;
    }
    WR8(CRAM->adpcm_step, CPLAY_STEP);
    jsr_push(0xF672);
    r = card_exit_over(r, cplay_advance());
    jsr_pop();
    WR8(CCDU->data_auto, CPLAY_LENGTH_ALL);
    WR8(CCDU->adpcm_address_hi, CPLAY_LENGTH_ALL);
    jsr_push(0xF67D);
    (void)ad_latch_length();
    jsr_pop();
    WR8(CRAM->adpcm_active, 0);
    held = TSB8(CCDU->irq_control, CPLAY_IRQ_BITS);
    r.n = (held & 0x80) != 0;
    r.v = (held & 0x40) != 0;
    r.z = false;
    r.set_n = r.set_v = r.set_z = true;
    r.set_nz = false;
    WR8(CCDU->adpcm_control, AD_CONTROL_PLAY_BITS);
    r.a = 0;
    return r;
}

void abi_ad_cplay(void)
{
    card_exit_apply(ad_cplay(cpu.a, cpu.x));
}

/* The A ad_cplay leaves, and its exit: the call's CardExit. */
uint8_t ad_cplay_result(AdCplay play)
{
    return play.a;
}

void ad_cplay_exit_shim(AdCplay play)
{
    card_exit_apply(play);
}
