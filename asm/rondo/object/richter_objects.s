; @routine 01BD:238 RichterObjectMarkOn
; @run 6E:C238
; WHAT IT DOES: Sets bit 4 of an object's OBJ_2B18.
; HOW: ORed in.
; Entry: X object. Exit: A the new OBJ_2B18, N and Z from it. X, Y, C and V kept.
RichterObjectMarkOn:
    lda #$10
    ora a:OBJ_2B18,x
    sta a:OBJ_2B18,x
    rts

; @routine 01BD:3EC RichterObjectMarkOff
; @run 6E:C3EC
; WHAT IT DOES: Clears bit 4 of an object's OBJ_2B18.
; HOW: ANDed out.
; Entry: X object. Exit: A the new OBJ_2B18, N and Z from it. X, Y, C and V kept.
RichterObjectMarkOff:
    lda #$EF
    and a:OBJ_2B18,x
    sta a:OBJ_2B18,x
    rts

; @routine 01BD:3F8 RichterObjectFromPlayer
; @run 6E:C3F8
; WHAT IT DOES: Gives an object Richter's facing and his OBJ_2FE8, doubled.
; HOW: RichterObjectTake2FE8, then the facing copied.
; Entry: X object. Exit: A the facing, N and Z from it, C as RichterObjectTake2FE8. X, Y and V
; kept.
RichterObjectFromPlayer:
    bsr RichterObjectTake2FE8
    lda a:OBJ_2AC0
    sta a:OBJ_2AC0,x
    rts

; @routine 01BD:401 RichterObjectTake2FE8
; @run 6E:C401
; WHAT IT DOES: Gives an object Richter's OBJ_2FE8, doubled.
; HOW: Shifted up a place.
; Entry: X object. Exit: A the value, N and Z from it, C Richter's bit 7. X, Y and V kept.
RichterObjectTake2FE8:
    lda a:OBJ_2FE8
    asl a
    sta a:OBJ_2FE8,x
    rts

; @routine 01BD:409 RichterObjectSlot
; @run 6E:C409
; WHAT IT DOES: Gives the slot of the object being run.
; HOW: zRichterObject.
; Entry: nothing. Exit: X the slot, N and Z from it. A, Y, C and V kept.
RichterObjectSlot:
    ldx z:zRichterObject
    rts

; @routine 01BD:47C RichterObjectQuicken
; @run 6E:C47C
; WHAT IT DOES: Once, at its animation's step 6 and while PlayerByte3BE1 is set, makes an object's
; frame last 4 longer and Richter's one frame shorter than that.
; HOW: OBJ_35C0 marks it done; OBJ_2F38 the step; OBJ_2F90 the frame's time.
; Entry: X object. Exit: X and Y kept. Stopped, A what was read last, N and Z from it, or from
; comparing the step with 6 and C with it. Done, A Richter's new time, N and Z from the mark, C
; and V from the add.
RichterObjectQuicken:
    lda a:OBJ_35C0,x
    bne RichterObjectQuicken_at0
    lda a:OBJ_FLAGS_2F38,x
    cmp #$06
    bne RichterObjectQuicken_at0
    lda a:PlayerByte3BE1
    beq RichterObjectQuicken_at0
    lda #$04
    clc
    adc a:OBJ_2F90,x
    sta a:OBJ_2F90,x
    dec a
    sta a:OBJ_2F90
    inc a:OBJ_35C0,x
    rts
RichterObjectQuicken_at0:
    rts

; @routine 01BD:49F RichterObjectFlash
; @run 6E:C49F
; WHAT IT DOES: While an object is charged, shows the flashing form of its frame, if it is one
; of the four that have one, with its OBJ_2B18 for the flash.
; HOW: OBJ_3568 is the charge; frames $07, $0E, $15 and $1C become $8E to $91; RichterFlashFlags
; by OBJ_34B8 less $0E, with bit 7.
; Entry: X object. Exit: X kept. Not charged, A 0, Z set, Y $8E. Not one of the four, A the
; frame, N, Z and C from comparing it with $1C, Y $91. Flashed, A the flags, N and Z from them, Y
; OBJ_34B8 less $0E, C and V from that subtract. V kept otherwise.
RichterObjectFlash:
    ldy #$8E
    lda a:OBJ_3568,x
    beq RichterObjectFlash_at0
    lda a:OBJ_2A68,x
    cmp #$07
    beq RichterObjectFlash_at1
    iny
    cmp #$0E
    beq RichterObjectFlash_at1
    iny
    cmp #$15
    beq RichterObjectFlash_at1
    iny
    cmp #$1C
    beq RichterObjectFlash_at1
RichterObjectFlash_at0:
    rts
RichterObjectFlash_at1:
    tya
    sta a:OBJ_2A68,x
    lda a:OBJ_34B8,x
    sec
    sbc #$0E
    tay
    lda a:RichterFlashFlags,y
    ora #$80
    sta a:OBJ_2B18,x
    rts

; @routine 01BD:4D1 RichterObjectCharge
; @run 6E:C4D1
; WHAT IT DOES: Charges an object from OBJ_34B8 $0C on, the first time the way it faces is newly
; pressed.
; HOW: OBJ_3568 counted up; RichterChargePads by OBJ_2AC0 against zPadPressed.
; Entry: X object. Exit: X kept. Before $0C, A OBJ_34B8, N, Z and C from comparing it with $0C.
; Charged already, A the charge, N and Z from it, C set. Not pressed, A 0, Z set, C set, Y the
; facing. Charged now, A the press, N and Z from the charge, C set, Y the facing. V kept.
RichterObjectCharge:
    lda a:OBJ_34B8,x
    cmp #$0C
    bcc RichterObjectCharge_at0
    lda a:OBJ_3568,x
    bne RichterObjectCharge_at0
    ldy a:OBJ_2AC0,x
    lda z:zPadPressed
    and a:RichterChargePads,y
    beq RichterObjectCharge_at0
    inc a:OBJ_3568,x
RichterObjectCharge_at0:
    rts

; @routine 01BD:5C5 RichterObjectNextState
; @run 6E:C5C5
; WHAT IT DOES: Moves an object on to its next state.
; HOW: OBJ_27A8 counted up.
; Entry: X object. Exit: N and Z from the new state. A, X, Y, C and V kept.
RichterObjectNextState:
    inc a:OBJ_27A8,x
    rts

; @routine 01BE:67B RichterObjectAtPlayer
; @run 6E:CE7B
; WHAT IT DOES: Puts an object at Richter's place plus an offset, signed, the X offset turned
; round when the object faces left.
; HOW: zTemp04 and zTemp06 the offsets, made 16-bit in zTemp05 and zTemp07.
; Entry: X object, zTemp04 and zTemp06. Exit: A the Y position's high byte, N, Z, C and V from its
; add. X and Y kept.
RichterObjectAtPlayer:
    stz z:zTemp05
    stz z:zTemp07
    lda z:zTemp04
    bpl RichterObjectAtPlayer_at0
    dec z:zTemp05
RichterObjectAtPlayer_at0:
    lda z:zTemp06
    bpl RichterObjectAtPlayer_at1
    dec z:zTemp07
RichterObjectAtPlayer_at1:
    lda a:OBJ_2AC0,x
    beq RichterObjectAtPlayer_at2
    cla
    sec
    sbc z:zTemp04
    sta z:zTemp04
    cla
    sbc z:zTemp05
    sta z:zTemp05
RichterObjectAtPlayer_at2:
    lda a:OBJ_X_LO
    clc
    adc z:zTemp04
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI
    adc z:zTemp05
    sta a:OBJ_X_HI,x
    lda a:OBJ_Y_LO
    clc
    adc z:zTemp06
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI
    adc z:zTemp07
    sta a:OBJ_Y_HI,x
    rts
