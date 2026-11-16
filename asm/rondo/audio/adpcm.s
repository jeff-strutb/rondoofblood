; @routine 0046:4C6 CardIsVersionOne
; @run 69:5CC6
; WHAT IT DOES: Says whether the System Card is version 1.00, which has no ad_play.
; HOW: ex_getver: the major version 1 and the minor 0.
; Entry: nothing. Exit: Z set for 1.00. X the major version less one, Y the minor; with major 1, A
; the minor and N from it, else N from X and A kept. C and V kept.
CardIsVersionOne:
    jsr BiosExGetver
    dex
    bne CardIsVersionOne_at0
    tya
CardIsVersionOne_at0:
    rts

; @routine 0046:511 KernelAdpcmStat
; @run 69:5D11
; WHAT IT DOES: Whether ADPCM is busy, and whether it is playing, from the ports.
; HOW: Each is one port's bit, or if clear another's: as the card's ad_stat.
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
