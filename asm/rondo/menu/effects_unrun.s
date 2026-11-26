; @routine 1E65:371 EndingEffectFadeA
; @run 6C:8371
; WHAT IT DOES: A palette effect that fades palettes down: once its timer has run down it counts a round, gives the slot back after the
; eighth, and otherwise takes the list's palettes (an address of the VCE and a count each) and darkens every colour of them by one step
; in each of its three components, writing them back to the VCE.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402 (or a table at $9412); the VCE read into $3C60 by a TAI,
; changed word by word, and written back by a TIA.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X $20 when a palette
; was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
EndingEffectFadeA:
    lda a:ScriptSlotTimer,x
    beq EndingEffectFadeA_at0
    dec a:ScriptSlotTimer,x
    rts
EndingEffectFadeA_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EndingEffectFadeA_at1
    jmp EndingEffectList_at5
EndingEffectFadeA_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EndingEffectFadeA_at2
    lda a:$9412,y
    sta z:zTemp00
    lda a:$9413,y
    sta z:zTemp01
    bra EndingEffectFadeA_at3
EndingEffectFadeA_at2:
    lda a:$9402
    sta z:zBiosAl
    lda a:$9403
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EndingEffectFadeA_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EndingEffectFadeA_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EndingEffectFadeA_at4
    rts
EndingEffectFadeA_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EndingEffectFadeA_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EndingEffectFadeA_at5
    dec a
EndingEffectFadeA_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EndingEffectFadeA_at6
    sec
    sbc #$08
EndingEffectFadeA_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EndingEffectFadeA_at7
    sec
    sbc #$20
EndingEffectFadeA_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EndingEffectFadeA_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EndingEffectFadeA_at9

; @routine 20C0:371 EndingEffectFadeB
; @run 6C:8371
; WHAT IT DOES: A palette effect that fades palettes down: once its timer has run down it counts a round, gives the slot back after the
; eighth, and otherwise takes the list's palettes (an address of the VCE and a count each) and darkens every colour of them by one step
; in each of its three components, writing them back to the VCE.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402 (or a table at $940E); the VCE read into $3C60 by a TAI,
; changed word by word, and written back by a TIA.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X $20 when a palette
; was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
EndingEffectFadeB:
    lda a:ScriptSlotTimer,x
    beq EndingEffectFadeB_at0
    dec a:ScriptSlotTimer,x
    rts
EndingEffectFadeB_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EndingEffectFadeB_at1
    jmp EndingEffectList_at5
EndingEffectFadeB_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EndingEffectFadeB_at2
    lda a:$940E,y
    sta z:zTemp00
    lda a:$940F,y
    sta z:zTemp01
    bra EndingEffectFadeB_at3
EndingEffectFadeB_at2:
    lda a:$9402
    sta z:zBiosAl
    lda a:$9403
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EndingEffectFadeB_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EndingEffectFadeB_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EndingEffectFadeB_at4
    rts
EndingEffectFadeB_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EndingEffectFadeB_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EndingEffectFadeB_at5
    dec a
EndingEffectFadeB_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EndingEffectFadeB_at6
    sec
    sbc #$08
EndingEffectFadeB_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EndingEffectFadeB_at7
    sec
    sbc #$20
EndingEffectFadeB_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EndingEffectFadeB_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EndingEffectFadeB_at9

; @routine 224F:371 EndingEffectFadeC
; @run 6C:8371
; WHAT IT DOES: A palette effect that fades palettes down: once its timer has run down it counts a round, gives the slot back after the
; eighth, and otherwise takes the list's palettes (an address of the VCE and a count each) and darkens every colour of them by one step
; in each of its three components, writing them back to the VCE.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402 (or a table at $9410); the VCE read into $3C60 by a TAI,
; changed word by word, and written back by a TIA.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X $20 when a palette
; was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
EndingEffectFadeC:
    lda a:ScriptSlotTimer,x
    beq EndingEffectFadeC_at0
    dec a:ScriptSlotTimer,x
    rts
EndingEffectFadeC_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EndingEffectFadeC_at1
    jmp EndingEffectList_at5
EndingEffectFadeC_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EndingEffectFadeC_at2
    lda a:$9410,y
    sta z:zTemp00
    lda a:$9411,y
    sta z:zTemp01
    bra EndingEffectFadeC_at3
EndingEffectFadeC_at2:
    lda a:$9402
    sta z:zBiosAl
    lda a:$9403
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EndingEffectFadeC_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EndingEffectFadeC_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EndingEffectFadeC_at4
    rts
EndingEffectFadeC_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EndingEffectFadeC_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EndingEffectFadeC_at5
    dec a
EndingEffectFadeC_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EndingEffectFadeC_at6
    sec
    sbc #$08
EndingEffectFadeC_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EndingEffectFadeC_at7
    sec
    sbc #$20
EndingEffectFadeC_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EndingEffectFadeC_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EndingEffectFadeC_at9

; @routine 23FA:371 EndingEffectFadeD
; @run 6C:8371
; WHAT IT DOES: A palette effect that fades palettes down: once its timer has run down it counts a round, gives the slot back after the
; eighth, and otherwise takes the list's palettes (an address of the VCE and a count each) and darkens every colour of them by one step
; in each of its three components, writing them back to the VCE.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402 (or a table at $9416); the VCE read into $3C60 by a TAI,
; changed word by word, and written back by a TIA.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X $20 when a palette
; was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
EndingEffectFadeD:
    lda a:ScriptSlotTimer,x
    beq EndingEffectFadeD_at0
    dec a:ScriptSlotTimer,x
    rts
EndingEffectFadeD_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EndingEffectFadeD_at1
    jmp EndingEffectList_at5
EndingEffectFadeD_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EndingEffectFadeD_at2
    lda a:$9416,y
    sta z:zTemp00
    lda a:$9417,y
    sta z:zTemp01
    bra EndingEffectFadeD_at3
EndingEffectFadeD_at2:
    lda a:$9402
    sta z:zBiosAl
    lda a:$9403
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EndingEffectFadeD_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EndingEffectFadeD_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EndingEffectFadeD_at4
    rts
EndingEffectFadeD_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EndingEffectFadeD_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EndingEffectFadeD_at5
    dec a
EndingEffectFadeD_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EndingEffectFadeD_at6
    sec
    sbc #$08
EndingEffectFadeD_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EndingEffectFadeD_at7
    sec
    sbc #$20
EndingEffectFadeD_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EndingEffectFadeD_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EndingEffectFadeD_at9
