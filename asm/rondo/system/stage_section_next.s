; @routine 07CD:66A StageSectionNext07CD
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (stage 04): zField82 counted on and zField83
; set from this overlay's section order ($9CD6) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext07CDOrder = $9CD6 ; the section record by the step through the stage
StageSectionNext07CD:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext07CDOrder,y
    sta z:zField83
    rts

; @routine 092D:66A StageSectionNext092D
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (stage 05): zField82 counted on and zField83
; set from this overlay's section order ($9CD1) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext092DOrder = $9CD1 ; the section record by the step through the stage
StageSectionNext092D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext092DOrder,y
    sta z:zField83
    rts

