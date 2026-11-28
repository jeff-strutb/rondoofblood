; @routine 024E:6ED StepStageSequence
; @run 6B:8EED
; WHAT IT DOES: Steps the stage's opening sequence one count: at the last count ($1A) it
; starts again with the blend flag toggled; on an even count it starts the count's two script
; slots, sets a sprite frame and starts the count's list; at count $11 it starts the score
; blends.
; HOW: The count is StageVar350F (the second slot of the blend column). The tables of
; scripts, frames and lists are at $8F64, $8F71, $8F7E and $8F8B, a byte each, by half the count.
; An odd count other than $11 does nothing.
; Every stage overlay holds the same code here.
; WAITS: no.
; Entry: X and Y as the blends take them. Exit: as the last step left it.
StepStageSequenceValues = $8F64 ; table StepStageSequence reads
StepStageSequenceTables = $8F71 ; table StepStageSequence reads
StepStageSequenceTables2 = $8F7E ; table StepStageSequence reads
StepStageSequenceValues2 = $8F8B ; table StepStageSequence reads
StepStageSequence:
    lda a:OBJ_FALL_DRIFT_DELAY+86+1
    sta z:zTemp00
    cmp #$1A
    bne StepStageSequence_at0
    rmb1 z:zPaused
    stz a:OBJ_FALL_DRIFT_DELAY+86+1
    lda a:OBJ_PAD_FORCED+86+1
    eor #$01
    sta a:OBJ_PAD_FORCED+86+1
    beq StepStageSequence_at1
    rmb0 z:zPaused
StepStageSequence_at1:
    rts
StepStageSequence_at0:
    inc a:OBJ_FALL_DRIFT_DELAY+86+1
    bbs0 zTemp00,StepStageSequence_at2
    lsr z:zTemp00
    ldx z:zTemp00
    lda a:StepStageSequenceValues,x
    ldx #$02
    jsr VecStartScriptSlotKind4
    ldx z:zTemp00
    lda a:StepStageSequenceTables,x
    ldx #$0B
    jsr VecStartScriptSlotKind4
    ldx z:zTemp00
    lda a:StepStageSequenceTables2,x
    sta a:OBJ_FRAME+87
    ldx z:zTemp00
    lda a:StepStageSequenceValues2,x
    beq StepStageSequence_at1
    bmi StepStageSequence_at3
    ldx #$02
    jmp VecStartListChannelKind2
StepStageSequence_at3:
    lda #$64
    clc
    adc a:OBJ_PAD_FORCED+86+1
    ldx #$02
    jmp VecStartListChannelKind2
StepStageSequence_at2:
    lda z:zTemp00
    cmp #$11
    bne StepStageSequence_at1
    lda a:OBJ_PAD_FORCED+86+1
    bne StepStageSequence_at4
    lda #$01
    sta a:StageBlendOn+1
    jsr ClearStageStateBytes
    jmp BlendStageScoreRows
StepStageSequence_at4:
    stz a:StageBlendOn+1
    jsr BlendStageStockRows
    jmp BlendStageScoreAddRows
