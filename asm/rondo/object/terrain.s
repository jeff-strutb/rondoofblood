; @routine 050F:5AB TerrainLookup
; @run 6B:95AB
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
; attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
; HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
; reads there. Answers go to TileHit, TileDepthX and TileDepthY.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
; read or add. Clobbers zTemp00-0D.
TerrainLookup:
    asl a
    tay
    lda a:TerrainProbes,y
    sta z:zTemp0C
    lda a:TerrainProbes+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookup_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookup_at0
    plp
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
    sta z:zTemp03
    iny
    phy
    jsr TerrainTileRead
TerrainLookup_at2:
    ldy z:zTemp0A
    sta a:TileHit,y
    lda z:zTemp00
    sta a:TileDepthX,y
    lda z:zTemp02
    sta a:TileDepthY,y
    inc z:zTemp0A
    ply
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookup_at1
    rts
TerrainLookup_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookup_at2

; @routine 050F:60B TerrainLookupAlt
; @run 6B:960B
; WHAT IT DOES: TerrainLookup, then a second look at each column that found floor: eight
; pixels above it, and if that too is solid the floor bits come off the column's answer.
; HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
; as the original has it, not the high.
; Entry and exit: as TerrainLookup.
TerrainLookupAlt:
    pha
    jsr TerrainLookup
    pla
    asl a
    tay
    lda a:TerrainProbes,y
    sta z:zTemp0C
    lda a:TerrainProbes+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAlt_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAlt_at0
    beq TerrainLookupAlt_at0
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    lda (zTemp0C),y
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
    sta z:zTemp03
    iny
    lda #$F8
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$FF
    adc z:zTemp02
    sta z:zTemp02
    phy
    jsr TerrainTileRead
    ldy z:zTemp0A
    and #$01
    beq TerrainLookupAlt_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAlt_at1:
    ply
TerrainLookupAlt_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAlt_at2
    rts
TerrainLookupAlt_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAlt_at3

; @routine 050F:67F TerrainTileRead
; @run 6B:967F
; WHAT IT DOES: Says what the map holds at a point: the attribute of the background tile
; there, 0 if its page has none, $81 for a slope or step, which moves the row in the tile.
; HOW: The point plus the scroll gives the tile's place in the 64 by 128 background; its
; entry is read from VRAM. The entry's low byte and a page by its high bits give the
; attribute. Attributes $80-$9B go through a table of handlers.
; Entry: zTemp00-01 X, zTemp02-03 Y. Exit: A the answer, zTemp00 the column, zTemp02 the row.
; X kept. Clobbers zTemp04-09.
TerrainTileRead:
    lda z:zTemp00
    clc
    adc a:ScrollXLo
    sta z:zTemp04
    lda z:zTemp01
    adc a:ScrollXHi
    and #$01
    lsr a
    lda z:zTemp04
    ror a
    lsr a
    lsr a
    and #$3F
    sta z:zTemp05
    lda z:zTemp04
    and #$07
    sta z:zTemp00
    lda z:zTemp02
    clc
    adc a:ScrollYLo
    sta z:zTemp06
    lda z:zTemp03
    adc a:ScrollYHi
    and #$01
    sta z:zTemp07
    lda z:zTemp06
    asl a
    rol z:zTemp07
    asl a
    rol z:zTemp07
    asl a
    rol z:zTemp07
    and #$C0
    clc
    adc z:zTemp05
    sta z:zTemp05
    lda z:zTemp06
    and #$07
    sta z:zTemp02
    lda z:zTemp05
    sta z:zTemp06
    jsr VecSelectVramReadAddress
    lda z:zTemp05
    sta a:VDC_DATA_LO
    lda z:zTemp07
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    sta z:zTemp08
    lda a:VDC_DATA_HI
    and #$07
    sta z:zTemp09
    tay
    lda a:SaveBuffer,y
    bmi TerrainTileRead_at0
    lda z:zTemp08
    clc
    adc #$00
    sta z:zTemp08
    lda a:SaveBuffer,y
    adc #$7D
    sta z:zTemp09
    lda (zTemp08)
    bmi TerrainTileRead_at1
    rts
TerrainTileRead_at0:
    lda #$00
TerrainTileRead_at2:
    rts
TerrainTileRead_at1:
    cmp #$9C
    bcs TerrainTileRead_at2
    pha
    sbc #$7F
    asl a
    tay
    lda a:TileHandlers,y
    sta z:zTemp04
    lda a:TileHandlers+1,y
    sta z:zTemp05
    pla
    jmp (zTemp04+$2000)

; @routine 050F:752 TileStepC
; @run 6B:9752
; WHAT IT DOES: A slope tile: moves the point's row by a table on its column.
; HOW: TileStepsC by column, added to the row.
; Entry: zTemp00 column, zTemp02 row. Exit: as tile_handler_exit_shim for the tile in A.
TileStepC:
    ldy z:zTemp00
    lda a:TileStepsC,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 050F:75F TileRowA
; @run 6B:975F
; WHAT IT DOES: A slope tile: gives the point a new row from a table on its row and column.
; HOW: TileRowsA, eight rows of eight.
; Entry and exit: as TileStepC.
TileRowA:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowsA,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 050F:770 TileRowB
; @run 6B:9770
; WHAT IT DOES: A slope tile: gives the point a new row from a table on its row and column.
; HOW: TileRowsB, eight rows of eight.
; Entry and exit: as TileStepC.
TileRowB:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowsB,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 050F:781 TileHalfStepA
; @run 6B:9781
; WHAT IT DOES: The tiles $87-$8A: a step that differs between the tile's left and right
; halves.
; HOW: A pair of bytes in TileHalfStepsA by tile, the second for the right half.
; Entry: A tile, zTemp00 column, zTemp02 row. Exit: as TileStepC.
TileHalfStepA:
    sec
    sbc #$83
    asl a
    tay
    bbr2 zTemp00,TileHalfStepA_at0
    iny
TileHalfStepA_at0:
    lda a:TileHalfStepsA,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 050F:795 TileHalfStepB
; @run 6B:9795
; WHAT IT DOES: The tiles $8C-$8F: as TileHalfStepA, the other way round.
; HOW: A pair of bytes in TileHalfStepsB by tile, the second for the left half.
; Entry and exit: as TileHalfStepA.
TileHalfStepB:
    sec
    sbc #$8C
    asl a
    tay
    bbs2 zTemp00,TileHalfStepB_at0
    iny
TileHalfStepB_at0:
    lda a:TileHalfStepsB,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 050F:7A9 TileNudgeA
; @run 6B:97A9
; WHAT IT DOES: The tiles $86 and $8B: the point's row one lower.
; HOW: An increment.
; Entry: zTemp02 row. Exit: as TileStepC, Y, C and V kept.
TileNudgeA:
    inc z:zTemp02
    lda #$81
    rts

; @routine 050F:7AE TileStepD
; @run 6B:97AE
; WHAT IT DOES: A slope tile: moves the point's row by a table on its column.
; HOW: TileStepsD by column, added to the row.
; Entry and exit: as TileStepC.
TileStepD:
    ldy z:zTemp00
    lda a:TileStepsD,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 050F:7BB TileRowC
; @run 6B:97BB
; WHAT IT DOES: A slope tile: gives the point a new row from a table on its row and column.
; HOW: TileRowsC, eight rows of eight.
; Entry and exit: as TileStepC.
TileRowC:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowsC,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 050F:7CC TileRowD
; @run 6B:97CC
; WHAT IT DOES: A slope tile: gives the point a new row from a table on its row and column.
; HOW: TileRowsD, eight rows of eight.
; Entry and exit: as TileStepC.
TileRowD:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowsD,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 0510:0FD TileStepTwice
; @run 6B:98FD
; WHAT IT DOES: A slope tile: moves the row by the offset found through the column's offset.
; HOW: One past the column's byte of TileRowOffsets indexes that table again.
; Entry and exit: as TileStepC.
TileStepTwice:
    ldy z:zTemp00
    lda a:TileRowOffsets,y
    inc a
    tay
    bra TileStepOffset_at0

; @routine 0510:106 TileRowOffset
; @run 6B:9906
; WHAT IT DOES: A slope tile: a new row from TileRowsE, with the column's offset for its column.
; HOW: Row times eight, plus the offset and one, indexes the table.
; Entry and exit: as TileStepC.
TileRowOffset:
    lda z:zTemp02
    asl a
    asl a
    asl a
    ldy z:zTemp00
    clc
    adc a:TileRowOffsets,y
    inc a
    bra TileRowE_at0

; @routine 0510:114 TileStepOffset
; @run 6B:9914
; WHAT IT DOES: A slope tile: moves the row by one more than the column's offset.
; HOW: TileRowOffsets by column, added to the row with one.
; Entry and exit: as TileStepC.
TileStepOffset:
    ldy z:zTemp00
TileStepOffset_at0:
    lda z:zTemp02
    clc
    adc a:TileRowOffsets,y
    inc a
    bra TileRowE_at1

; @routine 0510:11F TileRowE
; @run 6B:991F
; WHAT IT DOES: A slope tile: gives the point a new row from a table on its row and column.
; HOW: TileRowsE, eight rows of eight.
; Entry and exit: as TileStepC.
TileRowE:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
TileRowE_at0:
    tay
    lda a:TileRowsE,y
TileRowE_at1:
    sta z:zTemp02
    lda #$81
    rts

; @routine 0510:170 TileNudgeB
; @run 6B:9970
; WHAT IT DOES: The tile $94: the point's row one lower.
; HOW: An increment.
; Entry and exit: as TileNudgeA.
TileNudgeB:
    inc z:zTemp02
    lda #$81
    rts

; @routine 0510:175 TileLift7
; @run 6B:9975
; WHAT IT DOES: The tile $95: the point's row seven higher.
; HOW: A subtract.
; Entry: zTemp02 row. Exit: as TileStepC, Y kept.
TileLift7:
    lda z:zTemp02
    sec
    sbc #$07
    sta z:zTemp02
    lda #$81
    rts

; @routine 0510:17F TileLift6
; @run 6B:997F
; WHAT IT DOES: The tile $96: the point's row six higher.
; HOW: A subtract.
; Entry and exit: as TileLift7.
TileLift6:
    lda z:zTemp02
    sec
    sbc #$06
    sta z:zTemp02
    lda #$81
    rts

; @routine 0510:189 TileLift5
; @run 6B:9989
; WHAT IT DOES: The tile $97: the point's row five higher.
; HOW: A subtract.
; Entry and exit: as TileLift7.
TileLift5:
    lda z:zTemp02
    sec
    sbc #$05
    sta z:zTemp02
    lda #$81
    rts

; @routine 0510:193 TileLift4
; @run 6B:9993
; WHAT IT DOES: The tile $98: the point's row four higher.
; HOW: A subtract.
; Entry and exit: as TileLift7.
TileLift4:
    lda z:zTemp02
    sec
    sbc #$04
    sta z:zTemp02
    lda #$81
    rts

; @routine 0510:19D TileLift3
; @run 6B:999D
; WHAT IT DOES: The tile $99: the point's row three higher.
; HOW: A subtract.
; Entry and exit: as TileLift7.
TileLift3:
    lda z:zTemp02
    sec
    sbc #$03
    sta z:zTemp02
    lda #$81
    rts

; @routine 0510:1A7 TileLift2
; @run 6B:99A7
; WHAT IT DOES: The tile $9A: the point's row two higher.
; HOW: A subtract.
; Entry and exit: as TileLift7.
TileLift2:
    lda z:zTemp02
    sec
    sbc #$02
    sta z:zTemp02
    lda #$81
    rts

; @routine 0510:1B1 TileLift1
; @run 6B:99B1
; WHAT IT DOES: The tile $9B: the point's row one higher.
; HOW: A subtract.
; Entry and exit: as TileLift7.
TileLift1:
    lda z:zTemp02
    sec
    sbc #$01
    sta z:zTemp02
    lda #$81
    rts
