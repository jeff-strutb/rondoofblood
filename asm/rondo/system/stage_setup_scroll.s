; @routine 0256:14C StageSetUpScroll
; @run 6F:A94C
; WHAT IT DOES: The section's first set-up: layer entry 0 drawn in columns of $10 blocks with the offset, five columns
; for the narrow camera modes ($0D, $11), else twelve with the scroll first moved back $80.
; HOW: ScrollColumnBlocks of entry 0 $90; CameraMode; ScrollXAddWrap; then the columns (draw_columns): DrawBlockColumn
; and AdvanceScrollX by turns, zSetUpColumnsLeft counting them.
; WAITS: no.
; Entry: nothing (V the caller's). Exit: X 0; A, Y, N, Z, C and V as the last DrawBlockColumn left them.
StageSetUpScroll:
    clx
    lda #$90
    sta a:ScrollColumnBlocks,x
    lda a:CameraMode
    cmp #$0D
    beq StageSetUpScroll_at0
    cmp #$11
    bne StageSetUpScroll_at1
StageSetUpScroll_at0:
    lda #$05
    sta z:zSetUpColumnsLeft
    bra StageSetUpScroll_at2
StageSetUpScroll_at1:
    lda #$0C
    sta z:zSetUpColumnsLeft
    lda #$80
    bsr ScrollXAddWrap
StageSetUpScroll_at2:
    lda #$20
    sta a:ScrollStepXHi
StageSetUpScroll_at3:
    cly
    jsr DrawBlockColumn
    jsr AdvanceScrollX
    dec z:zSetUpColumnsLeft
    bne StageSetUpScroll_at3
    cly
    jsr DrawBlockColumn
    rts

; @routine 0256:1B1 StageSetUpScrollByEffect
; @run 6F:A9B1
; WHAT IT DOES: The section's second set-up, when its raster effect has one: layer entry 0's column blocks from
; EffectColumnBlocks by the effect, the scroll moved back $C0, its screen Y counted on but for effects 6, 7, $0C,
; $10 and $18, and fifteen columns drawn from $A96B.
; HOW: zRasterEffect in Y through a chain of CPYs; ScrollXAddWrap; zSetUpColumnsLeft $10 first for effects $0C, $0F
; and $18, then $0F; the tail JMP into StageSetUpScroll's end.
; WAITS: no.
; Entry: nothing. Exit: X 0; A, Y, N, Z, C and V as the last DrawBlockColumn left them.
StageSetUpScrollByEffect:
    ldy z:zRasterEffect
    lda a:EffectColumnBlocks,y
    clx
    sta a:ScrollColumnBlocks,x
    lda #$C0
    bsr ScrollXAddWrap
    cpy #$0F
    beq StageSetUpScrollByEffect_at0
    cpy #$18
    beq StageSetUpScrollByEffect_at1
    cpy #$0C
    bne StageSetUpScrollByEffect_at2
StageSetUpScrollByEffect_at1:
    lda #$10
    sta z:zSetUpColumnsLeft
    bra StageSetUpScrollByEffect_at3
StageSetUpScrollByEffect_at0:
    lda #$10
    sta z:zSetUpColumnsLeft
    bra StageSetUpScrollByEffect_at4
StageSetUpScrollByEffect_at2:
    cpy #$06
    beq StageSetUpScrollByEffect_at3
    cpy #$07
    beq StageSetUpScrollByEffect_at3
    cpy #$10
    beq StageSetUpScrollByEffect_at3
StageSetUpScrollByEffect_at4:
    inc a:ScrollYHiScreen,x
StageSetUpScrollByEffect_at3:
    lda #$0F
    sta z:zSetUpColumnsLeft
    jmp StageSetUpScroll_at2

