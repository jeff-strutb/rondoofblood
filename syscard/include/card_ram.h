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
#define CardCdSectors 0x2280 /* the sectors a read asks for */
#define CardCdWhole 0x2281 /* 1 when a byte count is a whole number of sectors */
#define CardSavedMprBank 0x2268 /* the bank a mapped read replaced */
#define CardMprStub 0x228E /* a stub built in RAM: TMA or TAM with a mask */
#define CardMprStubMask 0x228F /* the stub's MPR mask */
#define CardMprStubReturn 0x2290 /* the stub's return */
#define AdpcmAddressLo 0x1808 /* the ADPCM unit's address: the same port as CdDataAuto */
#define AdpcmAddressHi 0x1809 /* the ADPCM unit's address */
#define CardVdcCrLo 0x20F3 /* the copy of the VDC's control register */
#define CardCopyLo 0x20EE /* the second pointer the card keeps for itself */
#define CardCopyHi 0x20EF /* the second pointer the card keeps for itself */
#define CardFileLo 0x20F0 /* the backup file the card is at */
#define CardFileHi 0x20F1 /* the backup file the card is at */
#define CardByteCount 0x22B3 /* bytes left of a backup copy or sum */
#define CardPointerLo 0x20EC /* a pointer the card keeps for itself */
#define CardPointerHi 0x20ED /* a pointer the card keeps for itself */
#define BackupUnlock 0x1807 /* opens the backup memory: the same port as CdSubcode */
#define CardSavedMprs 0x22B5 /* the eight MPRs while backup memory is open */
#define BackupHeader 0x8000 /* the backup memory's header */
#define BackupEnd 0x8004 /* where the backup memory ends */
#define BackupNextFree 0x8006 /* where its next free byte is */
#define BackupFirstFile 0x8010 /* its first file */
#define BackupSignature 0xF8A9 /* HUBM: what a formatted header starts with */
#define BackupFormatPassword 0xF8AD /* what bm_format must be handed */
#define AdpcmData 0x180A /* the ADPCM unit's data port */
#define AdpcmDma 0x180B /* the ADPCM unit's transfer from the CD */
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
