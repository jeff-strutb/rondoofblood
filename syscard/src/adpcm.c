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
