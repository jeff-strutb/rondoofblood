; @routine 01E2:73B MariaItem3
; @run 6E:CF3B
; WHAT IT DOES: The handler of item 3's object (kind 3): its state's routine from MariaItem3States (start, fly,
; rush).
; HOW: MariaStateDoubled, then the JMP (table,X).
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
MariaItem3:
    jsr MariaStateDoubled
    jmp (MariaItem3States,x)

; @routine 01E2:747 MariaItem3Start
; @run 6E:CF47
; WHAT IT DOES: State 0 of item 3's object: from her (facing and flip), $10 back and $10 up from her, a hitbox $20
; by $40 at 5 across and 4 down, live, OBJ_WORK_N $0C; its angle by facing (MariaItem3Angles), speed $0200; then
; animation 8 and its next state.
; HOW: MariaObjectFromPlayer, MariaObjectAtPlayer; the tail JMP to MariaObjectStartAnimation.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as MariaObjectStartAnimation.
MariaItem3Start:
    ldx z:zClimbStepBefore
    jsr MariaObjectFromPlayer
    lda #$F0
    sta z:zTemp04
    lda #$F0
    sta z:zTemp06
    jsr MariaObjectAtPlayer
    lda #$04
    sta a:OBJ_HITBOX_DY,x
    lda #$05
    sta a:OBJ_HITBOX_DX,x
    lda #$20
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$10
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    lda #$0C
    sta a:OBJ_WORK_N,x
    ldy a:OBJ_FACING_LEFT,x
    lda a:MariaItem3Angles,y
    sta a:OBJ_FLY_ANGLE,x
    lda #$00
    sta a:OBJ_FLY_SPEED_LO,x
    lda #$02
    sta a:OBJ_FLY_SPEED_HI,x
    lda #$08
    jmp MariaObjectStartAnimation

; @routine 01E2:78F MariaItem3Fly
; @run 6E:CF8F
; WHAT IT DOES: State 1 of item 3's object: it moves; while its animation runs its angle turns by MariaItem3Turns the
; way it faces and its velocity follows (on into MariaObjectVelocityFromWork); when the animation has stopped,
; OBJ_WORK_N $0D, the next animation and its next state.
; HOW: MariaObjectMove, MariaObjectAnimationRunning's Z; MariaObjectSetAnimation, the tail JMP to MariaObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as MariaObjectVelocityFromWork, or MariaObjectNextState.
MariaItem3Fly:
    ldx z:zClimbStepBefore
    jsr MariaObjectMove
    jsr MariaObjectAnimationRunning
    bne MariaItem3Fly_at0
    lda #$0D
    sta a:OBJ_WORK_N,x
    lda a:OBJ_WORK_A,x
    inc a
    jsr MariaObjectSetAnimation
    jmp MariaObjectNextState
MariaItem3Fly_at0:
    ldy a:OBJ_FACING_LEFT,x
    lda a:OBJ_FLY_ANGLE,x
    clc
    adc a:MariaItem3Turns,y
    sta a:OBJ_FLY_ANGLE,x

; @routine 01E2:7CA MariaItem3Rush
; @run 6E:CFCA
; WHAT IT DOES: State 2 of item 3's object: it animates, its speed grows by $0030 a frame, every eighth frame its angle
; turns by the second pair of MariaItem3Turns; its velocity follows and it moves; on a screen past the first with its
; X from $30 to $CF it is cleared.
; HOW: MariaObjectAnimationTick, the 16-bit add, zWaveCounter, a BSR to MariaObjectVelocityFromWork, MariaObjectMove;
; the tail JMP to ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the last test, or ClearObject.
MariaItem3Rush:
    ldx z:zClimbStepBefore
    jsr MariaObjectAnimationTick
    lda a:OBJ_FLY_SPEED_LO,x
    clc
    adc #$30
    sta a:OBJ_FLY_SPEED_LO,x
    cla
    adc a:OBJ_FLY_SPEED_HI,x
    sta a:OBJ_FLY_SPEED_HI,x
    lda z:zWaveCounter
    and #$07
    bne MariaItem3Rush_at0
    ldy a:OBJ_FACING_LEFT,x
    lda a:OBJ_FLY_ANGLE,x
    clc
    adc a:MariaItem3Turns+2,y
    sta a:OBJ_FLY_ANGLE,x
MariaItem3Rush_at0:
    bsr MariaObjectVelocityFromWork
    jsr MariaObjectMove
    lda a:OBJ_X_HI,x
    beq MariaObjectVelocityFromWork+16
    lda a:OBJ_X_LO,x
    cmp #$30
    bcc MariaObjectVelocityFromWork+16
    cmp #$D0
    bcs MariaObjectVelocityFromWork+16
    jmp VecClearObject

; @routine 01E3:00A MariaItem6
; @run 6E:D00A
; WHAT IT DOES: The handler of item 6's object (kind 6), by state. 0: from her, 2 across and 2 up, a hitbox $10 by $0A,
; live; X speed entry 9 the way it faces, Y speed entry 7, a fall of $0024, OBJ_WORK_N $0C, sprite set 1, frame $2D,
; its next state. 1: it moves and falls; below the screen it is cleared; on the way down 8 rows above solid ground it
; lands (its Y up by the row, stopped, Y speed entry $0E on bit 4, animation $0B); with something of bit 1 ahead it
; stops along X. 2: it moves; when its animation has stopped, slots after it up to $0A are emptied and become pieces of
; kind 20 at slot 5's place and flip, and it shows frame $30 for $20 frames. 3: it moves until that timer runs out,
; then is cleared.
; HOW: DEC of the state in A for each; the calls named; the tail JMPs to MariaObjectNextState, MariaKind2Clear,
; MariaObjectStartAnimation and ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the last step left it.
MariaItem6:
    ldx z:zClimbStepBefore
    lda a:OBJ_STATE,x
    bne MariaItem6_at0
    jsr MariaObjectFromPlayer
    lda #$02
    sta z:zTemp04
    lda #$FE
    sta z:zTemp06
    jsr MariaObjectAtPlayer
    lda #$08
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$05
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    lda #$09
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity
    lda #$07
    jsr MariaSetVelocityYFromTable
    lda #$24
    sta a:OBJ_FALL_LO,x
    lda #$00
    sta a:OBJ_FALL_HI,x
    lda #$0C
    sta a:OBJ_WORK_N,x
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    lda #$2D
    sta a:OBJ_FRAME,x
    jmp MariaObjectNextState
MariaItem6_at0:
    dec a
    bne MariaItem6_at1
    jsr MariaObjectMove
    jsr MariaObjectFallCapped
    lda a:OBJ_Y_HI,x
    cmp #$01
    bne MariaItem6_at2
    jmp MariaKind2Clear
MariaItem6_at2:
    cmp #$FF
    beq MariaItem6_at3
    lda a:OBJ_VY_HI,x
    bmi MariaItem6_at3
    lda #$08
    jsr MariaObjectProbeBelowAny
    beq MariaItem6_at3
    lda #$00
    jsr MariaObjectProbeBelow
    beq MariaItem6_at4
MariaItem6_at3:
    ldy a:OBJ_FACING_LEFT,x
    lda a:MariaKind2WallAhead,y
    cly
    jsr MariaObjectProbeAtBit1
    beq MariaItem6_at5
    jsr MariaObjectStopX
MariaItem6_at5:
    rts
MariaItem6_at4:
    lda #$08
    jsr MariaObjectProbeBelowAny
    pha
    lda a:OBJ_Y_LO,x
    sec
    sbc z:zTemp02
    sta a:OBJ_Y_LO,x
    jsr MariaObjectStop
    stz a:OBJ_Y_FRAC,x
    pla
    and #$10
    beq MariaItem6_at6
    lda #$0E
    jsr MariaSetVelocityYFromTable
MariaItem6_at6:
    lda #$0B
    jmp MariaObjectStartAnimation
MariaItem6_at1:
    dec a
    bne MariaItem6_at7
    jsr MariaObjectMove
    jsr MariaObjectAnimationRunning
    beq MariaItem6_at8
    rts
MariaItem6_at8:
    phx
MariaItem6_at9:
    inx
    jsr VecClearObject
    lda a:OBJ_X_LO+5
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI+5
    sta a:OBJ_X_HI,x
    lda a:OBJ_Y_LO+5
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI+5
    sta a:OBJ_Y_HI,x
    lda #$14
    sta a:OBJ_KIND,x
    lda a:OBJ_SPRITE_FLIP+5
    sta a:OBJ_SPRITE_FLIP,x
    cpx #$0B
    bne MariaItem6_at9
    plx
    lda #$30
    sta a:OBJ_FRAME,x
    lda #$20
    sta a:OBJ_HURT_TIMER,x
    jmp MariaObjectNextState
MariaItem6_at7:
    jsr MariaObjectMove
    dec a:OBJ_HURT_TIMER,x
    beq MariaItem6_at10
    rts
MariaItem6_at10:
    jmp VecClearObject

; @routine 01E3:107 MariaItem6Piece
; @run 6E:D107
; WHAT IT DOES: The handler of kind 20, one of item 6's pieces (slots 6 to 11). State 0: its facing, Y speed and X
; speed entry by its slot (MariaPieceFacings, MariaPieceVyLo and Hi, MariaPieceVx), a fall of $FFD0 (rising), then
; its hitbox (MariaItem6PieceHitbox). Later states: MariaItem6PieceFly.
; HOW: The slot less 6 indexes the tables; MariaSetVelocityXFromTable, MariaObjectFacingVelocity; on into
; MariaItem6PieceHitbox; the branch to MariaItem6PieceFly.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the routine run into or branched to.
MariaItem6Piece:
    ldx z:zClimbStepBefore
    lda a:OBJ_STATE,x
    bne MariaItem6PieceFly
    txa
    sec
    sbc #$06
    tay
    lda a:MariaPieceFacings,y
    sta a:OBJ_FACING_LEFT,x
    lda #$D0
    sta a:OBJ_FALL_LO,x
    lda #$FF
    sta a:OBJ_FALL_HI,x
    lda a:MariaPieceVyLo,y
    sta a:OBJ_VY_LO,x
    lda a:MariaPieceVyHi,y
    sta a:OBJ_VY_HI,x
    lda a:MariaPieceVx,y
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity

; @routine 01E3:138 MariaItem6PieceHitbox
; @run 6E:D138
; WHAT IT DOES: An item piece's hitbox: $08 high, $0C wide, 4 to the left, live; OBJ_WORK_N 5; animation 0 and its
; next state.
; HOW: The stores, then the tail JMP to MariaObjectStartAnimation.
; WAITS: no.
; Entry: X the object. Exit: as MariaObjectStartAnimation.
MariaItem6PieceHitbox:
    lda #$04
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$06
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$FC
    sta a:OBJ_HITBOX_DX,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    lda #$05
    sta a:OBJ_WORK_N,x
    lda #$00
    jmp MariaObjectStartAnimation

; @routine 01E3:156 MariaItem6PieceFly
; @run 6E:D156
; WHAT IT DOES: An item 6 piece after its start: it animates, moves and falls, and is cleared above the screen.
; HOW: MariaObjectAnimationTick, MariaObjectMove, MariaObjectFallCapped; the tail JMP to ClearObject.
; WAITS: no.
; Entry: X the object. Exit: as the compare with $FF, or ClearObject.
MariaItem6PieceFly:
    jsr MariaObjectAnimationTick
    jsr MariaObjectMove
    jsr MariaObjectFallCapped
    lda a:OBJ_Y_HI,x
    cmp #$FF
    beq MariaItem6PieceFly_at0
    rts
MariaItem6PieceFly_at0:
    jmp VecClearObject

; @routine 01E3:182 MariaItem8
; @run 6E:D182
; WHAT IT DOES: The handler of item 8's objects (kind 8, slots 5 to 12). State 0: the one in slot 5 makes slots 6 to
; 12 kind 8 as well and plays sound $0A; each starts from her (2 across, 2 up), a hitbox $10 by $0A, live, OBJ_WORK_N
; 6, X speed entry $0F, facing right, with its own angle, delay and animation by slot (MariaItem8Angles,
; MariaItem8Timers, MariaItem8Animations). Later: while its delay runs it shows no frame and moves against the scroll;
; then it falls along the cosine of its angle (speed $0080), the angle turning by 4 a frame, and moves. Struck by an
; object whose hitbox is 2 with OBJ_WORK_P 1 or 2, it ends next frame (timer 1, no hitbox); when its animation has
; stopped, the item's first use is due again and it is cleared.
; HOW: The slot less 5 indexes the tables; SoundCommand; ScaleByCosine and zProduct1-2; OBJ_HIT_BY_SLOT;
; MariaObjectAnimationRunning; the tail JMPs to MariaObjectStartAnimation, MariaObjectAgainstScroll and ClearObject.
; The add of MariaItem8Adds to $80 at $D1F8 is made and its result left unused, as the assembly does.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the last step left it.
MariaItem8:
    ldx z:zClimbStepBefore
    lda a:OBJ_STATE,x
    bne MariaItem8_at0
    cpx #$05
    bne MariaItem8_at1
    lda #$08
    cly
MariaItem8_at2:
    sta a:OBJ_KIND+6,y
    iny
    cpy #$07
    bne MariaItem8_at2
    lda #$0A
    jsr VecSoundCommand
MariaItem8_at1:
    jsr MariaObjectFromPlayer
    lda #$02
    sta z:zTemp04
    lda #$FE
    sta z:zTemp06
    jsr MariaObjectAtPlayer
    lda #$08
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$05
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    lda #$06
    sta a:OBJ_WORK_N,x
    lda #$0F
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity
    txa
    sec
    sbc #$05
    tay
    lda a:MariaItem8Angles,y
    sta a:OBJ_FLY_ANGLE,x
    lda a:MariaItem8Timers,y
    sta a:OBJ_ANIM_TIMER,x
    stz a:OBJ_FACING_LEFT,x
    lda a:MariaItem8Animations,y
    jmp MariaObjectStartAnimation
MariaItem8_at0:
    lda a:OBJ_ANIM_TIMER,x
    beq MariaItem8_at3
    stz a:OBJ_FRAME,x
    dec a:OBJ_ANIM_TIMER,x
    jmp MariaObjectAgainstScroll
MariaItem8_at3:
    txa
    sec
    sbc #$05
    tay
    lda #$80
    sta z:zTemp00
    lda #$80
    clc
    adc a:MariaItem8Adds,y
    stz z:zTemp01
    lda a:OBJ_FLY_ANGLE,x
    jsr VecScaleByCosine
    lda a:OBJ_FLY_ANGLE,x
    clc
    adc #$04
    sta a:OBJ_FLY_ANGLE,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    lda z:zPlaneRowsLeft
    sta a:OBJ_VY_HI,x
    jsr MariaObjectMove
    ldy a:OBJ_HIT_BY_SLOT,x
    lda a:OBJ_HITBOX_ON,y
    cmp #$02
    bne MariaItem8_at4
    lda a:OBJ_WORK_P,x
    cmp #$01
    beq MariaItem8_at5
    cmp #$02
    beq MariaItem8_at5
MariaItem8_at4:
    jsr MariaObjectAnimationRunning
    bne MariaItem8_at6
    lda #$01
    sta a:PlayerItemFirstUse
    jmp VecClearObject
MariaItem8_at6:
    rts
MariaItem8_at5:
    lda #$01
    sta a:OBJ_HURT_TIMER,x
    stz a:OBJ_WORK_P,x
    stz a:OBJ_HITBOX_ON,x
    rts
