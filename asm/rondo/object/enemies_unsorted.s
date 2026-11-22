; TEMPORARY: routines of the four stages' enemies whose kind is not yet identified.

; @routine 093C:068 EnterStateFourOnFlag
; @run 70:B868
; WHAT IT DOES: Switches an object that is not yet in state 4 into state 4, with its animation
; list restarted, once bit 7 of OBJ_31A0 is set.
; HOW: Reads the state and the flag once each; clears the two animation counters and stores 4.
; WAITS: no.
; Entry: X object. Exit: A and flags as the last compare, AND or load left them. X and Y kept.
; D clear.
EnterStateFourOnFlag:
    lda a:OBJ_27A8,x
    cmp #$04
    beq EnterStateFourOnFlag_out
    lda a:OBJ_31A0,x
    and #$80
    beq EnterStateFourOnFlag_out
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$04
    sta a:OBJ_27A8,x
EnterStateFourOnFlag_out:
    rts

; @routine 0CAB:260 SetHitOffsetsFromFrame
; @run 70:B260
; WHAT IT DOES: Loads an object's four hit offsets (OBJ_3930, OBJ_39E0, OBJ_3988, OBJ_3A38)
; from a four-byte row of a table, picked by the object's animation frame through a second table
; of row numbers; no frame, or a row number 0, clears the last two.
; HOW: Reads the frame (OBJ_2A68), the row number at $B292 + frame - 1, then the four bytes at
; $BA2E + 4 * (row - 1).
; WAITS: no.
; Entry: X object. Exit: A the last byte loaded (0 when cleared), N and Z from it, Y the
; table offset, C the last shift's carry. X kept. D clear.
SetHitOffsetsFromFrame:
    lda a:OBJ_2A68,x
    beq SetHitOffsetsFromFrame_none
    sec
    sbc #$01
    tay
    lda a:$B292,y
    beq SetHitOffsetsFromFrame_none
    dec a
    asl a
    asl a
    tay
    lda a:$BA2E,y
    sta a:OBJ_3930,x
    lda a:$BA2F,y
    sta a:OBJ_39E0,x
    lda a:$BA30,y
    sta a:OBJ_3988,x
    lda a:$BA31,y
    sta a:OBJ_3A38,x
    rts
SetHitOffsetsFromFrame_none:
    stz a:OBJ_3988,x
    stz a:OBJ_3A38,x
    rts
