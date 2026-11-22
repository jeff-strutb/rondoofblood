; @routine 024F:0B1 MaskVramRowsRandom
; @run 6B:90B1
; WHAT IT DOES: Fades part of a VRAM region out in a random pattern: reads $80 words from the
; VRAM address in zTemp10-11 into the mask buffer, masks rows of the buffer with the random
; bytes, as many rows as zMultiplierHi says (all of it cleared when that is $10 or more), and
; writes the words back.
; HOW: SetVramAddresses points the VDC at the region for reading and writing. Rows below the
; last are masked from the cells two on (MaskStageRowCopy) with a mask made of zRandom2 and its
; high bit; the last pass masks in place (MaskStageRow) with zRandom2 ORed with zRandom3.
; WAITS: no.
; Entry: zTemp10-11 the VRAM address, zMultiplierHi the row count. Exit: A the last byte
; written, X 0, N 0, Z 1, C and V from the last masking (C 1 when the buffer was cleared), Y 0
; or the row count when cleared.
MaskVramRowsRandom:
    ldx z:zTemp10
    ldy z:zTemp11
    jsr SetVramAddresses
    ldx #$80
MaskVramRowsRandom_at0:
    lda a:VDC_DATA_LO
    sta a:$3B3F,x
    lda a:VDC_DATA_HI
    sta a:$3B3E,x
    dex
    dex
    bne MaskVramRowsRandom_at0
    ldy z:SpritesLeft
    cpy #$10
    bcc MaskVramRowsRandom_at1
    ldx #$7F
MaskVramRowsRandom_at2:
    stz a:StageMaskBuffer,x
    dex
    bpl MaskVramRowsRandom_at2
    jmp MaskVramRowsRandom_at3
MaskVramRowsRandom_at1:
    dey
    beq MaskVramRowsRandom_at4
    clx
MaskVramRowsRandom_at5:
    lda z:zRandom2
    sec
    ror a
    ora z:zRandom2
    sta z:zTemp00
    jsr MaskStageRowCopy
    inx
    inx
    dey
    bne MaskVramRowsRandom_at5
MaskVramRowsRandom_at4:
    ldy z:SpritesLeft
    clx
MaskVramRowsRandom_at6:
    lda z:zRandom2
    ora z:zRandom3
    sta z:zTemp00
    jsr MaskStageRow
    inx
    inx
    dey
    bne MaskVramRowsRandom_at6
MaskVramRowsRandom_at3:
    ldx #$80
MaskVramRowsRandom_at7:
    lda a:$3B3F,x
    sta a:VDC_DATA_LO
    lda a:$3B3E,x
    sta a:VDC_DATA_HI
    dex
    dex
    bne MaskVramRowsRandom_at7
    rts

; @routine 12CF:0B1 MaskVramRowsRandomLong
; @run 6B:90B1
; WHAT IT DOES: MaskVramRowsRandom for the stages whose overlay keeps SetVramAddresses at $968F.
; HOW: The same code, calling SetVramAddressesLong.
; WAITS: no.
; Entry and exit: as MaskVramRowsRandom.
MaskVramRowsRandomLong:
    ldx z:zTemp10
    ldy z:zTemp11
    jsr SetVramAddressesLong
    ldx #$80
MaskVramRowsRandomLong_at0:
    lda a:VDC_DATA_LO
    sta a:$3B3F,x
    lda a:VDC_DATA_HI
    sta a:$3B3E,x
    dex
    dex
    bne MaskVramRowsRandomLong_at0
    ldy z:SpritesLeft
    cpy #$10
    bcc MaskVramRowsRandomLong_at1
    ldx #$7F
MaskVramRowsRandomLong_at2:
    stz a:StageMaskBuffer,x
    dex
    bpl MaskVramRowsRandomLong_at2
    jmp MaskVramRowsRandomLong_at3
MaskVramRowsRandomLong_at1:
    dey
    beq MaskVramRowsRandomLong_at4
    clx
MaskVramRowsRandomLong_at5:
    lda z:zRandom2
    sec
    ror a
    ora z:zRandom2
    sta z:zTemp00
    jsr MaskStageRowCopy
    inx
    inx
    dey
    bne MaskVramRowsRandomLong_at5
MaskVramRowsRandomLong_at4:
    ldy z:SpritesLeft
    clx
MaskVramRowsRandomLong_at6:
    lda z:zRandom2
    ora z:zRandom3
    sta z:zTemp00
    jsr MaskStageRow
    inx
    inx
    dey
    bne MaskVramRowsRandomLong_at6
MaskVramRowsRandomLong_at3:
    ldx #$80
MaskVramRowsRandomLong_at7:
    lda a:$3B3F,x
    sta a:VDC_DATA_LO
    lda a:$3B3E,x
    sta a:VDC_DATA_HI
    dex
    dex
    bne MaskVramRowsRandomLong_at7
    rts
