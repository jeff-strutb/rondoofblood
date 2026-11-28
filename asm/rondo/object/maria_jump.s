; @routine 01DE:1F0 MariaGravityLight
; @run 6D:A9F0
; WHAT IT DOES: Light gravity: her Y speed $0030 more, held to $07xx on the way down (the low byte cleared there).
; HOW: A 16-bit add; a high byte of 7 or more, not negative, clears the low.
; WAITS: no.
; Entry: nothing. Exit: A the high byte, N and Z from it, or from comparing it with 7; C the add's, or the compare's;
; V the add's. X and Y kept.
MariaGravityLight:
    lda a:OBJ_VY_LO
    clc
    adc #$30
    sta a:OBJ_VY_LO
    lda a:OBJ_VY_HI
    adc #$00
    sta a:OBJ_VY_HI
    bmi MariaGravityLight_at0
    cmp #$07
    bcc MariaGravityLight_at0
    stz a:OBJ_VY_LO
MariaGravityLight_at0:
    rts

; @routine 01DE:241 MariaFeetOnMarkedFloor
; @run 6D:AA41
; WHAT IT DOES: Whether her feet land on a marked floor: below row $10, under her feet a mark of bit 5, or of bit 2
; with up held, and the feet's row in it (zTemp02) under 3: C set. A row of 3 or more starts her falling at $0300
; instead, C clear.
; HOW: MariaReadUnderFeet; the tests; the tails of MariaOnMarkedFloor (SEC or CLC, RTS).
; WAITS: no.
; Entry: nothing. Exit: A the last value loaded or tested, N and Z from it or its compare; C the answer; Y and V as
; MariaReadUnderFeet left them.
MariaFeetOnMarkedFloor:
    lda a:OBJ_Y_LO
    cmp #$10
    bcc MariaOnMarkedFloor_at0
    jsr MariaReadUnderFeet
    and #$24
    beq MariaOnMarkedFloor_at0
    and #$20
    bne MariaFeetOnMarkedFloor_at0
    tst #$10,z:zPadHeld
    beq MariaOnMarkedFloor_at0
MariaFeetOnMarkedFloor_at0:
    lda z:zTemp02
    cmp #$03
    bcc MariaFeetOnMarkedFloor_at1
    bra MariaOnMarkedFloor_at1
MariaFeetOnMarkedFloor_at1:
    lda #$03
    sta a:OBJ_VY_HI
    stz a:OBJ_VY_LO
    bra MariaOnMarkedFloor_at0

; @routine 01DE:26A MariaStateJump
; @run 6D:AA6A
; WHAT IT DOES: States 3, 11 and 12, the jump: a turn in the air until one is counted (zFieldC6); on the way down, the
; landings (floor, marked floor, marked floor under her feet) and the bit-3 burst; the jump's gravity; her animation;
; the item press noted; the attack button; in step 0, the jump pressed again is her second jump.
; HOW: MariaAirTurn; the Y speed's sign; MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn's Z, MariaOnMarkedFloor,
; MariaFeetOnMarkedFloor, MariaCheckBit3Under; MariaJumpGravity, MariaAnimationTick, MariaNoteItemPress, MariaUseItem,
; MariaJumpPressed; each ending in its tail JMP.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the routine it ended in; else as the last check made.
MariaStateJump:
    lda z:zFieldC6
    bne MariaStateJump_at0
    jsr MariaAirTurn
MariaStateJump_at0:
    lda a:OBJ_VY_HI
    bmi MariaStateJump_at1
    jsr MariaReadColumns
    jsr MariaFloorCheck
    bcc MariaStateJump_at2
    jsr MariaSolidUnderColumn
    bne MariaStateJump_at2
    jmp MariaJumpLand
MariaStateJump_at2:
    jsr MariaOnMarkedFloor
    bcc MariaStateJump_at3
    jmp MariaLandOnMarked
MariaStateJump_at3:
    jsr MariaFeetOnMarkedFloor
    bcc MariaStateJump_at4
    jmp MariaLandOnMarkedFeet
MariaStateJump_at4:
    jsr MariaCheckBit3Under
MariaStateJump_at1:
    jsr MariaJumpGravity
    jsr MariaAnimationTick
    jsr MariaNoteItemPress
    jsr MariaUseItem
    bcc MariaStateJump_at5
    jmp MariaAttack
MariaStateJump_at5:
    lda a:OBJ_SUBSTATE
    bne MariaStateJump_at6
    jsr MariaJumpPressed
    bcc MariaStateJump_at6
    jmp MariaSecondJump
MariaStateJump_at6:
    rts

; @routine 01DE:2B8 MariaJumpLand
; @run 6D:AAB8
; WHAT IT DOES: Lands her on the floor her column found: lifted onto it; with a mark above her, the walls tried as in
; state $0D; then a soft landing stands her, a hard one (Y speed $06xx or more, or the mark still above) is state $0A
; with animation $10, slots 1 and 2 replaced as kind 5 and sound 2, and no speed.
; HOW: MariaLiftByColumn, MariaMarkedAbove's Z, PlayerWallLeft and PlayerWallRight; MariaStand by its tail JMP, or
; MariaPadForcesOff, MariaReplaceSlotsOneTwoKind5, SoundCommand, MariaSetAnimation and the tail JMP to MariaStopMoving.
; WAITS: no.
; Entry: X the column, Y the caller's. Exit: as MariaStand, or MariaSetAnimation.
MariaJumpLand:
    jsr MariaLiftByColumn
    jsr MariaMarkedAbove
    beq MariaJumpLand_soft
    lda #$0D
    sta a:OBJ_STATE
    jsr VecPlayerWallLeft
    jsr VecPlayerWallRight
    jsr MariaMarkedAbove
    bne MariaJumpLand_at0
MariaJumpLand_soft:
    lda a:OBJ_VY_HI
    cmp #$06
    bcs MariaJumpLand_at0
    jmp MariaStand
MariaJumpLand_at0:
    jsr MariaPadForcesOff
    jsr MariaReplaceSlotsOneTwoKind5
    lda #$02
    jsr VecSoundCommand
    lda #$0A
    sta a:OBJ_STATE
    lda #$00
    sta a:OBJ_SUBSTATE
    lda #$10
    jsr MariaSetAnimation
    jmp MariaStopMoving

; @routine 01DE:2F7 MariaLandOnMarkedFeet
; @run 6D:AAF7
; WHAT IT DOES: Lands her on the marked floor under her feet: her Y moved by 8 less zTemp02 (the row the feet's read
; left), then onto a ledge's hold.
; HOW: The add and subtract, then a branch into MariaLandOnMarked's landing.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaLandOnMarked.
MariaLandOnMarkedFeet:
    lda a:OBJ_Y_LO
    clc
    adc #$08
    sec
    sbc z:zTemp02
    sta a:OBJ_Y_LO
    bra MariaLandOnMarked_tail

; @routine 01DE:305 MariaLandOnMarked
; @run 6D:AB05
; WHAT IT DOES: Lands her on a marked floor: her Y up by column 0's row in its tile, then onto a ledge's hold.
; HOW: A subtract of RichterColumnRow; the landing on a mark (MariaLandOnMarked_tail on).
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the hold's last read or its Y's subtract.
MariaLandOnMarked:
    lda a:OBJ_Y_LO
    sec
    sbc a:RichterColumnRow
    sta a:OBJ_Y_LO
MariaLandOnMarked_tail:
    lda a:OBJ_VY_HI
    cmp #$07
    bne MariaLandOnMarked_at0
    jsr MariaReplaceSlotsOneTwoKind5
    lda #$02
    jsr VecSoundCommand
MariaLandOnMarked_at0:
    lda #$01
    sta a:PlayerMayLeaveTop
    jsr MariaStopMoving
    stz a:OBJ_X_FRAC
    stz a:OBJ_Y_FRAC
    lda a:OBJ_X_LO
    clc
    adc #$08
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$0E
    jsr MariaReadBelow
    and #$04
    beq MariaLandOnMarkedBack
MariaLandOnMarked_ahead:
    ldx a:RichterColumnX
    lda a:MariaLedgeNudgeX,x
    clc
    adc a:OBJ_X_LO
    sta a:OBJ_X_LO
    lda a:MariaLedgeNudgeY,x
    clc
    adc a:OBJ_Y_LO
    phx
    bsr MariaLedgeHoldAt
    bsr MariaLedgePose
    plx
    jsr MariaLedgeFloorBelow
    bne MariaLandOnMarked_at1
    lda a:OBJ_Y_LO
    sec
    sbc a:MariaLedgeNudgeY,x
    sta a:OBJ_Y_LO
MariaLandOnMarked_at1:
    rts

; @routine 01DE:36B MariaLedgePose
; @run 6D:AB6B
; WHAT IT DOES: Gives her the hold on a ledge for a pose: frame, state and OBJ_ANIMATION from her tables.
; HOW: MariaLedgeFrames, MariaLedgeStates, MariaLedgeAnimations by X.
; WAITS: no.
; Entry: X the pose (her facing, plus 2 for the ledge behind her). Exit: A the animation, N and Z from it. X, Y, C and V
; kept.
MariaLedgePose:
    lda a:MariaLedgeFrames,x
    sta a:OBJ_FRAME
    lda a:MariaLedgeStates,x
    sta a:OBJ_STATE
    lda a:MariaLedgeAnimations,x
    sta a:OBJ_WORK_A
    rts

; @routine 01DE:37E MariaLedgeHoldAt
; @run 6D:AB7E
; WHAT IT DOES: Puts her Y at the ledge's, her animation stopped and step 0, and loads her facing.
; HOW: The stores, then LDX of OBJ_FACING_LEFT.
; WAITS: no.
; Entry: A her new Y's low byte. Exit: A $FF, X the facing, N and Z from it. Y, C and V kept.
MariaLedgeHoldAt:
    sta a:OBJ_Y_LO
    lda #$FF
    sta a:OBJ_HURT_TIMER
    stz a:OBJ_SUBSTATE
    ldx a:OBJ_FACING_LEFT
    rts

; @routine 01DE:38D MariaLandOnMarkedBack
; @run 6D:AB8D
; WHAT IT DOES: The landing on a mark when nothing is 8 across from her: the hold of the ledge behind her where the
; point 8 back and $0E below reads bit 2; else of the ledge ahead where 8 back and $1E below does; else the one
; behind.
; HOW: Two reads by MariaReadBelow; the hold behind (poses 2 and 3, MariaLedgeNudgeXBehind and MariaLedgeNudgeYBehind),
; or a branch into MariaLandOnMarked's hold ahead.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the hold's last read or its Y's subtract.
MariaLandOnMarkedBack:
    lda a:OBJ_X_LO
    clc
    adc #$F8
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$0E
    jsr MariaReadBelow
    and #$04
    bne MariaLandOnMarkedBack_at0
    lda a:OBJ_X_LO
    clc
    adc #$F8
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$1E
    jsr MariaReadBelow
    and #$04
    bne MariaLandOnMarked_ahead
MariaLandOnMarkedBack_at0:
    ldx a:RichterColumnX
    lda a:MariaLedgeNudgeXBehind,x
    clc
    adc a:OBJ_X_LO
    sta a:OBJ_X_LO
    lda a:MariaLedgeNudgeYBehind,x
    clc
    adc a:OBJ_Y_LO
    phx
    bsr MariaLedgeHoldAt
    inx
    inx
    bsr MariaLedgePose
    plx
    bsr MariaLedgeFloorBelow
    bne MariaLandOnMarkedBack_at1
    lda a:OBJ_Y_LO
    sec
    sbc a:MariaLedgeNudgeYBehind,x
    sta a:OBJ_Y_LO
MariaLandOnMarkedBack_at1:
    rts

; @routine 01DE:3E2 MariaLedgeFloorBelow
; @run 6D:ABE2
; WHAT IT DOES: Reads the terrain at her X, $16 rows below her, for bit 2.
; HOW: zTemp00 her X; MariaReadBelow with her Y plus $16 and its carry; AND #4.
; WAITS: no.
; Entry: X and Y the caller's (the read's handler is entered with them). Exit: A bit 2 of the read, N and Z from it;
; Y, C and V as MariaReadBelow left them. X kept.
MariaLedgeFloorBelow:
    lda a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$16
    jsr MariaReadBelow
    and #$04
    rts

; @routine 01DE:41F MariaStateLanded
; @run 6D:AC1F
; WHAT IT DOES: State $0A, after a hard landing: the columns under her; no footing, she falls; a landing; while her
; animation runs, nothing more; when it has stopped, she crouches (state 2, step 0, frame $1B).
; HOW: MariaReadColumns, MariaHasNoFooting and the JMP to MariaToFall, MariaLandOnFloor, MariaAnimationWait.
; WAITS: no.
; Entry: nothing. Exit: as the routine it ended in; else A 0, Z set.
MariaStateLanded:
    jsr MariaReadColumns
    jsr MariaHasNoFooting
    bcc MariaStateLanded_footing
    jmp MariaToFall
MariaStateLanded_footing:
    jsr MariaLandOnFloor
    jsr MariaAnimationWait
    lda #$1B
    sta a:OBJ_FRAME
    lda #$02
    sta a:OBJ_STATE
    lda #$00
    sta a:OBJ_SUBSTATE
    rts

; @routine 01DE:440 MariaJumpGravity
; @run 6D:AC40
; WHAT IT DOES: Her jump's gravity, except in the hover: animation $11 on the way down in its first four steps.
; HOW: OBJ_ANIMATION, the Y speed's sign and OBJ_SCROLL_BAND (the animation's step); the tail JMP to MariaFallGravity.
; WAITS: no.
; Entry: nothing. Exit: hovering, A the step, N, Z and C from comparing it with 4; else as MariaFallGravity.
MariaJumpGravity:
    lda a:OBJ_WORK_A
    cmp #$11
    bne MariaJumpGravity_at0
    lda a:OBJ_VY_HI
    bmi MariaJumpGravity_at0
    lda a:OBJ_SCROLL_BAND
    cmp #$04
    bcs MariaJumpGravity_at0
    rts
MariaJumpGravity_at0:
    jmp MariaFallGravity

; @routine 01DE:457 MariaSecondJump
; @run 6D:AC57
; WHAT IT DOES: Her second jump in the air: slots 11 to 14 claimed (MariaClaimSlotsB), the turn count cleared, state 3
; step 4, Y speed $FD00 up, animation $0C; with right or left held she faces that way and takes the walking speed,
; else animation $14.
; HOW: MariaClaimSlotsB, MariaStopMoving, MariaSetAnimation; BBS5 and BBS7 of zPadHeld (two reads); the tail JMPs to
; MariaSetAnimation and MariaSpeedWalk.
; WAITS: no.
; Entry: nothing. Exit: as MariaSetAnimation with $14, or MariaSpeedWalk.
MariaSecondJump:
    jsr MariaClaimSlotsB
    stz z:zFieldC6
    lda #$03
    sta a:OBJ_STATE
    lda #$04
    sta a:OBJ_SUBSTATE
    jsr MariaStopMoving
    lda #$00
    sta a:OBJ_VY_LO
    lda #$FD
    sta a:OBJ_VY_HI
    lda #$0C
    jsr MariaSetAnimation
    cly
    bbs5 zPadHeld,MariaSecondJump_to1
    iny
    bbs7 zPadHeld,MariaSecondJump_to1
    lda #$14
    jmp MariaSetAnimation
MariaSecondJump_to1:
    sty a:OBJ_FACING_LEFT
    jmp MariaSpeedWalk

; @routine 01DE:4A0 MariaStateFall
; @run 6D:ACA0
; WHAT IT DOES: State 4, the fall: a floor her column stands on lands her; when the drift delay (OBJ_FALL_DRIFT_DELAY)
; runs out she takes the drift's speed; the marked-floor landings, the bit-3 burst; gravity $0048, held at $07xx; her
; animation; in step 0 the item press, the attack button and the second jump.
; HOW: MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn; DEC of the delay, MariaFallDriftSpeeds; MariaOnMarkedFloor,
; MariaFeetOnMarkedFloor, MariaCheckBit3Under; the 16-bit add; MariaAnimationTick, MariaNoteItemPress, MariaUseItem,
; MariaJumpPressed; each ending in its tail JMP.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the routine it ended in; else as the last check made.
MariaStateFall:
    jsr MariaReadColumns
    jsr MariaFloorCheck
    bcc MariaStateFall_drift
    jsr MariaSolidUnderColumn
    bne MariaStateFall_drift
    jmp MariaJumpLand
MariaStateFall_drift:
    lda a:OBJ_WORK_D
    beq MariaStateFall_at0
    dec a:OBJ_WORK_D
    bne MariaStateFall_at0
    lda a:OBJ_FACING_LEFT
    asl a
    tay
    lda a:MariaFallDriftSpeeds,y
    sta a:OBJ_VX_LO
    lda a:MariaFallDriftSpeeds+1,y
    sta a:OBJ_VX_HI
MariaStateFall_at0:
    jsr MariaOnMarkedFloor
    bcc MariaStateFall_at1
    jmp MariaLandOnMarked
MariaStateFall_at1:
    jsr MariaFeetOnMarkedFloor
    bcc MariaStateFall_at2
    jmp MariaLandOnMarkedFeet
MariaStateFall_at2:
    jsr MariaCheckBit3Under
    lda a:OBJ_VY_LO
    clc
    adc #$48
    sta a:OBJ_VY_LO
    lda a:OBJ_VY_HI
    adc #$00
    sta a:OBJ_VY_HI
    bmi MariaStateFall_at3
    cmp #$07
    bcc MariaStateFall_at3
    stz a:OBJ_VY_LO
MariaStateFall_at3:
    jsr MariaAnimationTick
    lda a:OBJ_SUBSTATE
    cmp #$00
    bne MariaStateFall_at4
    jsr MariaNoteItemPress
    jsr MariaUseItem
    bcc MariaStateFall_at5
    jmp MariaAttack
MariaStateFall_at5:
    jsr MariaJumpPressed
    bcc MariaStateFall_at4
    jmp MariaSecondJump
MariaStateFall_at4:
    rts
