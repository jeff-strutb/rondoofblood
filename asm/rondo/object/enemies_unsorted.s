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

; @routine 0CAB:260 SetHitboxFromFrame
; @run 70:B260
; WHAT IT DOES: Loads an object's hitbox (dx, dy, half width, half height: OBJ_3930, OBJ_39E0, OBJ_3988, OBJ_3A38)
; from a four-byte row of a table, picked by the object's animation frame through a second table
; of row numbers; no frame, or a row number 0, clears the last two.
; HOW: Reads the frame (OBJ_2A68), the row number at $B292 + frame - 1, then the four bytes at
; $BA2E + 4 * (row - 1).
; WAITS: no.
; Entry: X object. Exit: A the last byte loaded (0 when cleared), N and Z from it, Y the
; table offset, C the last shift's carry. X kept. D clear.
SetHitboxFromFrame:
    lda a:OBJ_2A68,x
    beq SetHitboxFromFrame_none
    sec
    sbc #$01
    tay
    lda a:$B292,y
    beq SetHitboxFromFrame_none
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
SetHitboxFromFrame_none:
    stz a:OBJ_3988,x
    stz a:OBJ_3A38,x
    rts

; @routine 0CAB:3B9 TurnAngleBack
; @run 70:B3B9
; WHAT IT DOES: Turns an object's 16-bit angle (OBJ_3568 high, OBJ_35C0 low) back by $0080;
; once it passes below zero, sets bit 2 of the stage flag byte zField8E, shows frame 1,
; switches the hitbox on (2) and moves the object to its next state.
; HOW: A two-byte subtract, low byte first; the borrow decides.
; WAITS: no.
; Entry: X object. Exit: plain: A the new high byte, N Z C V from that subtract; borrowed: A 2,
; N and Z from the new state, C 0, V from the subtract. X and Y kept. D clear.
TurnAngleBack:
    lda a:OBJ_35C0,x
    sec
    sbc #$80
    sta a:OBJ_35C0,x
    lda a:OBJ_3568,x
    sbc #$00
    sta a:OBJ_3568,x
    bcs TurnAngleBack_out
    lda z:zField8E
    ora #$04
    sta z:zField8E
    lda #$01
    sta a:OBJ_2A68,x
    lda #$02
    sta a:OBJ_3250,x
    inc a:OBJ_27A8,x
TurnAngleBack_out:
    rts

; @routine 09EB:5ED AdvanceOnTimer
; @run 70:B5ED
; WHAT IT DOES: Counts an object's life timer down; when it reaches zero, loads OBJ_3510, the
; angle pair and OBJ_3618 from a four-byte row of a table picked by its next kind, and moves it
; to its next state.
; HOW: DEC of the timer in memory; on zero, the row at $B614 + 4 * next kind.
; WAITS: no.
; Entry: X object. Exit: timer left: A kept, N and Z from the new timer. Moved on: A the last
; byte loaded, Y the row offset, N and Z from the new state, C bit 6 of the next kind. X kept.
; D clear.
AdvanceOnTimer:
    dec a:OBJ_34B8,x
    bne AdvanceOnTimer_out
    lda a:OBJ_3040,x
    asl a
    asl a
    tay
    lda a:$B614,y
    sta a:OBJ_3510,x
    lda a:$B615,y
    sta a:OBJ_3568,x
    lda a:$B616,y
    sta a:OBJ_35C0,x
    lda a:$B617,y
    sta a:OBJ_3618,x
    inc a:OBJ_27A8,x
AdvanceOnTimer_out:
    rts
