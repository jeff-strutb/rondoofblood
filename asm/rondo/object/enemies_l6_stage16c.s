; @routine 16CC:40C LoadThenRandomPlace
; @run 70:BC0C
; WHAT IT DOES: Loads the object's record and puts it at a random place relative to its screen: X from a random $00 to $7F plus a random $00 to $3F plus $20, Y from a random $00 to $3F plus $10 (high bytes 0).
; HOW: LoadObjectRecord, NextRandom three times, adds.
; WAITS: no.
; Entry: X object. Exit: A the Y low, N, Z, C and V from the last add.
LoadThenRandomPlace:
    jsr VecLoadObjectRecord
    stz a:OBJ_X_HI,x
    jsr VecNextRandom
    and #$7F
    sta z:zTemp00
    jsr VecNextRandom
    and #$3F
    clc
    adc z:zTemp00
    clc
    adc #$20
    sta a:OBJ_X_LO,x
    stz a:OBJ_Y_HI,x
    jsr VecNextRandom
    and #$3F
    clc
    adc #$10
    sta a:OBJ_Y_LO,x
    rts

; @routine 16CC:436 StepListBC5CThenAimAtPlayer
; @run 70:BC36
; WHAT IT DOES: Steps the animation list at $BC5C; when it ends the object sets OBJ_HITBOX_ON to 2, takes the angle to the player as OBJ_WORK_F, sets OBJ_WORK_E to $80 and OBJ_WORK_D to 0 and goes on to the next state.
; HOW: StepObjectFrames, AngleToPlayer, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step, or the new state's N and Z (A 0) when the list ended.
StepListBC5CThenAimAtPlayer:
    lda #$5C
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListBC5CThenAimAtPlayer_at0
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    jsr VecAngleToPlayer
    sta a:OBJ_WORK_F,x
    lda #$80
    sta a:OBJ_WORK_E,x
    lda #$00
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
StepListBC5CThenAimAtPlayer_at0:
    rts
