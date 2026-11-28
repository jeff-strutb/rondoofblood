; @routine 01BA:717 RichterClimbAttack
; @run 6D:AF17
; WHAT IT DOES: Starts an attack on the climb: the pad held across, other than the way it goes, turns
; him back first; then no whip strike yet, the forced press cleared, stopped, and the attack with the
; crash frame's animation.
; HOW: zPadHeld against $B01B by the state less 5; RichterClimbBack; RichterStopMoving;
; RichterCrashFrame; a jump to RichterAttackWithAnimation.
; WAITS: no.
; Entry: nothing. Exit: as RichterAttackWithAnimation.
RichterClimbAttack:
    lda a:OBJ_STATE
    sec
    sbc #$05
    tay
    lda z:zPadHeld
    and #$A0
    beq RichterClimbAttack_at0
    cmp a:RichterClimbPadOnAcross,y
    beq RichterClimbAttack_at0
    jsr RichterClimbBack
RichterClimbAttack_at0:
    stz a:PlayerWhipStruck
    stz a:PadForcedPressB
    jsr RichterStopMoving
    bsr RichterCrashFrame
    jmp RichterAttackWithAnimation

; @routine 01BA:754 RichterStateClimbLow
; @run 6D:AF54
; WHAT IT DOES: States 5 and 6, climbing up: runs the step's handler.
; HOW: The six words at $AF5A by RichterStepIndex.
; WAITS: no.
; Entry: nothing. Exit: as the handler leaves them.
RichterStateClimbLow:
    jsr RichterStepIndex
    jmp (RichterClimbLowSteps,x)

; @routine 01BA:766 RichterClimbStepSpeed
; @run 6D:AF66
; WHAT IT DOES: Stops him, then, each time the animation moves on a step while it runs, gives the
; climb's step: two pixels across and two up or down by the state.
; HOW: RichterStopMoving; OBJ_ANIMATION_AT kept in zp $D9 around RichterAnimationRunning; the eight
; bytes at $AF8F by twice the state less 5, X high and Y high speeds.
; WAITS: no.
; Entry: nothing. Exit: as RichterAnimationRunning, or A the Y speed, N and Z from it, Y the offset,
; C bit 7 of the state less 5, V from the subtract. X kept.
RichterClimbStepSpeed:
    jsr RichterStopMoving
    lda a:OBJ_ANIMATION_AT
    sta z:zRichterObject
    jsr RichterAnimationRunning
    beq RichterClimbStepSpeed_at0
    lda a:OBJ_ANIMATION_AT
    cmp z:zRichterObject
    beq RichterClimbStepSpeed_at0
    lda a:OBJ_STATE
    sec
    sbc #$05
    asl a
    tay
    lda a:RichterClimbStepSpeeds,y
    sta a:OBJ_VX_HI
    lda a:RichterClimbStepSpeeds+1,y
    sta a:OBJ_VY_HI
RichterClimbStepSpeed_at0:
    rts

; @routine 01BA:797 RichterClimbPad
; @run 6D:AF97
; WHAT IT DOES: The pad on the climb: held the way it goes (both directions, the one across, or with
; the forced press clear the one up or down) goes on; held the way back likewise turns him back;
; else nothing.
; HOW: zPadHeld's directions against the tables at $B017, $B01B, $B01F (on) and $B023, $B027, $B02B
; (back) by the state less 5, PadForcedPressB tested between; RichterClimbOn, RichterClimbBack.
; WAITS: no.
; Entry: nothing. Exit: as the routine it goes to; else A the directions held, N, Z and C from the
; last compare, or the TST's Z, N and V; Y the state less 5.
RichterClimbPad:
    lda a:OBJ_STATE
    sec
    sbc #$05
    tay
    lda z:zPadHeld
    and #$F0
    cmp a:RichterClimbPadOn,y
    beq RichterClimbOn
    cmp a:RichterClimbPadOnAcross,y
    beq RichterClimbOn
    tst #$FF,a:PadForcedPressB
    bne RichterClimbPad_at0
    cmp a:RichterClimbPadOnUpDown,y
    beq RichterClimbOn
RichterClimbPad_at0:
    lda z:zPadHeld
    and #$F0
    cmp a:RichterClimbPadBack,y
    beq RichterClimbBack
    cmp a:RichterClimbPadBackAcross,y
    beq RichterClimbBack
    tst #$FF,a:PadForcedPressB
    beq RichterClimbPad_at1
    rts
RichterClimbPad_at1:
    cmp a:RichterClimbPadBackUpDown,y
    beq RichterClimbBack
    rts
RichterClimbPad_at2:
    inc z:zFieldC4
    rts

; @routine 01BA:7D5 RichterClimbBack
; @run 6D:AFD5
; WHAT IT DOES: Turns him back along the climb once zFieldC4 has counted 7 frames (counting it up
; till then): the way back's animation, frame, facing and state, the animation stopped, step 0, the
; fractions cleared.
; HOW: The four-byte tables at $B02F, $B033, $B037 and $B03B by Y; below 7 the INC and RTS of
; RichterClimbPad ($AFD2).
; WAITS: no.
; Entry: Y the state less 5. Exit: counting, N and Z from the count, C clear, A its old value. Turned,
; A the new state, N and Z from it, C set. X, Y and V kept.
RichterClimbBack:
    lda z:zFieldC4
    cmp #$07
    bcc RichterClimbPad_at2
    stz z:zFieldC4
    lda a:RichterClimbBackAnimations,y
    sta a:OBJ_ANIMATION
    lda #$FF
    sta a:OBJ_HURT_TIMER
    lda a:RichterClimbBackFrames,y
    sta a:OBJ_FRAME
    lda a:RichterClimbBackFacings,y
    sta a:OBJ_FACING_LEFT
    lda a:RichterClimbBackStates,y
    sta a:OBJ_STATE
    stz a:OBJ_SUBSTATE
    stz a:OBJ_X_FRAC
    stz a:OBJ_Y_FRAC
    rts

; @routine 01BB:004 RichterClimbOn
; @run 6D:B004
; WHAT IT DOES: Goes on along the climb, unless a button is held: the climb's animation for the state,
; by the other of the two (OBJ_ANIMATION plus 1, its bit 0).
; HOW: TST of zPadHeld; the four bytes at $B03F by Y; a tail jump to RichterSetAnimation.
; WAITS: no.
; Entry: Y the state less 5. Exit: a button held, Z clear, N and V bits 7 and 6 of zPadHeld; else as
; RichterSetAnimation.
RichterClimbOn:
    tst #$03,z:zPadHeld
    beq RichterClimbOn_at0
    rts
RichterClimbOn_at0:
    lda a:OBJ_ANIMATION
    inc a
    and #$01
    clc
    adc a:RichterClimbAnimations,y
    jmp RichterSetAnimation

; @routine 01BB:043 RichterClimbLowStep0
; @run 6D:B043
; WHAT IT DOES: States 5 and 6, step 0: the step's speed, the latch; a jump or a climb off it as
; RichterUpWhipLeft; when the animation ends, the climb's pad override for the way, frame $32, and a
; look below by the way: on the mark, an item crash, the item's use (an attack), or the pad; off it,
; step 4 with frame 7 for 4 frames.
; HOW: RichterClimbStepSpeed, RichterLatchButtons, RichterJumpOrClimb; PadOverrides by the state less
; 5; RichterReadAtOffset by the two bytes at $B09E; RichterTryItemCrash and RichterLedgeHurtA,
; RichterUseItemOrUpPress and RichterClimbAttack, RichterClimbPad.
; WAITS: no.
; Entry: nothing. Exit: as the routine it goes to; still animating, A the timer, N, Z and C from
; comparing it with $FF; stepping, A 4, N and Z from it.
RichterClimbLowStep0:
    jsr RichterClimbStepSpeed
    jsr RichterLatchButtons
    jsr RichterJumpOrClimb
    bcc RichterClimbLowStep0_at0
    jmp RichterUpWhipLeft
RichterClimbLowStep0_at0:
    lda a:OBJ_HURT_TIMER
    cmp #$FF
    beq RichterClimbLowStep0_at1
    rts
RichterClimbLowStep0_at1:
    lda a:OBJ_STATE
    sec
    sbc #$05
    tax
    lda #$01
    sta a:PadOverrides,x
    lda #$32
    sta a:OBJ_FRAME
    lda a:OBJ_STATE
    sec
    sbc #$05
    tax
    ldy a:RichterClimbLowOffsets,x
    jsr RichterReadAtOffset
    and #$04
    beq RichterClimbLowStep0_at2
    jsr RichterTryItemCrash
    bcc RichterClimbLowStep0_at3
    jmp RichterLedgeHurtA
RichterClimbLowStep0_at3:
    jsr RichterUseItemOrUpPress
    bcc RichterClimbLowStep0_at4
    jmp RichterClimbAttack
RichterClimbLowStep0_at4:
    jmp RichterClimbPad
RichterClimbLowStep0_at2:
    lda #$04
    sta a:OBJ_SUBSTATE
    lda #$07
    sta a:OBJ_FRAME
    lda #$04
    sta a:OBJ_HURT_TIMER
    rts

; @routine 01BB:0C6 RichterClimbStep1
; @run 6D:B0C6
; WHAT IT DOES: The climb's step 1, an item crash on it: the pad's turn for the animation's first three
; steps; the crash frame's animation; at its end, as RichterEndItemCrash.
; HOW: OBJ_ANIMATION_AT against 3; RichterTurnOnPad; RichterCrashFrame into OBJ_ANIMATION;
; RichterAnimationWait; a jump to RichterEndItemCrash.
; WAITS: no.
; Entry: nothing. Exit: waiting, as RichterAnimationWait; else as RichterEndItemCrash.
RichterClimbStep1:
    lda a:OBJ_ANIMATION_AT
    cmp #$03
    bcs RichterClimbStep1_at0
    jsr RichterTurnOnPad
RichterClimbStep1_at0:
    lda a:OBJ_ANIMATION_AT
    cmp #$06
    bcs RichterClimbStep1_at1
    bra RichterClimbStep1_at1
RichterClimbStep1_at1:
    jsr RichterCrashFrame
    sta a:OBJ_ANIMATION
    jsr RichterAnimationWait
    jmp RichterEndItemCrash

; @routine 01BB:0E5 RichterClimbReach
; @run 6D:B0E5
; WHAT IT DOES: The climb's step 2: a jump takes him into the air; while PlayerClimbReach is not zero,
; moves him a pixel a frame toward it, counting it to zero; then, in states 5 and 6, frame 6 for 6
; frames, step 3, the facing by the way; in 7 and 8, step 0 with animation 8 and the facing.
; HOW: RichterJumpPressed, a jump to RichterToAir; RichterAnimationTick; the facings at $B1A0;
; RichterStopX; A pushed around RichterSetAnimation.
; WAITS: no.
; Entry: nothing. Exit: as RichterToAir; reaching, N and Z from the count, A the speed's high byte;
; else A the facing, N and Z from it, Y the way, the rest as RichterStopX keeps them.
RichterClimbReach:
    jsr RichterJumpPressed
    bcc RichterClimbReach_at0
    jmp RichterToAir
RichterClimbReach_at0:
    lda a:PlayerClimbReach
    beq RichterClimbReach_at1
    bmi RichterClimbReach_at2
    jsr RichterAnimationTick
    lda #$00
    sta a:OBJ_VX_LO
    lda #$01
    sta a:OBJ_VX_HI
    dec a:PlayerClimbReach
    rts
RichterClimbReach_at2:
    jsr RichterAnimationTick
    lda #$00
    sta a:OBJ_VX_LO
    lda #$FF
    sta a:OBJ_VX_HI
    inc a:PlayerClimbReach
    rts
RichterClimbReach_at1:
    lda a:OBJ_STATE
    cmp #$07
    bcs RichterClimbReach_at3
    lda #$06
    sta a:OBJ_FRAME
    lda #$06
    sta a:OBJ_HURT_TIMER
    inc a:OBJ_SUBSTATE
    lda a:OBJ_STATE
    sec
    sbc #$05
RichterClimbReach_at4:
    tay
    lda a:RichterClimbReachFacings,y
    sta a:OBJ_FACING_LEFT
    stz a:OBJ_X_FRAC
    stz a:OBJ_Y_FRAC
    jsr RichterStopX
    rts
RichterClimbReach_at3:
    stz a:OBJ_SUBSTATE
    pha
    lda #$08
    jsr RichterSetAnimation
    pla
    sec
    sbc #$07
    tay
    bra RichterClimbReach_at4

; @routine 01BB:151 RichterClimbPullUp
; @run 6D:B151
; WHAT IT DOES: The climb's step 3: a jump takes him into the air; when the timer runs out, a step
; across by the way and, with the pad held up that way, the pull's animation (4) at step 0; else as
; RichterEndItemCrash.
; HOW: RichterJumpPressed, RichterToAir; OBJ_HURT_TIMER; the steps at $B1A2 and pads at $B1A4 by the
; state less 5; RichterSetAnimation; a branch to RichterEndItemCrash.
; WAITS: no.
; Entry: nothing. Exit: as RichterToAir; counting, N and Z from the count; pulled up, as
; RichterSetAnimation; else as RichterEndItemCrash.
RichterClimbPullUp:
    jsr RichterJumpPressed
    bcc RichterClimbPullUp_at0
    jmp RichterToAir
RichterClimbPullUp_at0:
    dec a:OBJ_HURT_TIMER
    beq RichterClimbPullUp_at1
    rts
RichterClimbPullUp_at1:
    lda a:OBJ_STATE
    sec
    sbc #$05
    tay
    lda a:OBJ_X_LO
    clc
    adc a:RichterClimbPullSteps,y
    sta a:OBJ_X_LO
    lda z:zPadHeld
    and a:RichterClimbPullPads,y
    beq RichterEndItemCrash
    lda #$04
    jsr RichterSetAnimation
    stz a:OBJ_SUBSTATE
    rts

; @routine 01BB:1B3 RichterClimbStandTimer
; @run 6D:B1B3
; WHAT IT DOES: The climb's steps 5 (and 3 of states 7 and 8): as RichterStandStep2Timer.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as RichterStandStep2Timer.
RichterClimbStandTimer:
    jmp RichterStandStep2Timer

; @routine 01BB:1B6 RichterStateClimbHigh
; @run 6D:B1B6
; WHAT IT DOES: States 7 and 8, climbing down: runs the step's handler.
; HOW: The six words at $B1BC by RichterStepIndex.
; WAITS: no.
; Entry: nothing. Exit: as the handler leaves them.
RichterStateClimbHigh:
    jsr RichterStepIndex
    jmp (RichterClimbHighSteps,x)

; @routine 01BB:1C8 RichterClimbHighStep0
; @run 6D:B1C8
; WHAT IT DOES: States 7 and 8, step 0: as RichterClimbLowStep0, with RichterUpWhipRight, frame $33,
; the overrides from the third; an item crash as RichterLedgeHurtB; then a look below by the way: on
; the mark the pad, off it the walk's speed and step 4 with frame 5 for 2 frames.
; HOW: As RichterClimbLowStep0; RichterReadAtOffset at twice the state less 7, plus 4; RichterSpeedWalk.
; WAITS: no.
; Entry: nothing. Exit: as RichterClimbLowStep0; stepping, A 2, N and Z from it.
RichterClimbHighStep0:
    jsr RichterClimbStepSpeed
    jsr RichterLatchButtons
    jsr RichterJumpOrClimb
    bcc RichterClimbHighStep0_at0
    jmp RichterUpWhipRight
RichterClimbHighStep0_at0:
    lda a:OBJ_HURT_TIMER
    cmp #$FF
    beq RichterClimbHighStep0_at1
    rts
RichterClimbHighStep0_at1:
    lda #$33
    sta a:OBJ_FRAME
    lda a:OBJ_STATE
    sec
    sbc #$07
    tay
    lda #$01
    sta a:PadOverrides+2,y
    jsr RichterTryItemCrash
    bcc RichterClimbHighStep0_at2
    jmp RichterLedgeHurtB
RichterClimbHighStep0_at2:
    jsr RichterUseItemOrUpPress
    bcc RichterClimbHighStep0_at3
    jmp RichterClimbAttack
RichterClimbHighStep0_at3:
    lda a:OBJ_STATE
    sec
    sbc #$07
    asl a
    clc
    adc #$04
    tay
    jsr RichterReadAtOffset
    and #$04
    beq RichterClimbHighStep0_at4
    jmp RichterClimbPad
RichterClimbHighStep0_at4:
    jsr RichterSpeedWalk
    lda #$04
    sta a:OBJ_SUBSTATE
    lda #$05
    sta a:OBJ_FRAME
    lda #$02
    sta a:OBJ_HURT_TIMER
    rts
