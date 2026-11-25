; @routine 16CB:20E LoadThenStartRise
; @run 70:B20E
; WHAT IT DOES: Loads the object's record, sets its flip byte (OBJ_2FE8) to $80, its Y speed to $FE80 and OBJ_3720 to $E0.
; HOW: LoadObjectRecord, stores.
; WAITS: no.
; Entry: X object. Exit: A $E0, N set, Z clear; C and Y from the record.
LoadThenStartRise:
    jsr VecLoadObjectRecord
    lda #$80
    sta a:OBJ_2FE8,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda #$E0
    sta a:OBJ_3720,x
    rts

; @routine 16CB:24C ClearSlotsWhenScrollZero
; @run 70:B24C
; WHAT IT DOES: When the level's X position high byte and the scroll X low are both zero, clears the objects in slots $11 to $4E, sets OBJ_34B8 to $60 and goes on to the next OBJ_2800.
; HOW: Two reads, a loop of ClearObjectInY, a store, an INC.
; WAITS: no.
; Entry: X object. Exit: the OR, or the new OBJ_2800's N and Z.
ClearSlotsWhenScrollZero:
    lda a:ScrollXHiScreen
    ora a:ScrollXLo
    bne ClearSlotsWhenScrollZero_at0
    ldy #$11
ClearSlotsWhenScrollZero_at1:
    jsr VecClearObjectInY
    iny
    cpy #$4F
    bcc ClearSlotsWhenScrollZero_at1
    lda #$60
    sta a:OBJ_34B8,x
    inc a:OBJ_2800,x
ClearSlotsWhenScrollZero_at0:
    rts

; @routine 16CB:3EE StepListB404ThenTimer
; @run 70:B3EE
; WHAT IT DOES: Steps the animation list at $B404; when it ends, OBJ_34B8 becomes $48 and the object goes to the next OBJ_2800.
; HOW: StepObjectFrames, a store, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step, or the new OBJ_2800's N and Z (A $48) when the list ended.
StepListB404ThenTimer:
    lda #$04
    sta z:zTemp10
    lda #$B4
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListB404ThenTimer_at0
    lda #$48
    sta a:OBJ_34B8,x
    inc a:OBJ_2800,x
StepListB404ThenTimer_at0:
    rts
