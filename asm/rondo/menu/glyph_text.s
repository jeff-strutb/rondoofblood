; @routine 00C1:4AC DrawGlyphText
; @run 6B:84AC
; WHAT IT DOES: Draws a string of glyph pairs: the string's first four bytes are the map place (two
; bytes, zTemp08 and zTemp09) and the tile place (zTemp10 and zTemp11); then, until a zero byte, each pair
; of glyphs is drawn (DrawGlyphPair) and its map entries written (WriteGlyphMapRows).
; HOW: Four bytes read through zTemp00 and zTemp01, IncrementTempWord after each; the loop.
; WAITS: no.
; Entry: the string at zTemp00 and zTemp01. Exit: A 0, Z set, N clear; X, Y, C and V as the last pair's
; map rows left them, or as the pointer's last step left them when there was no pair.
; not checked: matches only with the callees as filed wrappers on both sides.
DrawGlyphText:
    lda (zTemp00)
    sta z:zTemp08
    jsr IncrementTempWord
    lda (zTemp00)
    sta z:zTemp09
    jsr IncrementTempWord
    lda (zTemp00)
    sta z:zTemp10
    jsr IncrementTempWord
    lda (zTemp00)
    sta z:zTemp11
    jsr IncrementTempWord
DrawGlyphText_at1:
    lda (zTemp00)
    beq DrawGlyphText_at0
    jsr DrawGlyphPair
    jsr WriteGlyphMapRows
    bra DrawGlyphText_at1
DrawGlyphText_at0:
    rts

; @routine 00C1:4D5 DrawGlyphPair
; @run 6B:84D5
; WHAT IT DOES: Draws the next two glyphs of the string as one tile pair: each glyph's 16 by 16 dots
; from the card's font (or from a table at $8620, $8600, $8640 or $85E0 for the codes $9C??, $9CC9, $9C72
; and $E081; a zero byte in place of the second glyph leaves it blank), the second shifted right four
; dots and merged over the first in the buffer at $3B40, then written to VRAM at the tile place in zTemp10
; and zTemp11 (which moves on by $60).
; HOW: SelectVramWriteAddress and SelectVramData; ex_getfnt for each glyph the tables do not hold; the merge
; by LSR and ROR of pairs of buffer bytes four times and an OR; WriteGlyphRowsTwice with X $00, $01, $21,
; $10, $11 and $31.
; WAITS: no.
; Entry: the string at zTemp00 and zTemp01. Exit: as WriteGlyphRowsTwice: A the last byte written, X
; that plus 16, Y 8, Z and C set, N clear. V as the tile place's add left it.
; not checked: agrees for 40 events, then differs inside the card's sjis_to_font_index.
DrawGlyphPair:
    jsr VecSelectVramWriteAddress
    lda z:zTemp10
    sta a:VDC_DATA_LO
    clc
    adc #$60
    sta z:zTemp10
    lda z:zTemp11
    sta a:VDC_DATA_HI
    bcc DrawGlyphPair_at0
    inc z:zTemp11
DrawGlyphPair_at0:
    jsr VecSelectVramData
    lda (zTemp00)
    sta z:zBiosAh
    tax
    jsr IncrementTempWord
    lda (zTemp00)
    sta z:zBiosAl
    tay
    jsr IncrementTempWord
    cpx #$9C
    bne DrawGlyphPair_at1
    cpy #$66
    beq DrawGlyphPair_at2
DrawGlyphPair_at2:
    ldx #$1F
DrawGlyphPair_at3:
    lda a:$8620,x
    sta a:StageMaskBuffer,x
    dex
    bpl DrawGlyphPair_at3
    bra DrawGlyphPair_at4
DrawGlyphPair_at1:
    lda #$40
    sta z:zBiosBl
    lda #$3B
    sta z:zBiosBh
    lda #$01
    sta z:zBiosDh
    jsr $E060
DrawGlyphPair_at4:
    lda (zTemp00)
    bne DrawGlyphPair_at5
    ldx #$1F
DrawGlyphPair_at6:
    stz a:$3B60,x
    dex
    bpl DrawGlyphPair_at6
    bra DrawGlyphPair_at7
DrawGlyphPair_at5:
    sta z:zBiosAh
    tax
    jsr IncrementTempWord
    lda (zTemp00)
    sta z:zBiosAl
    tay
    jsr IncrementTempWord
    cpx #$9C
    bne DrawGlyphPair_at8
    cpy #$C9
    beq DrawGlyphPair_at9
    cpy #$72
    beq DrawGlyphPair_at10
DrawGlyphPair_at8:
    cpx #$E0
    bne EndingStartScript18
    cpy #$81
    bne EndingStartScript18
    ldx #$1F
DrawGlyphPair_at11:
    lda a:$85E0,x
    sta a:$3B60,x
    dex
    bpl DrawGlyphPair_at11
    bra DrawGlyphPair_at7
DrawGlyphPair_at9:
    ldx #$1F
DrawGlyphPair_at12:
    lda a:$8600,x
    sta a:$3B60,x
    dex
    bpl DrawGlyphPair_at12
    bra DrawGlyphPair_at7
DrawGlyphPair_at10:
    ldx #$1F
DrawGlyphPair_at13:
    lda a:$8640,x
    sta a:$3B60,x
    dex
    bpl DrawGlyphPair_at13
    bra DrawGlyphPair_at7
    lda #$60
    sta z:zBiosBl
    lda #$3B
    sta z:zBiosBh
    lda #$01
    sta z:zBiosDh
    jsr $E060
DrawGlyphPair_at7:
    ldx #$1F
DrawGlyphPair_at14:
    lsr a:$3B5F,x
    ror a:$3B60,x
    lsr a:$3B5F,x
    ror a:$3B60,x
    lsr a:$3B5F,x
    ror a:$3B60,x
    lsr a:$3B5F,x
    ror a:$3B60,x
    lda a:StageMaskBuffer,x
    ora a:$3B5F,x
    sta a:StageMaskBuffer,x
    dex
    dex
    bpl DrawGlyphPair_at14
    ldx #$00
    jsr WriteGlyphRowsTwice
    ldx #$01
    jsr WriteGlyphRowsTwice
    ldx #$21
    jsr WriteGlyphRowsTwice
    ldx #$10
    jsr WriteGlyphRowsTwice
    ldx #$11
    jsr WriteGlyphRowsTwice
    ldx #$31

; @routine 00C1:660 WriteGlyphMapRows
; @run 6B:8660
; WHAT IT DOES: Writes the map entries of a glyph pair: three tile numbers at the place in zTemp08 and
; zTemp09 (which then moves on by 3), and under it, at that place plus $40, three more. The tile numbers
; are the tile place in zTemp10 and zTemp11 shifted down four, less 6, 5 and 4 for the first row and
; less 3, 2 and 1 for the second.
; HOW: SelectVramWriteAddress and SelectVramData for each row; the tile place shifted down four with ones
; shifted in at the top, in zBiosAl and zBiosAh, the six words by subtracting from it.
; WAITS: no.
; Entry: nothing. Exit: A the last high byte, N and Z from it, C and V from its subtract.
WriteGlyphMapRows:
    jsr VecSelectVramWriteAddress
    lda z:zTemp08
    sta a:VDC_DATA_LO
    clc
    adc #$03
    sta z:zTemp08
    lda z:zTemp09
    sta a:VDC_DATA_HI
    bcc WriteGlyphMapRows_at0
    inc z:zTemp09
WriteGlyphMapRows_at0:
    jsr VecSelectVramData
    lda z:zTemp10
    sta z:zBiosAl
    lda z:zTemp11
    sec
    ror a
    ror z:zBiosAl
    sec
    ror a
    ror z:zBiosAl
    sec
    ror a
    ror z:zBiosAl
    sec
    ror a
    ror z:zBiosAl
    sta z:zBiosAh
    lda z:zBiosAl
    sec
    sbc #$06
    sta a:VDC_DATA_LO
    lda z:zBiosAh
    sbc #$00
    sta a:VDC_DATA_HI
    lda z:zBiosAl
    sec
    sbc #$05
    sta a:VDC_DATA_LO
    lda z:zBiosAh
    sbc #$00
    sta a:VDC_DATA_HI
    lda z:zBiosAl
    sec
    sbc #$04
    sta a:VDC_DATA_LO
    lda z:zBiosAh
    sbc #$00
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    lda z:zTemp08
    clc
    adc #$3D
    sta a:VDC_DATA_LO
    cla
    adc z:zTemp09
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda z:zBiosAl
    sec
    sbc #$03
    sta a:VDC_DATA_LO
    lda z:zBiosAh
    sbc #$00
    sta a:VDC_DATA_HI
    lda z:zBiosAl
    sec
    sbc #$02
    sta a:VDC_DATA_LO
    lda z:zBiosAh
    sbc #$00
    sta a:VDC_DATA_HI
    lda z:zBiosAl
    sec
    sbc #$01
    sta a:VDC_DATA_LO
    lda z:zBiosAh
    sbc #$00
    sta a:VDC_DATA_HI
    rts
