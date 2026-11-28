; @routine 01DE:51A MariaStateClimbLow
; @run 6D:AD1A
; WHAT IT DOES: States 5 and 6, the hold on the ledge ahead: the step's handler from MariaClimbLowSteps (hold, wait,
; across, turn, up, the item crash).
; HOW: MariaStepIndex, then the JMP (table,X).
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the step's handler, after A and X the step doubled and C its bit 7.
MariaStateClimbLow:
    jsr MariaStepIndex
    jmp (MariaClimbLowSteps,x)

; @routine 01DE:52C MariaClimbMove
; @run 6D:AD2C
; WHAT IT DOES: On a ledge: stopped, unless her animation has just gone to a new step, when she moves by her state's
; speeds ($0200 or $FE00 across and down).
; HOW: MariaStopMoving; OBJ_ANIMATION_AT kept in zClimbStepBefore around MariaAnimationRunning; MariaClimbMoveSpeeds by
; state less 5 doubled.
; WAITS: no.
; Entry: nothing. Exit: stopped, as MariaAnimationRunning; the same step, A it, Z and C set; moved, A the Y speed's
; high byte, N and Z from it, Y the index, C bit 7 of state less 5, V from that subtract.
MariaClimbMove:
    jsr MariaStopMoving
    lda a:OBJ_SCROLL_BAND
    sta z:zClimbStepBefore
    jsr MariaAnimationRunning
    beq MariaClimbMove_to1
    lda a:OBJ_SCROLL_BAND
    cmp z:zClimbStepBefore
    beq MariaClimbMove_to1
    lda a:OBJ_STATE
    sec
    sbc #$05
    asl a
    tay
    lda a:MariaClimbMoveSpeeds,y
    sta a:OBJ_VX_HI
    lda a:MariaClimbMoveSpeeds+1,y
    sta a:OBJ_VY_HI
    stz a:OBJ_VX_LO
    stz a:OBJ_VY_LO
MariaClimbMove_to1:
    rts

; @routine 01DE:563 MariaClimbPad
; @run 6D:AD63
; WHAT IT DOES: The pad on a ledge's hold: a climbing direction (MariaClimbPadsA, B, and C when the pad's second
; forced press is clear) with neither button held starts her animation again; a turning direction
; (MariaClimbTurnPadsA, B, and C when the forced press is clear) held for eight frames (zFieldC4) changes her hold:
; animation, frame, facing and state from the tables, her animation stopped, step 0, no X or Y fraction.
; HOW: zPadHeld's directions compared with the tables by state less 5; INC of zFieldC4; MariaSetAnimation with
; OBJ_ANIMATION by the tail JMP.
; WAITS: no.
; Entry: nothing. Exit: as the last compare or test, the INC, the stores (A the state), or MariaSetAnimation; Y the
; index, V from its subtract or the forced press's TST.
MariaClimbPad:
    lda a:OBJ_STATE
    sec
    sbc #$05
    tay
    lda z:zPadHeld
    and #$F0
    cmp a:MariaClimbPadsA,y
    beq MariaClimbPad_at0
    cmp a:MariaClimbPadsB,y
    beq MariaClimbPad_at0
    tst #$FF,a:PadForcedPressB
    bne MariaClimbPad_at1
    cmp a:MariaClimbPadsC,y
    beq MariaClimbPad_at0
MariaClimbPad_at1:
    lda z:zPadHeld
    and #$F0
    cmp a:MariaClimbTurnPadsA,y
    beq MariaClimbPad_at2
    cmp a:MariaClimbTurnPadsB,y
    beq MariaClimbPad_at2
    tst #$FF,a:PadForcedPressB
    beq MariaClimbPad_at3
    rts
MariaClimbPad_at3:
    cmp a:MariaClimbTurnPadsC,y
    beq MariaClimbPad_at2
    rts
MariaClimbPad_to1:
    inc z:zFieldC4
    rts
MariaClimbPad_at2:
    lda z:zFieldC4
    cmp #$07
    bcc MariaClimbPad_to1
    stz z:zFieldC4
    lda a:MariaClimbTurnAnimations,y
    sta a:OBJ_WORK_A
    lda #$FF
    sta a:OBJ_HURT_TIMER
    lda a:MariaClimbTurnFrames,y
    sta a:OBJ_FRAME
    lda a:MariaClimbTurnFacings,y
    sta a:OBJ_FACING_LEFT
    lda a:MariaClimbTurnStates,y
    sta a:OBJ_STATE
    stz a:OBJ_SUBSTATE
    stz a:OBJ_X_FRAC
    stz a:OBJ_Y_FRAC
    rts
MariaClimbPad_at0:
    tst #$03,z:zPadHeld
    beq MariaClimbPad_at4
    rts
MariaClimbPad_at4:
    lda a:OBJ_WORK_A
    jmp MariaSetAnimation

; @routine 01DE:608 MariaClimbLowFrame
; @run 6D:AE08
; WHAT IT DOES: Step 0 of the hold on the ledge ahead (states 5 and 6): the move, the presses, letting go on the jump;
; once her animation has stopped: the pad override of the hold set, frame 5, and a look ahead (MariaClimbLowLooks):
; with nothing there, she goes up (step 4, animation 0, the walking speed, OBJ_ANIMATION $15); else the item crash,
; the attack, or the pad.
; HOW: MariaClimbMove, MariaNotePressesB, MariaJumpOrClimb and MariaClimbLetGo; MariaReadAtOffset; MariaTryItemCrash,
; MariaItemOrAttack, MariaClimbPad by their tail JMPs; MariaSetAnimationZero, MariaSpeedWalk.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the routine it ended in; going up, A $15, N and Z from it, with Y and C as
; MariaSpeedWalk left them.
MariaClimbLowFrame:
    jsr MariaClimbMove
    jsr MariaNotePressesB
    jsr MariaJumpOrClimb
    bcc MariaClimbLowFrame_at0
    jmp MariaClimbLetGo
MariaClimbLowFrame_at0:
    lda a:OBJ_HURT_TIMER
    cmp #$FF
    beq MariaClimbLowFrame_at1
    rts
MariaClimbLowFrame_at1:
    lda a:OBJ_STATE
    sec
    sbc #$05
    tax
    lda #$01
    sta a:PadOverrides,x
    lda #$05
    sta a:OBJ_FRAME
    lda a:OBJ_STATE
    sec
    sbc #$05
    tax
    ldy a:MariaClimbLowLooks,x
    jsr MariaReadAtOffset
    and #$04
    beq MariaClimbLowFrame_at2
    jsr MariaTryItemCrash
    bcc MariaClimbLowFrame_at3
    jmp MariaCrashInAir
MariaClimbLowFrame_at3:
    jsr MariaItemOrAttack
    bcc MariaClimbLowFrame_to1
    jmp MariaClimbAttack
MariaClimbLowFrame_to1:
    jmp MariaClimbPad
MariaClimbLowFrame_at2:
    lda #$04
    sta a:OBJ_SUBSTATE
    jsr MariaSetAnimationZero
    jsr MariaSpeedWalk
    lda #$15
    sta a:OBJ_WORK_A
    rts

; @routine 01DE:666 MariaClimbAttack
; @run 6D:AE66
; WHAT IT DOES: Her attack from a ledge: what the attack button said (MariaAttack); if her attack's object started,
; animation $20 and step 1 in the state she is in.
; HOW: MariaAttack's Z; the JMP into MariaStandAttack past its state's store.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaAttack, or MariaStandAttack.
MariaClimbAttack:
    jsr MariaAttack
    beq MariaClimbAttack_at0
    rts
MariaClimbAttack_at0:
    lda #$20
    jmp MariaStandAttack_animate

; @routine 01DE:671 MariaClimbWait
; @run 6D:AE71
; WHAT IT DOES: A step of the hold that waits on her animation; when it has stopped, she is back in her ledge's hold.
; HOW: MariaAnimationWait, which ends the step while the animation runs; the tail JMP to MariaClimbHold.
; WAITS: no.
; Entry: nothing. Exit: as MariaAnimationWait, or MariaClimbHold.
MariaClimbWait:
    jsr MariaAnimationWait
    jmp MariaClimbHold

; @routine 01DE:677 MariaClimbAcross
; @run 6D:AE77
; WHAT IT DOES: Step 2 of a hold, going across to the ledge's edge: the jump lets go into a jump; while PlayerClimbReach
; is not zero she moves a pixel a frame toward the edge ($0100 or $FF00) with her animation; at the edge, on the
; ledge ahead frame 6 for 6 frames and the next step, on the ledge behind animation $13 and step 0; she faces up the
; ledge and stops.
; HOW: MariaJumpOrClimb and the JMP to MariaJump; DEC or INC of the reach; MariaAnimationTick; MariaClimbFacings by
; state less 5 or 7; the tail JMP to MariaStopX.
; WAITS: no.
; Entry: nothing. Exit: as the routine it ended in.
MariaClimbAcross:
    jsr MariaJumpOrClimb
    bcc MariaClimbAcross_at0
    jmp MariaJump
MariaClimbAcross_at0:
    lda a:PlayerClimbReach
    beq MariaClimbAcross_at1
    bmi MariaClimbAcross_at2
    jsr MariaAnimationTick
    lda #$00
    sta a:OBJ_VX_LO
    lda #$01
    sta a:OBJ_VX_HI
    dec a:PlayerClimbReach
    rts
MariaClimbAcross_at2:
    jsr MariaAnimationTick
    lda #$00
    sta a:OBJ_VX_LO
    lda #$FF
    sta a:OBJ_VX_HI
    inc a:PlayerClimbReach
    rts
MariaClimbAcross_at1:
    lda a:OBJ_STATE
    cmp #$07
    bcs MariaClimbAcross_to1
    lda #$06
    sta a:OBJ_FRAME
    lda #$06
    sta a:OBJ_HURT_TIMER
    inc a:OBJ_SUBSTATE
    lda a:OBJ_STATE
    sec
    sbc #$05
MariaClimbAcross_at3:
    tay
    lda a:MariaClimbFacings,y
    sta a:OBJ_FACING_LEFT
    stz a:OBJ_X_FRAC
    stz a:OBJ_Y_FRAC
    jmp MariaStopX
MariaClimbAcross_to1:
    stz a:OBJ_SUBSTATE
    pha
    lda #$13
    jsr MariaSetAnimation
    pla
    sec
    sbc #$07
    tay
    bra MariaClimbAcross_at3

; @routine 01DE:6E2 MariaClimbTurn
; @run 6D:AEE2
; WHAT IT DOES: Step 3 of the hold on the ledge ahead: the jump lets go; when the timer runs out, with the hold's pad
; (MariaClimbTurnPads) held she turns (animation $12, step 0), else she is back in the hold.
; HOW: MariaJumpOrClimb and the JMP to MariaJump; DEC of OBJ_HURT_TIMER; the AND with zPadHeld; MariaSetAnimation or a
; branch to MariaClimbHold.
; WAITS: no.
; Entry: nothing. Exit: as the routine it ended in; turned, as MariaSetAnimation.
MariaClimbTurn:
    jsr MariaJumpOrClimb
    bcc MariaClimbTurn_at0
    jmp MariaJump
MariaClimbTurn_at0:
    dec a:OBJ_HURT_TIMER
    beq MariaClimbTurn_at1
    rts
MariaClimbTurn_at1:
    lda a:OBJ_STATE
    sec
    sbc #$05
    tay
    lda z:zPadHeld
    and a:MariaClimbTurnPads,y
    beq MariaClimbHold
    lda #$12
    jsr MariaSetAnimation
    stz a:OBJ_SUBSTATE
    rts

; @routine 01DE:731 MariaClimbUp
; @run 6D:AF31
; WHAT IT DOES: Step 4, going up onto the ledge: when her animation has stopped, she stands.
; HOW: MariaAnimationRunning's Z; the tail JMP to MariaStand.
; WAITS: no.
; Entry: nothing. Exit: as MariaAnimationRunning, or MariaStand.
MariaClimbUp:
    jsr MariaAnimationRunning
    beq MariaClimbUp_at0
    rts
MariaClimbUp_at0:
    jmp MariaStand

; @routine 01DE:73A MariaStateClimbHigh
; @run 6D:AF3A
; WHAT IT DOES: States 7 and 8, the hold on the ledge behind her: the step's handler from MariaClimbHighSteps (hold,
; wait, across, wait, wait, the item crash).
; HOW: MariaStepIndex, then the JMP (table,X).
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the step's handler, after A and X the step doubled and C its bit 7.
MariaStateClimbHigh:
    jsr MariaStepIndex
    jmp (MariaClimbHighSteps,x)

; @routine 01DE:74C MariaClimbHighFrame
; @run 6D:AF4C
; WHAT IT DOES: Step 0 of the hold on the ledge behind her (states 7 and 8): the move, the presses, letting go on the
; jump; once her animation has stopped: frame 5, the hold's pad override set; the item crash, the attack; then a look
; (by state less 7, doubled, plus 4): with bit 2 there, the pad; else she stands.
; HOW: as MariaClimbLowFrame, with PadOverrides from its third byte and the tail JMPs to MariaClimbPad and MariaStand.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the routine it ended in.
MariaClimbHighFrame:
    jsr MariaClimbMove
    jsr MariaNotePressesB
    jsr MariaJumpOrClimb
    bcc MariaClimbHighFrame_to1
    jmp MariaClimbLetGo
MariaClimbHighFrame_to1:
    lda a:OBJ_HURT_TIMER
    cmp #$FF
    beq MariaClimbHighFrame_at0
    rts
MariaClimbHighFrame_at0:
    lda #$05
    sta a:OBJ_FRAME
    lda a:OBJ_STATE
    sec
    sbc #$07
    tay
    lda #$01
    sta a:PadOverrides+2,y
    jsr MariaTryItemCrash
    bcc MariaClimbHighFrame_at1
    jmp MariaCrashInAir
MariaClimbHighFrame_at1:
    jsr MariaItemOrAttack
    bcc MariaClimbHighFrame_at2
    jmp MariaClimbAttack
MariaClimbHighFrame_at2:
    lda a:OBJ_STATE
    sec
    sbc #$07
    asl a
    clc
    adc #$04
    tay
    jsr MariaReadAtOffset
    and #$04
    beq MariaClimbHighFrame_at3
    jmp MariaClimbPad
MariaClimbHighFrame_at3:
    jmp MariaStand

; @routine 01DE:79B MariaClimbLetGo
; @run 6D:AF9B
; WHAT IT DOES: Lets go of the ledge on the jump: the pad's overrides and second forced press cleared, the turn count
; cleared; with down held, she drops if there is a ladder below her (and stays with none); else she jumps.
; HOW: MariaClearPadOverrides; TST of zPadHeld; MariaLadderCheck's carry; the tail JMPs to MariaToFall and MariaJump.
; WAITS: no.
; Entry: nothing. Exit: no ladder, as MariaLadderCheck; else as MariaToFall or MariaJump.
MariaClimbLetGo:
    jsr MariaClearPadOverrides
    stz a:PadForcedPressB
    stz z:zFieldC6
    tst #$40,z:zPadHeld
    beq MariaClimbLetGo_at0
    jsr MariaLadderCheck
    bcs MariaClimbLetGo_at1
    rts
MariaClimbLetGo_at1:
    stz a:PlayerMayLeaveTop
    jmp MariaToFall
MariaClimbLetGo_at0:
    stz a:PlayerMayLeaveTop
    jmp MariaJump

; @routine 01E0:2C7 MariaReadAtOffset
; @run 6D:BAC7
; WHAT IT DOES: RichterReadAtOffset in Maria's overlay: reads the terrain at a point offset from her own X and Y low
; bytes by a pair from MariaProbeOffsets.
; HOW: X low plus the table's byte at Y into zTemp00; Y low plus the next becomes the row, its carry the Y high's; the
; tail JMP to MariaReadOnScreen.
; WAITS: no.
; Entry: Y the table's offset. Exit: as MariaReadOnScreen.
MariaReadAtOffset:
    lda a:OBJ_X_LO
    clc
    adc a:MariaProbeOffsets,y
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc a:MariaProbeOffsets+1,y
    jmp MariaReadOnScreen

; @routine 01E0:37A MariaJumpOrClimb
; @run 6D:BB7A
; WHAT IT DOES: RichterJumpOrClimb in Maria's overlay: with the jump pressed, the pad's second forced press is
; cleared and, with down held, the answer is whether there is a ladder below her (MariaLadderCheck); else the jump.
; HOW: A BSR to MariaJumpPressed; its CLC and SEC tails; the tail JMP to MariaLadderCheck.
; WAITS: no.
; Entry: nothing. Exit: as RichterJumpOrClimb (C the answer).
MariaJumpOrClimb:
    bsr MariaJumpPressed
    bcc MariaJumpPressed+3
    stz a:PadForcedPressB
    tst #$40,z:zPadHeld
    beq MariaJumpPressed+5
    jmp MariaLadderCheck

; @routine 01E0:414 MariaNotePressesB
; @run 6D:BC14
; WHAT IT DOES: Keeps this frame's presses of buttons II and I in the pad's second forced press.
; HOW: Bit 1 of zPadPressed by a BSR to MariaOrForcedPressB, ORed in once more, then bit 0, on into it.
; WAITS: no.
; Entry: nothing. Exit: A the forced press, N and Z from it. X, Y, C and V kept.
MariaNotePressesB:
    lda z:zPadPressed
    and #$02
    bsr MariaOrForcedPressB
    ora a:PadForcedPressB
    sta a:PadForcedPressB
    lda z:zPadPressed
    and #$01

; @routine 01E0:424 MariaOrForcedPressB
; @run 6D:BC24
; WHAT IT DOES: ORs A into the pad's second forced press.
; HOW: ORA, STA.
; WAITS: no.
; Entry: A the bits. Exit: A the new forced press, N and Z from it. X, Y, C and V kept.
MariaOrForcedPressB:
    ora a:PadForcedPressB
    sta a:PadForcedPressB
    rts

; @routine 01E0:42B MariaItemOrAttack
; @run 6D:BC2B
; WHAT IT DOES: On a ledge: the attack button (MariaUseItem); if it did nothing, with button II held and up pressed,
; her item thrown as MariaUseItem throws it with down held.
; HOW: A BSR to MariaUseItem; BBR1 of zPadHeld, TST of zPadPressed; branches into MariaUseItem's CLC and its throw.
; WAITS: no.
; Entry: nothing. Exit: as MariaUseItem; neither, C clear.
MariaItemOrAttack:
    bsr MariaUseItem
    bcc MariaItemOrAttack_to1
    rts
MariaItemOrAttack_to1:
    bbr1 zPadHeld,MariaUseItem+8
    tst #$10,z:zPadPressed
    beq MariaUseItem+8
    bra MariaUseItem+$1C
