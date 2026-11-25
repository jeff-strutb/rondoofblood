; @routine 0046:4C6 CardIsVersionOne
; @run 69:5CC6
; WHAT IT DOES: Says whether the System Card is version 1.00, which has no ad_play.
; HOW: ex_getver: the major version 1 and the minor 0.
; WAITS: no.
; Entry: nothing. Exit: Z set for 1.00. X the major version less one, Y the minor; with major 1, A
; the minor and N from it, else N from X and A kept. C and V kept.
CardIsVersionOne:
    jsr BiosExGetver
    dex
    bne CardIsVersionOne_at0
    tya
CardIsVersionOne_at0:
    rts

; @routine 0046:4CE KernelAdpcmPlay
; @run 69:5CCE
; WHAT IT DOES: Plays ADPCM from the BIOS's arguments: _AX bytes from _BX at rate _DH, once. On
; a card past 1.00 that is the card's ad_play; on 1.00 the kernel latches the address and length
; and starts the unit itself, unless it is playing or the rate is bad.
; HOW: zBiosDl cleared for a single play; CardIsVersionOne; ad_play, or the ADPCM ports through
; KernelAdpcmStat and KernelAdpcmLatchWrite.
; WAITS: no.
; Entry: zBiosAl to zBiosDh. Exit: the card's, as ad_play after X the major version less one and
; Y the minor. 1.00: Y 0; busy, as KernelAdpcmStat; bad rate, A the rate, N and Z from comparing
; it with $10, C set, X as KernelAdpcmStat; played, A 0, N and Z clear, C clear, V bit 6 of the
; interrupt enables before, X as KernelAdpcmStat.
KernelAdpcmPlay:
    stz z:zBiosDl
    bsr CardIsVersionOne
    beq KernelAdpcmPlay_at0
    jmp BiosAdPlay
KernelAdpcmPlay_at0:
    bsr KernelAdpcmStat
    bne KernelAdpcmPlay_at1
    lda z:zBiosDh
    cmp #$10
    bcs KernelAdpcmPlay_at1
    sta a:AdpcmRate
    lda z:zBiosBl
    sta a:AdpcmAddressLo
    lda z:zBiosBh
    sta a:AdpcmAddressHi
    jsr KernelAdpcmLatchWrite
    lda z:zBiosAl
    sta a:AdpcmAddressLo
    lda z:zBiosAh
    sta a:AdpcmAddressHi
    lda #$10
    tsb a:AdpcmControl
    lda #$10
    trb a:AdpcmControl
    lda #$08
    tsb a:CdIrqControl
    lda #$60
    sta a:AdpcmControl
    cla
KernelAdpcmPlay_at1:
    rts

; @routine 0046:511 KernelAdpcmStat
; @run 69:5D11
; WHAT IT DOES: Whether ADPCM is busy, and whether it is playing, from the ports.
; HOW: Each is one port's bit, or if clear another's: as the card's ad_stat.
; WAITS: no.
; Entry: nothing. Exit: X busy, A playing, N and Z from A. Y, C and V kept.
KernelAdpcmStat:
    lda a:AdpcmStatus
    and #$01
    bne KernelAdpcmStat_at0
    lda a:CdIrqStatus
    and #$04
KernelAdpcmStat_at0:
    tax
    lda a:AdpcmControl
    and #$20
    bne KernelAdpcmStat_at1
    lda a:AdpcmStatus
    and #$08
KernelAdpcmStat_at1:
    rts

; @routine 0046:52B KernelAdpcmLatchWrite
; @run 69:5D2B
; WHAT IT DOES: Latches the address in the ADPCM address ports as where the unit will write.
; HOW: The control's bit 3 on, a read of the data port, a short wait, the bit off.
; WAITS: no.
; Entry: nothing. Exit: A 8; Z if the control had no other bits, N and V its bits 7 and 6. X, Y
; and C kept.
KernelAdpcmLatchWrite:
    lda #$08
    tsb a:AdpcmControl
    lda a:AdpcmData
    lda #$05
KernelAdpcmLatchWrite_at0:
    dec a
    bne KernelAdpcmLatchWrite_at0
    lda #$08
    trb a:AdpcmControl
    rts
