; @routine 05C0:0BD ProbeBoxes05C0
; @run 6B:98BD
; WHAT IT DOES: TerrainLookup in the overlay at sector 05BF: reads the terrain at the four columns of a probe
; around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
; HOW: The shared lookup body with this overlay's probe table at $BCEC and 05BF's group's ProbeTileRead9991.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
; Clobbers zTemp00-0D.
ProbeBoxes05C0:
    asl a
    tay
    lda a:$BCEC,y
    sta z:zTemp0C
    lda a:$BCED,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq ProbeBoxes05C0_at0
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
    jsr ProbeTileRead9991
ProbeBoxes05C0_at1:
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
    bne DrawSelectedFileDigit
    rts
ProbeBoxes05C0_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra ProbeBoxes05C0_at1

; @routine 05C0:11D ProbeBoxesFlagged05C0
; @run 6B:991D
; WHAT IT DOES: TerrainLookupAlt in the overlay at sector 05BF: ProbeBoxes05C0, then a second look eight pixels
; above each column that found floor; if that too is solid the floor bits come off the column's answer.
; HOW: The shared second-look body with the same table at $BCEC.
; WAITS: no.
; Entry, exit: as ProbeBoxes05C0.
ProbeBoxesFlagged05C0:
    pha
    jsr ProbeBoxes05C0
    pla
    asl a
    tay
    lda a:$BCEC,y
    sta z:zTemp0C
    lda a:$BCED,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
ProbeBoxesFlagged05C0_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi ProbeBoxesFlagged05C0_at0
    beq ProbeBoxesFlagged05C0_at0
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
    jsr ProbeTileRead9991
    ldy z:zTemp0A
    and #$01
    beq ProbeBoxesFlagged05C0_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
ProbeBoxesFlagged05C0_at1:
    ply
ProbeBoxesFlagged05C0_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne ProbeBoxesFlagged05C0_at2
    rts
ProbeBoxesFlagged05C0_at0:
    iny
    iny
    iny
    iny
    bra ProbeBoxesFlagged05C0_at3

; @routine 0720:0BD ProbeBoxes0720
; @run 6B:98BD
; WHAT IT DOES: TerrainLookup in the overlay at sector 071F: reads the terrain at the four columns of a probe
; around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
; HOW: The shared lookup body with this overlay's probe table at $C0D4 and 05BF's group's ProbeTileRead9991.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
; Clobbers zTemp00-0D.
ProbeBoxes0720:
    asl a
    tay
    lda a:$C0D4,y
    sta z:zTemp0C
    lda a:$C0D5,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq ProbeBoxes0720_at0
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
    jsr ProbeTileRead9991
ProbeBoxes0720_at1:
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
    bne DrawSelectedFileDigit
    rts
ProbeBoxes0720_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra ProbeBoxes0720_at1

; @routine 0720:11D ProbeBoxesFlagged0720
; @run 6B:991D
; WHAT IT DOES: TerrainLookupAlt in the overlay at sector 071F: ProbeBoxes0720, then a second look eight pixels
; above each column that found floor; if that too is solid the floor bits come off the column's answer.
; HOW: The shared second-look body with the same table at $C0D4.
; WAITS: no.
; Entry, exit: as ProbeBoxes0720.
ProbeBoxesFlagged0720:
    pha
    jsr ProbeBoxes0720
    pla
    asl a
    tay
    lda a:$C0D4,y
    sta z:zTemp0C
    lda a:$C0D5,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
ProbeBoxesFlagged0720_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi ProbeBoxesFlagged0720_at0
    beq ProbeBoxesFlagged0720_at0
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
    jsr ProbeTileRead9991
    ldy z:zTemp0A
    and #$01
    beq ProbeBoxesFlagged0720_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
ProbeBoxesFlagged0720_at1:
    ply
ProbeBoxesFlagged0720_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne ProbeBoxesFlagged0720_at2
    rts
ProbeBoxesFlagged0720_at0:
    iny
    iny
    iny
    iny
    bra ProbeBoxesFlagged0720_at3

; @routine 0880:0BD ProbeBoxes0880
; @run 6B:98BD
; WHAT IT DOES: TerrainLookup in the overlay at sector 087F: reads the terrain at the four columns of a probe
; around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
; HOW: The shared lookup body with this overlay's probe table at $C251 and 05BF's group's ProbeTileRead9991.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
; Clobbers zTemp00-0D.
ProbeBoxes0880:
    asl a
    tay
    lda a:$C251,y
    sta z:zTemp0C
    lda a:$C252,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq ProbeBoxes0880_at0
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
    jsr ProbeTileRead9991
ProbeBoxes0880_at1:
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
    bne DrawSelectedFileDigit
    rts
ProbeBoxes0880_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra ProbeBoxes0880_at1

; @routine 0880:11D ProbeBoxesFlagged0880
; @run 6B:991D
; WHAT IT DOES: TerrainLookupAlt in the overlay at sector 087F: ProbeBoxes0880, then a second look eight pixels
; above each column that found floor; if that too is solid the floor bits come off the column's answer.
; HOW: The shared second-look body with the same table at $C251.
; WAITS: no.
; Entry, exit: as ProbeBoxes0880.
ProbeBoxesFlagged0880:
    pha
    jsr ProbeBoxes0880
    pla
    asl a
    tay
    lda a:$C251,y
    sta z:zTemp0C
    lda a:$C252,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
ProbeBoxesFlagged0880_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi ProbeBoxesFlagged0880_at0
    beq ProbeBoxesFlagged0880_at0
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
    jsr ProbeTileRead9991
    ldy z:zTemp0A
    and #$01
    beq ProbeBoxesFlagged0880_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
ProbeBoxesFlagged0880_at1:
    ply
ProbeBoxesFlagged0880_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne ProbeBoxesFlagged0880_at2
    rts
ProbeBoxesFlagged0880_at0:
    iny
    iny
    iny
    iny
    bra ProbeBoxesFlagged0880_at3

; @routine 09E0:0BD ProbeBoxes09E0
; @run 6B:98BD
; WHAT IT DOES: TerrainLookup in the overlay at sector 09DF: reads the terrain at the four columns of a probe
; around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
; HOW: The shared lookup body with this overlay's probe table at $C186 and 05BF's group's ProbeTileRead9991.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
; Clobbers zTemp00-0D.
ProbeBoxes09E0:
    asl a
    tay
    lda a:$C186,y
    sta z:zTemp0C
    lda a:$C187,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq ProbeBoxes09E0_at0
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
    jsr ProbeTileRead9991
ProbeBoxes09E0_at1:
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
    bne DrawSelectedFileDigit
    rts
ProbeBoxes09E0_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra ProbeBoxes09E0_at1

; @routine 09E0:11D ProbeBoxesFlagged09E0
; @run 6B:991D
; WHAT IT DOES: TerrainLookupAlt in the overlay at sector 09DF: ProbeBoxes09E0, then a second look eight pixels
; above each column that found floor; if that too is solid the floor bits come off the column's answer.
; HOW: The shared second-look body with the same table at $C186.
; WAITS: no.
; Entry, exit: as ProbeBoxes09E0.
ProbeBoxesFlagged09E0:
    pha
    jsr ProbeBoxes09E0
    pla
    asl a
    tay
    lda a:$C186,y
    sta z:zTemp0C
    lda a:$C187,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
ProbeBoxesFlagged09E0_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi ProbeBoxesFlagged09E0_at0
    beq ProbeBoxesFlagged09E0_at0
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
    jsr ProbeTileRead9991
    ldy z:zTemp0A
    and #$01
    beq ProbeBoxesFlagged09E0_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
ProbeBoxesFlagged09E0_at1:
    ply
ProbeBoxesFlagged09E0_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne ProbeBoxesFlagged09E0_at2
    rts
ProbeBoxesFlagged09E0_at0:
    iny
    iny
    iny
    iny
    bra ProbeBoxesFlagged09E0_at3

; @routine 0B40:0BD ProbeBoxes0B40
; @run 6B:98BD
; WHAT IT DOES: TerrainLookup in the overlay at sector 0B3F: reads the terrain at the four columns of a probe
; around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
; HOW: The shared lookup body with this overlay's probe table at $B38E and 05BF's group's ProbeTileRead9991.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
; Clobbers zTemp00-0D.
ProbeBoxes0B40:
    asl a
    tay
    lda a:$B38E,y
    sta z:zTemp0C
    lda a:$B38F,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq ProbeBoxes0B40_at0
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
    jsr ProbeTileRead9991
ProbeBoxes0B40_at1:
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
    bne DrawSelectedFileDigit
    rts
ProbeBoxes0B40_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra ProbeBoxes0B40_at1

; @routine 0B40:11D ProbeBoxesFlagged0B40
; @run 6B:991D
; WHAT IT DOES: TerrainLookupAlt in the overlay at sector 0B3F: ProbeBoxes0B40, then a second look eight pixels
; above each column that found floor; if that too is solid the floor bits come off the column's answer.
; HOW: The shared second-look body with the same table at $B38E.
; WAITS: no.
; Entry, exit: as ProbeBoxes0B40.
ProbeBoxesFlagged0B40:
    pha
    jsr ProbeBoxes0B40
    pla
    asl a
    tay
    lda a:$B38E,y
    sta z:zTemp0C
    lda a:$B38F,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
ProbeBoxesFlagged0B40_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi ProbeBoxesFlagged0B40_at0
    beq ProbeBoxesFlagged0B40_at0
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
    jsr ProbeTileRead9991
    ldy z:zTemp0A
    and #$01
    beq ProbeBoxesFlagged0B40_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
ProbeBoxesFlagged0B40_at1:
    ply
ProbeBoxesFlagged0B40_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne ProbeBoxesFlagged0B40_at2
    rts
ProbeBoxesFlagged0B40_at0:
    iny
    iny
    iny
    iny
    bra ProbeBoxesFlagged0B40_at3

; @routine 0CA0:0BD ProbeBoxes0CA0
; @run 6B:98BD
; WHAT IT DOES: TerrainLookup in the overlay at sector 0C9F: reads the terrain at the four columns of a probe
; around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
; HOW: The shared lookup body with this overlay's probe table at $C1A5 and 05BF's group's ProbeTileRead9991.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
; Clobbers zTemp00-0D.
ProbeBoxes0CA0:
    asl a
    tay
    lda a:$C1A5,y
    sta z:zTemp0C
    lda a:$C1A6,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq ProbeBoxes0CA0_at0
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
    jsr ProbeTileRead9991
ProbeBoxes0CA0_at1:
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
    bne DrawSelectedFileDigit
    rts
ProbeBoxes0CA0_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra ProbeBoxes0CA0_at1

; @routine 0CA0:11D ProbeBoxesFlagged0CA0
; @run 6B:991D
; WHAT IT DOES: TerrainLookupAlt in the overlay at sector 0C9F: ProbeBoxes0CA0, then a second look eight pixels
; above each column that found floor; if that too is solid the floor bits come off the column's answer.
; HOW: The shared second-look body with the same table at $C1A5.
; WAITS: no.
; Entry, exit: as ProbeBoxes0CA0.
ProbeBoxesFlagged0CA0:
    pha
    jsr ProbeBoxes0CA0
    pla
    asl a
    tay
    lda a:$C1A5,y
    sta z:zTemp0C
    lda a:$C1A6,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
ProbeBoxesFlagged0CA0_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi ProbeBoxesFlagged0CA0_at0
    beq ProbeBoxesFlagged0CA0_at0
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
    jsr ProbeTileRead9991
    ldy z:zTemp0A
    and #$01
    beq ProbeBoxesFlagged0CA0_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
ProbeBoxesFlagged0CA0_at1:
    ply
ProbeBoxesFlagged0CA0_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne ProbeBoxesFlagged0CA0_at2
    rts
ProbeBoxesFlagged0CA0_at0:
    iny
    iny
    iny
    iny
    bra ProbeBoxesFlagged0CA0_at3

; @routine 0E00:0BD ProbeBoxes0E00
; @run 6B:98BD
; WHAT IT DOES: TerrainLookup in the overlay at sector 0DFF: reads the terrain at the four columns of a probe
; around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
; HOW: The shared lookup body with this overlay's probe table at $AF23 and 05BF's group's ProbeTileRead9991.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
; Clobbers zTemp00-0D.
ProbeBoxes0E00:
    asl a
    tay
    lda a:$AF23,y
    sta z:zTemp0C
    lda a:$AF24,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq ProbeBoxes0E00_at0
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
    jsr ProbeTileRead9991
ProbeBoxes0E00_at1:
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
    bne DrawSelectedFileDigit
    rts
ProbeBoxes0E00_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra ProbeBoxes0E00_at1

; @routine 0E00:11D ProbeBoxesFlagged0E00
; @run 6B:991D
; WHAT IT DOES: TerrainLookupAlt in the overlay at sector 0DFF: ProbeBoxes0E00, then a second look eight pixels
; above each column that found floor; if that too is solid the floor bits come off the column's answer.
; HOW: The shared second-look body with the same table at $AF23.
; WAITS: no.
; Entry, exit: as ProbeBoxes0E00.
ProbeBoxesFlagged0E00:
    pha
    jsr ProbeBoxes0E00
    pla
    asl a
    tay
    lda a:$AF23,y
    sta z:zTemp0C
    lda a:$AF24,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
ProbeBoxesFlagged0E00_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi ProbeBoxesFlagged0E00_at0
    beq ProbeBoxesFlagged0E00_at0
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
    jsr ProbeTileRead9991
    ldy z:zTemp0A
    and #$01
    beq ProbeBoxesFlagged0E00_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
ProbeBoxesFlagged0E00_at1:
    ply
ProbeBoxesFlagged0E00_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne ProbeBoxesFlagged0E00_at2
    rts
ProbeBoxesFlagged0E00_at0:
    iny
    iny
    iny
    iny
    bra ProbeBoxesFlagged0E00_at3

; @routine 0F60:0BD ProbeBoxes0F60
; @run 6B:98BD
; WHAT IT DOES: TerrainLookup in the overlay at sector 0F5F: reads the terrain at the four columns of a probe
; around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
; HOW: The shared lookup body with this overlay's probe table at $B01F and 05BF's group's ProbeTileRead9991.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
; Clobbers zTemp00-0D.
ProbeBoxes0F60:
    asl a
    tay
    lda a:$B01F,y
    sta z:zTemp0C
    lda a:$B020,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq ProbeBoxes0F60_at0
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
    jsr ProbeTileRead9991
ProbeBoxes0F60_at1:
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
    bne DrawSelectedFileDigit
    rts
ProbeBoxes0F60_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra ProbeBoxes0F60_at1

; @routine 0F60:11D ProbeBoxesFlagged0F60
; @run 6B:991D
; WHAT IT DOES: TerrainLookupAlt in the overlay at sector 0F5F: ProbeBoxes0F60, then a second look eight pixels
; above each column that found floor; if that too is solid the floor bits come off the column's answer.
; HOW: The shared second-look body with the same table at $B01F.
; WAITS: no.
; Entry, exit: as ProbeBoxes0F60.
ProbeBoxesFlagged0F60:
    pha
    jsr ProbeBoxes0F60
    pla
    asl a
    tay
    lda a:$B01F,y
    sta z:zTemp0C
    lda a:$B020,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
ProbeBoxesFlagged0F60_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi ProbeBoxesFlagged0F60_at0
    beq ProbeBoxesFlagged0F60_at0
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
    jsr ProbeTileRead9991
    ldy z:zTemp0A
    and #$01
    beq ProbeBoxesFlagged0F60_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
ProbeBoxesFlagged0F60_at1:
    ply
ProbeBoxesFlagged0F60_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne ProbeBoxesFlagged0F60_at2
    rts
ProbeBoxesFlagged0F60_at0:
    iny
    iny
    iny
    iny
    bra ProbeBoxesFlagged0F60_at3

; @routine 10C0:0BD ProbeBoxes10C0
; @run 6B:98BD
; WHAT IT DOES: TerrainLookup in the overlay at sector 10BF: reads the terrain at the four columns of a probe
; around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
; HOW: The shared lookup body with this overlay's probe table at $C511 and 05BF's group's ProbeTileRead9991.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
; Clobbers zTemp00-0D.
ProbeBoxes10C0:
    asl a
    tay
    lda a:$C511,y
    sta z:zTemp0C
    lda a:$C512,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq ProbeBoxes10C0_at0
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
    jsr ProbeTileRead9991
ProbeBoxes10C0_at1:
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
    bne DrawSelectedFileDigit
    rts
ProbeBoxes10C0_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra ProbeBoxes10C0_at1

; @routine 10C0:11D ProbeBoxesFlagged10C0
; @run 6B:991D
; WHAT IT DOES: TerrainLookupAlt in the overlay at sector 10BF: ProbeBoxes10C0, then a second look eight pixels
; above each column that found floor; if that too is solid the floor bits come off the column's answer.
; HOW: The shared second-look body with the same table at $C511.
; WAITS: no.
; Entry, exit: as ProbeBoxes10C0.
ProbeBoxesFlagged10C0:
    pha
    jsr ProbeBoxes10C0
    pla
    asl a
    tay
    lda a:$C511,y
    sta z:zTemp0C
    lda a:$C512,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
ProbeBoxesFlagged10C0_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi ProbeBoxesFlagged10C0_at0
    beq ProbeBoxesFlagged10C0_at0
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
    jsr ProbeTileRead9991
    ldy z:zTemp0A
    and #$01
    beq ProbeBoxesFlagged10C0_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
ProbeBoxesFlagged10C0_at1:
    ply
ProbeBoxesFlagged10C0_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne ProbeBoxesFlagged10C0_at2
    rts
ProbeBoxesFlagged10C0_at0:
    iny
    iny
    iny
    iny
    bra ProbeBoxesFlagged10C0_at3

; @routine 12D0:0BD ProbeBoxes12D0
; @run 6B:98BD
; WHAT IT DOES: TerrainLookup in the overlay at sector 12CF: reads the terrain at the four columns of a probe
; around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
; HOW: The shared lookup body with this overlay's probe table at $D09B and 05BF's group's ProbeTileRead9991.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
; Clobbers zTemp00-0D.
ProbeBoxes12D0:
    asl a
    tay
    lda a:$D09B,y
    sta z:zTemp0C
    lda a:$D09C,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq ProbeBoxes12D0_at0
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
    jsr ProbeTileRead9991
ProbeBoxes12D0_at1:
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
    bne DrawSelectedFileDigit
    rts
ProbeBoxes12D0_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra ProbeBoxes12D0_at1

; @routine 12D0:11D ProbeBoxesFlagged12D0
; @run 6B:991D
; WHAT IT DOES: TerrainLookupAlt in the overlay at sector 12CF: ProbeBoxes12D0, then a second look eight pixels
; above each column that found floor; if that too is solid the floor bits come off the column's answer.
; HOW: The shared second-look body with the same table at $D09B.
; WAITS: no.
; Entry, exit: as ProbeBoxes12D0.
ProbeBoxesFlagged12D0:
    pha
    jsr ProbeBoxes12D0
    pla
    asl a
    tay
    lda a:$D09B,y
    sta z:zTemp0C
    lda a:$D09C,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
ProbeBoxesFlagged12D0_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi ProbeBoxesFlagged12D0_at0
    beq ProbeBoxesFlagged12D0_at0
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
    jsr ProbeTileRead9991
    ldy z:zTemp0A
    and #$01
    beq ProbeBoxesFlagged12D0_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
ProbeBoxesFlagged12D0_at1:
    ply
ProbeBoxesFlagged12D0_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne ProbeBoxesFlagged12D0_at2
    rts
ProbeBoxesFlagged12D0_at0:
    iny
    iny
    iny
    iny
    bra ProbeBoxesFlagged12D0_at3

; @routine 16C0:0BD ProbeBoxes16C0
; @run 6B:98BD
; WHAT IT DOES: TerrainLookup in the overlay at sector 16BF: reads the terrain at the four columns of a probe
; around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
; HOW: The shared lookup body with this overlay's probe table at $BDA9 and 05BF's group's ProbeTileRead9991.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
; Clobbers zTemp00-0D.
ProbeBoxes16C0:
    asl a
    tay
    lda a:$BDA9,y
    sta z:zTemp0C
    lda a:$BDAA,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq ProbeBoxes16C0_at0
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
    jsr ProbeTileRead9991
ProbeBoxes16C0_at1:
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
    bne DrawSelectedFileDigit
    rts
ProbeBoxes16C0_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra ProbeBoxes16C0_at1

; @routine 16C0:11D ProbeBoxesFlagged16C0
; @run 6B:991D
; WHAT IT DOES: TerrainLookupAlt in the overlay at sector 16BF: ProbeBoxes16C0, then a second look eight pixels
; above each column that found floor; if that too is solid the floor bits come off the column's answer.
; HOW: The shared second-look body with the same table at $BDA9.
; WAITS: no.
; Entry, exit: as ProbeBoxes16C0.
ProbeBoxesFlagged16C0:
    pha
    jsr ProbeBoxes16C0
    pla
    asl a
    tay
    lda a:$BDA9,y
    sta z:zTemp0C
    lda a:$BDAA,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
ProbeBoxesFlagged16C0_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi ProbeBoxesFlagged16C0_at0
    beq ProbeBoxesFlagged16C0_at0
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
    jsr ProbeTileRead9991
    ldy z:zTemp0A
    and #$01
    beq ProbeBoxesFlagged16C0_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
ProbeBoxesFlagged16C0_at1:
    ply
ProbeBoxesFlagged16C0_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne ProbeBoxesFlagged16C0_at2
    rts
ProbeBoxesFlagged16C0_at0:
    iny
    iny
    iny
    iny
    bra ProbeBoxesFlagged16C0_at3

; @routine 13F0:0BD ProbeBoxes13F0
; @run 6B:98BD
; WHAT IT DOES: TerrainLookup in the overlay at sector 13EF: reads the terrain at the four columns of a probe
; around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
; HOW: The shared lookup body with this overlay's probe table at $C4B9 and 05BF's group's ProbeTileRead9991.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
; Clobbers zTemp00-0D.
ProbeBoxes13F0:
    asl a
    tay
    lda a:$C4B9,y
    sta z:zTemp0C
    lda a:$C4BA,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
ProbeBoxes13F0_at2:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq ProbeBoxes13F0_at0
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
    jsr ProbeTileRead9991
ProbeBoxes13F0_at1:
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
    bne ProbeBoxes13F0_at2
    rts
ProbeBoxes13F0_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra ProbeBoxes13F0_at1

; @routine 13F0:11D ProbeBoxesFlagged13F0
; @run 6B:991D
; WHAT IT DOES: TerrainLookupAlt in the overlay at sector 13EF: ProbeBoxes13F0, then a second look eight pixels
; above each column that found floor; if that too is solid the floor bits come off the column's answer.
; HOW: The shared second-look body with the same table at $C4B9.
; WAITS: no.
; Entry, exit: as ProbeBoxes13F0.
ProbeBoxesFlagged13F0:
    pha
    jsr ProbeBoxes13F0
    pla
    asl a
    tay
    lda a:$C4B9,y
    sta z:zTemp0C
    lda a:$C4BA,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
ProbeBoxesFlagged13F0_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi ProbeBoxesFlagged13F0_at0
    beq ProbeBoxesFlagged13F0_at0
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
    jsr ProbeTileRead9991
    ldy z:zTemp0A
    and #$01
    beq ProbeBoxesFlagged13F0_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
ProbeBoxesFlagged13F0_at1:
    ply
ProbeBoxesFlagged13F0_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne ProbeBoxesFlagged13F0_at2
    rts
ProbeBoxesFlagged13F0_at0:
    iny
    iny
    iny
    iny
    bra ProbeBoxesFlagged13F0_at3

; @routine 18D0:0BD ProbeBoxes18D0
; @run 6B:98BD
; WHAT IT DOES: TerrainLookup in the overlay at sector 18CF: reads the terrain at the four columns of a probe
; around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
; HOW: The shared lookup body with this overlay's probe table at $C285 and 05BF's group's ProbeTileRead9991.
; WAITS: no.
; Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
; Clobbers zTemp00-0D.
ProbeBoxes18D0:
    asl a
    tay
    lda a:$C285,y
    sta z:zTemp0C
    lda a:$C286,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
ProbeBoxes18D0_at2:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq ProbeBoxes18D0_at0
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
    jsr ProbeTileRead9991
ProbeBoxes18D0_at1:
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
    bne ProbeBoxes18D0_at2
    rts
ProbeBoxes18D0_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra ProbeBoxes18D0_at1

; @routine 18D0:11D ProbeBoxesFlagged18D0
; @run 6B:991D
; WHAT IT DOES: TerrainLookupAlt in the overlay at sector 18CF: ProbeBoxes18D0, then a second look eight pixels
; above each column that found floor; if that too is solid the floor bits come off the column's answer.
; HOW: The shared second-look body with the same table at $C285.
; WAITS: no.
; Entry, exit: as ProbeBoxes18D0.
ProbeBoxesFlagged18D0:
    pha
    jsr ProbeBoxes18D0
    pla
    asl a
    tay
    lda a:$C285,y
    sta z:zTemp0C
    lda a:$C286,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
ProbeBoxesFlagged18D0_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi ProbeBoxesFlagged18D0_at0
    beq ProbeBoxesFlagged18D0_at0
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
    jsr ProbeTileRead9991
    ldy z:zTemp0A
    and #$01
    beq ProbeBoxesFlagged18D0_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
ProbeBoxesFlagged18D0_at1:
    ply
ProbeBoxesFlagged18D0_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne ProbeBoxesFlagged18D0_at2
    rts
ProbeBoxesFlagged18D0_at0:
    iny
    iny
    iny
    iny
    bra ProbeBoxesFlagged18D0_at3
