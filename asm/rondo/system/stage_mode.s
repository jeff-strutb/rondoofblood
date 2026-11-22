; @routine 024E:023 StageWaitTimerStep
; @run 6B:8823
; WHAT IT DOES: Counts a wait down and, when it reaches zero, moves the stage on one step.
; HOW: zWaitTimer3 down by one; when that gives zero, zField80 up by one.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the byte last changed. A, X, Y, C and V kept.
StageWaitTimerStep:
    dec z:zWaitTimer3
    beq StageWaitTimerStep_at0
    rts
StageWaitTimerStep_at0:
    inc z:zField80
    rts

; @routine 050E:019 StageWaitTimerToStep2
; @run 6B:8819
; WHAT IT DOES: Counts a wait down and, when it reaches zero, sets the stage's step to 2.
; HOW: zWaitTimer3 down by one; when that gives zero, zField80 is set to 2.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the byte last handled (the count, or the 2 stored). A is 2
; when the count reached zero, else kept. X, Y, C and V kept.
StageWaitTimerToStep2:
    dec z:zWaitTimer3
    beq StageWaitTimerToStep2_at0
    rts
StageWaitTimerToStep2_at0:
    lda #$02
    sta z:zField80
    rts
