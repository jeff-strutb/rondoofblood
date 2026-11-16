/* The BIOS's ADPCM entries. */
#include "card.h"
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
    wr8(CardAdpcmPlayLo, 0);
    wr8(CardAdpcmPlayMid, 0);
    wr8(CardAdpcmPlayHi, 0);
    wr8(CardAdpcmPlayEnd, 0);
    return control;
}

void abi_ad_stop(void)
{
    uint8_t control = ad_stop();

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
    AdStat stat = ad_stat();

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
    AdPlay play = { AD_PLAY_BUSY, { 0, 0 }, 0, 0 };
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
    AdPlay play = ad_play();

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
        cpu.x = rd8(CardAdpcmLength);
        cpu.y = rd8((uint16_t)(CardAdpcmLength + 1));
        abi_flag(P_C, false);
    }
    cpu.a = 0;
    abi_nz(AD_CONTROL_PLAY_BITS);
    abi_flag(P_V, (play.irq_before & 0x40) != 0);
}
