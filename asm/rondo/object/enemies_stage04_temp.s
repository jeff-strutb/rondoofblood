; TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
;
;
;
;
;
;
;
; TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
;
;
;
;
;
;
;
; @routine 07DB:5FD ClampSpeedXToTwo
; @run 70:B5FD
; WHAT IT DOES: Keeps an object's X speed from passing 2 pixels a frame either way: a speed over
; +2 becomes +2.00 and one under -2 becomes -2.00.
; HOW: Reads the speed's high byte (twice, as the sign test and the compare each load it) and,
; past the limit, stores the limit in the high byte and zero in the low byte.
; WAITS: no.
; Entry: X object. Exit: A and N, Z, C from the last compare or load (C the compare's, also when the
; speed is clamped); X and Y kept.
ClampSpeedXToTwo:
    lda a:OBJ_VX_HI,x
    bmi ClampSpeedXToTwo_minus
    lda a:OBJ_VX_HI,x
    cmp #$02
    bcc ClampSpeedXToTwo_out
    lda #$02
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
ClampSpeedXToTwo_out:
    rts
ClampSpeedXToTwo_minus:
    lda a:OBJ_VX_HI,x
    cmp #$FE
    bcs ClampSpeedXToTwo_out
    lda #$FE
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    rts

; @routine 07DB:622 ThrowUpward
; @run 70:B622
; WHAT IT DOES: Throws an object upward at 3 pixels a frame, unless it is already rising slowly
; (its Y speed between -2.00 and 0 counted as a high byte of $FE or $FF).
; HOW: Reads the Y speed's high byte; below $FE it stores $FD there and zero in the low byte.
; WAITS: no.
; Entry: X object. Exit: A and N, Z, C from the last compare or load; X and Y kept.
ThrowUpward:
    lda a:OBJ_VY_HI,x
    cmp #$FE
ThrowUpward_at0:
    bcs ThrowUpward_at1
    lda #$FD
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
ThrowUpward_at1:
    rts

; @routine 07DB:71B CountdownThenState4
; @run 70:B71B
; WHAT IT DOES: Counts the object's f3510 timer down and, when it reaches zero, sends the
; object to state 4.
; HOW: One decrement of the timer; the state byte is written only at zero.
; WAITS: no.
; Entry: X object. Exit: A and N, Z from the timer, or A 4 at zero; X, Y and C kept.
CountdownThenState4:
    dec a:OBJ_3510,x
    bne CountdownThenState4_at1
CountdownThenState4_at0:
    lda #$04
    sta a:OBJ_27A8,x
CountdownThenState4_at1:
    rts

; @routine 07DB:785 ChangeToType9B
; @run 70:B785
; WHAT IT DOES: Turns the object into one of type $9B, in state 0.
; HOW: Stores $9B in its type byte (OBJ_2750) and zero in its state.
; WAITS: no.
; Entry: X object. Exit: A $9B, N set, Z clear; X, Y and C kept.
ChangeToType9B:
    lda #$9B
    sta a:OBJ_2750,x
    stz a:OBJ_27A8,x
    rts

; @routine 07DB:7B1 EnterState6IfFlagged
; @run 70:B7B1
; WHAT IT DOES: Sends the object to state 6 when bit 7 of its f31a0 is set.
; HOW: Reads f31a0 once and masks bit 7; writes the state only when it is set.
; WAITS: no.
; Entry: X object. Exit: A the masked bit, or 6 when entered; N, Z from it; X, Y and C kept.
EnterState6IfFlagged:
    lda a:OBJ_31A0,x
    and #$80
    beq EnterState6IfFlagged_at0
    lda #$06
    sta a:OBJ_27A8,x
EnterState6IfFlagged_at0:
    rts

; @routine 07DC:09D CountdownStopThenState4
; @run 70:B89D
; WHAT IT DOES: Counts the object's f34b8 timer down and, when it reaches zero, clears its f3408
; and f3460 and sends it to state 4.
; HOW: One decrement of the timer; the rest only at zero.
; WAITS: no.
; Entry: X object. Exit: A and N, Z from the timer, or A 4 at zero; X, Y and C kept.
CountdownStopThenState4:
    dec a:OBJ_34B8,x
    bne CountdownStopThenState4_at0
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$04
    sta a:OBJ_27A8,x
CountdownStopThenState4_at0:
    rts

; @routine 07DF:349 RightOfObject4DWithin
; @run 71:D349
; WHAT IT DOES: Says, in the carry, whether the object stands to the right of the object in slot
; $4D by less than the distance in A, when slot $4D holds an object of type $AC.
; HOW: Stores A as the limit in zTemp00; checks slot $4D's type; subtracts the two X positions
; (low bytes into zTemp01, then high bytes); the answer is yes only when the high bytes are
; equal and the low difference is under the limit.
; WAITS: no.
; Entry: X object, A limit. Exit: C set for yes; A the last value compared; N, Z from it. Clobbers
; zTemp00-01. The arms after the type check ran in no script, so the draft stops there and
; liftcmp judges only the type check; the rest waits for coverage (PROCEDURE section 6).
RightOfObject4DWithin:
    sta z:zTemp00
    lda a:OBJ_2750+$4D
    cmp #$AC
    bne RightOfObject4DWithin_at0
    lda a:OBJ_X_LO,x
    sec
    sbc a:OBJ_X_LO+$4D
    sta z:zTemp01
    lda a:OBJ_X_HI,x
    sbc a:OBJ_X_HI+$4D
    bne RightOfObject4DWithin_at0
    lda z:zTemp01
    cmp z:zTemp00
    bcs RightOfObject4DWithin_at0
    sec
    rts
RightOfObject4DWithin_at0:
    clc
    rts
