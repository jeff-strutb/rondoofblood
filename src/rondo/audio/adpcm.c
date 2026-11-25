/*
 * The kernel's ADPCM play: the System Card's ad_play on cards past 1.00, and on 1.00, which lacks
 * it, the kernel's own play through the ADPCM ports. The retail game runs on 3.0, so the kernel's
 * own play is code it carries for older cards.
 *
 * A body returns plain values; its wrapper works out what the assembly leaves in the registers
 * and flags.
 */
#include "machine.h"
#include "ram.h"
#include "syscard.h"
#include "routines.h"

enum {
    AD_STATUS_BUSY_BIT = 0x01, CD_IRQ_HALF_BIT = 0x04, AD_CONTROL_PLAYING_BIT = 0x20,
    AD_STATUS_PLAYING_BIT = 0x08, AD_CONTROL_LATCH_WRITE = 0x08, AD_CONTROL_LATCH_LENGTH = 0x10,
    CD_IRQ_ADPCM_HALF = 0x08, AD_CONTROL_PLAY = 0x60, AD_RATE_LIMIT = 0x10
};

/*
 * @routine 0046:4C6 CardIsVersionOne
 * @run 69:5CC6
 * WHAT IT DOES: Says whether the System Card is version 1.00, which has no ad_play.
 * HOW: ex_getver: the major version 1 and the minor 0.
 * WAITS: no.
 * Entry: nothing. Exit: Z set for 1.00. X the major version less one, Y the minor; with major 1, A
 * the minor and N from it, else N from X and A kept. C and V kept.
 */
CardVersion CardIsVersionOne(void)
{
    CardVersion version;

    jsr_push(0x5CC9);
    version = ex_getver();
    jsr_pop();
    return version;
}

void abi_CardIsVersionOne(void)
{
    CardVersion version = CardIsVersionOne();

    cpu.x = (uint8_t)(version.major - 1);
    cpu.y = version.minor;
    if (version.major != 1) {
        abi_nz(cpu.x);
        return;
    }
    cpu.a = version.minor;
    abi_nz(version.minor);
}

/*
 * @routine 0046:511 KernelAdpcmStat
 * @run 69:5D11
 * WHAT IT DOES: Whether ADPCM is busy, and whether it is playing, from the ports.
 * HOW: Each is one port's bit, or if clear another's: as the card's ad_stat.
 * WAITS: no.
 * Entry: nothing. Exit: X busy, A playing, N and Z from A. Y, C and V kept.
 */
AdStat KernelAdpcmStat(void)
{
    AdStat stat;

    stat.busy = (uint8_t)(rd8(AdpcmStatus) & AD_STATUS_BUSY_BIT);
    if (stat.busy == 0)
        stat.busy = (uint8_t)(rd8(CdIrqStatus) & CD_IRQ_HALF_BIT);
    stat.playing = (uint8_t)(rd8(AdpcmControl) & AD_CONTROL_PLAYING_BIT);
    if (stat.playing == 0)
        stat.playing = (uint8_t)(rd8(AdpcmStatus) & AD_STATUS_PLAYING_BIT);
    return stat;
}

void abi_KernelAdpcmStat(void)
{
    AdStat stat = KernelAdpcmStat();

    cpu.x = stat.busy;
    cpu.a = stat.playing;
    abi_nz(stat.playing);
}

/*
 * @routine 0046:52B KernelAdpcmLatchWrite
 * @run 69:5D2B
 * WHAT IT DOES: Latches the address in the ADPCM address ports as where the unit will write.
 * HOW: The control's bit 3 on, a read of the data port, a short wait, the bit off.
 * WAITS: no.
 * Entry: nothing. Exit: A 8; Z if the control had no other bits, N and V its bits 7 and 6. X, Y
 * and C kept.
 */
uint8_t KernelAdpcmLatchWrite(void)
{
    uint8_t control;

    wr8(AdpcmControl, (uint8_t)(rd8(AdpcmControl) | AD_CONTROL_LATCH_WRITE));
    rd8(AdpcmData);
    control = rd8(AdpcmControl);
    wr8(AdpcmControl, (uint8_t)(control & ~AD_CONTROL_LATCH_WRITE));
    return control;
}

void abi_KernelAdpcmLatchWrite(void)
{
    uint8_t control = KernelAdpcmLatchWrite();

    cpu.a = AD_CONTROL_LATCH_WRITE;
    abi_flag(P_Z, (control & ~AD_CONTROL_LATCH_WRITE) == 0);
    abi_flag(P_N, (control & 0x80) != 0);
    abi_flag(P_V, (control & 0x40) != 0);
}

/*
 * @routine 0046:4CE KernelAdpcmPlay
 * @run 69:5CCE
 * WHAT IT DOES: Plays ADPCM from the BIOS's arguments: _AX bytes from _BX at rate _DH, once. On
 * a card past 1.00 that is the card's ad_play; on 1.00 the kernel latches the address and length
 * and starts the unit itself, unless it is playing or the rate is bad.
 * HOW: zBiosDl cleared for a single play; CardIsVersionOne; ad_play, or the ADPCM ports through
 * KernelAdpcmStat and KernelAdpcmLatchWrite.
 * WAITS: no.
 * Entry: zBiosAl to zBiosDh. Exit: the card's, as ad_play after X the major version less one and
 * Y the minor. 1.00: Y 0; busy, as KernelAdpcmStat; bad rate, A the rate, N and Z from comparing
 * it with $10, C set, X as KernelAdpcmStat; played, A 0, N and Z clear, C clear, V bit 6 of the
 * interrupt enables before, X as KernelAdpcmStat.
 */
KernelPlay KernelAdpcmPlay(void)
{
    KernelPlay play;
    uint8_t enables;

    wr8(zBiosDl, 0);
    jsr_push(0x5CD2);
    play.version = CardIsVersionOne();
    jsr_pop();
    if (play.version.major != 1 || play.version.minor != 0) {
        play.how = KERNEL_PLAY_CARD;
        play.card = ad_play();
        return play;
    }
    jsr_push(0x5CD9);
    play.stat = KernelAdpcmStat();
    jsr_pop();
    play.how = KERNEL_PLAY_BUSY;
    if (play.stat.playing != 0)
        return play;
    play.rate = rd8(zBiosDh);
    play.how = KERNEL_PLAY_BAD_RATE;
    if (play.rate >= AD_RATE_LIMIT)
        return play;
    wr8(AdpcmRate, play.rate);
    wr8(AdpcmAddressLo, rd8(zBiosBl));
    wr8(AdpcmAddressHi, rd8(zBiosBh));
    jsr_push(0x5CF1);
    KernelAdpcmLatchWrite();
    jsr_pop();
    wr8(AdpcmAddressLo, rd8(zBiosAl));
    wr8(AdpcmAddressHi, rd8(zBiosAh));
    wr8(AdpcmControl, (uint8_t)(rd8(AdpcmControl) | AD_CONTROL_LATCH_LENGTH));
    wr8(AdpcmControl, (uint8_t)(rd8(AdpcmControl) & ~AD_CONTROL_LATCH_LENGTH));
    enables = rd8(CdIrqControl);
    wr8(CdIrqControl, (uint8_t)(enables | CD_IRQ_ADPCM_HALF));
    wr8(AdpcmControl, AD_CONTROL_PLAY);
    play.enables = enables;
    play.how = KERNEL_PLAY_STARTED;
    return play;
}

void kernel_play_exit_shim(KernelPlay play)
{
    cpu.x = (uint8_t)(play.version.major - 1);
    cpu.y = play.version.minor;
    if (play.how == KERNEL_PLAY_CARD) {
        abi_nz(play.version.major != 1 ? cpu.x : play.version.minor);
        if (play.version.major == 1)
            cpu.a = play.version.minor;
        ad_play_exit_shim(play.card);
        return;
    }
    cpu.x = play.stat.busy;
    switch (play.how) {
    case KERNEL_PLAY_BUSY:
        cpu.a = play.stat.playing;
        abi_nz(play.stat.playing);
        break;
    case KERNEL_PLAY_BAD_RATE:
        cpu.a = play.rate;
        abi_nz((uint8_t)(play.rate - AD_RATE_LIMIT));
        abi_flag(P_C, true);
        break;
    default:
        cpu.a = 0;
        abi_nz(AD_CONTROL_PLAY);
        abi_flag(P_C, false);
        abi_flag(P_V, (play.enables & 0x40) != 0);
        break;
    }
}

void abi_KernelAdpcmPlay(void)
{
    kernel_play_exit_shim(KernelAdpcmPlay());
}
