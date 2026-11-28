; @routine 0E0C:0F1 StopWhenSlot16Marked0E0C
; @run 70:B8F1
; WHAT IT DOES: While in states 1 to 6, stops the object once bit 7 of slot $10's OBJ_HIT_STATUS is set (Secret stage
; 08's overlay): Y velocity, hit box and animation cleared, attribute bit 4, frame $1E, OBJ_WORK_E $60, state 7.
; HOW: Two tests of the state, one of slot $10's byte, the stores.
; WAITS: no.
; Entry: X object. Exit: A the state, zero, or 7, N and Z from it; C from the state's compare; X, Y and V kept.
StopWhenSlot16Marked0E0C:
    lda a:OBJ_STATE,x
    beq StopWhenSlot16Marked0E0C_at0
    cmp #$07
    bcs StopWhenSlot16Marked0E0C_at0
    lda a:OBJ_HIT_STATUS+16
    and #$80
    beq StopWhenSlot16Marked0E0C_at0
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda a:OBJ_SPRITE_ATTR,x
    ora #$10
    sta a:OBJ_SPRITE_ATTR,x
    lda #$1E
    sta a:OBJ_FRAME,x
    lda #$60
    sta a:OBJ_TALLY_WAIT,x
    lda #$07
    sta a:OBJ_STATE,x
StopWhenSlot16Marked0E0C_at0:
    rts

; @routine 0EBC:6B7 SumXVelocity0EBC
; @run 70:BEB7
; WHAT IT DOES: Adds the object's X velocity into a 24-bit sum of its own (Stage 09's overlay), keeping the sum's
; middle byte between -$12 and $18.
; HOW: The velocity sign extended through zTemp00 and added; two compares clamp.
; WAITS: no.
; Entry: X object, D clear. Exit: A the middle byte or its bound, N and Z from the last load or compare, C from the
; compare, V from the high byte's add; X and Y kept.
SumXVelocity0EBC:
    stz z:zTemp00
    lda a:OBJ_VX_HI,x
    bpl SumXVelocity0EBC_at0
    dec z:zTemp00
SumXVelocity0EBC_at0:
    lda a:OBJ_VX_LO,x
    clc
    adc a:OBJ_VX_SUM_FRAC,x
    sta a:OBJ_VX_SUM_FRAC,x
    lda a:OBJ_VX_HI,x
    adc a:OBJ_VX_SUM,x
    sta a:OBJ_VX_SUM,x
    lda z:zTemp00
    adc a:OBJ_VX_SUM_HI,x
    sta a:OBJ_VX_SUM_HI,x
    lda a:OBJ_VX_SUM,x
    bmi SumXVelocity0EBC_at1
    cmp #$18
    bcs SumXVelocity0EBC_at2
    rts
SumXVelocity0EBC_at1:
    cmp #$EE
    bcc SumXVelocity0EBC_at3
    rts
SumXVelocity0EBC_at2:
    lda #$18
    sta a:OBJ_VX_SUM,x
    rts
SumXVelocity0EBC_at3:
    lda #$EE
    sta a:OBJ_VX_SUM,x
    rts

; @routine 101B:7ED HitBoxByFrameRow101B
; @run 70:B7ED
; WHAT IT DOES: Gives the object the hit box of its animation frame (Stage 0A's overlay): frames from $25 on name a
; row by the table at $B841, and the row's bytes become the box and OBJ_WORK_C and OBJ_WORK_B.
; HOW: The frame less $25, the row, then six table reads and stores.
; WAITS: no.
; Entry: X object. Exit: A the last byte, N and Z from it, Y the row, C and V from the subtract, X kept.
HitBoxByFrameRow101BDx = $B81D ; the box's X offset by row
HitBoxByFrameRow101BDy = $B823 ; its Y offset by row
HitBoxByFrameRow101BHalfWidths = $B829 ; its half width by row
HitBoxByFrameRow101BHalfHeights = $B82F ; its half height by row
HitBoxByFrameRow101BWorkC = $B835 ; OBJ_WORK_C by row
HitBoxByFrameRow101BWorkB = $B83B ; OBJ_WORK_B by row
HitBoxByFrameRow101BRows = $B841 ; the row by frame less $25
HitBoxByFrameRow101B:
    lda a:OBJ_FRAME,x
    sec
    sbc #$25
    tay
    lda a:HitBoxByFrameRow101BRows,y
    tay
    lda a:HitBoxByFrameRow101BDx,y
    sta a:OBJ_HITBOX_DX,x
    lda a:HitBoxByFrameRow101BDy,y
    sta a:OBJ_HITBOX_DY,x
    lda a:HitBoxByFrameRow101BHalfWidths,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:HitBoxByFrameRow101BHalfHeights,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:HitBoxByFrameRow101BWorkC,y
    sta a:OBJ_WORK_C,x
    lda a:HitBoxByFrameRow101BWorkB,y
    sta a:OBJ_WORK_B,x
    rts

; @routine 117B:299 MoveXWithScrollBand117B
; @run 70:B299
; WHAT IT DOES: Takes from the object's X what the scroll is ahead of its band's (Stage 0B's overlay); the Y
; differences are left in zTemp02-03 and the Y fraction's also at ScrollBandYFracStep by band.
; HOW: The band (OBJ_SCROLL_BAND, bits 0 to 6); four subtracts; the X difference, sign extended into zTemp10, off the
; 24-bit position.
; WAITS: no.
; Entry: X object. Exit: A the new X high byte, N, Z, C and V from its subtract, Y the band, X kept.
MoveXWithScrollBand117B:
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    lda a:ScrollXFrac,y
    sec
    sbc a:ScrollBandFirst,y
    sta z:zTemp00
    lda a:ScrollXLo,y
    sbc a:ScrollBgxLo,y
    sta z:zTemp01
    lda a:ScrollYFrac,y
    sec
    sbc a:ScrollBandFourth,y
    sta z:zTemp02
    sta a:ScrollBandYFracStep,y
    lda a:ScrollYLo,y
    sbc a:ScrollBgyLo,y
    sta z:zTemp03
    stz z:zTemp10
    lda z:zTemp01
    bpl MoveXWithScrollBand117B_at0
    dec z:zTemp10
MoveXWithScrollBand117B_at0:
    lda a:OBJ_X_FRAC,x
    sec
    sbc z:zTemp00
    sta a:OBJ_X_FRAC,x
    lda a:OBJ_X_LO,x
    sbc z:zTemp01
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp10
    sta a:OBJ_X_HI,x
    rts

; @routine 161C:31A HitBoxByFrameRange161C
; @run 70:BB1A
; WHAT IT DOES: Gives the object a low, forward hit box while its frame is 7 to 13, else a tall one (Stage 0E's overlay).
; HOW: Two compares of the frame, four stores.
; WAITS: no.
; Entry: X object. Exit: A the half height, N 0, Z 0; C set only for frames from 14; X, Y and V kept.
HitBoxByFrameRange161C:
    lda a:OBJ_FRAME,x
    cmp #$07
    bcc HitBoxByFrameRange161C_at0
    cmp #$0E
    bcs HitBoxByFrameRange161C_at0
    lda #$F1
    sta a:OBJ_HITBOX_DX,x
    lda #$0D
    sta a:OBJ_HITBOX_DY,x
    lda #$10
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$0B
    sta a:OBJ_HITBOX_HALF_H,x
    rts
HitBoxByFrameRange161C_at0:
    stz a:OBJ_HITBOX_DX,x
    stz a:OBJ_HITBOX_DY,x
    lda #$0C
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$16
    sta a:OBJ_HITBOX_HALF_H,x
    rts
