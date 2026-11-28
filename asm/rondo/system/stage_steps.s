; @routine 024D:48D StageWalkOutScroll
; @run 6B:848D
; WHAT IT DOES: A frame of walking the player out of a cleared room: the scroll stepped by the route's direction, the
; player moved with it, the camera and objects run; at the route's stop the scroll halts and the next step comes.
; HOW: (zField8C - 1) AND 1 picks the route; WalkScrollStepByRoute into entry 0's X step, WalkPlayerStepByRoute added
; to the player's X; StageCameraStep and StepScrollAndRunObjects; the X compared with WalkStopXByRoute.
; WAITS: as the calls.
; Entry: nothing. Exit: walking: A the player's X, N, Z and C from the compare, Y the route; stopped: N and Z from
; zField85's INC.
StageWalkOutScroll:
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:WalkScrollStepByRoute,y
    sta a:ScrollStepXHi
    stz a:ScrollStepXLo
    stz a:ScrollXFrac
    lda a:OBJ_X_LO
    clc
    adc a:WalkPlayerStepByRoute,y
    sta a:OBJ_X_LO
    jsr StageCameraStep
    jsr StepScrollAndRunObjects
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:OBJ_X_LO
    cmp a:WalkStopXByRoute,y
    beq StageWalkOutScroll_at0
    rts
StageWalkOutScroll_at0:
    jsr StageScrollSpeedsOff
    stz a:ScrollStepXHi
    inc z:zField85
    rts

; @routine 024D:50A StageShowAndStep
; @run 6B:850A
; WHAT IT DOES: The player placed anew with his start frame, the next step.
; HOW: The player's $A003, StageShowStartFrame, INC zField85.
; WAITS: no.
; Entry: nothing. Exit: N and Z from zField85's INC; the rest as StageShowStartFrame left them.
StageShowAndStep:
    jsr MariaVecStart
    jsr StageShowStartFrame
    inc z:zField85
    rts

; @routine 024D:68C StageStopAndStep
; @run 6B:868C
; WHAT IT DOES: The scroll stopped, a frame of the objects, the next step.
; HOW: The $80C5 and $8A4F calls, INC zField85.
; WAITS: as the calls.
; Entry: nothing. Exit: N and Z from zField85's INC; the rest as the objects left them.
StageStopAndStep:
    jsr StageScrollSpeedsOff
    jsr StepScrollAndRunObjects
    inc z:zField85
    rts

; @routine 024D:720 StageLeaveByRoute
; @run 6B:8720
; WHAT IT DOES: Keeps the route in zField8C, ends the mode, holds the display two frames and hides the player.
; HOW: Stores and an INC.
; WAITS: no.
; Entry: A the route. Exit: A 2, N 0, Z 0. X, Y, C, V kept.
StageLeaveByRoute:
    sta z:zField8C
    inc z:zModeDone
    lda #$02
    sta z:zDisplayDelay
    stz a:OBJ_FRAME
    rts

; @routine 024D:7F3 StagePauseCdThenStock
; @run 6B:87F3
; WHAT IT DOES: Pauses the CD, retrying while the card reports an error, then takes a life (StageStockTake), or with
; none left sets zModeDone $80.
; HOW: JSR cd_pause until A is zero; LDA zStock, BNE to StageStockTake.
; WAITS: yes, for the card's cd_pause; the stack there is the caller's and the call's two bytes.
; Entry: X as the card takes it. Exit: with stock as StageStockTake; else A $80, N 1, Z 0.
StagePauseCdThenStock:
    jsr BiosCdPause
    tax
    bne StagePauseCdThenStock
    lda z:zStock
    bne StageStockTake
    lda #$80
    sta z:zModeDone
    rts

; @routine 024E:002 StageStockCheck
; @run 6B:8802
; WHAT IT DOES: With no stock left goes straight to the wait; else step 2.
; HOW: LDA zStock, BEQ to StageStockWait.
; WAITS: no.
; Entry: nothing. Exit: stock 0: as StageStockWait; else A 2, N 0, Z 0.
StageStockCheck:
    lda z:zStock
    beq StageStockWait
    lda #$02
    sta z:zField80
    rts

; @routine 024E:00B StageStockTake
; @run 6B:880B
; @stop 8812
; WHAT IT DOES: Takes one from the stock, in decimal, then StageStockWait.
; HOW: SED, SEC, SBC #1, CLD; running into StageStockWait.
; WAITS: no.
; Entry: A the stock. Exit: as StageStockWait, C 1 when the stock was not zero.
StageStockTake:
    sed
    sec
    sbc #$01
    cld
    sta z:zStock

; @routine 024E:012 StageStockWait
; @run 6B:8812
; WHAT IT DOES: Starts the wait after a death: $60 frames in zWaitTimer3, the next step.
; HOW: A store and an INC of zField80.
; WAITS: no.
; Entry: nothing. Exit: A $60, N and Z from zField80's INC. X, Y, C, V kept.
StageStockWait:
    lda #$60
    sta z:zWaitTimer3
    inc z:zField80
    rts

; @routine 024E:019 StageWaitThenStepTwo
; @run 6B:8819
; WHAT IT DOES: Counts zWaitTimer3 down; when it runs out, step 2.
; HOW: DEC, BEQ.
; WAITS: no.
; Entry: nothing. Exit: counting: N and Z from the DEC; else A 2, N 0, Z 0. X, Y, C, V kept.
StageWaitThenStepTwo:
    dec z:zWaitTimer3
    beq StageWaitThenStepTwo_at0
    rts
StageWaitThenStepTwo_at0:
    lda #$02
    sta z:zField80
    rts

