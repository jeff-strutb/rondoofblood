; @routine 16CC:436 StepListBC5CThenAimAtPlayer
; @run 70:BC36
; WHAT IT DOES: Steps the animation list at $BC5C; when it ends the object sets OBJ_3250 to 2, takes the angle to the player as OBJ_3568, sets OBJ_3510 to $80 and OBJ_34B8 to 0 and goes on to the next state.
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
    sta a:OBJ_3250,x
    jsr VecAngleToPlayer
    sta a:OBJ_3568,x
    lda #$80
    sta a:OBJ_3510,x
    lda #$00
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8,x
StepListBC5CThenAimAtPlayer_at0:
    rts
