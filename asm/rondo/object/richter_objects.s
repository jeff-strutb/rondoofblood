; @routine 01BD:00C RichterObjectAddXLow
; @run 6E:C00C
; WHAT IT DOES: Adds to an object's X position's low byte.
; HOW: A plain add, the carry left for RichterObjectAddXHigh.
; WAITS: no.
; Entry: A the amount, X object. Exit: A the low byte, N, Z, C and V from the add. X and Y kept.
RichterObjectAddXLow:
    clc
    adc a:OBJ_X_LO,x
    sta a:OBJ_X_LO,x
    rts

; @routine 01BD:014 RichterObjectAddXHigh
; @run 6E:C014
; WHAT IT DOES: Adds to an object's X position's high byte, with the carry from the low byte.
; HOW: An add with the carry in.
; WAITS: no.
; Entry: A the amount, C the carry, X object. Exit: A the high byte, N, Z, C and V from the add.
; X and Y kept.
RichterObjectAddXHigh:
    adc a:OBJ_X_HI,x
    sta a:OBJ_X_HI,x
    rts

; @routine 01BD:1CF RichterObjectFall
; @run 6E:C1CF
; WHAT IT DOES: Pulls an object down: its Y speed grows by 6.
; HOW: Runs on into RichterObjectSetFall.
; WAITS: no.
; Entry: X object. Exit: as RichterObjectSetFall.
RichterObjectFall:
    lda a:OBJ_VY_LO,x
    clc
    adc #$06

; @routine 01BD:1D5 RichterObjectSetFall
; @run 6E:C1D5
; WHAT IT DOES: Sets an object's Y speed's low byte, carrying into its high byte.
; HOW: The carry added to OBJ_VY_HI.
; WAITS: no.
; Entry: A the low byte, C the carry, X object. Exit: A the high byte, N, Z, C and V from adding
; the carry. X and Y kept.
RichterObjectSetFall:
    sta a:OBJ_VY_LO,x
    cla
    adc a:OBJ_VY_HI,x
    sta a:OBJ_VY_HI,x
    rts

; @routine 01BD:1E0 RichterObjectFallSlow
; @run 6E:C1E0
; WHAT IT DOES: Pulls an object down more gently: its Y speed grows by 3.
; HOW: RichterObjectSetFall.
; WAITS: no.
; Entry: X object. Exit: as RichterObjectSetFall.
RichterObjectFallSlow:
    lda a:OBJ_VY_LO,x
    clc
    adc #$03
    bra RichterObjectSetFall

; @routine 01BD:238 RichterObjectMarkOn
; @run 6E:C238
; WHAT IT DOES: Sets bit 4 of an object's OBJ_2B18.
; HOW: ORed in.
; WAITS: no.
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
; WAITS: no.
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
; WAITS: no.
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
; WAITS: no.
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
; WAITS: no.
; Entry: nothing. Exit: X the slot, N and Z from it. A, Y, C and V kept.
RichterObjectSlot:
    ldx z:zRichterObject
    rts

; @routine 01BD:47C RichterObjectQuicken
; @run 6E:C47C
; WHAT IT DOES: Once, at its animation's step 6 and while PlayerByte3BE1 is set, makes an object's
; frame last 4 longer and Richter's one frame shorter than that.
; HOW: OBJ_35C0 marks it done; OBJ_2F38 the step; OBJ_2F90 the frame's time.
; WAITS: no.
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
; WAITS: no.
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
; WAITS: no.
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
; WAITS: no.
; Entry: X object. Exit: N and Z from the new state. A, X, Y, C and V kept.
RichterObjectNextState:
    inc a:OBJ_27A8,x
    rts

; @routine 01BE:67B RichterObjectAtPlayer
; @run 6E:CE7B
; WHAT IT DOES: Puts an object at Richter's place plus an offset, signed, the X offset turned
; round when the object faces left.
; HOW: zTemp04 and zTemp06 the offsets, made 16-bit in zTemp05 and zTemp07.
; WAITS: no.
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

; @routine 01BE:6BE RichterObjectAtPlayerPlace
; @run 6E:CEBE
; WHAT IT DOES: Puts an object exactly where Richter is.
; HOW: His position into zTemp00 to 03; runs on into RichterObjectToPoint.
; WAITS: no.
; Entry: X object. Exit: as RichterObjectToPoint.
RichterObjectAtPlayerPlace:
    lda a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_X_HI
    sta z:zTemp01
    lda a:OBJ_Y_LO
    sta z:zTemp02
    lda a:OBJ_Y_HI
    sta z:zTemp03

; @routine 01BE:6D2 RichterObjectToPoint
; @run 6E:CED2
; WHAT IT DOES: Puts an object at the point in zTemp00 to 03.
; HOW: X low and high, Y low and high.
; WAITS: no.
; Entry: X object. Exit: A the Y position's high byte, N and Z from it. X, Y, C and V kept.
RichterObjectToPoint:
    lda z:zTemp00
    sta a:OBJ_X_LO,x
    lda z:zTemp01
    sta a:OBJ_X_HI,x
    lda z:zTemp02
    sta a:OBJ_Y_LO,x
    lda z:zTemp03
    sta a:OBJ_Y_HI,x
    rts

; @routine 01C0:240 RichterObjectAnimationWait
; @run 6E:DA40
; WHAT IT DOES: Steps an object's animation and, unless it has ended or shows frame 0, returns to
; its caller's caller, so the caller's own work waits for the end.
; HOW: RichterObjectAnimationTick; Z from it; the assembly drops the return address with two pulls.
; WAITS: no.
; Entry: X object. Exit, Z set: as the tick. Otherwise the caller's return address is gone, A its
; high byte, N and Z from it; its caller then ends at once.
RichterObjectAnimationWait:
    jsr RichterObjectAnimationTick
    beq RichterObjectAnimationWait_at0
    pla
    pla
RichterObjectAnimationWait_at0:
    rts

; @routine 01C0:248 RichterObjectAnimationRunning
; @run 6E:DA48
; WHAT IT DOES: Steps an object's animation; its caller looks at whether it has ended.
; HOW: RichterObjectAnimationTick, then the timer against $FF.
; WAITS: no.
; Entry: X object. Exit: A the timer, N, Z and C from comparing it with $FF. Y as the tick. X and
; V kept. Clobbers zTemp00-01.
RichterObjectAnimationRunning:
    jsr RichterObjectAnimationTick
    lda a:OBJ_2F90,x
    cmp #$FF
    rts

; @routine 01C0:251 RichterObjectAnimationTick
; @run 6E:DA51
; WHAT IT DOES: Counts down an object's frame on show; at zero moves to the next step of its list.
; A timer of $FF means its animation has stopped.
; HOW: OBJ_2F90 the timer, OBJ_FLAGS_2F38 the step, OBJ_2A68 the frame shown.
; WAITS: no.
; Entry: X object. Exit: as RichterAnimationTick.
RichterObjectAnimationTick:
    lda a:OBJ_2F90,x
    cmp #$FF
    bne RichterObjectAnimationTick_at0
    rts
RichterObjectAnimationTick_at0:
    dec a:OBJ_2F90,x
    bne RichterObjectSetAnimation_at0
    inc a:OBJ_FLAGS_2F38,x
    bra RichterObjectSetAnimation_at1

; @routine 01C0:263 RichterObjectSetAnimation
; @run 6E:DA63
; @stop DA84
; WHAT IT DOES: Starts an object's animation from its first step.
; HOW: The animation into OBJ_FIELDS_2EE0, step zero, and its first entry read.
; WAITS: no.
; Entry: A animation, X object. Exit: as RichterAnimationTick. Clobbers zTemp00-01.
RichterObjectSetAnimation:
    sta a:OBJ_FIELDS_2EE0,x
RichterObjectSetAnimation_at3:
    stz a:OBJ_FLAGS_2F38,x
RichterObjectSetAnimation_at1:
    bsr RichterObjectAnimationList
    lda (zTemp00),y
    sta a:OBJ_2F90,x
    cmp #$FF
    bne RichterObjectSetAnimation_at2
    rts
RichterObjectSetAnimation_at2:
    cmp #$FE
    beq RichterObjectSetAnimation_at3
RichterObjectSetAnimation_at4:
    iny
    lda (zTemp00),y
    sta a:OBJ_2A68,x
    rts
RichterObjectSetAnimation_at0:
    bsr RichterObjectAnimationList
    bra RichterObjectSetAnimation_at4

; @routine 01C0:284 RichterObjectAnimationList
; @run 6E:DA84
; WHAT IT DOES: Points at an object's animation list and its step's entry.
; HOW: RichterObjectAnimations by OBJ_FIELDS_2EE0 into zTemp00 and 01; the step doubled.
; WAITS: no.
; Entry: X object. Exit: A and Y twice the step, N and Z from it, C the step's bit 7. X and V
; kept.
RichterObjectAnimationList:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    tay
    lda a:RichterObjectAnimations,y
    sta z:zTemp00
    lda a:RichterObjectAnimations+1,y
    sta z:zTemp01
    lda a:OBJ_FLAGS_2F38,x
    asl a
    tay
    rts

; @routine 01C0:51B RichterObjectHitboxOff
; @run 6E:DD1B
; WHAT IT DOES: Takes an object's hitbox away.
; HOW: OBJ_3250 cleared.
; WAITS: no.
; Entry: X object. Exit: all registers and flags kept.
RichterObjectHitboxOff:
    stz a:OBJ_3250,x
    rts

; @routine 01C0:51F RichterObjectHitboxOn
; @run 6E:DD1F
; WHAT IT DOES: Marks an object as having a hitbox and gives it its frame's.
; HOW: OBJ_3250 1; runs on into RichterObjectHitboxByFrame.
; WAITS: no.
; Entry: X object. Exit: as RichterObjectHitboxByFrame.
RichterObjectHitboxOn:
    lda #$01
    sta a:OBJ_3250,x

; @routine 01C0:524 RichterObjectHitboxByFrame
; @run 6E:DD24
; WHAT IT DOES: Gives an object the hitbox of the frame it shows, or none.
; HOW: RichterFrameHitboxes by OBJ_2A68; runs on into RichterObjectHitboxFrom.
; WAITS: no.
; Entry: X object. Exit: none, A 0, Z set, N clear, Y the frame. Otherwise as
; RichterObjectHitboxFrom.
RichterObjectHitboxByFrame:
    ldy a:OBJ_2A68,x
    lda a:RichterFrameHitboxes,y

; @routine 01C0:52A RichterObjectHitboxFrom
; @run 6E:DD2A
; WHAT IT DOES: Gives an object the hitbox picked, or none.
; HOW: Z from the caller says none; otherwise the hitbox's four bytes from RichterObjectHitboxes,
; four bytes a hitbox.
; WAITS: no.
; Entry: A the hitbox, Z set for none, X object. Exit: none, as RichterObjectHitboxOff. Otherwise
; A the last byte, N and Z from it, Y four times the hitbox, C its bit 6. X and V kept.
RichterObjectHitboxFrom:
    beq RichterObjectHitboxOff
    asl a
    asl a
    tay
    lda a:RichterObjectHitboxes,y
    sta a:OBJ_3930,x
    lda a:RichterObjectHitboxes+1,y
    sta a:OBJ_39E0,x
    lda a:RichterObjectHitboxes+2,y
    sta a:OBJ_3988,x
    lda a:RichterObjectHitboxes+3,y
    sta a:OBJ_3A38,x
    rts
