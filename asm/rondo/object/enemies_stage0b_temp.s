; TEMPORARY: the stage 0B overlay's object routines whose enemy kind is not yet identified, kept
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
; TEMPORARY: the stage 0B overlay's object routines whose enemy kind is not yet identified, kept
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
; @routine 117C:0EA CountdownThenState4B
; @run 70:B8EA
; WHAT IT DOES: The same as CountdownThenState4, at this overlay's copy of it.
; HOW: The shared body; the f3510 timer is counted down and at zero the object enters state 4.
; WAITS: no.
; Entry, exit: as CountdownThenState4.
CountdownThenState4B:
    dec a:OBJ_3510,x
    bne CountdownThenState4B_at0
    lda #$04
    sta a:OBJ_27A8,x
CountdownThenState4B_at0:
    rts

; @routine 117C:154 ChangeToType9BB
; @run 70:B954
; WHAT IT DOES: The same as ChangeToType9B, at this overlay's copy of it.
; HOW: The shared body.
; WAITS: no.
; Entry, exit: as ChangeToType9B.
ChangeToType9BB:
    lda #$9B
    sta a:OBJ_2750,x
    stz a:OBJ_27A8,x
ChangeToType9BB_at0:
    rts

; @routine 117C:1C7 SixteenBitTimerToState8
; @run 70:B9C7
; WHAT IT DOES: While the object is in state 5, counts its sixteen bit timer (f35c0 low, f3568
; high) down by one a frame; when the whole timer reaches zero the object enters state 8.
; HOW: A subtract of one from the low byte with the borrow carried into the high byte; the two
; are then ORed (the low byte read again) to test for zero.
; WAITS: no.
; Entry: X object. Exit: A and N, Z from the last compare, OR or load; C and V from the high
; byte's subtract when it ran; X and Y kept.
SixteenBitTimerToState8:
    lda a:OBJ_27A8,x
    cmp #$05
    bne SixteenBitTimerToState8_at0
    lda a:OBJ_35C0,x
    sec
    sbc #$01
    sta a:OBJ_35C0,x
    lda a:OBJ_3568,x
    sbc #$00
    sta a:OBJ_3568,x
    ora a:OBJ_35C0,x
    bne SixteenBitTimerToState8_at0
    lda #$08
    sta a:OBJ_27A8,x
SixteenBitTimerToState8_at0:
    rts

; @routine 117C:46E NoOpStateA
; @run 70:BC6E
; WHAT IT DOES: A state handler that does nothing.
; HOW: Returns at once.
; WAITS: no.
; Entry: nothing. Exit: everything kept.
NoOpStateA:
    rts

; @routine 117C:4ED SetXSpeedFromFacing
; @run 70:BCED
; WHAT IT DOES: Sets the object's X speed from a two byte entry of a table by which way it faces
; (the facing byte times two).
; HOW: The table is at overlay address $BD0F: the low byte first, then the high byte.
; WAITS: no.
; Entry: X object. Exit: A the low speed byte; N, Z from it; C bit 7 of the facing byte; Y the
; entry's offset; X kept.
SetXSpeedFromFacing:
    lda a:OBJ_2AC0,x
    asl a
    tay
    lda a:$BD10,y
    sta a:OBJ_VX_HI,x
    lda a:$BD0F,y
    sta a:OBJ_VX_LO,x
    rts

; @routine 117C:69D EnterState6WhenHitHard
; @run 70:BE9D
; WHAT IT DOES: Sends the object to state 6, step 0, when its f3670 is set, its state is under 6
; and the hit it has taken is 5 or more.
; HOW: The three tests in that order, each stopping at the first that fails. The last arm ran in
; no script: the draft stops there; it waits for coverage (PROCEDURE section 6).
; WAITS: no.
; Entry: X object. Exit: A and N, Z, C from the last load or compare; X and Y kept.
EnterState6WhenHitHard:
    lda a:OBJ_3670,x
    beq EnterState6WhenHitHard_at0
    lda a:OBJ_27A8,x
    cmp #$06
    bcs EnterState6WhenHitHard_at0
    lda a:OBJ_33B0,x
    cmp #$05
    bcc EnterState6WhenHitHard_at0
    lda #$06
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
EnterState6WhenHitHard_at0:
    rts

; @routine 117C:6B9 EnterState7IfYHighOrFlagged
; @run 70:BEB9
; WHAT IT DOES: Sends the object to state 7 when its Y position's high byte is not zero or bit 7
; of its status flags is set.
; HOW: Tests the high byte, then the flag. The arm that enters the state ran in no script: the
; draft stops there; it waits for coverage (PROCEDURE section 6).
; WAITS: no.
; Entry: X object. Exit: A the last value loaded; N, Z from it; X, Y and C kept.
EnterState7IfYHighOrFlagged:
    lda a:OBJ_Y_HI,x
    bne EnterState7IfYHighOrFlagged_at0
    lda a:OBJ_31A0,x
    and #$80
    beq EnterState7IfYHighOrFlagged_at1
EnterState7IfYHighOrFlagged_at0:
    lda #$07
    sta a:OBJ_27A8,x
EnterState7IfYHighOrFlagged_at1:
    rts

; @routine 117C:7C5 NoOpStateB
; @run 70:BFC5
; WHAT IT DOES: A state handler that does nothing.
; HOW: Returns at once.
; WAITS: no.
; Entry: nothing. Exit: everything kept.
NoOpStateB:
    rts

; @routine 117C:7FC LaunchHopByFacing
; @run 70:BFFC
; WHAT IT DOES: Launches the object in a hop like LaunchHopByF3510, with frame $38, this
; overlay's table of X speeds at $C01E, indexed by which way the object faces.
; HOW: The shared hop body.
; WAITS: no.
; Entry: X object. Exit: A 8; N, Z from the step byte's new value; Y the facing byte; X and C
; kept.
LaunchHopByFacing:
    ldy a:OBJ_2AC0,x
    lda a:$C01E,y
    sta a:OBJ_VX_HI,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_LO,x
    lda #$38
    sta a:OBJ_2A68,x
    lda #$08
    sta a:OBJ_35C0,x
    inc a:OBJ_2800,x
    rts

; @routine 117D:041 CountdownStopThenState2
; @run 71:C041
; WHAT IT DOES: Counts the object's f35c0 timer down and, when it reaches zero, stops its
; animation and sends it to state 2.
; HOW: One decrement of the timer; the rest only at zero.
; WAITS: no.
; Entry: X object. Exit: A and N, Z from the timer, or A 2 at zero; X, Y and C kept.
CountdownStopThenState2:
    dec a:OBJ_35C0,x
    bne CountdownStopThenState2_at1
    stz a:OBJ_3408,x
CountdownStopThenState2_at0:
    stz a:OBJ_3460,x
    lda #$02
    sta a:OBJ_27A8,x
CountdownStopThenState2_at1:
    rts
