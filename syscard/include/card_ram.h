/* Generated from syscard/config/symbols.csv by syscard/tools/ramnames.py. */
#ifndef CARD_RAM_H
#define CARD_RAM_H

#define CardVdcReg 0x20F7 /* the VDC register last selected */
#define CardAl 0x20F8 /* BIOS argument _AL */
#define CardBl 0x20FA /* BIOS argument _BL */
#define CardBh 0x20FB /* BIOS argument _BH */
#define CardCl 0x20FC /* BIOS argument _CL */
#define CardCh 0x20FD /* BIOS argument _CH */
#define CardDl 0x20FE /* BIOS argument _DL */
#define CardIrqVectors 0x2200 /* interrupt handlers: a word each */
#define CardVdcMwr 0x2242 /* the copy of the VDC's memory width register */
#define CardCdCommand 0x224C /* the CD command being built */
#define CardCdResult 0x2256 /* what the drive last answered */
#define CardSavedArgs 0x2260 /* _AX to _DX kept across a command */
#define CardVramPending 0x2272 /* set when _BX was written to the VDC's data port */
#define CardCdBaseIndex 0x2273 /* which disc base a request adds */
#define CardCdBases 0x2274 /* disc addresses added to a request: three bytes each */
#define CardSubcodeByte 0x227E /* the subcode byte last read */
#define CardSoftResetLo 0x2284 /* the soft reset's handler */
#define CardSoftResetHi 0x2285 /* the soft reset's handler */
#define CardAdpcmPlayLo 0x229D /* ADPCM play state */
#define CardAdpcmPlayMid 0x229E /* ADPCM play state */
#define CardAdpcmPlayHi 0x229F /* ADPCM play state */
#define CardAdpcmPlayEnd 0x22A0 /* ADPCM play state */
#define CardAdpcmActive 0x22A5 /* ADPCM play state */
#define CardSavedMpr6 0x22BB /* MPR6 while the math bank is mapped */
#define VdcAddress 0x0000 /* the VDC's address port */
#define VdcDataLo 0x0002 /* the VDC's data port */
#define VdcDataHi 0x0003 /* the VDC's data port */
#define CdBusStatus 0x1800 /* the CD bus's phase and handshake bits */
#define CdData 0x1801 /* the CD bus's data port */
#define CardAh 0x20F9 /* BIOS argument _AH */
#define CdIrqControl 0x1802 /* the CD unit's interrupt enables and the bus acknowledge */
#define CdDataAuto 0x1808 /* the CD data port with its handshake done for each read */
#define CardCdNotReady 0x2247 /* set when the drive's sense code said not ready */
#define CardDiscFirstTrack 0x226A /* the first and last tracks from the table of contents */
#define CardDiscLength 0x226C /* the disc's length from the table of contents */
#define CardDh 0x20FF /* BIOS argument _DH */
#define CardDiscEnd 0x226F /* the disc's end as a play takes it: minutes then seconds then frames */
#define CardCdPlayMode 0x229C /* the mode of the last play */
#define CardCdSenseKey 0x227D /* the sense data's key byte */
#define CardCdFailedCommand 0x2291 /* the command that last failed */
#define CardFrameTick 0x2241 /* counted by the card's frame interrupt */
#define CardCdNoWait 0x2248 /* set: a command returns without waiting on the drive */
#define CardBusPhase 0x227A /* the bus phase last read */
#define CardCdStatus 0x227B /* the status byte the drive last gave */
#define CardCdSense 0x227C /* the drive's sense code */
#define CardCdTimedOut 0x229B /* set: the drive did not answer a command in time */
#define CardCdBusy 0x22A4 /* set while an interrupt-driven transfer runs */
#define CdResetPort 0x1804 /* the CD unit's reset line */
#define CdIrqStatus 0x1803 /* the CD unit's interrupt status */
#define CdPcmLatch 0x1805 /* CD audio sample latch and low byte */
#define CdPcmHigh 0x1806 /* CD audio sample high byte */
#define CdSubcode 0x1807 /* subcode byte */
#define AdpcmStatus 0x180C /* ADPCM status */
#define AdpcmControl 0x180D /* ADPCM control */
#define CdFade 0x180F /* CD and ADPCM volume fade */
#define SuperRamSigA 0x18C1 /* signature of the Super System Card's RAM */
#define SuperRamSigB 0x18C2 /* signature of the Super System Card's RAM */
#define SuperRamBank 0x18C3 /* the Super System Card RAM's bank */
#define ArcadeRamSigA 0x18C5 /* signature of the extra RAM read first */
#define ArcadeRamSigB 0x18C6 /* signature of the extra RAM read first */
#define ArcadeRamBank 0x18C7 /* that RAM's bank */
#define CardVersionByte 0xFFF4 /* the byte of the card ex_memopen returns */

#endif
