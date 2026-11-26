; @card 00:F124 ex_getfnt
; WHAT IT DOES: Copies the glyph of the Shift-JIS code in _AX to _BX: 16 by 16 dots, or 12 by 12
; if bit 0 of _DH is set, as 32 bytes either way.
; HOW: sjis_to_font_index; the glyph's offset in the font is its number times 32, or 18 for 12
; dots, in 24 bits. Its bank, counted from bank 8 for 16 dots or 24 for 12, goes to a window
; that misses the caller's buffer, $6000 if _BX is at or above $A000 and $A000 if not; then the
; glyph is copied or expanded, and the MPRs put back.
; WAITS: no.
; Entry: _AX, _BX, _DH. Exit, copied: A 0, X 0, Y $20 (or $FF for 12 dots), N and Z from the
; second MPR put back, C set, V clear. Refused: A 1, N 0, Z 0, C set, V as sjis_to_font_index,
; X and Y kept. _CL holds the window, CardPointerLo the glyph's address there.
ex_getfnt:
    jsr sjis_to_font_index
    bcc ex_getfnt_at0
    lda #$01
    rts
ex_getfnt_at0:
    lda #$A0
    sta z:CardCl
    lda z:CardBh
    sec
    sbc #$A0
    bpl ex_getfnt_at1
    tma #$20
    sta a:CardFontSavedMprs+2
    tma #$40
    sta a:CardFontSavedMprs+3
    bra ex_getfnt_at2
ex_getfnt_at1:
    lda #$60
    sta z:CardCl
    tma #$08
    sta a:CardFontSavedMprs
    tma #$10
    sta a:CardFontSavedMprs+1
ex_getfnt_at2:
    lda z:CardAl
    sta z:CardPointerLo
    lda z:CardAh
    sta z:CardPointerHi
    stz z:CardCopyLo
    bbr0 CardDh,ex_getfnt_at3
    ldx #$08
ex_getfnt_at4:
    clc
    lda z:CardPointerLo
    adc z:CardAl
    sta z:CardPointerLo
    lda z:CardPointerHi
    adc z:CardAh
    sta z:CardPointerHi
    cla
    adc z:CardCopyLo
    sta z:CardCopyLo
    dex
    bne ex_getfnt_at4
    asl z:CardPointerLo
    rol z:CardPointerHi
    rol z:CardCopyLo
    bra ex_getfnt_at5
ex_getfnt_at3:
    ldx #$05
ex_getfnt_at6:
    asl z:CardPointerLo
    rol z:CardPointerHi
    rol z:CardCopyLo
    dex
    bne ex_getfnt_at6
ex_getfnt_at5:
    lda z:CardDh
    asl a
    inc a
    adc z:CardCopyLo
    sta z:CardCopyLo
    lda z:CardPointerHi
    ldx #$05
ex_getfnt_at7:
    lsr z:CardCopyLo
    ror a
    dex
    bne ex_getfnt_at7
    sta z:CardCopyLo
    lda z:CardPointerHi
    and #$1F
    clc
    adc z:CardCl
    sta z:CardPointerHi
    lda z:CardCopyLo
    bbs7 CardCl,ex_getfnt_at8
    tam #$08
    inc a
    tam #$10
    bra ex_getfnt_at9
ex_getfnt_at8:
    tam #$20
    inc a
    tam #$40
ex_getfnt_at9:
    bbs0 CardDh,ex_getfnt_at10
    cly
ex_getfnt_at11:
    lda (CardPointerLo),y
    sta (CardBl),y
    iny
    cpy #$20
    bne ex_getfnt_at11
    bra ex_getfnt_at12
ex_getfnt_at10:
    jsr expand_12dot_glyph
ex_getfnt_at12:
    bbs7 CardCl,ex_getfnt_at13
    lda a:CardFontSavedMprs
    tam #$08
    lda a:CardFontSavedMprs+1
    tam #$10
    bra ex_getfnt_at14
ex_getfnt_at13:
    lda a:CardFontSavedMprs+2
    tam #$20
    lda a:CardFontSavedMprs+3
    tam #$40
ex_getfnt_at14:
    cla
    rts

; @card 00:F1E3 sjis_to_font_index
; WHAT IT DOES: Makes the Shift-JIS code in _AH, _AL into the number of its glyph in the font,
; or refuses a code the font does not hold.
; HOW: Shift-JIS to JIS: the first byte gives two rows, the second the column and which of the
; two. The alphanumeric, Greek and Cyrillic rows close up their gaps; rows $28-$2F are empty and
; those after them move down eight. The number is the column plus the row's first glyph from
; FontRowStarts.
; WAITS: no.
; Entry: _AH, _AL. Exit, found: _AX the number, A its high byte, N and Z from it, C clear, V from
; adding the carry, Y the row's offset in the table. Refused: C set; A, N, Z and V from where it
; stopped. X kept, and Y if refused.
sjis_to_font_index:
    lda z:CardAh
    sec
    sbc #$81
    cmp #$6F
    bcs sjis_to_font_index_at0
    cmp #$1F
    bcc sjis_to_font_index_at1
    cmp #$3F
    bcc sjis_to_font_index_at0
    sbc #$40
sjis_to_font_index_at1:
    asl a
    adc #$21
    sta z:CardAh
    lda z:CardAl
    sec
    sbc #$40
    cmp #$BD
    bcs sjis_to_font_index_at0
    cmp #$3F
    beq sjis_to_font_index_at0
    bcs sjis_to_font_index_at2
    inc a
sjis_to_font_index_at2:
    cmp #$5F
    bcs sjis_to_font_index_at3
    clc
    adc #$20
    bra sjis_to_font_index_at4
sjis_to_font_index_at0:
    sec
    rts
sjis_to_font_index_at3:
    inc z:CardAh
    sec
    sbc #$3E
sjis_to_font_index_at4:
    sta z:CardAl
    lda z:CardAh
    cmp #$23
    beq sjis_to_font_index_at5
    cmp #$26
    beq sjis_to_font_index_at6
    cmp #$27
    beq sjis_to_font_index_at7
    cmp #$28
    bcc sjis_to_font_index_at8
    cmp #$30
    bcc sjis_to_font_index_at0
    cmp #$50
    bcs sjis_to_font_index_at0
    sec
    sbc #$08
    bra sjis_to_font_index_at8
sjis_to_font_index_at5:
    lda z:CardAl
    cmp #$61
    bcc sjis_to_font_index_at9
    sbc #$06
sjis_to_font_index_at9:
    cmp #$41
    bcc sjis_to_font_index_at10
    sbc #$07
sjis_to_font_index_at10:
    sec
    sbc #$0F
    bra sjis_to_font_index_at11
sjis_to_font_index_at6:
    lda z:CardAl
    cmp #$41
    bcc sjis_to_font_index_at11
    sbc #$08
    bra sjis_to_font_index_at11
sjis_to_font_index_at7:
    lda z:CardAl
    cmp #$51
    bcc sjis_to_font_index_at11
    sbc #$0F
sjis_to_font_index_at11:
    sta z:CardAl
    lda z:CardAh
sjis_to_font_index_at8:
    sec
    sbc #$21
    asl a
    tay
    lda z:CardAl
    sec
    sbc #$21
    clc
    adc a:FontRowStarts,y
    sta z:CardAl
    lda a:FontRowStarts+1,y
    adc #$00
    sta z:CardAh
    clc
    rts

; @card 00:F2CC expand_12dot_glyph
; WHAT IT DOES: Writes a 12-dot glyph as sixteen rows of two bytes where _BX points.
; HOW: The font packs two rows of twelve dots in three bytes; each row is written as twelve
; dots and four clear ones. Twelve rows, then eight bytes of zero for the last four.
; WAITS: no.
; Entry: CardPointerLo the glyph, _BX. Exit: A 0, X 0, Y $FF, N 1, Z 0, C 1. V kept. _BX moved on
; by 24.
expand_12dot_glyph:
    cly
expand_12dot_glyph_at1:
    lda (CardPointerLo),y
    iny
    bsr put_font_byte
    lda (CardPointerLo),y
    iny
    pha
    and #$F0
    bsr put_font_byte
    lda (CardPointerLo),y
    iny
    sta a:CardFontShift
    pla
    ldx #$04
expand_12dot_glyph_at0:
    asl a:CardFontShift
    rol a
    dex
    bne expand_12dot_glyph_at0
    bsr put_font_byte
    lda a:CardFontShift
    and #$F0
    bsr put_font_byte
    cpy #$12
    bcc expand_12dot_glyph_at1
    cla
    ldy #$07
expand_12dot_glyph_at2:
    sta (CardBl),y
    dey
    bpl expand_12dot_glyph_at2
    rts

; @card 00:F300 put_font_byte
; WHAT IT DOES: Writes a byte of a glyph where _BX points, and moves _BX on.
; HOW: A store through _BX and a 16-bit increment.
; WAITS: no.
; Entry: A, _BX. Exit: N and Z from _BL after the increment, or from _BH if _BL went to zero. A,
; X, Y, C, V kept.
put_font_byte:
    sta (CardBl)
    inc z:CardBl
    bne put_font_byte_at0
    inc z:CardBh
put_font_byte_at0:
    rts

