; @routine 024F:2D9 ClearStageBitRowsFour
; @run 6B:92D9
; WHAT IT DOES: Runs ClearStageBitRows four times, for four stages' entries.
; HOW: Three calls and then the run the last call returns into.
; Every stage overlay holds this code.
; WAITS: no.
; Entry, exit: as ClearStageBitRows.
ClearStageBitRowsFour:
    bsr ClearStageBitRows
    bsr ClearStageBitRows
    bsr ClearStageBitRows

; @routine 024F:2DF ClearStageBitRows
; @run 6B:92DF
; WHAT IT DOES: Masks four rows of VRAM words by the stage's cleared bit and moves zTemp0F on to the
; next stage's entry.
; HOW: SetVramAddresses points the VDC, the word's two bytes are read through the data ports, ANDed
; with StageClearBitLo and StageClearBitHi by the stage index and written back.
; Nine stage overlays hold this code, the ones whose VRAM routine is at $937D.
; WAITS: no.
; Entry: X kept. Exit: A zTemp0F's new value, Y the stage index, C and V from that add.
ClearStageBitRows:
    lda z:zTemp0F
    and #$0F
    tay
    lda z:zTemp0F
    lsr a
    lsr a
    lsr a
    lsr a
    clc
    adc z:zTemp08
    sta z:zTemp0A
    lda z:zTemp09
    sta z:zTemp0B
    lda #$04
    sta z:zTemp0E
ClearStageBitRows_at0:
    phx
    phy
    ldx z:zTemp0A
    ldy z:zTemp0B
    jsr SetVramAddresses
    ply
    lda a:VDC_DATA_LO
    and a:StageClearBitLo,y
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and a:StageClearBitHi,y
    sta a:VDC_DATA_HI
    plx
    lda z:zTemp0A
    clc
    adc #$10
    sta z:zTemp0A
    dec z:zTemp0E
    bne ClearStageBitRows_at0
    lda z:zTemp0F
    clc
    adc #$23
    sta z:zTemp0F
    rts

; @routine 05BF:2DF ClearStageBitRowsLong
; @run 6B:92DF
; WHAT IT DOES: ClearStageBitRows for the other stage group, whose VRAM routine is at $968F.
; HOW: The same code calling SetVramAddressesLong.
; WAITS: no.
; Entry, exit: as ClearStageBitRows.
ClearStageBitRowsLong:
    lda z:zTemp0F
    and #$0F
    tay
    lda z:zTemp0F
    lsr a
    lsr a
    lsr a
    lsr a
    clc
    adc z:zTemp08
    sta z:zTemp0A
    lda z:zTemp09
    sta z:zTemp0B
    lda #$04
    sta z:zTemp0E
ClearStageBitRowsLong_at0:
    phx
    phy
    ldx z:zTemp0A
    ldy z:zTemp0B
    jsr SetVramAddressesLong
    ply
    lda a:VDC_DATA_LO
    and a:StageClearBitLo,y
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and a:StageClearBitHi,y
    sta a:VDC_DATA_HI
    plx
    lda z:zTemp0A
    clc
    adc #$10
    sta z:zTemp0A
    dec z:zTemp0E
    bne ClearStageBitRowsLong_at0
    lda z:zTemp0F
    clc
    adc #$23
    sta z:zTemp0F
    rts
