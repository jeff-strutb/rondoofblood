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
        stat.busy = RD8(CCDU->irq_status) & CD_IRQ_ADPCM_HALF;
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

enum {
    AD_HALF_IRQ = 0x08, AD_READ_REFUSED = 0x22, AD_READ_BANK_LIMIT = 0x07, STUB_TMA = 0x43, STUB_TAM = 0x53,
    STUB_RTS = 0x60, AD_READ_TO_VRAM = 0xFF, BANK_PAGES = 0x20, BANK_WINDOW_SHIFT = 5,
    VDC_SELECT_MAWR = 0x00, VDC_SELECT_MARR = 0x01, VDC_SELECT_VWR = 0x02, AD_STATUS_FETCHING_BIT = 0x80,
    AD_STATUS_WRITING = 0x04, AD_DMA_BITS = 0x03, AD_ADDRESS_NONE = 0xFF
};

/* ad_set_address's frame and call: X and Y as the caller holds them. */
static void call_set_address(uint8_t x, uint8_t y, uint16_t ret)
{
    jsr_push(ret);
    ad_set_address(x, y);
    jsr_pop();
}

/*
 * @card 00:F5C6 ad_play
 * WHAT IT DOES: Plays ADPCM: _AX bytes from address _BX at rate _DH; bit 7 of _DL repeats it,
 * and any other bit of _DL plays what is already latched.
 * HOW: Does nothing while ADPCM plays. Keeps _BX, _AX and the rate, latches the address and the
 * length, sets the rate, then turns on the half-way interrupt and starts the unit.
 * WAITS: no.
 * Entry: _AX, _BX, _DL, _DH. Exit, busy: as ad_stat. Bad rate: A _DH, C set, N and Z from
 * comparing it with $10, X as ad_stat. Played: A 0, N 0, Z 0, V bit 6 of the interrupt enables
 * as they were; latched: X and Y _AX, C clear; played again: X as ad_stat. Y, C, V kept
 * otherwise.
 * The address and length words are read by their addresses, the high bytes plus one: no record yet
 * (row request to the typed access session).
 */
AdPlay ad_play(void)
{
    AdPlay play = { AD_PLAY_BUSY, { 0 } };
    CardExit *r = &play.exit;
    AdStat stat;
    uint8_t value;
    uint8_t rate;
    uint8_t x;
    uint8_t y;

    jsr_push(0xF5C9);
    stat = ad_stat();
    jsr_pop();
    r->x = stat.busy;
    r->a = r->nz = stat.playing;
    r->set_x = r->set_a = r->set_nz = true;
    if (stat.playing != 0)
        return play;
    value = RD8(CZP->d.lo);
    WR8(CRAM->adpcm_active, (uint8_t)(value & AD_PLAY_REPEAT_BIT));
    value = RD8(CZP->d.lo);
    play.how = AD_PLAY_AGAIN;
    if ((value & AD_PLAY_MODE_BITS) == 0) {
        value = RD8(CZP->b.lo);
        WR8(CRAM->adpcm_address, value);
        value = RD8(CZP->b.hi);
        wr8((uint16_t)(CardAdpcmAddress + 1), value);
        value = RD8(CZP->a.lo);
        WR8(CRAM->adpcm_length, value);
        value = RD8(CZP->a.hi);
        wr8((uint16_t)(CardAdpcmLength + 1), value);
        rate = RD8(CZP->d.hi);
        r->a = rate;
        r->nz = (uint8_t)(rate - AD_RATE_LIMIT);
        r->c = rate >= AD_RATE_LIMIT;
        r->set_c = true;
        if (rate >= AD_RATE_LIMIT) {
            play.how = AD_PLAY_BAD_RATE;
            return play;
        }
        WR8(CRAM->adpcm_rate, rate);
        x = RD8(CRAM->adpcm_address);
        y = rd8((uint16_t)(CardAdpcmAddress + 1));
        call_set_address(x, y, 0xF5FE);
        jsr_push(0xF601);
        (void)ad_latch_read_address();
        jsr_pop();
        x = RD8(CRAM->adpcm_length);
        y = rd8((uint16_t)(CardAdpcmLength + 1));
        call_set_address(x, y, 0xF60A);
        jsr_push(0xF60D);
        (void)ad_latch_length();
        jsr_pop();
        value = RD8(CRAM->adpcm_rate);
        WR8(CCDU->adpcm_rate, value);
        r->x = x;
        r->y = y;
        r->set_y = true;
        r->c = false;
        play.how = AD_PLAY_STARTED;
    }
    value = TSB8(CCDU->irq_control, AD_HALF_IRQ);
    WR8(CCDU->adpcm_control, AD_CONTROL_PLAY_BITS);
    r->a = 0;
    r->nz = AD_CONTROL_PLAY_BITS;
    r->v = (value & 0x40) != 0;
    r->set_v = true;
    return play;
}

void abi_ad_play(void)
{
    ad_play_exit_shim(ad_play());
}

/* ad_play's exit, for its wrapper and for a caller's that ends in it. */
void ad_play_exit_shim(AdPlay play)
{
    card_exit_apply(play.exit);
}

/*
 * @card 00:F6F5 aim_bx_at_vram_port
 * WHAT IT DOES: Makes _BX the VDC's write address, then points _BX at the VDC's data port, for a
 * copy that writes through [_BX].
 * HOW: write_vram_word_pending, with the address register selected; then _BX = $0002.
 * WAITS: no.
 * Entry: _BX, the VDC's address register selected. Exit: A 2, N 0, Z 0. X, Y, C, V kept.
 * Checked by unit calls (card_unit_adpcm).
 */
CardExit aim_bx_at_vram_port(void)
{
    CardExit e = { 0 };

    jsr_push(0xF6F8);
    (void)write_vram_word_pending();
    jsr_pop();
    WR8(CZP->b.lo, 2);
    WR8(CZP->b.hi, 0);
    e.a = e.nz = 2;
    e.set_a = e.set_nz = true;
    return e;
}

void abi_aim_bx_at_vram_port(void)
{
    card_exit_apply(aim_bx_at_vram_port());
}

/* ad_read_byte, called from where the assembly calls it. */
static uint8_t read_one(uint16_t ret)
{
    uint8_t value;

    jsr_push(ret);
    value = ad_read_byte();
    jsr_pop();
    return value;
}

/* The count in _AX less one, as the loops take it: the low byte tested before the high is changed. Gives what is left. */
static uint8_t count_down_ax(void)
{
    uint8_t low = RD8(CZP->a.lo);
    uint8_t high;

    if (low == 0)
        (void)DEC8(CZP->a.hi);
    (void)DEC8(CZP->a.lo);
    low = RD8(CZP->a.lo);
    high = RD8(CZP->a.hi);
    return (uint8_t)(low | high);
}

/* INC _BL, and INC _BH if it wrapped. */
static void step_bx(void)
{
    if (INC8(CZP->b.lo) == 0)
        (void)INC8(CZP->b.hi);
}

/* The TST of ad_write's loops: the status port read until its writing bit is clear; the last read. */
static uint8_t wait_for_adpcm_write(void)
{
    uint8_t status;

    do
        status = RD8(CCDU->adpcm_status);
    while ((status & AD_STATUS_WRITING) != 0);
    return status;
}

/* The mask of the MPR numbered mode, as the assembly makes it: a one shifted left mode times, in a byte. */
static uint8_t mpr_mask(uint8_t mode)
{
    uint8_t mask = 1;

    for (uint8_t count = mode; count != 0; count--)
        mask = (uint8_t)(mask << 1);
    return mask;
}

/*
 * @card 00:F46B ad_read_to_banks
 * WHAT IT DOES: ad_read's reading to banks: _AX bytes of ADPCM memory to bank _BL and the banks
 * after it, through the MPR numbered _DH (A on entry). A number of 7 or more is refused with $22.
 * HOW: A TMA stub written in RAM keeps the bank the MPR held; each bank is mapped with a TAM stub
 * and filled, 8 KB, at the MPR's window; the kept bank goes back at the end.
 * WAITS: no.
 * Entry: A _DH, the ADPCM address latched. Exit, refused: A $22, N 0, Z 0, C from comparing with
 * 7. Read: A 0, N and Z from the bank put back, C bit 3 of _DH, X the pages and Y the bytes left
 * of the last bank. V kept.
 * Not checked: the TMA and TAM stubs it writes in RAM are done in C, until the checker compares RAM-built code with its caller.
 */
CardExit ad_read_to_banks(uint8_t mode)
{
    CardExit e = { 0 };
    uint8_t mask;
    uint8_t value;
    uint8_t x;
    uint8_t y;

    e.c = mode >= AD_READ_BANK_LIMIT;
    e.set_c = true;
    if (((uint8_t)(mode - AD_READ_BANK_LIMIT) & 0x80) == 0) {
        e.a = e.nz = AD_READ_REFUSED;
        e.set_a = e.set_nz = true;
        return e;
    }
    mask = mpr_mask(mode);
    WR8(CRAM->mpr_stub_mask, mask);
    WR8(CRAM->mpr_stub, STUB_TMA);
    WR8(CRAM->mpr_stub_return, STUB_RTS);
    jsr_push(0xF489);
    value = tma(mask);
    jsr_pop();
    WR8(CRAM->saved_mpr_bank, value);
    value = RD8(CZP->b.lo);
    WR8(CZP->d.lo, value);
    for (;;) {
        bool more;

        value = RD8(CZP->d.hi);
        e.c = (value & 0x08) != 0;      /* the last ASL of the five that make the window's high byte */
        WR8(CZP->b.hi, (uint8_t)(value << BANK_WINDOW_SHIFT));
        WR8(CZP->b.lo, 0);
        WR8(CRAM->mpr_stub, STUB_TAM);
        value = RD8(CZP->d.lo);
        jsr_push(0xF4A5);
        tam(mask, value);
        jsr_pop();
        x = BANK_PAGES;
        y = 0;
        for (;;) {
            value = read_one(0xF4AB);
            WR8_PTR(CZP->b.lo, value);
            step_bx();
            more = count_down_ax() != 0;
            if (!more)
                break;
            y--;
            if (y != 0)
                continue;
            x--;
            if (x == 0)
                break;
        }
        if (!more)
            break;
        (void)INC8(CZP->d.lo);
    }
    WR8(CRAM->mpr_stub, STUB_TAM);
    value = RD8(CRAM->saved_mpr_bank);
    jsr_push(0xF4D6);
    tam(mask, value);
    jsr_pop();
    e.a = 0;
    e.nz = value;
    e.x = x;
    e.y = y;
    e.set_a = e.set_nz = e.set_x = e.set_y = true;
    return e;
}

void abi_ad_read_to_banks(void)
{
    card_exit_apply(ad_read_to_banks(cpu.a));
}

/*
 * @card 00:F407 ad_read
 * WHAT IT DOES: Reads _AX bytes of ADPCM memory from address _CX: to memory at _BX if _DH is 0,
 * to VRAM at _BX if _DH is $FF, or else to banks from _BL on, mapped through the MPR numbered _DH
 * (1 to 6). Any other _DH gives $22.
 * HOW: Waits until ADPCM is not playing, latches the address, drops the byte the unit had
 * fetched, then reads byte after byte. VRAM goes through the data port, its two halves in turn;
 * banks through a TMA or TAM stub it writes in RAM, 8 KB a bank, the MPR put back at the end.
 * WAITS: on ad_stat, with the stack as the caller's.
 * Entry: _AX, _BX, _CX, _DH, and _BL for banks. Exit: X _CL and Y _CH, V bit 6 of the control
 * port as the latch read it. To memory: A 0, N 0, Z 1, C kept. To VRAM: the same, C 1. Banks and
 * refused: as ad_read_to_banks.
 * Memory and VRAM arms by unit calls (card_unit_adpcm); banks arm not checked: the TMA and TAM stubs it writes in RAM are done in C, until the checker compares RAM-built code with its caller.
 */
CardExit ad_read(void)
{
    CardExit r = { 0 };
    AdStat stat;
    uint8_t value;
    uint8_t mode;
    uint8_t x;
    uint8_t y;
    uint8_t held;

    do {
        jsr_push(0xF40A);
        stat = ad_stat();
        jsr_pop();
    } while (stat.playing != 0);
    x = RD8(CZP->c.lo);
    y = RD8(CZP->c.hi);
    call_set_address(x, y, 0xF413);
    jsr_push(0xF416);
    held = ad_latch_read_address();
    jsr_pop();
    (void)read_one(0xF418);
    mode = RD8(CZP->d.hi);
    r.x = x;
    r.y = y;
    r.v = (held & 0x40) != 0;
    r.set_x = r.set_y = r.set_v = true;
    if (mode == 0) {
        do {
            value = read_one(0xF425);
            WR8_PTR(CZP->b.lo, value);
            step_bx();
        } while (count_down_ax() != 0);
        r.a = r.nz = 0;
        r.set_a = r.set_nz = true;
        return r;
    }
    if (mode == AD_READ_TO_VRAM) {
        WR8(CZP->vdc_reg, VDC_SELECT_MAWR);
        WR8(CVDC->addr, VDC_SELECT_MAWR);
        jsr_push(0xF447);
        (void)aim_bx_at_vram_port();
        jsr_pop();
        WR8(CZP->vdc_reg, VDC_SELECT_VWR);
        WR8(CVDC->addr, VDC_SELECT_VWR);
        do {
            value = read_one(0xF450);
            WR8_PTR(CZP->b.lo, value);
            value = RD8(CZP->b.lo);
            WR8(CZP->b.lo, (uint8_t)(value ^ 1));
        } while (count_down_ax() != 0);
        WR8(CRAM->vram_pending, 0);
        r.a = r.nz = 0;
        r.c = true;
        r.set_a = r.set_nz = r.set_c = true;
        return r;
    }
    return card_exit_over(r, ad_read_to_banks(mode));
}

void abi_ad_read(void)
{
    card_exit_apply(ad_read());
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
 * WAITS: on the unit's writing bit, with the stack as the caller's.
 * Entry: _AX, _BX, _CX, _DH, and _BL for banks. Exit, a transfer running: A its bits, N 0, Z 0;
 * X, Y, C, V kept. Else, if _CX was latched, X _CL, Y _CH, C 0; if not, X, Y kept and C 1. Bad
 * _DH: A $22, N 0, Z 0, C 1. Written: A 0, V bit 6 of the status as the last wait read it; from
 * memory N 0, Z 1; from VRAM N 0, Z 1, C 1; from banks N and Z from the bank put back, C bit 3
 * of _DH, X the pages and Y the bytes left of the last bank.
 * Banks arm not checked: the TMA and TAM stubs it writes in RAM are done in C, until the checker compares RAM-built code with its caller. The rest by unit calls (card_unit_adpcm).
 */
CardExit ad_write(void)
{
    CardExit r = { 0 };
    uint8_t value;
    uint8_t other;
    uint8_t mode;
    uint8_t mask;
    uint8_t status = 0;
    uint8_t x;
    uint8_t y;
    bool more;

    value = (uint8_t)(RD8(CCDU->adpcm_dma) & AD_DMA_BITS);
    if (value != 0) {
        r.a = r.nz = value;
        r.set_a = r.set_nz = true;
        return r;
    }
    value = RD8(CZP->c.lo);
    other = RD8(CZP->c.hi);
    r.c = (uint8_t)(value & other) >= AD_ADDRESS_NONE;
    r.set_c = true;
    if ((uint8_t)(value & other) != AD_ADDRESS_NONE) {
        x = RD8(CZP->c.lo);
        y = RD8(CZP->c.hi);
        call_set_address(x, y, 0xF4EE);
        jsr_push(0xF4F1);
        (void)ad_latch_write_address();
        jsr_pop();
        r.x = x;
        r.y = y;
        r.set_x = r.set_y = true;
    }
    mode = RD8(CZP->d.hi);
    if (mode == 0) {
        do {
            value = RD8_PTR(CZP->b.lo);
            WR8(CCDU->adpcm_data, value);
            status = wait_for_adpcm_write();
            step_bx();
        } while (count_down_ax() != 0);
        r.a = r.nz = 0;
        r.v = (status & 0x40) != 0;
        r.set_a = r.set_nz = r.set_v = true;
        return r;
    }
    if (mode == AD_READ_TO_VRAM) {
        WR8(CZP->vdc_reg, VDC_SELECT_MARR);
        WR8(CVDC->addr, VDC_SELECT_MARR);
        jsr_push(0xF527);
        (void)aim_bx_at_vram_port();
        jsr_pop();
        WR8(CZP->vdc_reg, VDC_SELECT_VWR);
        WR8(CVDC->addr, VDC_SELECT_VWR);
        do {
            status = wait_for_adpcm_write();
            value = RD8_PTR(CZP->b.lo);
            WR8(CCDU->adpcm_data, value);
            value = RD8(CZP->b.lo);
            WR8(CZP->b.lo, (uint8_t)(value ^ 1));
        } while (count_down_ax() != 0);
        WR8(CRAM->vram_pending, 0);
        r.a = r.nz = 0;
        r.c = true;
        r.v = (status & 0x40) != 0;
        r.set_a = r.set_nz = r.set_c = r.set_v = true;
        return r;
    }
    r.c = mode >= AD_READ_BANK_LIMIT;
    if (((uint8_t)(mode - AD_READ_BANK_LIMIT) & 0x80) == 0) {
        r.a = r.nz = AD_READ_REFUSED;
        r.set_a = r.set_nz = r.set_c = true;
        return r;
    }
    mask = mpr_mask(mode);
    WR8(CRAM->mpr_stub_mask, mask);
    WR8(CRAM->mpr_stub, STUB_TMA);
    WR8(CRAM->mpr_stub_return, STUB_RTS);
    jsr_push(0xF571);
    value = tma(mask);
    jsr_pop();
    WR8(CRAM->saved_mpr_bank, value);
    value = RD8(CZP->b.lo);
    WR8(CZP->d.lo, value);
    do {
        value = RD8(CZP->d.hi);
        r.c = (value & 0x08) != 0;      /* the last ASL of the five that make the window's high byte */
        WR8(CZP->b.hi, (uint8_t)(value << BANK_WINDOW_SHIFT));
        WR8(CZP->b.lo, 0);
        WR8(CRAM->mpr_stub, STUB_TAM);
        value = RD8(CZP->d.lo);
        jsr_push(0xF58D);
        tam(mask, value);
        jsr_pop();
        x = BANK_PAGES;
        y = 0;
        for (;;) {
            status = wait_for_adpcm_write();
            value = RD8_PTR(CZP->b.lo);
            WR8(CCDU->adpcm_data, value);
            step_bx();
            more = count_down_ax() != 0;
            if (!more)
                break;
            y--;
            if (y != 0)
                continue;
            x--;
            if (x == 0)
                break;
        }
        if (more)
            (void)INC8(CZP->d.lo);
    } while (more);
    WR8(CRAM->mpr_stub, STUB_TAM);
    value = RD8(CRAM->saved_mpr_bank);
    jsr_push(0xF5C4);
    tam(mask, value);
    jsr_pop();
    r.a = 0;
    r.nz = value;
    r.x = x;
    r.y = y;
    r.v = (status & 0x40) != 0;
    r.set_a = r.set_nz = r.set_x = r.set_y = r.set_v = true;
    return r;
}

void abi_ad_write(void)
{
    card_exit_apply(ad_write());
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
    WR8_IDX(COL(CRAM->cd_command), 0, AD_TRANS_COMMAND_READ);
    jsr_push(0xF741);
    added = add_cd_base();
    jsr_pop();
    value = RD8(CZP->c.lo);
    WR8_IDX(COL(CRAM->cd_command), 1, value);
    value = RD8(CZP->c.hi);
    WR8_IDX(COL(CRAM->cd_command), 2, value);
    value = RD8(CZP->d.lo);
    WR8_IDX(COL(CRAM->cd_command), 3, value);
    value = RD8(CZP->a.lo);
    WR8_IDX(COL(CRAM->cd_command), 4, value);
    WR8_IDX(COL(CRAM->cd_command), 5, 0);
    value = RD8(CZP->b.lo);
    WR8(CRAM->ad_trans_address, value);
    value = RD8(CZP->b.hi);
    wr8((uint16_t)(CardAdTransAddress + 1), value);
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
    base = RD8_IDX(COL(CRAM->cd_bases), 2);
    low = add8(value, base, false);
    wr8((uint16_t)(CardAdpcmNextSector + 2), low.value);
    value = RD8(CZP->c.hi);
    base = RD8_IDX(COL(CRAM->cd_bases), 1);
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
