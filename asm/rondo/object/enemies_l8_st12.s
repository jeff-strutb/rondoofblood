; @routine 025B:0F0 StepVerticalScrollTable
; @run 70:B0F0
; WHAT IT DOES: While OBJ_3568 is not 0, steps through the table at $B129 and $B133: OBJ_3510 takes the entry and the scroll's Y (ScrollYLo+1, ScrollYHi+1) falls by it; OBJ_3568 counts up and wraps to 0 at 9. When it is 0 and the scroll byte ScrollXLo+5 is 0, a random number with its top bit set starts the table (OBJ_3568 1).
; HOW: A table read by OBJ_3568 as the index, a 16-bit subtract, and NextRandom.
; WAITS: no.
; Entry: X object. Exit: the last instruction that ran.
StepVerticalScrollTable:
    lda a:OBJ_3568,x
    beq StepVerticalScrollTable_at0
    tay
    lda a:$B129,y
    sta a:OBJ_3510,x
    lda a:ScrollYLo+1
    sec
    sbc a:$B129,y
    sta a:ScrollYLo+1
    lda a:ScrollYHi+1
    sbc a:$B133,y
    sta a:ScrollYHi+1
    inc a:OBJ_3568,x
    cpy #$09
    bne StepVerticalScrollTable_at1
    stz a:OBJ_3568,x
StepVerticalScrollTable_at1:
    rts
StepVerticalScrollTable_at0:
    lda a:ScrollXLo+5
    bne StepVerticalScrollTable_at2
    jsr VecNextRandom
    asl a
    bcc StepVerticalScrollTable_at2
    inc a:OBJ_3568,x
StepVerticalScrollTable_at2:
    rts
