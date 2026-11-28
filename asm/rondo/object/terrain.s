; @routine 024F:5AB TerrainLookupB
; @run 6B:95AB
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
; attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
; HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
; reads there. Answers go to TileHit, TileDepthX and TileDepthY.
; The same code as TerrainLookup; this overlay's probe table is at $C11F.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
; read or add. Clobbers zTemp00-0D.
TerrainLookupB:
    asl a
    tay
    lda a:$C11F,y
    sta z:zTemp0C
    lda a:$C120,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupB_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookupB_at0
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
TerrainLookupB_at2:
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
    bne TerrainLookupB_at1
    rts
TerrainLookupB_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookupB_at2

; @routine 024F:60B TerrainLookupAltB
; @run 6B:960B
; WHAT IT DOES: TerrainLookupB, then a second look at each column that found floor: eight
; pixels above it, and if that too is solid the floor bits come off the column's answer.
; HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
; as the original has it, not the high.
; The same code as TerrainLookupAlt; this overlay's probe table is at $C11F.
; WAITS: no.
; Entry and exit: as TerrainLookupB.
TerrainLookupAltB:
    pha
    jsr TerrainLookupB
    pla
    asl a
    tay
    lda a:$C11F,y
    sta z:zTemp0C
    lda a:$C120,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAltB_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAltB_at0
    beq TerrainLookupAltB_at0
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
    beq TerrainLookupAltB_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAltB_at1:
    ply
TerrainLookupAltB_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAltB_at2
    rts
TerrainLookupAltB_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAltB_at3

; @routine 050F:5AB TerrainLookup
; @run 6B:95AB
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
; attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
; HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
; reads there. Answers go to TileHit, TileDepthX and TileDepthY.
; WAITS: no.
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
; WAITS: no.
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
; attribute. Attributes $80-$9B go through a table of handlers, entered by a jump.
; WAITS: no.
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
    jmp (zTemp04+ZeroPage)

; @routine 050F:752 TileStepC
; @run 6B:9752
; WHAT IT DOES: A slope tile: moves the point's row by a table on its column.
; HOW: TileStepsC by column, added to the row.
; WAITS: no.
; Entry: zTemp00 column, zTemp02 row. Exit: A $81; Y, C and V as the handler's last step
; leaves them (handler_exit_shim).
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
; WAITS: no.
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
; WAITS: no.
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
; WAITS: no.
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
; WAITS: no.
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
; WAITS: no.
; Entry: zTemp02 row. Exit: A $81; Y, C and V kept.
TileNudgeA:
    inc z:zTemp02
    lda #$81
    rts

; @routine 050F:7AE TileStepD
; @run 6B:97AE
; WHAT IT DOES: A slope tile: moves the point's row by a table on its column.
; HOW: TileStepsD by column, added to the row.
; WAITS: no.
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
; WAITS: no.
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
; WAITS: no.
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
; WAITS: no.
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
; WAITS: no.
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
; WAITS: no.
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
; WAITS: no.
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
; WAITS: no.
; Entry and exit: as TileNudgeA.
TileNudgeB:
    inc z:zTemp02
    lda #$81
    rts

; @routine 0510:175 TileLift7
; @run 6B:9975
; WHAT IT DOES: The tile $95: the point's row seven higher.
; HOW: A subtract.
; WAITS: no.
; Entry: zTemp02 row. Exit: A $81; C and V from the subtract; Y kept.
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
; WAITS: no.
; Entry: zTemp02 row. Exit: A $81; C and V from the subtract; Y kept.
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
; WAITS: no.
; Entry: zTemp02 row. Exit: A $81; C and V from the subtract; Y kept.
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
; WAITS: no.
; Entry: zTemp02 row. Exit: A $81; C and V from the subtract; Y kept.
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
; WAITS: no.
; Entry: zTemp02 row. Exit: A $81; C and V from the subtract; Y kept.
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
; WAITS: no.
; Entry: zTemp02 row. Exit: A $81; C and V from the subtract; Y kept.
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
; WAITS: no.
; Entry: zTemp02 row. Exit: A $81; C and V from the subtract; Y kept.
TileLift1:
    lda z:zTemp02
    sec
    sbc #$01
    sta z:zTemp02
    lda #$81
    rts

; @routine 07CF:5AB TerrainLookupC
; @run 6B:95AB
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
; attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
; HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
; reads there. Answers go to TileHit, TileDepthX and TileDepthY.
; The same code as TerrainLookup; this overlay's probe table is at $D9DE.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
; read or add. Clobbers zTemp00-0D.
TerrainLookupC:
    asl a
    tay
    lda a:$D9DE,y
    sta z:zTemp0C
    lda a:$D9DF,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupC_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookupC_at0
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
TerrainLookupC_at2:
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
    bne TerrainLookupC_at1
    rts
TerrainLookupC_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookupC_at2

; @routine 07CF:60B TerrainLookupAltC
; @run 6B:960B
; WHAT IT DOES: TerrainLookupC, then a second look at each column that found floor: eight
; pixels above it, and if that too is solid the floor bits come off the column's answer.
; HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
; as the original has it, not the high.
; The same code as TerrainLookupAlt; this overlay's probe table is at $D9DE.
; WAITS: no.
; Entry and exit: as TerrainLookupC.
TerrainLookupAltC:
    pha
    jsr TerrainLookupC
    pla
    asl a
    tay
    lda a:$D9DE,y
    sta z:zTemp0C
    lda a:$D9DF,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAltC_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAltC_at0
    beq TerrainLookupAltC_at0
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
    beq TerrainLookupAltC_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAltC_at1:
    ply
TerrainLookupAltC_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAltC_at2
    rts
TerrainLookupAltC_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAltC_at3

; @routine 092F:5AB TerrainLookupD
; @run 6B:95AB
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
; attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
; HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
; reads there. Answers go to TileHit, TileDepthX and TileDepthY.
; The same code as TerrainLookup; this overlay's probe table is at $B547.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
; read or add. Clobbers zTemp00-0D.
TerrainLookupD:
    asl a
    tay
    lda a:$B547,y
    sta z:zTemp0C
    lda a:$B548,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupD_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookupD_at0
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
TerrainLookupD_at2:
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
    bne TerrainLookupD_at1
    rts
TerrainLookupD_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookupD_at2

; @routine 092F:60B TerrainLookupAltD
; @run 6B:960B
; WHAT IT DOES: TerrainLookupD, then a second look at each column that found floor: eight
; pixels above it, and if that too is solid the floor bits come off the column's answer.
; HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
; as the original has it, not the high.
; The same code as TerrainLookupAlt; this overlay's probe table is at $B547.
; WAITS: no.
; Entry and exit: as TerrainLookupD.
TerrainLookupAltD:
    pha
    jsr TerrainLookupD
    pla
    asl a
    tay
    lda a:$B547,y
    sta z:zTemp0C
    lda a:$B548,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAltD_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAltD_at0
    beq TerrainLookupAltD_at0
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
    beq TerrainLookupAltD_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAltD_at1:
    ply
TerrainLookupAltD_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAltD_at2
    rts
TerrainLookupAltD_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAltD_at3

; @routine 0A8F:5AB TerrainLookupE
; @run 6B:95AB
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
; attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
; HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
; reads there. Answers go to TileHit, TileDepthX and TileDepthY.
; The same code as TerrainLookup; this overlay's probe table is at $BD92.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
; read or add. Clobbers zTemp00-0D.
TerrainLookupE:
    asl a
    tay
    lda a:$BD92,y
    sta z:zTemp0C
    lda a:$BD93,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupE_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookupE_at0
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
TerrainLookupE_at2:
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
    bne TerrainLookupE_at1
    rts
TerrainLookupE_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookupE_at2

; @routine 0A8F:60B TerrainLookupAltE
; @run 6B:960B
; WHAT IT DOES: TerrainLookupE, then a second look at each column that found floor: eight
; pixels above it, and if that too is solid the floor bits come off the column's answer.
; HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
; as the original has it, not the high.
; The same code as TerrainLookupAlt; this overlay's probe table is at $BD92.
; WAITS: no.
; Entry and exit: as TerrainLookupE.
TerrainLookupAltE:
    pha
    jsr TerrainLookupE
    pla
    asl a
    tay
    lda a:$BD92,y
    sta z:zTemp0C
    lda a:$BD93,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAltE_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAltE_at0
    beq TerrainLookupAltE_at0
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
    beq TerrainLookupAltE_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAltE_at1:
    ply
TerrainLookupAltE_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAltE_at2
    rts
TerrainLookupAltE_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAltE_at3

; @routine 0BEF:5AB TerrainLookupF
; @run 6B:95AB
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
; attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
; HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
; reads there. Answers go to TileHit, TileDepthX and TileDepthY.
; The same code as TerrainLookup; this overlay's probe table is at $D658.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
; read or add. Clobbers zTemp00-0D.
TerrainLookupF:
    asl a
    tay
    lda a:$D658,y
    sta z:zTemp0C
    lda a:$D659,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupF_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookupF_at0
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
TerrainLookupF_at2:
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
    bne TerrainLookupF_at1
    rts
TerrainLookupF_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookupF_at2

; @routine 0BEF:60B TerrainLookupAltF
; @run 6B:960B
; WHAT IT DOES: TerrainLookupF, then a second look at each column that found floor: eight
; pixels above it, and if that too is solid the floor bits come off the column's answer.
; HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
; as the original has it, not the high.
; The same code as TerrainLookupAlt; this overlay's probe table is at $D658.
; WAITS: no.
; Entry and exit: as TerrainLookupF.
TerrainLookupAltF:
    pha
    jsr TerrainLookupF
    pla
    asl a
    tay
    lda a:$D658,y
    sta z:zTemp0C
    lda a:$D659,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAltF_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAltF_at0
    beq TerrainLookupAltF_at0
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
    beq TerrainLookupAltF_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAltF_at1:
    ply
TerrainLookupAltF_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAltF_at2
    rts
TerrainLookupAltF_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAltF_at3

; @routine 0D4F:5AB TerrainLookupG
; @run 6B:95AB
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
; attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
; HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
; reads there. Answers go to TileHit, TileDepthX and TileDepthY.
; The same code as TerrainLookup; this overlay's probe table is at $D771.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
; read or add. Clobbers zTemp00-0D.
TerrainLookupG:
    asl a
    tay
    lda a:$D771,y
    sta z:zTemp0C
    lda a:$D772,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupG_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookupG_at0
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
TerrainLookupG_at2:
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
    bne TerrainLookupG_at1
    rts
TerrainLookupG_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookupG_at2

; @routine 0D4F:60B TerrainLookupAltG
; @run 6B:960B
; WHAT IT DOES: TerrainLookupG, then a second look at each column that found floor: eight
; pixels above it, and if that too is solid the floor bits come off the column's answer.
; HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
; as the original has it, not the high.
; The same code as TerrainLookupAlt; this overlay's probe table is at $D771.
; WAITS: no.
; Entry and exit: as TerrainLookupG.
TerrainLookupAltG:
    pha
    jsr TerrainLookupG
    pla
    asl a
    tay
    lda a:$D771,y
    sta z:zTemp0C
    lda a:$D772,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAltG_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAltG_at0
    beq TerrainLookupAltG_at0
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
    beq TerrainLookupAltG_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAltG_at1:
    ply
TerrainLookupAltG_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAltG_at2
    rts
TerrainLookupAltG_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAltG_at3

; @routine 0EAF:5AB TerrainLookupH
; @run 6B:95AB
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
; attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
; HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
; reads there. Answers go to TileHit, TileDepthX and TileDepthY.
; The same code as TerrainLookup; this overlay's probe table is at $B23F.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
; read or add. Clobbers zTemp00-0D.
TerrainLookupH:
    asl a
    tay
    lda a:$B23F,y
    sta z:zTemp0C
    lda a:$B240,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupH_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookupH_at0
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
TerrainLookupH_at2:
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
    bne TerrainLookupH_at1
    rts
TerrainLookupH_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookupH_at2

; @routine 0EAF:60B TerrainLookupAltH
; @run 6B:960B
; WHAT IT DOES: TerrainLookupH, then a second look at each column that found floor: eight
; pixels above it, and if that too is solid the floor bits come off the column's answer.
; HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
; as the original has it, not the high.
; The same code as TerrainLookupAlt; this overlay's probe table is at $B23F.
; WAITS: no.
; Entry and exit: as TerrainLookupH.
TerrainLookupAltH:
    pha
    jsr TerrainLookupH
    pla
    asl a
    tay
    lda a:$B23F,y
    sta z:zTemp0C
    lda a:$B240,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAltH_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAltH_at0
    beq TerrainLookupAltH_at0
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
    beq TerrainLookupAltH_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAltH_at1:
    ply
TerrainLookupAltH_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAltH_at2
    rts
TerrainLookupAltH_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAltH_at3

; @routine 100F:5AB TerrainLookupI
; @run 6B:95AB
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
; attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
; HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
; reads there. Answers go to TileHit, TileDepthX and TileDepthY.
; The same code as TerrainLookup; this overlay's probe table is at $D2CF.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
; read or add. Clobbers zTemp00-0D.
TerrainLookupI:
    asl a
    tay
    lda a:$D2CF,y
    sta z:zTemp0C
    lda a:$D2D0,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupI_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookupI_at0
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
TerrainLookupI_at2:
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
    bne TerrainLookupI_at1
    rts
TerrainLookupI_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookupI_at2

; @routine 100F:60B TerrainLookupAltI
; @run 6B:960B
; WHAT IT DOES: TerrainLookupI, then a second look at each column that found floor: eight
; pixels above it, and if that too is solid the floor bits come off the column's answer.
; HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
; as the original has it, not the high.
; The same code as TerrainLookupAlt; this overlay's probe table is at $D2CF.
; WAITS: no.
; Entry and exit: as TerrainLookupI.
TerrainLookupAltI:
    pha
    jsr TerrainLookupI
    pla
    asl a
    tay
    lda a:$D2CF,y
    sta z:zTemp0C
    lda a:$D2D0,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAltI_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAltI_at0
    beq TerrainLookupAltI_at0
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
    beq TerrainLookupAltI_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAltI_at1:
    ply
TerrainLookupAltI_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAltI_at2
    rts
TerrainLookupAltI_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAltI_at3

; @routine 116F:5AB TerrainLookupJ
; @run 6B:95AB
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
; attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
; HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
; reads there. Answers go to TileHit, TileDepthX and TileDepthY.
; The same code as TerrainLookup; this overlay's probe table is at $D9A2.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
; read or add. Clobbers zTemp00-0D.
TerrainLookupJ:
    asl a
    tay
    lda a:$D9A2,y
    sta z:zTemp0C
    lda a:$D9A3,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupJ_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookupJ_at0
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
TerrainLookupJ_at2:
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
    bne TerrainLookupJ_at1
    rts
TerrainLookupJ_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookupJ_at2

; @routine 116F:60B TerrainLookupAltJ
; @run 6B:960B
; WHAT IT DOES: TerrainLookupJ, then a second look at each column that found floor: eight
; pixels above it, and if that too is solid the floor bits come off the column's answer.
; HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
; as the original has it, not the high.
; The same code as TerrainLookupAlt; this overlay's probe table is at $D9A2.
; WAITS: no.
; Entry and exit: as TerrainLookupJ.
TerrainLookupAltJ:
    pha
    jsr TerrainLookupJ
    pla
    asl a
    tay
    lda a:$D9A2,y
    sta z:zTemp0C
    lda a:$D9A3,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAltJ_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAltJ_at0
    beq TerrainLookupAltJ_at0
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
    beq TerrainLookupAltJ_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAltJ_at1:
    ply
TerrainLookupAltJ_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAltJ_at2
    rts
TerrainLookupAltJ_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAltJ_at3

; @routine 160F:5AB TerrainLookupK
; @run 6B:95AB
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object: the
; attribute, and how far into the tile, for each. A column marked as absent reads as nothing.
; HOW: The probe table gives each column's offsets from the object, and TerrainTileRead
; reads there. Answers go to TileHit, TileDepthX and TileDepthY.
; The same code as TerrainLookup; this overlay's probe table is at $D6C8.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's
; read or add. Clobbers zTemp00-0D.
TerrainLookupK:
    asl a
    tay
    lda a:$D6C8,y
    sta z:zTemp0C
    lda a:$D6C9,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupK_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookupK_at0
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
TerrainLookupK_at2:
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
    bne TerrainLookupK_at1
    rts
TerrainLookupK_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookupK_at2

; @routine 160F:60B TerrainLookupAltK
; @run 6B:960B
; WHAT IT DOES: TerrainLookupK, then a second look at each column that found floor: eight
; pixels above it, and if that too is solid the floor bits come off the column's answer.
; HOW: The same probe's offsets. The second add at $9657 goes back into the low byte of Y,
; as the original has it, not the high.
; The same code as TerrainLookupAlt; this overlay's probe table is at $D6C8.
; WAITS: no.
; Entry and exit: as TerrainLookupK.
TerrainLookupAltK:
    pha
    jsr TerrainLookupK
    pla
    asl a
    tay
    lda a:$D6C8,y
    sta z:zTemp0C
    lda a:$D6C9,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAltK_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAltK_at0
    beq TerrainLookupAltK_at0
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
    beq TerrainLookupAltK_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAltK_at1:
    ply
TerrainLookupAltK_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAltK_at2
    rts
TerrainLookupAltK_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAltK_at3
