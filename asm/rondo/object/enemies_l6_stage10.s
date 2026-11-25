; @routine 10CB:2CB ClearSlot4EUnlessCamera18
; @run 70:B2CB
; WHAT IT DOES: Unless the camera mode is $18, clears the object in slot $4E, sets OBJ_3510 to $40 and goes on to the next OBJ_2800.
; HOW: A compare, ClearObjectInY, a store, an INC.
; WAITS: no.
; Entry: X object. Exit: the compare, or the new OBJ_2800's N and Z (A $40).
ClearSlot4EUnlessCamera18:
    lda a:CameraMode
    cmp #$18
    beq ClearSlot4EUnlessCamera18_at0
    ldy #$4E
    jsr VecClearObjectInY
    lda #$40
    sta a:OBJ_3510,x
    inc a:OBJ_2800,x
ClearSlot4EUnlessCamera18_at0:
    rts

; @routine 10CB:680 StepListB697ThenStopY
; @run 70:B680
; WHAT IT DOES: Steps the animation list at $B697; when it ends, the object's Y speed is cleared and it goes to the next OBJ_2800.
; HOW: StepObjectFrames, then stores and an increment.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step, or the new OBJ_2800's N and Z when the list ended.
StepListB697ThenStopY:
    lda #$97
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListB697ThenStopY_at0
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_2800,x
StepListB697ThenStopY_at0:
    rts

; @routine 10CB:702 StepListB728ThenRiseSlowly
; @run 70:B702
; WHAT IT DOES: Steps the animation list at $B728; while it runs the object moves and its Y speed loses $0010 a frame; when it ends the object goes to the next OBJ_2800.
; HOW: StepObjectFrames, ApplyVelocity, a 16-bit subtract, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the subtract's, or the new OBJ_2800's N and Z.
StepListB728ThenRiseSlowly:
    lda #$28
    sta z:zTemp10
    lda #$B7
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq StepListB728ThenRiseSlowly_at0
    jsr VecApplyVelocity
    lda a:OBJ_VY_LO,x
    sec
    sbc #$10
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    rts
StepListB728ThenRiseSlowly_at0:
    inc a:OBJ_2800,x
    rts

; @routine 10CD:181 StepListC192ThenClear
; @run 71:C181
; WHAT IT DOES: Steps the animation list at $C192; when it ends the object is cleared.
; HOW: StepObjectFrames, a tail jump into ClearObject.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the step.
StepListC192ThenClear:
    lda #$92
    sta z:zTemp10
    lda #$C1
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne WaitThenState2B
    jmp VecClearObject
    rts
; @routine 10CD:334 StartFirstObjectWhenCamera2
; @run 71:C334
; WHAT IT DOES: When the level's Y high byte is 2, the scroll Y low is under $30 and the first object's state and OBJ_2800 are both 0, it sets that object's OBJ_3510 to $40 and OBJ_3568 to $10 and this object goes to the next state.
; HOW: Compares and stores, an INC.
; WAITS: no.
; Entry: X object. Exit: the last compare or load, or the new state's N and Z (A $10).
StartFirstObjectWhenCamera2:
    lda a:ScrollYHiScreen
    cmp #$02
    bne StartFirstObjectWhenCamera2_at0
    lda a:ScrollYLo
    cmp #$30
    bcs StartFirstObjectWhenCamera2_at0
    lda a:OBJ_27A8
    ora a:OBJ_2800
    bne StartFirstObjectWhenCamera2_at0
    lda #$40
    sta a:OBJ_3510
    lda #$10
    sta a:OBJ_3568
    inc a:OBJ_27A8,x
StartFirstObjectWhenCamera2_at0:
    rts
