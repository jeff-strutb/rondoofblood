; @routine 05C0:0BD ProbeBoxes05C0
; @run 6B:98BD
; WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
; TileDepthX and TileDepthY.
; HOW: The table at $BCEC names an entry's box offsets; each is added to the object's position and read with
; ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
; place in the entry, X kept. Clobbers zTemp00-0D.
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
; WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes05C0, then for each box whose answer has bits of $A1
; and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
; second look finds bit 0.
; HOW: ProbeBoxes05C0, then the same table entry walked again.
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
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
; WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
; TileDepthX and TileDepthY.
; HOW: The table at $C0D4 names an entry's box offsets; each is added to the object's position and read with
; ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
; place in the entry, X kept. Clobbers zTemp00-0D.
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
; WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes0720, then for each box whose answer has bits of $A1
; and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
; second look finds bit 0.
; HOW: ProbeBoxes0720, then the same table entry walked again.
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
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
; WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
; TileDepthX and TileDepthY.
; HOW: The table at $C251 names an entry's box offsets; each is added to the object's position and read with
; ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
; place in the entry, X kept. Clobbers zTemp00-0D.
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
; WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes0880, then for each box whose answer has bits of $A1
; and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
; second look finds bit 0.
; HOW: ProbeBoxes0880, then the same table entry walked again.
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
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
; WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
; TileDepthX and TileDepthY.
; HOW: The table at $C186 names an entry's box offsets; each is added to the object's position and read with
; ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
; place in the entry, X kept. Clobbers zTemp00-0D.
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
; WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes09E0, then for each box whose answer has bits of $A1
; and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
; second look finds bit 0.
; HOW: ProbeBoxes09E0, then the same table entry walked again.
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
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
; WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
; TileDepthX and TileDepthY.
; HOW: The table at $B38E names an entry's box offsets; each is added to the object's position and read with
; ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
; place in the entry, X kept. Clobbers zTemp00-0D.
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
; WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes0B40, then for each box whose answer has bits of $A1
; and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
; second look finds bit 0.
; HOW: ProbeBoxes0B40, then the same table entry walked again.
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
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
; WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
; TileDepthX and TileDepthY.
; HOW: The table at $C1A5 names an entry's box offsets; each is added to the object's position and read with
; ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
; place in the entry, X kept. Clobbers zTemp00-0D.
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
; WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes0CA0, then for each box whose answer has bits of $A1
; and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
; second look finds bit 0.
; HOW: ProbeBoxes0CA0, then the same table entry walked again.
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
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
; WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
; TileDepthX and TileDepthY.
; HOW: The table at $AF23 names an entry's box offsets; each is added to the object's position and read with
; ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
; place in the entry, X kept. Clobbers zTemp00-0D.
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
; WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes0E00, then for each box whose answer has bits of $A1
; and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
; second look finds bit 0.
; HOW: ProbeBoxes0E00, then the same table entry walked again.
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
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
; WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
; TileDepthX and TileDepthY.
; HOW: The table at $B01F names an entry's box offsets; each is added to the object's position and read with
; ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
; place in the entry, X kept. Clobbers zTemp00-0D.
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
; WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes0F60, then for each box whose answer has bits of $A1
; and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
; second look finds bit 0.
; HOW: ProbeBoxes0F60, then the same table entry walked again.
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
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
; WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
; TileDepthX and TileDepthY.
; HOW: The table at $C511 names an entry's box offsets; each is added to the object's position and read with
; ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
; place in the entry, X kept. Clobbers zTemp00-0D.
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
; WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes10C0, then for each box whose answer has bits of $A1
; and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
; second look finds bit 0.
; HOW: ProbeBoxes10C0, then the same table entry walked again.
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
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
; WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
; TileDepthX and TileDepthY.
; HOW: The table at $D09B names an entry's box offsets; each is added to the object's position and read with
; ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
; place in the entry, X kept. Clobbers zTemp00-0D.
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
; WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes12D0, then for each box whose answer has bits of $A1
; and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
; second look finds bit 0.
; HOW: ProbeBoxes12D0, then the same table entry walked again.
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
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
; WHAT IT DOES: Probes the four contact boxes of table entry A for the object in X and leaves what each found in TileHit,
; TileDepthX and TileDepthY.
; HOW: The table at $BDA9 names an entry's box offsets; each is added to the object's position and read with
; ProbeTileRead9991. An offset of $80 marks a box that is not probed (answer 0, depths 0).
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's
; place in the entry, X kept. Clobbers zTemp00-0D.
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
; WHAT IT DOES: Probes the four boxes of table entry A as ProbeBoxes16C0, then for each box whose answer has bits of $A1
; and not bit 7 probes a second point (the box's offsets, $08 higher) and clears the answer's other bits when that
; second look finds bit 0.
; HOW: ProbeBoxes16C0, then the same table entry walked again.
; WAITS: no.
; Entry: A the table entry, X the object. Exit: A 4, N, Z, C from comparing the count with 4, Y the last box's place.
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
