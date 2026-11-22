; @routine 00D1:066 EmitObjectSprites
; @run 6C:8066
; WHAT IT DOES: Writes the sprites of one object into the VDC's sprite table, a part at a
; time, leaving out the parts off the screen, until the object's parts or the frame's room
; for sprites run out.
; HOW: The object's position plus a bias is the screen origin. Its kind picks a table of
; frames, its frame picks a description: a count and five bytes a part (flags, Y and X offsets
; from the origin, pattern, attributes). A part whose Y or X falls outside the screen is
; skipped; one that shows has its four words (Y, X, pattern, attributes) written to the VDC.
; A mirrored object has each X offset taken from the width and bit 3 of the flags flipped.
; The attribute's palette comes from the object's flags (bit 7: the part's own bit 7 and the
; object's palette; bit 6 with bit 7 clear, untraced: the part's attribute plus the palette).
; The scratch is zTemp00 to zTemp11 and three more zero page bytes, left as the assembly leaves them.
; An object with no parts runs on at $819E (untraced, original code).
; WAITS: no.
; Entry: X the object. Exit: N set when the frame's room ran out, else Z set (the parts ran out);
; A, X, Y, C and V as the last part's work leaves them (SpriteExit).
; Untraced bytes: 00D1:059 (the tail at $819E) and 00D1:0F1 to 0F9 (the bit 6 palette arm).
; Names: the object fields f2a10, f2a68, f2ac0, f2b18 and f2fe8 hold the sprite set, frame,
; mirror flag, palette byte and flip mask (renames requested); the three zero page bytes
; and the disc tables are by address until the owner of the records adds them.
EmitObjectSprites:
    lda a:OBJ_Y_LO,x
    clc
    adc #$40
    sta z:zTemp08
    lda a:OBJ_Y_HI,x
    adc #$00
    sta z:zTemp09
    lda a:OBJ_X_LO,x
    clc
    adc #$20
    sta z:zTemp0A
    lda a:OBJ_X_HI,x
    adc #$00
    sta z:zTemp0B
    lda a:OBJ_2AC0,x
    sta z:zTemp0D
    lda a:OBJ_2FE8,x
    sta z:zProduct1
    lda a:OBJ_2B18,x
    sta z:zTemp0E
    and #$0F
    sta z:zTemp0F
    lda a:OBJ_2A10,x
    asl a
    tay
    lda a:$81A3,y
    sta z:zTemp00
    lda a:$81A4,y
    sta z:zTemp01
    lda a:OBJ_2A68,x
    asl a
    tay
    bcc EmitObjectSprites_at0
    inc z:zTemp01
EmitObjectSprites_at0:
    lda (zTemp00),y
    sta z:zTemp10
    iny
    lda (zTemp00),y
    sta z:zTemp11
    lda (zTemp10)
    sta z:zMultiplierLo
    bne EmitObjectSprites_at1
    jmp a:$819E
EmitObjectSprites_at1:
    inc z:zTemp10
    bne EmitObjectSprites_at2
    inc z:zTemp11
EmitObjectSprites_at2:
    clx
    cly
    lda (zTemp10),y
    sta z:zTemp00
    iny
    stz z:zTemp06
    lda (zTemp10),y
    bpl EmitObjectSprites_at3
    dec z:zTemp06
EmitObjectSprites_at3:
    clc
    adc z:zTemp08
    sta z:zTemp07
    lda z:zTemp06
    adc z:zTemp09
    sta z:zTemp06
    beq EmitObjectSprites_at6
    bpl EmitObjectSprites_at4
    jmp EmitObjectSprites_skip
EmitObjectSprites_at4:
    cmp #$01
    beq EmitObjectSprites_at5
    jmp EmitObjectSprites_skip
EmitObjectSprites_at5:
    lda z:zTemp07
    cmp #$30
    bcc EmitObjectSprites_at6
    jmp EmitObjectSprites_skip
EmitObjectSprites_at6:
    iny
    lda z:zTemp00
    and #$01
    tax
    stz z:zTemp04
    lda z:zTemp0D
    beq EmitObjectSprites_at7
    lda z:zTemp00
    eor #$08
    sta z:zTemp00
    lda a:$819F,x
    sec
    sbc (zTemp10),y
    bra EmitObjectSprites_at8
EmitObjectSprites_at7:
    lda (zTemp10),y
EmitObjectSprites_at8:
    bpl EmitObjectSprites_at9
    dec z:zTemp04
EmitObjectSprites_at9:
    clc
    adc z:zTemp0A
    sta z:zTemp05
    lda z:zTemp04
    adc z:zTemp0B
    sta z:zTemp04
    beq EmitObjectSprites_at11
    bpl EmitObjectSprites_at10
    jmp EmitObjectSprites_skip
EmitObjectSprites_at10:
    lda z:zTemp04
    cmp #$01
    bne EmitObjectSprites_skip
    lda z:zTemp05
    cmp #$20
    bcs EmitObjectSprites_skip
    bra EmitObjectSprites_at12
EmitObjectSprites_at11:
    lda z:zTemp05
    cmp a:$81A1,x
    bcc EmitObjectSprites_skip
EmitObjectSprites_at12:
    iny
    lda (zTemp10),y
    sta z:zTemp03
    iny
    lda (zTemp10),y
    and #$70
    lsr a
    lsr a
    lsr a
    lsr a
    sta z:zTemp02
    lda z:zTemp0E
    asl a
    bcs EmitObjectSprites_at14
    bpl EmitObjectSprites_at15
    lda (zTemp10),y
    clc
    adc z:zTemp0F
    and #$8F
    bra EmitObjectSprites_at16
EmitObjectSprites_at14:
    lda (zTemp10),y
    and #$80
    ora z:zTemp0F
    bra EmitObjectSprites_at16
EmitObjectSprites_at15:
    lda (zTemp10),y
    and #$8F
EmitObjectSprites_at16:
    sta z:zTemp01
    lda z:zProduct1
    and #$80
    eor z:zTemp01
    sta z:zTemp01
    iny
    ldy #$07
EmitObjectSprites_at17:
    lda a:$2000,y
    sta a:VDC_DATA_LO
    dey
    lda a:$2000,y
    sta a:VDC_DATA_HI
    dey
    bpl EmitObjectSprites_at17
    dec z:zMultiplierHi
    bmi EmitObjectSprites_at18
EmitObjectSprites_skip:
    lda #$05
    clc
    adc z:zTemp10
    sta z:zTemp10
    bcc EmitObjectSprites_at19
    inc z:zTemp11
EmitObjectSprites_at19:
    dec z:zMultiplierLo
    beq EmitObjectSprites_at18
    jmp EmitObjectSprites_at2
EmitObjectSprites_at18:
    rts
