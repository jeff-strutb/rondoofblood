; TEMPORARY: the stage 07 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
; The disc tables the routines read are named here by their overlay address until the typed data
; layer reads them.
;
;
;
;
;
;
;
; TEMPORARY: the stage 07 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
; The disc tables the routines read are named here by their overlay address until the typed data
; layer reads them.
;
;
;
;
;
;
;
; TEMPORARY: the stage 07 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
; The disc tables the routines read are named here by their overlay address until the typed data
; layer reads them.
;
;
;
;
;
;
;
; @routine 0BFB:3DC EnterState4IfFlagged
; @run 70:B3DC
; WHAT IT DOES: Sends the object to state 4 when bit 7 of its status flags is set.
; HOW: Reads the flags once and masks bit 7; writes the state only when it is set.
; WAITS: no.
; Entry: X object. Exit: A the masked bit, or 4 when entered; N, Z from it; X, Y and C kept.
EnterState4IfFlagged:
    lda a:OBJ_31A0,x
    and #$80
    beq EnterState4IfFlagged_at0
    lda #$04
    sta a:OBJ_27A8,x
EnterState4IfFlagged_at0:
    rts

; @routine 0BFC:141 ScrollNotAtRightEnd
; @run 70:B941
; WHAT IT DOES: Says, in the carry, that the scroll has not come to the right end of the level:
; carry set when zField82 is set, when two or more screens remain, or when one screen remains
; and the scroll's low byte is under $E0; clear otherwise.
; HOW: Subtracts the first scroll entry's high screen byte from the scroll's limit
; (ScrollXMaxHi) and tests the difference.
; WAITS: no.
; Entry: nothing. Exit: C the answer; A and N, Z from the last load or compare; V from the
; subtract. Y and X kept. The arms past the screen difference of 0 or 1 ran in no script: the
; draft stops there and liftcmp judges only the first tests; the rest waits for coverage
; (PROCEDURE section 6).
ScrollNotAtRightEnd:
    lda z:zField82
    bne ScrollNotAtRightEnd_set
    lda a:ScrollXMaxHi
    sec
    sbc a:ScrollXHiScreen
    cmp #$02
    bcs ScrollNotAtRightEnd_out
    cmp #$00
    beq ScrollNotAtRightEnd_clear
    lda a:ScrollXLo
    cmp #$E0
    bcs ScrollNotAtRightEnd_clear
ScrollNotAtRightEnd_set:
    sec
ScrollNotAtRightEnd_out:
    rts
ScrollNotAtRightEnd_clear:
    clc
    rts

; @routine 0BFC:2AB SetHitboxSizeFromFrame
; @run 70:BAAB
; WHAT IT DOES: Sets the object's hit rectangle's half width and half height from a two byte
; row of a table by its frame minus $2A; frame 0 clears both.
; HOW: The table is at overlay address $BAC9.
; WAITS: no.
; Entry: X object. Exit: A the last byte loaded; N, Z from it; C bit 7 of the frame minus $2A
; (the shift); V from the subtract; Y the row's offset; X kept.
SetHitboxSizeFromFrame:
    lda a:OBJ_2A68,x
    beq SetHitboxSizeFromFrame_clear
    sec
    sbc #$2A
    asl a
    tay
    lda a:$BAC9,y
    sta a:OBJ_3988,x
    lda a:$BACA,y
    sta a:OBJ_3A38,x
    rts
SetHitboxSizeFromFrame_clear:
    stz a:OBJ_3988,x
    stz a:OBJ_3A38,x
    rts

; @routine 0BFC:3B3 ChangeToType9C
; @run 70:BBB3
; WHAT IT DOES: Turns the object into one of type $9C, in state 0.
; HOW: Stores $9C in its kind byte and zero in its state.
; WAITS: no.
; Entry: X object. Exit: A $9C, N set, Z clear; X, Y and C kept.
ChangeToType9C:
    lda #$9C
    sta a:OBJ_2750,x
ChangeToType9C_at0:
    stz a:OBJ_27A8,x
    rts

; @routine 0BFB:3E9 EnterNextStateWhenPlayerNear
; @run 70:B3E9
; WHAT IT DOES: When the player is nearer along X than the distance in the object's next_kind byte,
; sets f3250 to 2 and life_timer to $20 and moves the object on to its next state.
; HOW: DistanceXToPlayer compared with next_kind (read once).
; WAITS: no.
; Entry: X object, D clear. Exit: as the compare left them (V from the distance's subtract), or
; A $20 and N, Z from the new state; X, Y kept.
EnterNextStateWhenPlayerNear:
    jsr VecDistanceXToPlayer
    cmp a:OBJ_3040,x
    bcs EnterNextStateWhenPlayerNear_at0
    lda #$02
    sta a:OBJ_3250,x
    lda #$20
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8,x
EnterNextStateWhenPlayerNear_at0:
    rts

; @routine 0BFC:1C6 SwingOnCosine
; @run 70:B9C6
; WHAT IT DOES: Animates the object along the list at overlay address $B9F6, gives it a Y speed
; of its life_timer's angle's cosine (speed 1.0), moves it, and turns life_timer on by 2.
; HOW: StepObjectFrames, ScaleByCosine with zTemp00-01 set to 256, ApplyVelocity.
; WAITS: no.
; Entry: X object, D clear. Exit: A the new life_timer, N, Z, C and V from the add of 2; Y 0.
; Clobbers zTemp00-01, zTemp10-12, zProduct0-2, zMultiplierLo-Hi.
SwingOnCosine:
    lda #$F6
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$00
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_34B8,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VY_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocity
    lda a:OBJ_34B8,x
    clc
    adc #$02
    sta a:OBJ_34B8,x
    rts

; @routine 0BFC:330 AnimateThenFrameRecord
; @run 70:BB30
; WHAT IT DOES: Animates the object along the list at overlay address $BB4B; when the list has
; ended, sets f3358 to 2 and f3300 to $0E and moves the object on to its next state.
; HOW: StepObjectFrames; its result says the list ended.
; WAITS: no.
; Entry: X object, D clear. Exit: as StepObjectFrames, or at the end A $0E and N, Z from the new
; state; X kept. Clobbers zTemp10-11.
AnimateThenFrameRecord:
    lda #$4B
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne AnimateThenFrameRecord_at0
    lda #$02
    sta a:OBJ_3358,x
    lda #$0E
    sta a:OBJ_3300,x
    inc a:OBJ_27A8,x
AnimateThenFrameRecord_at0:
    rts

; @routine 0BFF:1A3 StartThenAnimate
; @run 71:D1A3
; WHAT IT DOES: The first time the object runs (state 0), starts it from its record and sets its
; f2a10 to 6. Every time, it animates it along the list at overlay address $D1BC.
; HOW: LoadObjectRecord when the state is 0, then StepObjectFrames as the last call.
; WAITS: no.
; Entry: X object, D clear. Exit: as StepObjectFrames (Y 11 from the record start when the
; animation does not change it); X and V kept. (A JSR then RTS, not a tail jump.) Clobbers zTemp10-11.
StartThenAnimate:
    lda a:OBJ_27A8,x
    bne StartThenAnimate_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_2A10,x
StartThenAnimate_at0:
    lda #$D1
    sta z:zTemp11
    lda #$BC
    sta z:zTemp10
    jsr VecStepObjectFrames
    rts
