; @routine 01BA:163 RichterStateFall
; @run 6D:A963
; WHAT IT DOES: State 4, falling: lands on a floor whose column answers 0; once OBJ_FALL_DRIFT_DELAY runs
; out drifts by facing; debris on marked columns; lands on a marked floor or marked ground; gravity at
; $48 a frame to $0700; at step 0 the item may be used (an attack in the air), at step 1 (attacking)
; the attack runs to its end, frame $12, step 0.
; HOW: The landing check; RichterFallDriftSpeeds; RichterDebrisOnMarkedColumns; RichterOnMarkedFloor and
; RichterOnMarkedUnderFeet; RichterNoteItemPress, RichterUseItem, RichterAirAttack; RichterAnimationWait.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: as the path's last routine; attack ended, A 0, Z set.
RichterStateFall:
    jsr RichterReadColumns
    jsr RichterFloorCheck
    bcc RichterStateFall_at0
    jsr RichterSolidUnderColumn
    bne RichterStateFall_at0
    jmp RichterLandOnColumn
RichterStateFall_at0:
    lda a:OBJ_WORK_D
    beq RichterStateFall_at1
    dec a:OBJ_WORK_D
    bne RichterStateFall_at1
    lda a:OBJ_FACING_LEFT
    asl a
    tay
    lda a:RichterFallDriftSpeeds,y
    sta a:OBJ_VX_LO
    lda a:RichterFallDriftSpeeds+1,y
    sta a:OBJ_VX_HI
RichterStateFall_at1:
    jsr RichterDebrisOnMarkedColumns
    jsr RichterOnMarkedFloor
    bcc RichterStateFall_at2
    jmp RichterLandOnMarked
RichterStateFall_at2:
    jsr RichterOnMarkedUnderFeet
    bcc RichterStateFall_at3
    jmp RichterLandOnMarkedFeet
RichterStateFall_at3:
    lda a:OBJ_VY_LO
    clc
    adc #$48
    sta a:OBJ_VY_LO
    lda a:OBJ_VY_HI
    adc #$00
    sta a:OBJ_VY_HI
    bmi RichterStateFall_at4
    cmp #$07
    bcc RichterStateFall_at4
    stz a:OBJ_VY_LO
RichterStateFall_at4:
    lda a:OBJ_SUBSTATE
    cmp #$00
    bne RichterStateFall_at5
    jsr RichterNoteItemPress
    jsr RichterUseItem
    bcc RichterStateFall_at6
    jmp RichterAirAttack
RichterStateFall_at6:
    rts
RichterStateFall_at5:
    jsr RichterAnimationWait
    lda #$12
    sta a:OBJ_FRAME
    stz a:OBJ_SUBSTATE
    stz a:PlayerAttackKind
    rts

; @routine 01BA:1DD RichterFallAttacking
; @run 6D:A9DD
; WHAT IT DOES: The floor goes from under an attack: the attack's animation in the air, then, while
; that runs, a fall at step 1 with Y speed $0300 and the drift after 4 frames; once it has ended, an
; ordinary fall.
; HOW: RichterPadOverridesOff; RichterAirAttackAnimations by PlayerAttackKind into OBJ_ANIMATION;
; RichterAnimationRunning; tail jumps to RichterToFall and RichterStopX.
; WAITS: no.
; Entry: nothing. Exit: falling attacking, A 4, N and Z from it, X the attack kind; Y and C as
; RichterAnimationRunning. Ended, as RichterToFall.
RichterFallAttacking:
    jsr RichterPadOverridesOff
    ldx a:PlayerAttackKind
    lda a:RichterAirAttackAnimations,x
    sta a:OBJ_ANIMATION
    jsr RichterAnimationRunning
    bne RichterFallAttacking_at0
    jmp RichterToFall
RichterFallAttacking_at0:
    lda #$04
    sta a:OBJ_STATE
    lda #$01
    sta a:OBJ_SUBSTATE
    lda #$00
    sta a:OBJ_VY_LO
    lda #$03
    sta a:OBJ_VY_HI
    lda #$04
    sta a:OBJ_WORK_D
    jmp RichterStopX

; @routine 01BA:211 RichterDownOnFirstColumnFloor
; @run 6D:AA11
; WHAT IT DOES: Says whether, with down held and not rising, the columns read have a floor at the
; first column that answers 0. Nothing is known to reach it.
; HOW: TST of zPadHeld; RichterReadColumns, RichterFloorCheck, RichterSolidUnderColumn; the column in X.
; WAITS: no.
; Entry: nothing. Exit: C the answer; A, X, Y, N, Z and V as the last test left them.
RichterDownOnFirstColumnFloor:
    tst #$10,z:zPadHeld
    beq RichterDownOnFirstColumnFloor_at0
    lda a:OBJ_VY_HI
    bmi RichterDownOnFirstColumnFloor_at0
    jsr RichterReadColumns
    jsr RichterFloorCheck
    bcc RichterDownOnFirstColumnFloor_at0
    jsr RichterSolidUnderColumn
    bne RichterDownOnFirstColumnFloor_at0
    txa
    bne RichterDownOnFirstColumnFloor_at0
    sec
    rts
RichterDownOnFirstColumnFloor_at0:
    clc
    rts

; @routine 01BA:256 RichterAirStep1
; @run 6D:AA56
; WHAT IT DOES: The air's step 1, an attack in the air: the pad's turn if he has not turned and his
; scroll band is below 3; while falling, the landings as RichterStateFall makes them and the debris;
; gravity; when the attack's animation ends, frame $12, step 0, no attack.
; HOW: RichterAirControl; the landing check, RichterOnMarkedFloor, RichterOnMarkedUnderFeet,
; RichterDebrisOnMarkedColumns; RichterFallGravity; RichterAnimationWait.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: as the path's last routine; ended, A $12, N and Z from it.
RichterAirStep1:
    lda z:zFieldC6
    bne RichterAirStep1_at0
    lda a:OBJ_ANIMATION_AT
    cmp #$03
    bcs RichterAirStep1_at0
    jsr RichterAirControl
RichterAirStep1_at0:
    lda a:OBJ_VY_HI
    bmi RichterAirStep1_at1
    jsr RichterReadColumns
    jsr RichterFloorCheck
    bcc RichterAirStep1_at2
    jsr RichterSolidUnderColumn
    bne RichterAirStep1_at2
    jmp RichterLandOnColumn
RichterAirStep1_at2:
    jsr RichterOnMarkedFloor
    bcc RichterAirStep1_at3
    jmp RichterLandOnMarked
RichterAirStep1_at3:
    jsr RichterOnMarkedUnderFeet
    bcc RichterAirStep1_at4
    jmp RichterLandOnMarkedFeet
RichterAirStep1_at4:
    jsr RichterDebrisOnMarkedColumns
RichterAirStep1_at1:
    jsr RichterFallGravity
    jsr RichterAnimationWait
    stz a:PlayerAttackKind
    lda #$12
    sta a:OBJ_FRAME
    stz a:OBJ_SUBSTATE
    rts

; @routine 01BA:29E RichterAirStep2
; @run 6D:AA9E
; WHAT IT DOES: The air's step 2: falls with gravity until the animation ends; then script $0C, the
; animation timer $40, no speed, and step 3.
; HOW: RichterFallGravity, RichterAnimationWait, RichterStartHitScript, RichterStopMoving.
; WAITS: no.
; Entry: nothing. Exit: waiting, as RichterAnimationWait; else N and Z from the step, A and Y $0C,
; X 7, C and V as the script left them.
RichterAirStep2:
    jsr RichterFallGravity
    jsr RichterAnimationWait
    lda #$0C
    jsr RichterStartHitScript
    lda #$40
    sta a:OBJ_HURT_TIMER
    jsr RichterStopMoving
    inc a:OBJ_SUBSTATE
    rts

; @routine 01BA:2B5 RichterAirStep3
; @run 6D:AAB5
; WHAT IT DOES: The air's step 3: shows one of two frames by bit 1 of the animation timer and counts
; it down; at its end script 9, OBJ_ANIM_TIMER 1 ($28 unless slot 5 holds kind $11), the item's
; first use set, the item's animation, step 0 (1 when slot 5 holds kind $0D), and the hitbox on.
; HOW: RichterAirStep3Frames; RichterStartHitScript, RichterSetAnimation by RichterAirStep3Animations.
; WAITS: no.
; Entry: nothing. Exit: counting, A the frame, N and Z from the count, Y the frame's index, C bit 0 of
; the timer. Ended, A slot 5's kind, N, Z and C from comparing it with $0D, or N and Z from the step;
; X and Y as RichterSetAnimation left them.
RichterAirStep3:
    lda a:OBJ_HURT_TIMER
    lsr a
    and #$01
    tay
    lda a:RichterAirStep3Frames,y
    sta a:OBJ_FRAME
    dec a:OBJ_HURT_TIMER
    bne RichterAirStep3_at0
    lda #$09
    jsr RichterStartHitScript
    lda #$01
    sta a:OBJ_ANIM_TIMER
    lda a:OBJ_KIND+5
    cmp #$11
    beq RichterAirStep3_at1
    lda #$28
    sta a:OBJ_ANIM_TIMER
RichterAirStep3_at1:
    lda #$01
    sta a:PlayerItemFirstUse
    ldy z:zField93
    lda a:RichterAirStep3Animations,y
    jsr RichterSetAnimation
    stz a:OBJ_SUBSTATE
    lda #$01
    sta a:OBJ_HITBOX_ON
    lda a:OBJ_KIND+5
    cmp #$0D
    bne RichterAirStep3_at0
    inc a:OBJ_SUBSTATE
RichterAirStep3_at0:
    rts

; @routine 01BA:308 RichterAirTurnStill
; @run 6D:AB08
; WHAT IT DOES: In the air with no X speed: as RichterAirTurnFacingLeft when he faces left; facing
; right, right held gives the turn's speed, left held turns him left, counts it and gives it.
; HOW: TSTs of zPadHeld; runs on into RichterAirTurnSpeed after the turn.
; WAITS: no.
; Entry: nothing. Exit: as RichterAirTurnFacingLeft.
RichterAirTurnStill:
    lda a:OBJ_FACING_LEFT
    bne RichterAirTurnFacingLeft
    tst #$20,z:zPadHeld
    bne RichterAirTurnSpeed
    tst #$80,z:zPadHeld
    bne RichterAirTurnStill_at0
    rts
RichterAirTurnStill_at0:
    inc z:zFieldC6
    lda #$01
    sta a:OBJ_FACING_LEFT

; @routine 01BA:31F RichterAirTurnSpeed
; @run 6D:AB1F
; WHAT IT DOES: After a turn in the air, unless attacking (step 1): the turn's X speed by facing.
; HOW: The two words at $AB85.
; WAITS: no.
; Entry: nothing. Exit: attacking, A 1, N, Z and C from comparing the step with 1; else A the speed's
; high byte, N and Z from it, Y twice the facing, C bit 7 of the facing. X and V kept.
RichterAirTurnSpeed:
    lda a:OBJ_SUBSTATE
    cmp #$01
    beq RichterAirTurnSpeed_at0
    lda a:OBJ_FACING_LEFT
    asl a
    tay
    lda a:RichterAirTurnSpeeds,y
    sta a:OBJ_VX_LO
    lda a:RichterAirTurnSpeeds+1,y
    sta a:OBJ_VX_HI
RichterAirTurnSpeed_at0:
    rts

; @routine 01BA:338 RichterAirTurnFacingLeft
; @run 6D:AB38
; WHAT IT DOES: Still in the air facing left: left held gives the turn's speed; right held turns him
; right, counts the turn (zFieldC6) and gives it.
; HOW: TSTs of zPadHeld; RichterAirTurnSpeed.
; WAITS: no.
; Entry: nothing. Exit: neither held, Z set, N and V bits 7 and 6 of zPadHeld; else as
; RichterAirTurnSpeed.
RichterAirTurnFacingLeft:
    tst #$80,z:zPadHeld
    bne RichterAirTurnSpeed
    tst #$20,z:zPadHeld
    bne RichterAirTurnFacingLeft_at0
    rts
RichterAirTurnFacingLeft_at0:
    inc z:zFieldC6
    stz a:OBJ_FACING_LEFT
    jmp RichterAirTurnSpeed

; @routine 01BA:34B RichterAirControl
; @run 6D:AB4B
; WHAT IT DOES: The pad's turn in the air: with no X speed, as RichterAirTurnStill; moving one way
; while facing the other, the pad held the way he moves turns him to face it, counts the turn
; (zFieldC6), and with I held gives the turn's speed.
; HOW: OBJ_VX_HI and LO; TSTs of zPadHeld; a jump to RichterAirTurnSpeed.
; WAITS: no.
; Entry: nothing. Exit: as the path it took; N, Z and V from the last TST where it ends on one.
RichterAirControl:
    lda a:OBJ_VX_HI
    ora a:OBJ_VX_LO
    beq RichterAirTurnStill
    lda a:OBJ_VX_HI
    bmi RichterAirControl_at0
    lda a:OBJ_FACING_LEFT
    beq RichterAirControl_at1
    rts
RichterAirControl_at1:
    tst #$80,z:zPadHeld
    bne RichterAirControl_at2
    rts
RichterAirControl_at2:
    lda #$01
    sta a:OBJ_FACING_LEFT
    bra RichterAirControl_at3
RichterAirControl_at0:
    lda a:OBJ_FACING_LEFT
    bne RichterAirControl_at4
    rts
RichterAirControl_at4:
    tst #$20,z:zPadHeld
    bne RichterAirControl_at5
    rts
RichterAirControl_at5:
    stz a:OBJ_FACING_LEFT
RichterAirControl_at3:
    inc z:zFieldC6
    tst #$01,z:zPadHeld
    bne RichterAirControl_at6
    rts
RichterAirControl_at6:
    jmp RichterAirTurnSpeed

; @routine 01BA:3A4 RichterOnMarkedUnderFeet
; @run 6D:ABA4
; WHAT IT DOES: RichterOnMarkedFloor for the ground under his feet: once he is $10 down the screen, a
; read under his soles with bit 5, or with bit 2 while down is held.
; HOW: OBJ_Y_LO; RichterReadUnderFeet's answer; bit 4 of zPadHeld; its CLC and SEC are
; RichterOnMarkedFloor's own ($ABA0, $ABA2).
; WAITS: no.
; Entry: nothing. Exit: C set if marked; A, N, Z and V as RichterOnMarkedFloor leaves them, from the
; read's answer; X and Y as RichterReadUnderFeet left them.
RichterOnMarkedUnderFeet:
    lda a:OBJ_Y_LO
    cmp #$10
    bcc RichterOnMarkedFloor_at0
    jsr RichterReadUnderFeet
    and #$24
    beq RichterOnMarkedFloor_at0
    and #$20
    bne RichterOnMarkedUnderFeet_at0
    tst #$10,z:zPadHeld
    beq RichterOnMarkedFloor_at0
RichterOnMarkedUnderFeet_at0:
    bra RichterOnMarkedFloor_at1

; @routine 01BA:3BD RichterStateAir
; @run 6D:ABBD
; WHAT IT DOES: States 3, 11 and 12, in the air: runs the step's handler.
; HOW: The four words at $ABC3 by RichterStepIndex.
; WAITS: no.
; Entry: Y for the sound driver. Exit: as the handler leaves them.
RichterStateAir:
    jsr RichterStepIndex
    jmp (RichterAirSteps,x)

; @routine 01BA:3E6 RichterAirStep0
; @run 6D:ABE6
; WHAT IT DOES: The air's step 0: in a plain jump not carrying him the way he faces, with his scroll band
; below 2 (band 1, or the animation timer below 8), a new press of I is a knock-back (RichterKnockBack);
; otherwise the pad's turn if he has not turned, the landings and the debris while falling as
; RichterAirStep1, gravity, the animation, and the item's use (an attack in the air).
; HOW: PlayerJumpKind; bit 0 of OBJ_VX_HI's shifts against the facing; OBJ_ANIMATION_AT; OBJ_HURT_TIMER at X;
; zPadPressed; then as RichterAirStep1's checks, RichterAnimationTick, RichterNoteItemPress, RichterUseItem.
; WAITS: no.
; Entry: X the step's offset (0); X and Y for the sound driver. Exit: as the path's last routine.
RichterAirStep0:
    lda a:PlayerJumpKind
    bne RichterAirStep0_at0
    lda a:OBJ_VX_HI
    beq RichterAirStep0_at1
    lsr a
    ror a
    eor a:OBJ_FACING_LEFT
    and #$01
    beq RichterAirStep0_at0
RichterAirStep0_at1:
    lda a:OBJ_ANIMATION_AT
    cmp #$02
    bcs RichterAirStep0_at0
    dec a
    beq RichterAirStep0_at2
    lda a:OBJ_HURT_TIMER,x
    cmp #$08
    bcs RichterAirStep0_at0
RichterAirStep0_at2:
    lda z:zPadPressed
    and #$01
    beq RichterAirStep0_at0
    jmp RichterKnockBack
RichterAirStep0_at0:
    lda z:zFieldC6
    bne RichterAirStep0_at3
    jsr RichterAirControl
RichterAirStep0_at3:
    lda a:OBJ_VY_HI
    bmi RichterAirStep0_at4
    jsr RichterReadColumns
    jsr RichterFloorCheck
    bcc RichterAirStep0_at5
    jsr RichterSolidUnderColumn
    bne RichterAirStep0_at5
    jmp RichterLandOnColumn
RichterAirStep0_at5:
    jsr RichterOnMarkedFloor
    bcc RichterAirStep0_at6
    jmp RichterLandOnMarked
RichterAirStep0_at6:
    jsr RichterOnMarkedUnderFeet
    bcc RichterAirStep0_at7
    jmp RichterLandOnMarkedFeet
RichterAirStep0_at7:
    jsr RichterDebrisOnMarkedColumns
RichterAirStep0_at4:
    jsr RichterFallGravity
    jsr RichterAnimationTick
    jsr RichterNoteItemPress
    jsr RichterUseItem
    bcs RichterAirAttack
    rts

; @routine 01BA:451 RichterLandOnColumn
; @run 6D:AC51
; WHAT IT DOES: Lands on the floor at a column: lifted out of it, PlayerWhipStruck cleared, then as
; RichterLand.
; HOW: RichterLiftByColumn; runs on into RichterLand.
; WAITS: no.
; Entry: X the column; Y for the sound driver. Exit: as RichterLand.
RichterLandOnColumn:
    jsr RichterLiftByColumn
    stz a:PlayerWhipStruck

; @routine 01BA:457 RichterLand
; @run 6D:AC57
; WHAT IT DOES: Lands: hard at full fall speed; attacking, as RichterLandAttacking; out of a jump the way
; he faced, or any plain jump, he stands; out of a knock-back, hard.
; HOW: OBJ_VY_HI against 7; OBJ_SUBSTATE against 1; PlayerJumpKind's bit 7, then not zero; jumps to
; RichterLandHard, RichterLandAttacking, RichterStand.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: as the routine it goes to.
RichterLand:
    lda a:OBJ_VY_HI
    cmp #$07
    beq RichterLandHard
    lda a:OBJ_SUBSTATE
    cmp #$01
    bne RichterLand_at0
    jmp RichterLandAttacking
RichterLand_at0:
    lda a:PlayerJumpKind
    bpl RichterLand_at1
    jmp RichterStand
RichterLand_at1:
    bne RichterLandHard
    jmp RichterStand

; @routine 01BA:475 RichterAirAttack
; @run 6D:AC75
; WHAT IT DOES: Starts an attack in the air: its animation by the attack's kind, then as
; RichterAttackWithAnimation.
; HOW: RichterPadOverridesOff; RichterAirAttackAnimations; a jump to RichterAttackWithAnimation.
; WAITS: no.
; Entry: nothing. Exit: as RichterAttackWithAnimation.
RichterAirAttack:
    jsr RichterPadOverridesOff
    ldx a:PlayerAttackKind
    lda a:RichterAirAttackAnimations,x
    jmp RichterAttackWithAnimation

; @routine 01BA:485 RichterLandHard
; @run 6D:AC85
; WHAT IT DOES: A hard landing, at full fall speed or out of a knock-back: slots 1 and 2 to kind 4; an
; attack lands as RichterLandAttacking; else sound 2 and the landing crouch (state 10, animation 7), a
; knock-back sliding on backward unless pushed; at step 1 (an attack's step), the crouch's animation
; while it runs, then a plain crouch (state 2) with down forced for 16 frames.
; HOW: RichterPadOverridesOff, RichterReplaceSlotsOneTwo, SoundCommand, RichterSetAnimation; bit 0 of
; PlayerJumpKind; TST of zPlayerPush; RichterKnockSpeedX by facing; RichterAnimationRunning.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: as the last routine it calls; the crouch, A $40, N and Z
; from it.
RichterLandHard:
    jsr RichterPadOverridesOff
    jsr RichterReplaceSlotsOneTwo
    lda a:PlayerAttackKind
    bne RichterLandAttacking
    lda #$02
    jsr VecSoundCommand
    lda a:OBJ_SUBSTATE
    cmp #$01
    beq RichterLandHard_at0
RichterLandHard_at3:
    lda #$0A
    sta a:OBJ_STATE
    lda #$00
    sta a:OBJ_SUBSTATE
    lda #$07
    jsr RichterSetAnimation
    lda a:PlayerJumpKind
    stz a:PlayerJumpKind
    and #$01
    bne RichterLandHard_at1
    jmp RichterStopMoving
RichterLandHard_at1:
    tst #$01,z:zPlayerPush
    beq RichterLandHard_at2
    jmp RichterStopMoving
RichterLandHard_at2:
    stz a:OBJ_VY_LO
    stz a:OBJ_VY_HI
    lda a:OBJ_FACING_LEFT
    jmp RichterKnockSpeedX
RichterLandHard_at0:
    lda #$06
    sta a:OBJ_ANIMATION
    jsr RichterAnimationRunning
    beq RichterLandHard_at3
    lda #$02
    sta a:OBJ_STATE
    jsr RichterStopMoving
    lda #$10
    sta a:OBJ_WORK_E
    lda #$40
    sta a:OBJ_WORK_F
    rts

; @routine 01BA:4E9 RichterLandAttacking
; @run 6D:ACE9
; WHAT IT DOES: Lands while attacking: state 0, the attack going on in the standing attack's
; animation for its kind; stands when it has ended, else stops.
; HOW: RichterPadOverridesOff, RichterLandAttackAnimations into OBJ_ANIMATION, RichterAnimationRunning;
; tail jumps to RichterStopMoving and RichterStand.
; WAITS: no.
; Entry: nothing. Exit: as RichterAnimationRunning, X the attack kind; ended, as RichterStand.
RichterLandAttacking:
    jsr RichterPadOverridesOff
    stz a:PlayerJumpKind
    lda #$00
    sta a:OBJ_STATE
    ldx a:PlayerAttackKind
    lda a:RichterLandAttackAnimations,x
    sta a:OBJ_ANIMATION
    jsr RichterAnimationRunning
    bne RichterLandAttacking_at0
    jmp RichterStand
RichterLandAttacking_at0:
    jmp RichterStopMoving

; @routine 01BA:514 RichterLandOnMarkedFeet
; @run 6D:AD14
; WHAT IT DOES: Lands on the mark under his feet: his Y set 8 below where the read met it, then as
; RichterLandOnMarked from its sound on.
; HOW: zTemp02, the read's row; a branch into RichterLandOnMarked at $AD2C.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: as RichterLandOnMarked.
RichterLandOnMarkedFeet:
    lda a:OBJ_Y_LO
    clc
    adc #$08
    sec
    sbc z:zTemp02
    sta a:OBJ_Y_LO
    bra RichterLandOnMarked_at0

; @routine 01BA:522 RichterLandOnMarked
; @run 6D:AD22
; WHAT IT DOES: Lands on a marked floor: stands on it (the first column's row), sounds the landing when
; knocked back or at full fall speed (and forces the pad for 6 frames), lets him go above the map, stops
; him; then a mark below him a step to either side puts him on the climb that way, at its tile's edge,
; going on with the attack if he was attacking; otherwise an ordinary landing.
; HOW: RichterColumnRow; RichterReplaceSlotsOneTwo and SoundCommand 2; RichterReadBelow at his X plus
; and less 8, and $0E and $1E below; RichterMoveStopped by RichterClimbLandStepsRight or Left; the
; RichterClimbLand tables; jumps to RichterLand.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: as RichterLand, or the climb's last store, or as
; RichterAnimationTick when attacking.
RichterLandOnMarked:
    lda a:OBJ_Y_LO
    sec
    sbc a:RichterColumnRow
    sta a:OBJ_Y_LO
RichterLandOnMarked_at0:
    lda a:PlayerJumpKind
    and #$01
    bne RichterLandOnMarked_at1
    lda a:OBJ_VY_HI
    cmp #$07
    bne RichterLandOnMarked_at2
RichterLandOnMarked_at1:
    jsr RichterReplaceSlotsOneTwo
    lda #$02
    jsr VecSoundCommand
    lda #$06
    sta a:OBJ_WORK_E
    stz a:OBJ_WORK_F
RichterLandOnMarked_at2:
    stz a:PlayerJumpKind
    lda #$01
    sta a:PlayerMayLeaveTop
    stz a:PlayerJumpKind
    stz a:PlayerClimbLandFacing
    jsr RichterStopMoving
    stz a:OBJ_X_FRAC
    stz a:OBJ_Y_FRAC
    lda a:OBJ_X_LO
    clc
    adc #$08
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$0E
    jsr RichterReadBelow
    and #$04
    beq RichterLandOnMarked_at3
RichterLandOnMarked_at7:
    lda a:PlayerClimbLandFacing
    beq RichterLandOnMarked_at4
    jmp RichterLand
RichterLandOnMarked_at4:
    lda a:OBJ_SUBSTATE
    cmp #$01
    beq RichterLandOnMarked_at5
    ldx a:RichterColumnX
    lda a:RichterClimbLandStepsRight,x
    jsr RichterMoveStopped
RichterLandOnMarked_at9:
    lda a:RichterClimbLandFrames,x
    sta a:OBJ_FRAME
    lda a:RichterClimbLandStates,x
    sta a:OBJ_STATE
    lda a:RichterClimbLandAnimations,x
    sta a:OBJ_ANIMATION
    rts
RichterLandOnMarked_at3:
    lda a:OBJ_X_LO
    clc
    adc #$F8
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$0E
    jsr RichterReadBelow
    and #$04
    bne RichterLandOnMarked_at6
    lda a:OBJ_FACING_LEFT
    sta a:PlayerClimbLandFacing
    lda a:OBJ_X_LO
    clc
    adc #$F8
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$1E
    jsr RichterReadBelow
    and #$04
    bne RichterLandOnMarked_at7
    lda a:OBJ_FACING_LEFT
    eor #$01
    beq RichterLandOnMarked_at6
    jmp RichterLand
RichterLandOnMarked_at6:
    lda a:OBJ_SUBSTATE
    cmp #$01
    beq RichterLandOnMarked_at8
    ldx a:RichterColumnX
    lda a:RichterClimbLandStepsLeft,x
    bsr RichterMoveStopped
    inx
    inx
    bra RichterLandOnMarked_at9
RichterLandOnMarked_at5:
    ldx a:RichterColumnX
    lda a:RichterClimbLandStepsRight,x
    clc
    adc a:OBJ_X_LO
    sta a:OBJ_X_LO
    ldx a:OBJ_FACING_LEFT
RichterLandOnMarked_at10:
    lda a:RichterClimbLandStates,x
    sta a:OBJ_STATE
    sec
    sbc #$05
    lsr a
    sta z:zTemp00
    lda a:PlayerAttackKind
    asl a
    clc
    adc z:zTemp00
    tax
    lda a:RichterClimbLandAttackAnimations,x
    sta a:OBJ_ANIMATION
    jmp RichterAnimationTick
RichterLandOnMarked_at8:
    ldx a:RichterColumnX
    lda a:RichterClimbLandStepsLeft,x
    clc
    adc a:OBJ_X_LO
    sta a:OBJ_X_LO
    ldx a:OBJ_FACING_LEFT
    inx
    inx
    bra RichterLandOnMarked_at10

; @routine 01BA:65E RichterStateLandCrouch
; @run 6D:AE5E
; WHAT IT DOES: State 10, the crouch a hard landing ends in: falls if the footing goes; else stands on
; the floor (stopped at a ledge) and, when the animation ends, crouches (state 2, step 0).
; HOW: RichterReadColumns, RichterHasNoFooting, RichterToFall (no drift); RichterLandOnFloor,
; RichterLedgeCheck, RichterStopX, RichterAnimationWait.
; WAITS: no.
; Entry: nothing. Exit: fell, A $FF, N set, Z clear, the rest as RichterToFall. Waiting, as
; RichterAnimationWait. Crouched, A 0, Z set; the rest as RichterAnimationWait.
RichterStateLandCrouch:
    jsr RichterReadColumns
    jsr RichterHasNoFooting
    bcs RichterStateLandCrouch_at0
    jsr RichterLandOnFloor
    jsr RichterLedgeCheck
    bcc RichterStateLandCrouch_at1
    jsr RichterStopX
RichterStateLandCrouch_at1:
    jsr RichterAnimationWait
    jsr RichterStopX
    lda #$02
    sta a:OBJ_STATE
    lda #$00
    sta a:OBJ_SUBSTATE
    rts
RichterStateLandCrouch_at0:
    jsr RichterToFall
    lda #$FF
    sta a:OBJ_WORK_D
    rts

; @routine 01BA:68B RichterDebrisOnMarkedColumns
; @run 6D:AE8B
; WHAT IT DOES: When a column read under him has bit 3: sound $23; each of slots 2 down to 1 holding
; kind 1 emptied, with script 8; a burst of debris 16 pixels below him.
; HOW: RichterColumnHit's three bytes ORed; SoundCommand and ClearObject through their kernel
; vectors, RichterStartHitScript. The script leaves X 7, so the count goes on down from 6 after it.
; His Y is pushed, moved down for RichterStartDebrisBurst with timer 0, and pulled back.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: no bit 3: A 0, N clear, Z set; X, Y, C and V kept.
; Otherwise A his Y low byte, N and Z from it; X 0; Y 8 if a slot was emptied, else kept; C as
; RichterStartDebrisBurst; V from adding the carry to his Y high byte.
RichterDebrisOnMarkedColumns:
    lda a:RichterColumnHit
    ora a:RichterColumnHit+1
    ora a:RichterColumnHit+2
    and #$08
    bne RichterDebrisOnMarkedColumns_at0
    rts
RichterDebrisOnMarkedColumns_at0:
    lda #$23
    jsr VecSoundCommand
    ldx #$02
RichterDebrisOnMarkedColumns_at2:
    lda a:OBJ_KIND,x
    dec a
    bne RichterDebrisOnMarkedColumns_at1
    jsr VecClearObject
    lda #$08
    jsr RichterStartHitScript
RichterDebrisOnMarkedColumns_at1:
    dex
    bne RichterDebrisOnMarkedColumns_at2
    lda a:OBJ_Y_LO
    pha
    clc
    adc #$10
    sta a:OBJ_Y_LO
    lda a:OBJ_Y_HI
    pha
    adc #$00
    sta a:OBJ_Y_HI
    cla
    bsr RichterStartDebrisBurst
    pla
    sta a:OBJ_Y_HI
    pla
    sta a:OBJ_Y_LO
    rts
