; @routine 0CAB:175 FallingWithGravity
; @run 70:B175
; WHAT IT DOES: Steps the animation list at $B187, applies unlimited gravity and moves the object.
; HOW: StepObjectFrames, ApplyGravityUnlimited, ApplyVelocity.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as ApplyVelocity, Y as the step left it.
FallingWithGravity:
    lda #$87
    sta z:zTemp10
    lda #$B1
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jsr VecApplyVelocity
    rts

; @routine 0CAB:203 StartWhenHurt
; @run 70:B203
; WHAT IT DOES: An object in state 3 that has taken a hit of 5 or more resets its animation, enters state 6 and clears OBJ_2800.
; HOW: Two compares.
; WAITS: no.
; Entry: X object. Exit: the last compare or load.
StartWhenHurt:
    lda a:OBJ_27A8,x
    cmp #$03
    bne StartWhenHurt_at0
    lda a:OBJ_33B0,x
    cmp #$05
    bcc StartWhenHurt_at0
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$06
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
StartWhenHurt_at0:
    rts

; @routine 0CAB:220 EnterStateSevenOnFlag
; @run 70:B220
; WHAT IT DOES: When bit 7 of the object's status flags is set and its state is below 7, it faces right, resets its animation, enters state 7 and clears OBJ_2800.
; HOW: Two compares.
; Untraced: every arm past the flag test (B227 to B23E).
; WAITS: no.
; Entry: X object. Exit: the last AND, compare or load.
EnterStateSevenOnFlag:
    lda a:OBJ_31A0,x
    and #$80
    beq EnterStateSevenOnFlag_at0
    lda a:OBJ_27A8,x
    cmp #$07
    bcs EnterStateSevenOnFlag_at0
    stz a:OBJ_2AC0,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$07
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
EnterStateSevenOnFlag_at0:
    rts

; @routine 0CAB:304 StartWithGuard
; @run 70:B304
; WHAT IT DOES: Loads the object's record and starts a kind $25 object at its place in slot $11; its timer ends as $80.
; HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: A $80, Y $11, C from the record, N set.
StartWithGuard:
    jsr VecLoadObjectRecord
    ldy #$11
    jsr VecClearObjectInY
    lda #$25
    jsr VecStartObjectAtPlace
    lda #$E0
    sta a:OBJ_34B8,x
    lda #$80
    sta a:OBJ_34B8,x
    rts

; @routine 0CAB:389 SlideAngleDown
; @run 70:B389
; WHAT IT DOES: Takes one off the 16-bit angle word (OBJ_3568 high, OBJ_35C0 low, in steps of $0100 less 0 on the low byte as the code subtracts $0100); when the high byte
; reaches 0 the object's OBJ_2800 goes up one.
; HOW: A 16-bit subtract.
; WAITS: no.
; Entry: X object. Exit: the last subtract or INC.
SlideAngleDown:
    lda a:OBJ_35C0,x
    sec
    sbc #$00
    sta a:OBJ_35C0,x
    lda a:OBJ_3568,x
    sbc #$01
    sta a:OBJ_3568,x
    bne SlideAngleDown_at0
    inc a:OBJ_2800,x
SlideAngleDown_at0:
    rts

; @routine 0CAB:3A0 SlideAngleUp
; @run 70:B3A0
; WHAT IT DOES: Adds $00C0 to the 16-bit angle word (OBJ_3568 high, OBJ_35C0 low); once the high byte reaches $10 the object's OBJ_2800 goes up one.
; HOW: A 16-bit add and a compare.
; WAITS: no.
; Entry: X object. Exit: the compare or the INC.
SlideAngleUp:
    lda a:OBJ_35C0,x
    clc
    adc #$C0
    sta a:OBJ_35C0,x
    lda a:OBJ_3568,x
    adc #$00
    sta a:OBJ_3568,x
    cmp #$10
    bcc SlideAngleUp_at0
    inc a:OBJ_2800,x
SlideAngleUp_at0:
    rts
