; @routine 0255:7A5 DrawBlockColumn
; @run 6F:A7A5
; WHAT IT DOES: Draws a column of a layer's tile blocks for scroll entry X: as many blocks as $3E66's low seven bits,
; from the place the entry has reached (its scroll X plus the overlay's offset of word Y, and its scroll Y, plus the
; offset at $A781 and $A783 when $3E66's bit 7 is set), down the screen and on into the screen below.
; HOW: BlockIndexForPlace for the first block; then each block number from the second table (zTemp04) by Y goes to
; CopyTileBlockToVram with the bank bit; Y steps by 8 (a row of blocks) with the bit read again, and past $40 the
; next screen row's map byte selects the tables again. zTemp08 counts the blocks. Each JSR goes to the overlay's
; routine when that is the one resident at its address, to the original otherwise, as the original does.
; RESIDENT: the code's callees at $A000 (CopyTileBlockToVram) and $A878 (BitmapBitShifted) are bytes $0000 and $0878
; of the bank that holds this routine (6F under MPR5: sectors 0255 to 0258 and each overlay's copy of them), so when
; these bytes run at $A7A5 those are the bytes at $A000 and $A878. Every scene record that loads one of the 21
; overlays reads its sector B+$44 for 12 sectors to $8000 under MPR4 7 (the op before reads B+$38 into bank 7's first
; three sectors, overwritten by it; the ops after go to banks 10 to 12), so 0255 and 0256 land together and whole;
; no other read the game makes reaches the bank (the boot's, the old-card program's, the five constant cd_reads), and
; nothing writes its code bytes after. The resident_at fallbacks are what the original does where other code is
; mapped; liftcmp's curated cases (tools/rondo/curated/layer_columns.txt) keep zRamBaseBank $68, the game's, so
; CopyTileBlockToVram's MPR6 lands on a mapper the trace has.
; WAITS: no.
; Entry: X the entry, Y the offset's index. Exit: A as the last block copy left it ($10), N 0, Z 1 (the count's
; decrement), C and V from the copy, Y the last block's index, X kept.
DrawBlockColumn:
    lda a:ScrollColumnBlocks,x
    and #$7F
    sta z:zTemp08
    jsr ScrollXPlusOffset
    lda a:ScrollYLo,x
    sta z:zTemp06
    lda a:ScrollYHiScreen,x
    sta z:zTemp07
    lda a:ScrollColumnBlocks,x
    bpl DrawBlockColumn_at0
    ldy #$01
    lda a:ScrollYLo,x
    clc
    adc a:LayerPlaceOffsetsLo,y
    sta z:zTemp06
    lda a:ScrollYHiScreen,x
    adc a:LayerPlaceOffsetsHi,y
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

; @routine 0256:0B4 RowDueUp
; @run 6F:A8B4
; WHAT IT DOES: For an entry moving up: when one less than its scroll Y and one less than its band's have bit 5
; different, it crossed a row, which is drawn.
; HOW: The two values less one in zTemp09 and A, EOR, AND #$20; the BNE and BRA to DrawBlockRow.
; WAITS: no.
; Entry: X the entry, Y its index for DrawBlockRow. Exit: not crossed: A 0, N 0, Z 1, X, Y, C and V kept; else as
; DrawBlockRow.
RowDueUp:
    lda a:ScrollBgyLo,x
    dec a
    sta z:zTemp09
    lda a:ScrollYLo,x
    dec a
    eor z:zTemp09
    and #$20
    bne RowDueUp_at0
    rts
RowDueUp_at0:
    bra DrawBlockRow

; @routine 0256:0C7 RowDueAt
; @run 6F:A8C7
; WHAT IT DOES: Draws the row an entry has crossed, if it has: moving up (Y zero) as RowDueUp; else when its scroll Y
; and its band's differ in bit 5.
; HOW: TYA; EOR of the two, AND #$20; the BNE into DrawBlockRow.
; WAITS: no.
; Entry: X the entry, Y 0 moving up, 1 moving down. Exit: not crossed: A 0, N 0, Z 1, X, Y, C and V kept; else as
; DrawBlockRow.
RowDueAt:
    tya
    beq RowDueUp
    lda a:ScrollYLo,x
    eor a:ScrollBgyLo,x
    and #$20
    bne DrawBlockRow
    rts

; @routine 0256:0D5 DrawBlockRow
; @run 6F:A8D5
; WHAT IT DOES: Draws a row of a layer's tile blocks for scroll entry X: as many blocks as entry 0's ScrollColumnBlocks'
; low seven bits, from the place the entry has reached (its scroll Y plus the overlay's offset of word Y, and its
; scroll X, plus the offset at $A781 and $A783 when that byte's bit 7 is set), across the screen and on into the next.
; HOW: The count in zTemp08; ScrollYPlusOffset; BlockIndexForPlace for the first block; each block from the table at
; zTemp04 by Y goes to CopyTileBlockToVram with the VRAM address zTemp10-11 kept on the stack across it and moved four
; words on (within $40); Y steps by one, with BitmapBitShifted for the bank bit, and past each eighth the next screen's
; map byte selects the tables again. Each JSR goes to the routine loaded at its address.
; WAITS: no.
; Entry: X the entry, Y the offset's index. Exit: A zTemp10's new value, N 0, Z 1 (the count's DEC), C and V from its
; add, Y the last block's index, X kept.
DrawBlockRow:
    lda a:ScrollColumnBlocks
    and #$7F
    sta z:zTemp08
    jsr ScrollYPlusOffset
    lda a:ScrollXLo,x
    sta z:zTemp02
    lda a:ScrollXHiScreen,x
    sta z:zTemp03
    lda a:ScrollColumnBlocks
    bpl DrawBlockRow_at0
    ldy #$01
    lda a:ScrollXLo,x
    clc
    adc a:LayerPlaceOffsetsLo,y
    sta z:zTemp02
    lda a:ScrollXHiScreen,x
    adc a:LayerPlaceOffsetsHi,y
    and #$0F
    sta z:zTemp03
DrawBlockRow_at0:
    jsr BlockIndexForPlace
DrawBlockRow_at3:
    phy
    lda z:zTemp10
    pha
    lda z:zTemp11
    pha
    lda (zTemp04),y
    jsr CopyTileBlockToVram
    pla
    sta z:zTemp11
    pla
    clc
    adc #$04
    and #$3F
    sta z:zTemp10
    ply
    dec z:zTemp08
    bne DrawBlockRow_at1
    rts
DrawBlockRow_at1:
    iny
    tya
    and #$07
    beq DrawBlockRow_at2
    jsr BitmapBitShifted
    bra DrawBlockRow_at3
DrawBlockRow_at2:
    lda z:zTemp03
    inc a
    and #$0F
    sta z:zTemp03
    sta z:zTemp00
    lda z:zTemp07
    jsr ScreenMapAt
    jsr SelectBlockTablesA
    lda z:zTemp06
    lsr a
    lsr a
    and #$38
    tay
    jsr BitmapBitShifted
    jmp DrawBlockRow_at3

