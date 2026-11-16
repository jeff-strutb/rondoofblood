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
#include "routines.h"
#include "syscard.h"

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
 * Entry: nothing. Exit: Z set for 1.00. X the major version less one, Y the minor; with major 1, A
 * the minor and N from it, else N from X and A kept. C and V kept.
 */
CardVersion CardIsVersionOne(void)
{
    return ex_getver();
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
