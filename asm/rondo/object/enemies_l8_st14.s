; @routine 025D:040 LoadRecordFlashByNumberThenMoveOrClear
; @run 71:C040
; WHAT IT DOES: In state 0 loads the record, gives it the speed X -$0400 and Y -$0180 (OBJ_VY_LO $80) and the flash flags from the table at $C061 by OBJ_34B8. Later it steps the animation list at $C076 and moves, or is cleared when the list ends.
; HOW: LoadObjectRecord, stores, StepObjectFrames, tail jumps into ClearObject or ApplyVelocity.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
LoadRecordFlashByNumberThenMoveOrClear:
    lda a:OBJ_27A8,x
    bne LoadRecordFlashByNumberThenMoveOrClear_at0
    jsr VecLoadObjectRecord
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda #$FC
    sta a:OBJ_VX_HI,x
    ldy a:OBJ_34B8,x
    lda a:$C061,y
    sta a:OBJ_2B18,x
    rts
    smb0 z:zTemp00
LoadRecordFlashByNumberThenMoveOrClear_at0:
    lda #$76
    sta z:zTemp10
    lda #$C0
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne LoadRecordFlashByNumberThenMoveOrClear_at1
    jmp VecClearObject
LoadRecordFlashByNumberThenMoveOrClear_at1:
    jmp VecApplyVelocity

; @routine 025D:0C0 LoadRecordRandomLiftThenMoveOrClear
; @run 71:C0C0
; WHAT IT DOES: In state 0 loads the record, gives it a random Y speed from -$10 to $0F and the X speed $18, moves it once and sets the speed from the angle $68 with the radius $0100. Later it steps the animation list at $C0FC and moves, or is cleared when the list ends.
; HOW: LoadObjectRecord, NextRandom, ApplyVelocity, SetVelocityFromAngle, StepObjectFrames, tail jumps into ClearObject or ApplyVelocity.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
LoadRecordRandomLiftThenMoveOrClear:
    lda a:OBJ_27A8,x
    bne LoadRecordRandomLiftThenMoveOrClear_at0
    jsr VecLoadObjectRecord
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$10
    sta a:OBJ_VY_HI,x
    lda #$18
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocity
    lda #$00
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda #$68
    jsr VecSetVelocityFromAngle
    rts
LoadRecordRandomLiftThenMoveOrClear_at0:
    lda #$FC
    sta z:zTemp10
    lda #$C0
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne LoadRecordRandomLiftThenMoveOrClear_at1
    jmp VecClearObject
LoadRecordRandomLiftThenMoveOrClear_at1:
    jmp VecApplyVelocity
