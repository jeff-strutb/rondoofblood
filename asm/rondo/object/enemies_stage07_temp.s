; TEMPORARY: the stage 07 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
; The disc tables the routines read are named here by their overlay address until the typed data
; layer reads them.
;
;
;
;
;
;
;
; TEMPORARY: the stage 07 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
; The disc tables the routines read are named here by their overlay address until the typed data
; layer reads them.
;
;
;
;
;
;
;
; @routine 0BFB:3DC EnterState4IfFlagged
; @run 70:B3DC
; WHAT IT DOES: Sends the object to state 4 when bit 7 of its status flags is set.
; HOW: Reads the flags once and masks bit 7; writes the state only when it is set.
; WAITS: no.
; Entry: X object. Exit: A the masked bit, or 4 when entered; N, Z from it; X, Y and C kept.
EnterState4IfFlagged:
    lda a:OBJ_31A0,x
    and #$80
    beq EnterState4IfFlagged_at0
    lda #$04
    sta a:OBJ_27A8,x
EnterState4IfFlagged_at0:
    rts

; @routine 0BFC:141 ScrollNotAtRightEnd
; @run 70:B941
; WHAT IT DOES: Says, in the carry, that the scroll has not come to the right end of the level:
; carry set when zField82 is set, when two or more screens remain, or when one screen remains
; and the scroll's low byte is under $E0; clear otherwise.
; HOW: Subtracts the first scroll entry's high screen byte from the scroll's limit
; (ScrollXMaxHi) and tests the difference.
; WAITS: no.
; Entry: nothing. Exit: C the answer; A and N, Z from the last load or compare; V from the
; subtract. Y and X kept. The arms past the screen difference of 0 or 1 ran in no script: the
; draft stops there and liftcmp judges only the first tests; the rest waits for coverage
; (PROCEDURE section 6).
ScrollNotAtRightEnd:
    lda z:zField82
    bne ScrollNotAtRightEnd_set
    lda a:ScrollXMaxHi
    sec
    sbc a:ScrollXHiScreen
    cmp #$02
    bcs ScrollNotAtRightEnd_out
    cmp #$00
    beq ScrollNotAtRightEnd_clear
    lda a:ScrollXLo
    cmp #$E0
    bcs ScrollNotAtRightEnd_clear
ScrollNotAtRightEnd_set:
    sec
ScrollNotAtRightEnd_out:
    rts
ScrollNotAtRightEnd_clear:
    clc
    rts

; @routine 0BFC:2AB SetHitboxSizeFromFrame
; @run 70:BAAB
; WHAT IT DOES: Sets the object's hit rectangle's half width and half height from a two byte
; row of a table by its frame minus $2A; frame 0 clears both.
; HOW: The table is at overlay address $BAC9.
; WAITS: no.
; Entry: X object. Exit: A the last byte loaded; N, Z from it; C bit 7 of the frame minus $2A
; (the shift); V from the subtract; Y the row's offset; X kept.
SetHitboxSizeFromFrame:
    lda a:OBJ_2A68,x
    beq SetHitboxSizeFromFrame_clear
    sec
    sbc #$2A
    asl a
    tay
    lda a:$BAC9,y
    sta a:OBJ_3988,x
    lda a:$BACA,y
    sta a:OBJ_3A38,x
    rts
SetHitboxSizeFromFrame_clear:
    stz a:OBJ_3988,x
    stz a:OBJ_3A38,x
    rts

; @routine 0BFC:3B3 ChangeToType9C
; @run 70:BBB3
; WHAT IT DOES: Turns the object into one of type $9C, in state 0.
; HOW: Stores $9C in its kind byte and zero in its state.
; WAITS: no.
; Entry: X object. Exit: A $9C, N set, Z clear; X, Y and C kept.
ChangeToType9C:
    lda #$9C
    sta a:OBJ_2750,x
ChangeToType9C_at0:
    stz a:OBJ_27A8,x
    rts
