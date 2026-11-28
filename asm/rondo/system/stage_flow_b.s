; @routine 05BD:66A StageSectionNext05BD
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (Secret stage 02): zField82 counted on and zField83
; set from this overlay's section order ($9E77) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext05BDOrder = $9E77 ; the section record by the step through the stage
StageSectionNext05BD:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext05BDOrder,y
    sta z:zField83
    rts

; @routine 071D:66A StageSectionNext071D
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (Secret stage 03): zField82 counted on and zField83
; set from this overlay's section order ($9F3B) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext071DOrder = $9F3B ; the section record by the step through the stage
StageSectionNext071D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext071DOrder,y
    sta z:zField83
    rts

; @routine 087D:66A StageSectionNext087D
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (Secret stage 04): zField82 counted on and zField83
; set from this overlay's section order ($9E64) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext087DOrder = $9E64 ; the section record by the step through the stage
StageSectionNext087D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext087DOrder,y
    sta z:zField83
    rts

; @routine 09DD:66A StageSectionNext09DD
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (Secret stage 05): zField82 counted on and zField83
; set from this overlay's section order ($9EAB) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext09DDOrder = $9EAB ; the section record by the step through the stage
StageSectionNext09DD:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext09DDOrder,y
    sta z:zField83
    rts

; @routine 0B3D:66A StageSectionNext0B3D
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (Secret stage 06): zField82 counted on and zField83
; set from this overlay's section order ($9E76) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext0B3DOrder = $9E76 ; the section record by the step through the stage
StageSectionNext0B3D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext0B3DOrder,y
    sta z:zField83
    rts

; @routine 0C9D:66A StageSectionNext0C9D
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (Secret stage 07): zField82 counted on and zField83
; set from this overlay's section order ($9E79) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext0C9DOrder = $9E79 ; the section record by the step through the stage
StageSectionNext0C9D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext0C9DOrder,y
    sta z:zField83
    rts

; @routine 0DFD:66A StageSectionNext0DFD
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (Secret stage 08): zField82 counted on and zField83
; set from this overlay's section order ($9EA7) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext0DFDOrder = $9EA7 ; the section record by the step through the stage
StageSectionNext0DFD:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext0DFDOrder,y
    sta z:zField83
    rts

; @routine 0EAD:66A StageSectionNext0EAD
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (Stage 09): zField82 counted on and zField83
; set from this overlay's section order ($9DF6) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext0EADOrder = $9DF6 ; the section record by the step through the stage
StageSectionNext0EAD:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext0EADOrder,y
    sta z:zField83
    rts

; @routine 0F5D:66A StageSectionNext0F5D
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (Secret stage 09): zField82 counted on and zField83
; set from this overlay's section order ($9F1C) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext0F5DOrder = $9F1C ; the section record by the step through the stage
StageSectionNext0F5D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext0F5DOrder,y
    sta z:zField83
    rts

; @routine 100D:66A StageSectionNext100D
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (Stage 0A): zField82 counted on and zField83
; set from this overlay's section order ($9C75) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext100DOrder = $9C75 ; the section record by the step through the stage
StageSectionNext100D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext100DOrder,y
    sta z:zField83
    rts

; @routine 10BD:66A StageSectionNext10BD
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (Secret stage 0A): zField82 counted on and zField83
; set from this overlay's section order ($9F06) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext10BDOrder = $9F06 ; the section record by the step through the stage
StageSectionNext10BD:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext10BDOrder,y
    sta z:zField83
    rts

; @routine 116D:66A StageSectionNext116D
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (Stage 0B): zField82 counted on and zField83
; set from this overlay's section order ($9C77) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext116DOrder = $9C77 ; the section record by the step through the stage
StageSectionNext116D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext116DOrder,y
    sta z:zField83
    rts

; @routine 12CD:66A StageSectionNext12CD
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (Stage 0C): zField82 counted on and zField83
; set from this overlay's section order ($9F35) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext12CDOrder = $9F35 ; the section record by the step through the stage
StageSectionNext12CD:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext12CDOrder,y
    sta z:zField83
    rts

; @routine 160D:66A StageSectionNext160D
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (Stage 0E): zField82 counted on and zField83
; set from this overlay's section order ($9CA5) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext160DOrder = $9CA5 ; the section record by the step through the stage
StageSectionNext160D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext160DOrder,y
    sta z:zField83
    rts

; @routine 16BD:66A StageSectionNext16BD
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (Secret stage 0E): zField82 counted on and zField83
; set from this overlay's section order ($9F42) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext16BDOrder = $9F42 ; the section record by the step through the stage
StageSectionNext16BD:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext16BDOrder,y
    sta z:zField83
    rts

; @routine 18CD:66A StageSectionNext18CD
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section (Stage 10): zField82 counted on and zField83
; set from this overlay's section order ($9EA3) by it.
; HOW: INC zField82, read again as the index, one table read, one store (StageSectionNext's body).
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext18CDOrder = $9EA3 ; the section record by the step through the stage
StageSectionNext18CD:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageSectionNext18CDOrder,y
    sta z:zField83
    rts
