; @routine 01E4:3DD MariaKind17
; @run 6E:DBDD
; WHAT IT DOES: The handler of kind 17. State 0: cleared when she is hit; at her frame $50 its next state, sprite set
; 1, frame $2D, her flip and facing, her place plus (9, -$11). After: at her frame $50 the same place; at $51 her place
; plus (7, -$17); at $52 the first time slots 6 to 14 become kind $17 at its place, then frame $30 and that place; at
; any other frame it is cleared.
; HOW: Her OBJ_STATE and OBJ_FRAME; MariaObjectNextState, MariaObjectFromPlayer, MariaObjectAtPlayer; OBJ_WORK_D as the
; spawn's flag; ClearObject by the tail JMP.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the last step made.
MariaKind17:
    ldx z:zRichterObject
    lda a:OBJ_STATE,x
    bne MariaKind17_at0
    lda a:OBJ_STATE
    cmp #$09
    bne MariaKind17_at1
    jmp VecClearObject
MariaKind17_at1:
    lda a:OBJ_FRAME
    cmp #$50
    beq MariaKind17_at2
    rts
MariaKind17_at2:
    jsr MariaObjectNextState
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    lda #$2D
    sta a:OBJ_FRAME,x
MariaKind17_at3:
    jsr MariaObjectFromPlayer
    lda #$09
    sta z:zTemp04
    lda #$EF
    sta z:zTemp06
    jsr MariaObjectAtPlayer
    rts
MariaKind17_at0:
    lda a:OBJ_FRAME
    cmp #$50
    beq MariaKind17_at3
    cmp #$51
    beq MariaKind17_at4
    cmp #$52
    beq MariaKind17_at5
    jmp VecClearObject
MariaKind17_at4:
    lda #$07
    sta z:zTemp04
    lda #$E9
    sta z:zTemp06
    jsr MariaObjectAtPlayer
    jmp MariaObjectFromPlayer
MariaKind17_at5:
    lda a:OBJ_WORK_D,x
    bne MariaKind17_at6
    cly
MariaKind17_at7:
    lda #$17
    sta a:OBJ_KIND+6,y
    lda a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO+6,y
    lda a:OBJ_X_LO,x
    sta a:OBJ_X_LO+6,y
    iny
    cpy #$09
    bne MariaKind17_at7
    inc a:OBJ_WORK_D,x
MariaKind17_at6:
    lda #$30
    sta a:OBJ_FRAME,x
    lda #$07
    sta z:zTemp04
    lda #$E9
    sta z:zTemp06
    jsr MariaObjectAtPlayer
    jmp MariaObjectFromPlayer

; @routine 01E4:464 MariaKind23
; @run 6E:DC64
; WHAT IT DOES: The handler of kind 23. State 0: her flip and facing, a fall of $0006 a frame, its speeds by slot
; (MariaKind23SpeedsY and X), slot 14 with sprite attribute $85, then its hitbox as item 6's pieces have it. State 1:
; its animation, move and fall; once her frame is past $52, no Y speed, X speed entry 0 and Y entry $10 the way it
; faces, a fall of $FFF0, and its next state. Later: the same, and cleared once on the next screen between $10 and $F0.
; HOW: MariaObjectFromPlayer, the tables by slot less 6, the tail JMP to MariaItem6PieceHitbox; MariaObjectAnimationTick,
; MariaObjectMove, MariaObjectFallCapped; MariaObjectStopY, the velocity tables, MariaObjectFacingVelocity,
; MariaObjectNextState; ClearObject by the tail JMP.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the last step made.
MariaKind23:
    ldx z:zRichterObject
    lda a:OBJ_STATE,x
    bne MariaKind23_at0
    jsr MariaObjectFromPlayer
    txa
    sec
    sbc #$06
    tay
    lda #$06
    sta a:OBJ_FALL_LO,x
    lda #$00
    sta a:OBJ_FALL_HI,x
    lda a:MariaKind23SpeedsYLo,y
    sta a:OBJ_VY_LO,x
    lda a:MariaKind23SpeedsYHi,y
    sta a:OBJ_VY_HI,x
    lda a:MariaKind23SpeedsXLo,y
    sta a:OBJ_VX_LO,x
    lda a:MariaKind23SpeedsXHi,y
    sta a:OBJ_VX_HI,x
    cpy #$08
    bne MariaKind23_at1
    lda #$85
    sta a:OBJ_SPRITE_ATTR,x
MariaKind23_at1:
    jmp MariaItem6PieceHitbox
MariaKind23_at0:
    dec a
    bne MariaKind23_at2
    jsr MariaObjectAnimationTick
    jsr MariaObjectMove
    jsr MariaObjectFallCapped
    lda a:OBJ_FRAME
    cmp #$52
    bne MariaKind23_at3
    rts
MariaKind23_at3:
    jsr MariaObjectStopY
    lda #$00
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity
    lda #$10
    jsr MariaSetVelocityYFromTable
    lda #$F0
    sta a:OBJ_FALL_LO,x
    lda #$FF
    sta a:OBJ_FALL_HI,x
    jmp MariaObjectNextState
MariaKind23_at2:
    jsr MariaObjectAnimationTick
    jsr MariaObjectMove
    jsr MariaObjectFallCapped
    lda a:OBJ_X_HI,x
    beq MariaKind23_at4
    lda a:OBJ_X_LO,x
    cmp #$10
    bcc MariaKind23_at4
    cmp #$F0
    bcs MariaKind23_at4
    jmp VecClearObject
MariaKind23_at4:
    rts

; @routine 01E4:513 MariaKind24
; @run 6E:DD13
; WHAT IT DOES: The handler of kind 24. State 0: her flip and facing, her place plus (-4, $17), its speeds by slot
; (MariaKind24Speeds) turned the way it faces, a fall of $0022 a frame, animation $14, and its animation's timer
; lengthened by its slot less 11. After: it falls and moves, and is cleared when its animation has stopped.
; HOW: MariaObjectFromPlayer, MariaObjectAtPlayer, the table by slot less 11 times four, MariaObjectFacingVelocity,
; MariaObjectStartAnimation; MariaObjectFallCapped, MariaObjectMove, MariaObjectAnimationRunning's Z, ClearObject by the
; tail JMP.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the last step made.
MariaKind24:
    ldx z:zRichterObject
    lda a:OBJ_STATE,x
    bne MariaKind24_at0
    jsr MariaObjectFromPlayer
    lda #$FC
    sta z:zTemp04
    lda #$17
    sta z:zTemp06
    jsr MariaObjectAtPlayer
    txa
    sec
    sbc #$0B
    asl a
    asl a
    tay
    lda a:MariaKind24Speeds,y
    sta a:OBJ_VX_LO,x
    lda a:MariaKind24Speeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:MariaKind24Speeds+2,y
    sta a:OBJ_VY_LO,x
    lda a:MariaKind24Speeds+3,y
    sta a:OBJ_VY_HI,x
    jsr MariaObjectFacingVelocity
    lda #$22
    sta a:OBJ_FALL_LO,x
    lda #$00
    sta a:OBJ_FALL_HI,x
    lda #$14
    jsr MariaObjectStartAnimation
    txa
    sec
    sbc #$0B
    clc
    adc a:OBJ_HURT_TIMER,x
    sta a:OBJ_HURT_TIMER,x
    rts
MariaKind24_at0:
    jsr MariaObjectFallCapped
    jsr MariaObjectMove
    jsr MariaObjectAnimationRunning
    beq MariaKind24_at1
    rts
MariaKind24_at1:
    jmp VecClearObject
