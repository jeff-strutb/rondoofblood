; @routine 024E:584 BlendStageTileRows
; @run 6B:8D84
; WHAT IT DOES: Writes five tile colour values into the VRAM words of one tile row step of the
; stage, taken from one row of StageTileRows, or, while a blend runs (StageBlendOn), from
; that row and a second one, StageBlendStep entries along.
; HOW: Each row is five bytes at StageTileRows plus five times its number, its address kept in
; zTemp00-01 and zTemp06-07. SetStageTileVramAddress points the VDC at the step's word.
; An even step packs each value into the word's top bits; an odd one writes the low word
; then, one step on, the next word.
; WAITS: no.
; Entry: X the step (zTemp04), A the first row, Y the second row. Exit: A the last byte
; written, X the blend step, Y 5, N 0, Z 1, C 1, V from the last add.
BlendStageTileRows:
    stx z:zTemp04
    sta z:zTemp00
    asl a
    asl a
    adc z:zTemp00
    adc #$34
    sta z:zTemp00
    cla
    adc #$C5
    sta z:zTemp01
    tya
    sta z:zTemp06
    asl a
    asl a
    adc z:zTemp06
    adc #$34
    sta z:zTemp06
    cla
    adc #$C5
    sta z:zTemp07
    jsr SetStageTileVramAddress
    lda z:zTemp04
    lsr a
    bcs BlendStageTileRows_at0
    cly
    ldx a:StageBlendStep
BlendStageTileRows_at4:
    cpx #$05
    beq BlendStageTileRows_at1
    lda a:StageBlendOn
    beq BlendStageTileRows_at1
    sty z:zTemp05
    txa
    clc
    adc z:zTemp05
    cmp #$05
    bcs BlendStageTileRows_at2
    phy
    tay
    lda (zTemp06),y
    ply
    bra BlendStageTileRows_at3
BlendStageTileRows_at2:
    sbc #$05
    phy
    tay
    lda (zTemp00),y
    ply
    bra BlendStageTileRows_at3
BlendStageTileRows_at1:
    lda (zTemp00),y
BlendStageTileRows_at3:
    stz z:zTemp03
    asl a
    rol z:zTemp03
    asl a
    rol z:zTemp03
    sta z:zTemp02
    lda a:VDC_DATA_LO
    and #$0F
    ora z:zTemp02
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and #$FC
    ora z:zTemp03
    sta a:VDC_DATA_HI
    iny
    cpy #$05
    bne BlendStageTileRows_at4
    rts
BlendStageTileRows_at0:
    cly
    ldx a:StageBlendStep
BlendStageTileRows_at8:
    cpx #$05
    beq BlendStageTileRows_at5
    lda a:StageBlendOn
    beq BlendStageTileRows_at5
    sty z:zTemp05
    txa
    clc
    adc z:zTemp05
    cmp #$05
    bcs BlendStageTileRows_at6
    phy
    tay
    lda (zTemp06),y
    ply
    bra BlendStageTileRows_at7
BlendStageTileRows_at6:
    sbc #$05
    phy
    tay
    lda (zTemp00),y
    ply
    bra BlendStageTileRows_at7
BlendStageTileRows_at5:
    lda (zTemp00),y
BlendStageTileRows_at7:
    asl a
    asl a
    sta z:zTemp02
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and #$0F
    ora z:zTemp02
    sta a:VDC_DATA_HI
    iny
    cpy #$05
    bne BlendStageTileRows_at8
    inc z:zTemp04
    bsr SetStageTileVramAddress
    cly
    ldx a:StageBlendStep
BlendStageTileRows_at12:
    cpx #$05
    beq BlendStageTileRows_at9
    lda a:StageBlendOn
    beq BlendStageTileRows_at9
    sty z:zTemp05
    txa
    clc
    adc z:zTemp05
    cmp #$05
    bcs BlendStageTileRows_at10
    phy
    tay
    lda (zTemp06),y
    ply
    bra BlendStageTileRows_at11
BlendStageTileRows_at10:
    sbc #$05
    phy
    tay
    lda (zTemp00),y
    ply
    bra BlendStageTileRows_at11
BlendStageTileRows_at9:
    lda (zTemp00),y
BlendStageTileRows_at11:
    stz z:zTemp02
    asl a
    rol z:zTemp02
    asl a
    rol z:zTemp02
    lda a:VDC_DATA_LO
    and #$FC
    ora z:zTemp02
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    iny
    cpy #$05
    bne BlendStageTileRows_at12
    rts

; @routine 024F:236 BlendStageRowChanges
; @run 6B:9236
; WHAT IT DOES: Blends the rows a byte changed: of the two bytes (the old in zBiosAl, the new in
; A), for each nibble that differs, the old nibble's row is blended in at the step X (low
; nibble) or the step after (high nibble).
; HOW: The low nibbles are compared, and BlendStageTileRows called with the old low nibble as
; the second row when they differ; then the same for the high nibbles one step on.
; WAITS: no.
; Entry: A the new byte, zBiosAl the old, X the step. Exit: A the high nibble when nothing
; was blended last, else as BlendStageTileRows; X the step plus one or BlendStageTileRows's; N 0, Z 1,
; C 1; Y 5 once a blend ran, else kept.
BlendStageRowChanges:
    phx
    sta z:zBiosBl
    lda z:zBiosAl
    and #$0F
    sta z:zBiosAh
    lda z:zBiosBl
    and #$0F
    cmp z:zBiosAh
    beq BlendStageRowChanges_at0
    ldy z:zBiosAh
    jsr BlendStageTileRows
BlendStageRowChanges_at0:
    lda z:zBiosAl
    lsr a
    lsr a
    lsr a
    lsr a
    sta z:zBiosAh
    lda z:zBiosBl
    lsr a
    lsr a
    lsr a
    lsr a
    plx
    inx
    cmp z:zBiosAh
    beq BlendStageRowChanges_at1
    ldy z:zBiosAh
    jsr BlendStageTileRows
BlendStageRowChanges_at1:
    rts

; @routine 024F:266 BlendStageRowPair
; @run 6B:9266
; WHAT IT DOES: Blends the two rows a byte names: its low nibble's row at the step X, its high
; nibble's row at the step after.
; HOW: BlendStageTileRows for the low nibble, called, then the same for the high nibble with
; the step one on, which this routine jumps to with no return of its own. The second row of
; both blends is the one in Y at entry for the first and 5 for the second (BlendStageTileRows
; leaves Y 5).
; WAITS: no.
; Entry: A the byte, X the step, Y the second row of the first blend. Exit: as BlendStageTileRows.
BlendStageRowPair:
    phx
    pha
    and #$0F
    jsr BlendStageTileRows
    pla
    lsr a
    lsr a
    lsr a
    lsr a
    plx
    inx
    jmp BlendStageTileRows
