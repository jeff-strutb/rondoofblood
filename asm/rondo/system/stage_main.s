; @routine 024E:051 RunStageFrame
; @run 6B:8851
; WHAT IT DOES: One frame of a stage: counts the wave, ticks the stage (StepStageFrame), runs the
; player and the camera and the objects, and steps the ending effect; when the ending is done it
; puts the game in mode 1 and sets the object timer by the stage.
; HOW: Calls in turn StepStageFrame, the overlay's two common pieces at $80AB and $80BF (original
; code), PlayerAndCameraFrame, StepScrollAndRunObjects and StepStageEnding.
; Every stage overlay holds the same code here.
; WAITS: it can wait where a handler's code does.
; Entry: X and Y as the tick takes them. Exit: as the last piece left it.
RunStageFrame:
    inc z:zWaveCounter
    jsr StepStageFrame
    jsr $80AB
    jsr VecPlayerAndCameraFrame
    jsr $80BF
    jsr StepScrollAndRunObjects
    jsr StepStageEnding
    bbr0 zField8E,RunStageFrame_at0
    stz a:PlayerByte3BD8
    lda #$01
    sta z:zGameSubMode
    lda z:zStage
    bne RunStageFrame_at1
    lda #$10
    sta a:OBJ_3510
    rts
RunStageFrame_at1:
    lda #$01
    sta a:OBJ_3510
    stz a:PlayerByte3BF0
RunStageFrame_at0:
    rts
