; @routine 024F:19F ClearStageStateBytes
; @run 6B:919F
; WHAT IT DOES: Zeroes four of the stage's state bytes, the ones a stage restart or its
; start-of-stage handler clears together.
; HOW: Four stores of zero, each into slot $56 of one column of the object table (columns
; $34B8, $3510, $3568 and $3670). The columns are the object table's, the use is the stage
; overlay's own; the field names are the table's until the typed layer names them.
; WAITS: no.
; Entry: nothing. Exit: A, X, Y and flags kept.
ClearStageStateBytes:
    stz a:OBJ_34B8+$56
    stz a:OBJ_3510+$56
    stz a:OBJ_3568+$56
    stz a:OBJ_3670+$56
    rts
