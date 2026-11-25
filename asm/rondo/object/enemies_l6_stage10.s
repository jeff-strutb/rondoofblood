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
