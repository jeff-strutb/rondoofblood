; @routine 05C0:191 ProbeTileRead9991
; @run 6B:9991
; WHAT IT DOES: Says what the map holds at a point: the attribute of the background tile there, 0 if its page has none,
; $81 for a slope or step, which moves the row in the tile.
; HOW: The point plus the scroll gives the tile's place in the 64 by 128 background; its entry is read from VRAM. The
; entry's low byte and a page by its high bits give the attribute. Attributes $80-$9B go through the table at $9A2C.
; WAITS: no.
; Entry: zTemp00-01 X, zTemp02-03 Y. Exit: A the answer, zTemp00 the column, zTemp02 the row. X kept. Clobbers zTemp04-09.
ProbeTileRead9991:
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
    bmi ProbeTileRead9991_at0
    lda z:zTemp08
    clc
    adc #$00
    sta z:zTemp08
    lda a:SaveBuffer,y
    adc #$7D
    sta z:zTemp09
    lda (zTemp08)
    bmi ProbeTileRead9991_at1
    rts
ProbeTileRead9991_at0:
    lda #$00
ProbeTileRead9991_at2:
    rts
ProbeTileRead9991_at1:
    cmp #$9C
    bcs ProbeTileRead9991_at2
    pha
    sbc #$7F
    asl a
    tay
    lda a:$9A2C,y
    sta z:zTemp04
    lda a:$9A2D,y
    sta z:zTemp05
    pla
    jmp (zTemp04+$2000)
