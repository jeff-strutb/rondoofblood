; @routine 0255:7A5 DrawBlockColumn
; @run 6F:A7A5
; WHAT IT DOES: Draws a column of a layer's tile blocks for scroll entry X: as many blocks as $3E66's low seven bits,
; from the place the entry has reached (its scroll X plus the overlay's offset of word Y, and its scroll Y, plus the
; offset at $A781 and $A783 when $3E66's bit 7 is set), down the screen and on into the screen below.
; HOW: BlockIndexForPlace for the first block; then each block number from the second table (zTemp04) by Y goes to
; CopyTileBlockToVram with the bank bit; Y steps by 8 (a row of blocks) with the bit read again, and past $40 the
; next screen row's map byte selects the tables again. zTemp08 counts the blocks. Each JSR goes to the overlay's
; routine when that is the one resident at its address, to the original otherwise, as the original does.
; WAITS: no.
; Entry: X the entry, Y the offset's index. Exit: A as the last block copy left it ($10), N 0, Z 1 (the count's
; decrement), C and V from the copy, Y the last block's index, X kept.
DrawBlockColumn:
    lda a:Scroll3E66,x
    and #$7F
    sta z:zTemp08
    jsr ScrollXPlusOffset
    lda a:ScrollYLo,x
    sta z:zTemp06
    lda a:ScrollYHiScreen,x
    sta z:zTemp07
    lda a:Scroll3E66,x
    bpl DrawBlockColumn_at0
    ldy #$01
    lda a:ScrollYLo,x
    clc
    adc a:$A780,y
    sta z:zTemp06
    lda a:ScrollYHiScreen,x
    adc a:$A782,y
    and #$0F
    sta z:zTemp07
DrawBlockColumn_at0:
    jsr BlockIndexForPlace
DrawBlockColumn_at3:
    lda (zTemp04),y
    phy
    jsr CopyTileBlockToVram
    ply
    dec z:zTemp08
    bne DrawBlockColumn_at1
    rts
DrawBlockColumn_at1:
    tya
    clc
    adc #$08
    tay
    cmp #$40
    bcs DrawBlockColumn_at2
    jsr BitmapBitShifted
    bra DrawBlockColumn_at3
DrawBlockColumn_at2:
    lda z:zTemp03
    sta z:zTemp00
    lda z:zTemp07
    inc a
    and #$0F
    sta z:zTemp07
    jsr ScreenMapAt
    jsr SelectBlockTablesA
    lda z:zTemp02
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    tay
    jsr BitmapBitShifted
    jmp DrawBlockColumn_at3

; @routine 0256:03D BlockIndexForPlace
; @run 6F:A83D
; WHAT IT DOES: For the place in zTemp02-03 (X: the low byte and the screen) and zTemp06-07 (Y: the low byte and the
; screen row): the VRAM address of the place's block in zTemp10-11, the screen map's byte for the place with the
; stage's block tables selected by it (zTemp0C, zTemp04), and the block's index within the screen in Y; then runs
; into BitmapBitShifted, whose bit is the block's bank.
; HOW: Shifts through the carry: the screen's bit 0 above the X byte, the row's bit 0 above the Y byte; ScreenMapAt
; and the resident SelectBlockTables (each the overlay's routine when it is resident, the original otherwise); Y the X
; byte's top three bits or'd with the Y byte's bits 5 to 3.
; WAITS: no.
; Entry: X the scroll entry, Y as the caller holds it. Exit: as BitmapBitShifted, V from SelectBlockTables' add.
; Clobbers zTemp00, 01, 04, 05, 0C to 0E, 10 and 11.
BlockIndexForPlace:
    lda z:zTemp03
    ror a
    lda z:zTemp02
    ror a
    lsr a
    lsr a
    and #$3C
    sta z:zTemp10
    lda z:zTemp07
    lsr a
    lda z:zTemp06
    sta z:zTemp11
    ror z:zTemp11
    lsr z:zTemp11
    lsr z:zTemp11
    lsr z:zTemp11
    lsr z:zTemp11
    lda z:zTemp03
    sta z:zTemp00
    lda z:zTemp07
    jsr ScreenMapAt
    jsr SelectBlockTablesA
    lda z:zTemp02
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zTemp06
    lsr a
    lsr a
    and #$38
    ora z:zTemp00
    tay
