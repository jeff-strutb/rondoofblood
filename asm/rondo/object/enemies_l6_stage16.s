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

; @routine 16CB:27E StepListB2A7ThenPlace
; @run 70:B27E
; WHAT IT DOES: Steps the animation list at $B2A7; while it runs the object moves along Y; when it ends the object is put at X $007C, Y $0026 relative to its screen, gets frame 0, OBJ_34B8 $30 and goes to the next OBJ_2800.
; HOW: StepObjectFrames, a tail jump into ApplyVelocityY, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as ApplyVelocityY, or the new OBJ_2800's N and Z (A $30).
StepListB2A7ThenPlace:
    lda #$A7
    sta z:zTemp10
    lda #$B2
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq StepListB2A7ThenPlace_at0
    jmp VecApplyVelocityY
StepListB2A7ThenPlace_at0:
    lda #$7C
    sta a:OBJ_X_LO,x
    stz a:OBJ_Y_HI,x
    lda #$26
    sta a:OBJ_Y_LO,x
    stz a:OBJ_2A68,x
    lda #$30
    sta a:OBJ_34B8,x
    inc a:OBJ_2800,x
    rts

; @routine 16CB:2F6 StepListB36AThenCount
; @run 70:B2F6
; WHAT IT DOES: Steps the animation list at $B36A and moves the object along Y; counts OBJ_34B8 down and at zero clears OBJ_2FE8, sets OBJ_3510 to $40 and goes on to the next OBJ_2800.
; HOW: StepObjectFrames, ApplyVelocityY, DEC, stores, INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the count, or the new OBJ_2800's N and Z (A $40).
StepListB36AThenCount:
    lda #$6A
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocityY
    dec a:OBJ_34B8,x
    bne StepListB36AThenCount_at0
    stz a:OBJ_2FE8,x
    lda #$40
    sta a:OBJ_3510,x
    inc a:OBJ_2800,x
StepListB36AThenCount_at0:
    rts

; @routine 16CB:3C8 CountThenJumpRight
; @run 70:B3C8
; WHAT IT DOES: Sets the object's frame to 1 and counts OBJ_34B8 down; at zero it gives the object an X speed high of $12 and a Y speed high of $E0, moves it once, clears its animation step and timer, sets frame 3 and goes on to the next OBJ_2800.
; HOW: DEC, stores, ApplyVelocity, INC.
; WAITS: no.
; Entry: X object. Exit: the count, or the new OBJ_2800's N and Z (A 3).
CountThenJumpRight:
    lda #$01
    sta a:OBJ_2A68,x
    dec a:OBJ_34B8,x
    bne StepFramesFacingPlayer
    lda #$12
    sta a:OBJ_VX_HI,x
    lda #$E0
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$03
    sta a:OBJ_2A68,x
    inc a:OBJ_2800,x
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
; @routine 16CB:54F StartKindFromTableAtSlot16
; @run 70:B54F
; WHAT IT DOES: Takes a kind from the table at $B56B by OBJ_3618, clears the object in slot $16, starts an object of that kind there at this object's place, counts OBJ_3618 up, sets OBJ_34B8 to $40 and goes on to the next OBJ_2800.
; HOW: A table read (bank 70), ClearObjectInY, StartObjectAtPlace, two INCs.
; WAITS: no.
; Entry: X object. Exit: A $40, Y $16, N and Z from the new OBJ_2800.
StartKindFromTableAtSlot16:
    ldy a:OBJ_3618,x
    lda a:$B56B,y
    pha
    ldy #$16
    jsr VecClearObjectInY
    pla
    jsr VecStartObjectAtPlace
    inc a:OBJ_3618,x
    lda #$40
    sta a:OBJ_34B8,x
    inc a:OBJ_2800,x
    rts
