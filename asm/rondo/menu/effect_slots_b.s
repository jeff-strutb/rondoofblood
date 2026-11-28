; @routine 05C1:4FF EffectColour05C1
; @run 6C:84FF
; WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
; negative list number ($89E5), Secret stage 02's overlay.
; HOW: EffectColour1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectColour1A31.
EffectColour05C1:
    lda a:ScriptSlotScript,x
    bmi EffectColour05C1_at0
    asl a
    asl a
    adc a:OverlayEffectColourLists
    sta z:zTemp00
    cla
    adc a:OverlayEffectColourLists+1
    sta z:zTemp01
    bra EffectColour05C1_at1
EffectColour05C1_at0:
    asl a
    asl a
    adc #$E5
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour05C1_at1:
    cly
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_HI
    jmp ClearEffectSlot1A31

; @routine 0721:362 EffectFade0721
; @run 6C:8362
; WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with this
; overlay's table of lists for a negative list number ($89E7), Secret stage 03's overlay.
; HOW: EffectFade1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectFade1A31.
EffectFade0721Lists = $89E7 ; kind 2's lists a negative list number picks, a word each
EffectFade0721:
    lda a:ScriptSlotTimer,x
    beq EffectFade0721_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade0721_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade0721_at1
    jmp ClearEffectSlot1A31
EffectFade0721_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade0721_at2
    lda a:EffectFade0721Lists,y
    sta z:zTemp00
    lda a:EffectFade0721Lists+1,y
    sta z:zTemp01
    bra EffectFade0721_at3
EffectFade0721_at2:
    lda a:OverlayEffectFadeLists
    sta z:zBiosAl
    lda a:OverlayEffectFadeLists+1
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade0721_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade0721_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade0721_at4
    rts
EffectFade0721_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade0721_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade0721_at5
    dec a
EffectFade0721_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade0721_at6
    sec
    sbc #$08
EffectFade0721_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade0721_at7
    sec
    sbc #$20
EffectFade0721_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade0721_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade0721_at9

; @routine 0721:4FF EffectColour0721
; @run 6C:84FF
; WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
; negative list number ($89E7), Secret stage 03's overlay.
; HOW: EffectColour1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectColour1A31.
EffectColour0721:
    lda a:ScriptSlotScript,x
    bmi EffectColour0721_at0
    asl a
    asl a
    adc a:OverlayEffectColourLists
    sta z:zTemp00
    cla
    adc a:OverlayEffectColourLists+1
    sta z:zTemp01
    bra EffectColour0721_at1
EffectColour0721_at0:
    asl a
    asl a
    adc #$E7
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour0721_at1:
    cly
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_HI
    jmp ClearEffectSlot1A31

; @routine 0881:362 EffectFade0881
; @run 6C:8362
; WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with this
; overlay's table of lists for a negative list number ($89E9), Secret stage 04's overlay.
; HOW: EffectFade1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectFade1A31.
EffectFade0881Lists = $89E9 ; kind 2's lists a negative list number picks, a word each
EffectFade0881:
    lda a:ScriptSlotTimer,x
    beq EffectFade0881_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade0881_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade0881_at1
    jmp ClearEffectSlot1A31
EffectFade0881_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade0881_at2
    lda a:EffectFade0881Lists,y
    sta z:zTemp00
    lda a:EffectFade0881Lists+1,y
    sta z:zTemp01
    bra EffectFade0881_at3
EffectFade0881_at2:
    lda a:OverlayEffectFadeLists
    sta z:zBiosAl
    lda a:OverlayEffectFadeLists+1
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade0881_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade0881_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade0881_at4
    rts
EffectFade0881_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade0881_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade0881_at5
    dec a
EffectFade0881_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade0881_at6
    sec
    sbc #$08
EffectFade0881_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade0881_at7
    sec
    sbc #$20
EffectFade0881_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade0881_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade0881_at9

; @routine 0881:4FF EffectColour0881
; @run 6C:84FF
; WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
; negative list number ($89E9), Secret stage 04's overlay.
; HOW: EffectColour1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectColour1A31.
EffectColour0881:
    lda a:ScriptSlotScript,x
    bmi EffectColour0881_at0
    asl a
    asl a
    adc a:OverlayEffectColourLists
    sta z:zTemp00
    cla
    adc a:OverlayEffectColourLists+1
    sta z:zTemp01
    bra EffectColour0881_at1
EffectColour0881_at0:
    asl a
    asl a
    adc #$E9
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour0881_at1:
    cly
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_HI
    jmp ClearEffectSlot1A31

; @routine 0B41:362 EffectFade0B41
; @run 6C:8362
; WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with this
; overlay's table of lists for a negative list number ($8A01), Secret stage 06's overlay.
; HOW: EffectFade1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectFade1A31.
EffectFade0B41Lists = $8A01 ; kind 2's lists a negative list number picks, a word each
EffectFade0B41:
    lda a:ScriptSlotTimer,x
    beq EffectFade0B41_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade0B41_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade0B41_at1
    jmp ClearEffectSlot1A31
EffectFade0B41_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade0B41_at2
    lda a:EffectFade0B41Lists,y
    sta z:zTemp00
    lda a:EffectFade0B41Lists+1,y
    sta z:zTemp01
    bra EffectFade0B41_at3
EffectFade0B41_at2:
    lda a:OverlayEffectFadeLists
    sta z:zBiosAl
    lda a:OverlayEffectFadeLists+1
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade0B41_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade0B41_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade0B41_at4
    rts
EffectFade0B41_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade0B41_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade0B41_at5
    dec a
EffectFade0B41_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade0B41_at6
    sec
    sbc #$08
EffectFade0B41_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade0B41_at7
    sec
    sbc #$20
EffectFade0B41_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade0B41_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade0B41_at9

; @routine 0B41:4FF EffectColour0B41
; @run 6C:84FF
; WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
; negative list number ($8A01), Secret stage 06's overlay.
; HOW: EffectColour1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectColour1A31.
EffectColour0B41:
    lda a:ScriptSlotScript,x
    bmi EffectColour0B41_at0
    asl a
    asl a
    adc a:OverlayEffectColourLists
    sta z:zTemp00
    cla
    adc a:OverlayEffectColourLists+1
    sta z:zTemp01
    bra EffectColour0B41_at1
EffectColour0B41_at0:
    asl a
    asl a
    adc #$01
    sta z:zTemp00
    cla
    adc #$8A
    sta z:zTemp01
EffectColour0B41_at1:
    cly
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_HI
    jmp ClearEffectSlot1A31

; @routine 0EB1:362 EffectFade0EB1
; @run 6C:8362
; WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with this
; overlay's table of lists for a negative list number ($89DF), Stage 09's overlay.
; HOW: EffectFade1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectFade1A31.
EffectFade0EB1Lists = $89DF ; kind 2's lists a negative list number picks, a word each
EffectFade0EB1:
    lda a:ScriptSlotTimer,x
    beq EffectFade0EB1_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade0EB1_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade0EB1_at1
    jmp ClearEffectSlot1A31
EffectFade0EB1_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade0EB1_at2
    lda a:EffectFade0EB1Lists,y
    sta z:zTemp00
    lda a:EffectFade0EB1Lists+1,y
    sta z:zTemp01
    bra EffectFade0EB1_at3
EffectFade0EB1_at2:
    lda a:OverlayEffectFadeLists
    sta z:zBiosAl
    lda a:OverlayEffectFadeLists+1
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade0EB1_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade0EB1_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade0EB1_at4
    rts
EffectFade0EB1_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade0EB1_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade0EB1_at5
    dec a
EffectFade0EB1_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade0EB1_at6
    sec
    sbc #$08
EffectFade0EB1_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade0EB1_at7
    sec
    sbc #$20
EffectFade0EB1_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade0EB1_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade0EB1_at9

; @routine 0EB1:4FF EffectColour0EB1
; @run 6C:84FF
; WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
; negative list number ($89DF), Stage 09's overlay.
; HOW: EffectColour1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectColour1A31.
EffectColour0EB1:
    lda a:ScriptSlotScript,x
    bmi EffectColour0EB1_at0
    asl a
    asl a
    adc a:OverlayEffectColourLists
    sta z:zTemp00
    cla
    adc a:OverlayEffectColourLists+1
    sta z:zTemp01
    bra EffectColour0EB1_at1
EffectColour0EB1_at0:
    asl a
    asl a
    adc #$DF
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour0EB1_at1:
    cly
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_HI
    jmp ClearEffectSlot1A31

; @routine 10C1:362 EffectFade10C1
; @run 6C:8362
; WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with this
; overlay's table of lists for a negative list number ($8A25), Secret stage 0A's overlay.
; HOW: EffectFade1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectFade1A31.
EffectFade10C1Lists = $8A25 ; kind 2's lists a negative list number picks, a word each
EffectFade10C1:
    lda a:ScriptSlotTimer,x
    beq EffectFade10C1_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade10C1_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade10C1_at1
    jmp ClearEffectSlot1A31
EffectFade10C1_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade10C1_at2
    lda a:EffectFade10C1Lists,y
    sta z:zTemp00
    lda a:EffectFade10C1Lists+1,y
    sta z:zTemp01
    bra EffectFade10C1_at3
EffectFade10C1_at2:
    lda a:OverlayEffectFadeLists
    sta z:zBiosAl
    lda a:OverlayEffectFadeLists+1
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade10C1_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade10C1_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade10C1_at4
    rts
EffectFade10C1_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade10C1_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade10C1_at5
    dec a
EffectFade10C1_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade10C1_at6
    sec
    sbc #$08
EffectFade10C1_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade10C1_at7
    sec
    sbc #$20
EffectFade10C1_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade10C1_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade10C1_at9

; @routine 10C1:4FF EffectColour10C1
; @run 6C:84FF
; WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
; negative list number ($8A25), Secret stage 0A's overlay.
; HOW: EffectColour1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectColour1A31.
EffectColour10C1:
    lda a:ScriptSlotScript,x
    bmi EffectColour10C1_at0
    asl a
    asl a
    adc a:OverlayEffectColourLists
    sta z:zTemp00
    cla
    adc a:OverlayEffectColourLists+1
    sta z:zTemp01
    bra EffectColour10C1_at1
EffectColour10C1_at0:
    asl a
    asl a
    adc #$25
    sta z:zTemp00
    cla
    adc #$8A
    sta z:zTemp01
EffectColour10C1_at1:
    cly
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_HI
    jmp ClearEffectSlot1A31

; @routine 12D1:362 EffectFade12D1
; @run 6C:8362
; WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with this
; overlay's table of lists for a negative list number ($8A0B), Stage 0C's overlay.
; HOW: EffectFade1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectFade1A31.
EffectFade12D1Lists = $8A0B ; kind 2's lists a negative list number picks, a word each
EffectFade12D1:
    lda a:ScriptSlotTimer,x
    beq EffectFade12D1_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade12D1_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade12D1_at1
    jmp ClearEffectSlot1A31
EffectFade12D1_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade12D1_at2
    lda a:EffectFade12D1Lists,y
    sta z:zTemp00
    lda a:EffectFade12D1Lists+1,y
    sta z:zTemp01
    bra EffectFade12D1_at3
EffectFade12D1_at2:
    lda a:OverlayEffectFadeLists
    sta z:zBiosAl
    lda a:OverlayEffectFadeLists+1
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade12D1_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade12D1_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade12D1_at4
    rts
EffectFade12D1_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade12D1_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade12D1_at5
    dec a
EffectFade12D1_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade12D1_at6
    sec
    sbc #$08
EffectFade12D1_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade12D1_at7
    sec
    sbc #$20
EffectFade12D1_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade12D1_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade12D1_at9

; @routine 12D1:4FF EffectColour12D1
; @run 6C:84FF
; WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
; negative list number ($8A0B), Stage 0C's overlay.
; HOW: EffectColour1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectColour1A31.
EffectColour12D1:
    lda a:ScriptSlotScript,x
    bmi EffectColour12D1_at0
    asl a
    asl a
    adc a:OverlayEffectColourLists
    sta z:zTemp00
    cla
    adc a:OverlayEffectColourLists+1
    sta z:zTemp01
    bra EffectColour12D1_at1
EffectColour12D1_at0:
    asl a
    asl a
    adc #$0B
    sta z:zTemp00
    cla
    adc #$8A
    sta z:zTemp01
EffectColour12D1_at1:
    cly
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_HI
    jmp ClearEffectSlot1A31

; @routine 1611:362 EffectFade1611
; @run 6C:8362
; WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with this
; overlay's table of lists for a negative list number ($89E1), Stage 0E's overlay.
; HOW: EffectFade1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectFade1A31.
EffectFade1611Lists = $89E1 ; kind 2's lists a negative list number picks, a word each
EffectFade1611:
    lda a:ScriptSlotTimer,x
    beq EffectFade1611_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade1611_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade1611_at1
    jmp ClearEffectSlot1A31
EffectFade1611_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade1611_at2
    lda a:EffectFade1611Lists,y
    sta z:zTemp00
    lda a:EffectFade1611Lists+1,y
    sta z:zTemp01
    bra EffectFade1611_at3
EffectFade1611_at2:
    lda a:OverlayEffectFadeLists
    sta z:zBiosAl
    lda a:OverlayEffectFadeLists+1
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade1611_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade1611_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade1611_at4
    rts
EffectFade1611_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade1611_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade1611_at5
    dec a
EffectFade1611_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade1611_at6
    sec
    sbc #$08
EffectFade1611_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade1611_at7
    sec
    sbc #$20
EffectFade1611_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade1611_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade1611_at9

; @routine 1611:4FF EffectColour1611
; @run 6C:84FF
; WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
; negative list number ($89E1), Stage 0E's overlay.
; HOW: EffectColour1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectColour1A31.
EffectColour1611:
    lda a:ScriptSlotScript,x
    bmi EffectColour1611_at0
    asl a
    asl a
    adc a:OverlayEffectColourLists
    sta z:zTemp00
    cla
    adc a:OverlayEffectColourLists+1
    sta z:zTemp01
    bra EffectColour1611_at1
EffectColour1611_at0:
    asl a
    asl a
    adc #$E1
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour1611_at1:
    cly
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_HI
    jmp ClearEffectSlot1A31

; @routine 16C1:362 EffectFade16C1
; @run 6C:8362
; WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with this
; overlay's table of lists for a negative list number ($89F3), Secret stage 0E's overlay.
; HOW: EffectFade1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectFade1A31.
EffectFade16C1Lists = $89F3 ; kind 2's lists a negative list number picks, a word each
EffectFade16C1:
    lda a:ScriptSlotTimer,x
    beq EffectFade16C1_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade16C1_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade16C1_at1
    jmp ClearEffectSlot1A31
EffectFade16C1_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade16C1_at2
    lda a:EffectFade16C1Lists,y
    sta z:zTemp00
    lda a:EffectFade16C1Lists+1,y
    sta z:zTemp01
    bra EffectFade16C1_at3
EffectFade16C1_at2:
    lda a:OverlayEffectFadeLists
    sta z:zBiosAl
    lda a:OverlayEffectFadeLists+1
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade16C1_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade16C1_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade16C1_at4
    rts
EffectFade16C1_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade16C1_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade16C1_at5
    dec a
EffectFade16C1_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade16C1_at6
    sec
    sbc #$08
EffectFade16C1_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade16C1_at7
    sec
    sbc #$20
EffectFade16C1_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade16C1_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade16C1_at9

; @routine 16C1:4FF EffectColour16C1
; @run 6C:84FF
; WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with this overlay's table of lists for a
; negative list number ($89F3), Secret stage 0E's overlay.
; HOW: EffectColour1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectColour1A31.
EffectColour16C1:
    lda a:ScriptSlotScript,x
    bmi EffectColour16C1_at0
    asl a
    asl a
    adc a:OverlayEffectColourLists
    sta z:zTemp00
    cla
    adc a:OverlayEffectColourLists+1
    sta z:zTemp01
    bra EffectColour16C1_at1
EffectColour16C1_at0:
    asl a
    asl a
    adc #$F3
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour16C1_at1:
    cly
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_HI
    jmp ClearEffectSlot1A31

; @routine 0251:362 EffectFade0251
; @run 6C:8362
; WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with the overlay at
; sector 0251's table of lists for a negative list number ($89E5).
; HOW: EffectFade1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectFade1A31.
EffectFade0251Lists = $89E5 ; kind 2's lists a negative list number picks, a word each
EffectFade0251:
    lda a:ScriptSlotTimer,x
    beq EffectFade0251_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade0251_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade0251_at1
    jmp ClearEffectSlot1A31
EffectFade0251_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade0251_at2
    lda a:EffectFade0251Lists,y
    sta z:zTemp00
    lda a:EffectFade0251Lists+1,y
    sta z:zTemp01
    bra EffectFade0251_at3
EffectFade0251_at2:
    lda a:OverlayEffectFadeLists
    sta z:zBiosAl
    lda a:OverlayEffectFadeLists+1
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade0251_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade0251_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade0251_at4
    rts
EffectFade0251_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade0251_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade0251_at5
    dec a
EffectFade0251_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade0251_at6
    sec
    sbc #$08
EffectFade0251_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade0251_at7
    sec
    sbc #$20
EffectFade0251_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade0251_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade0251_at9

; @routine 0511:362 EffectFade0511
; @run 6C:8362
; WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with the overlay at
; sector 0511's table of lists for a negative list number ($89DB).
; HOW: EffectFade1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectFade1A31.
EffectFade0511Lists = $89DB ; kind 2's lists a negative list number picks, a word each
EffectFade0511:
    lda a:ScriptSlotTimer,x
    beq EffectFade0511_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade0511_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade0511_at1
    jmp ClearEffectSlot1A31
EffectFade0511_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade0511_at2
    lda a:EffectFade0511Lists,y
    sta z:zTemp00
    lda a:EffectFade0511Lists+1,y
    sta z:zTemp01
    bra EffectFade0511_at3
EffectFade0511_at2:
    lda a:OverlayEffectFadeLists
    sta z:zBiosAl
    lda a:OverlayEffectFadeLists+1
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade0511_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade0511_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade0511_at4
    rts
EffectFade0511_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade0511_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade0511_at5
    dec a
EffectFade0511_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade0511_at6
    sec
    sbc #$08
EffectFade0511_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade0511_at7
    sec
    sbc #$20
EffectFade0511_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade0511_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade0511_at9

; @routine 07D1:362 EffectFade07D1
; @run 6C:8362
; WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with the overlay at
; sector 07D1's table of lists for a negative list number ($89E3).
; HOW: EffectFade1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectFade1A31.
EffectFade07D1Lists = $89E3 ; kind 2's lists a negative list number picks, a word each
EffectFade07D1:
    lda a:ScriptSlotTimer,x
    beq EffectFade07D1_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade07D1_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade07D1_at1
    jmp ClearEffectSlot1A31
EffectFade07D1_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade07D1_at2
    lda a:EffectFade07D1Lists,y
    sta z:zTemp00
    lda a:EffectFade07D1Lists+1,y
    sta z:zTemp01
    bra EffectFade07D1_at3
EffectFade07D1_at2:
    lda a:OverlayEffectFadeLists
    sta z:zBiosAl
    lda a:OverlayEffectFadeLists+1
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade07D1_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade07D1_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade07D1_at4
    rts
EffectFade07D1_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade07D1_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade07D1_at5
    dec a
EffectFade07D1_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade07D1_at6
    sec
    sbc #$08
EffectFade07D1_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade07D1_at7
    sec
    sbc #$20
EffectFade07D1_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade07D1_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade07D1_at9

; @routine 0931:362 EffectFade0931
; @run 6C:8362
; WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with the overlay at
; sector 0931's table of lists for a negative list number ($89D7).
; HOW: EffectFade1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectFade1A31.
EffectFade0931Lists = $89D7 ; kind 2's lists a negative list number picks, a word each
EffectFade0931:
    lda a:ScriptSlotTimer,x
    beq EffectFade0931_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade0931_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade0931_at1
    jmp ClearEffectSlot1A31
EffectFade0931_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade0931_at2
    lda a:EffectFade0931Lists,y
    sta z:zTemp00
    lda a:EffectFade0931Lists+1,y
    sta z:zTemp01
    bra EffectFade0931_at3
EffectFade0931_at2:
    lda a:OverlayEffectFadeLists
    sta z:zBiosAl
    lda a:OverlayEffectFadeLists+1
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade0931_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade0931_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade0931_at4
    rts
EffectFade0931_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade0931_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade0931_at5
    dec a
EffectFade0931_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade0931_at6
    sec
    sbc #$08
EffectFade0931_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade0931_at7
    sec
    sbc #$20
EffectFade0931_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade0931_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade0931_at9

; @routine 0BF1:362 EffectFade0BF1
; @run 6C:8362
; WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with the overlay at
; sector 0BF1's table of lists for a negative list number ($89C7).
; HOW: EffectFade1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectFade1A31.
EffectFade0BF1Lists = $89C7 ; kind 2's lists a negative list number picks, a word each
EffectFade0BF1:
    lda a:ScriptSlotTimer,x
    beq EffectFade0BF1_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade0BF1_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade0BF1_at1
    jmp ClearEffectSlot1A31
EffectFade0BF1_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade0BF1_at2
    lda a:EffectFade0BF1Lists,y
    sta z:zTemp00
    lda a:EffectFade0BF1Lists+1,y
    sta z:zTemp01
    bra EffectFade0BF1_at3
EffectFade0BF1_at2:
    lda a:OverlayEffectFadeLists
    sta z:zBiosAl
    lda a:OverlayEffectFadeLists+1
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade0BF1_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade0BF1_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade0BF1_at4
    rts
EffectFade0BF1_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade0BF1_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade0BF1_at5
    dec a
EffectFade0BF1_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade0BF1_at6
    sec
    sbc #$08
EffectFade0BF1_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade0BF1_at7
    sec
    sbc #$20
EffectFade0BF1_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade0BF1_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade0BF1_at9

; @routine 2707:362 EffectFade2707
; @run 6C:8362
; WHAT IT DOES: Kind 2 of the palette effects: fades palettes down a step a round; as EffectFade1A31 with the overlay at
; sector 2707's table of lists for a negative list number ($9414).
; HOW: EffectFade1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectFade1A31.
EffectFade2707Lists = $9414 ; kind 2's lists a negative list number picks, a word each
EffectFade2707:
    lda a:ScriptSlotTimer,x
    beq EffectFade2707_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade2707_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade2707_at1
    jmp ClearEffectSlot1A31
EffectFade2707_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade2707_at2
    lda a:EffectFade2707Lists,y
    sta z:zTemp00
    lda a:EffectFade2707Lists+1,y
    sta z:zTemp01
    bra EffectFade2707_at3
EffectFade2707_at2:
    lda a:OverlayEffectFadeLists
    sta z:zBiosAl
    lda a:OverlayEffectFadeLists+1
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade2707_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade2707_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade2707_at4
    rts
EffectFade2707_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade2707_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade2707_at5
    dec a
EffectFade2707_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade2707_at6
    sec
    sbc #$08
EffectFade2707_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade2707_at7
    sec
    sbc #$20
EffectFade2707_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade2707_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade2707_at9

; @routine 0251:4FF EffectColour0251
; @run 6C:84FF
; WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with the overlay at sector 0251's table of lists for a
; negative list number ($8A0D).
; HOW: EffectColour1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectColour1A31.
EffectColour0251:
    lda a:ScriptSlotScript,x
    bmi EffectColour0251_at0
    asl a
    asl a
    adc a:OverlayEffectColourLists
    sta z:zTemp00
    cla
    adc a:OverlayEffectColourLists+1
    sta z:zTemp01
    bra EffectColour0251_at1
EffectColour0251_at0:
    asl a
    asl a
    adc #$0D
    sta z:zTemp00
    cla
    adc #$8A
    sta z:zTemp01
EffectColour0251_at1:
    cly
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_HI
    jmp ClearEffectSlot1A31

; @routine 0511:4FF EffectColour0511
; @run 6C:84FF
; WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with the overlay at sector 0511's table of lists for a
; negative list number ($89DD).
; HOW: EffectColour1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectColour1A31.
EffectColour0511:
    lda a:ScriptSlotScript,x
    bmi EffectColour0511_at0
    asl a
    asl a
    adc a:OverlayEffectColourLists
    sta z:zTemp00
    cla
    adc a:OverlayEffectColourLists+1
    sta z:zTemp01
    bra EffectColour0511_at1
EffectColour0511_at0:
    asl a
    asl a
    adc #$DD
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour0511_at1:
    cly
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_HI
    jmp ClearEffectSlot1A31

; @routine 07D1:4FF EffectColour07D1
; @run 6C:84FF
; WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with the overlay at sector 07D1's table of lists for a
; negative list number ($89E3).
; HOW: EffectColour1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectColour1A31.
EffectColour07D1:
    lda a:ScriptSlotScript,x
    bmi EffectColour07D1_at0
    asl a
    asl a
    adc a:OverlayEffectColourLists
    sta z:zTemp00
    cla
    adc a:OverlayEffectColourLists+1
    sta z:zTemp01
    bra EffectColour07D1_at1
EffectColour07D1_at0:
    asl a
    asl a
    adc #$E3
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour07D1_at1:
    cly
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_HI
    jmp ClearEffectSlot1A31

; @routine 0931:4FF EffectColour0931
; @run 6C:84FF
; WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with the overlay at sector 0931's table of lists for a
; negative list number ($89D7).
; HOW: EffectColour1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectColour1A31.
EffectColour0931:
    lda a:ScriptSlotScript,x
    bmi EffectColour0931_at0
    asl a
    asl a
    adc a:OverlayEffectColourLists
    sta z:zTemp00
    cla
    adc a:OverlayEffectColourLists+1
    sta z:zTemp01
    bra EffectColour0931_at1
EffectColour0931_at0:
    asl a
    asl a
    adc #$D7
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour0931_at1:
    cly
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_HI
    jmp ClearEffectSlot1A31

; @routine 0BF1:4FF EffectColour0BF1
; @run 6C:84FF
; WHAT IT DOES: Kind 3 of the palette effects; as EffectColour1A31 with the overlay at sector 0BF1's table of lists for a
; negative list number ($89C7).
; HOW: EffectColour1A31's body.
; WAITS: no.
; Entry: X the slot. Exit: as EffectColour1A31.
EffectColour0BF1:
    lda a:ScriptSlotScript,x
    bmi EffectColour0BF1_at0
    asl a
    asl a
    adc a:OverlayEffectColourLists
    sta z:zTemp00
    cla
    adc a:OverlayEffectColourLists+1
    sta z:zTemp01
    bra EffectColour0BF1_at1
EffectColour0BF1_at0:
    asl a
    asl a
    adc #$C7
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour0BF1_at1:
    cly
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_HI
    jmp ClearEffectSlot1A31
