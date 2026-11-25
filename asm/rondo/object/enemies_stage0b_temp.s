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

; @routine 117B:5F5 StepFramesFacingPlayerD
; @run 70:B5F5
; WHAT IT DOES: Turns the object to face the player and steps its animation along the list whose
; pointer is the pair of bytes at overlay address $B60A plus Y (StepFramesFacingPlayerC in this
; stage's overlay).
; HOW: SideOfPlayer, the pointer into zTemp10-11, then StepObjectFrames as a tail jump.
; WAITS: no.
; Entry: X object, Y the list's offset, D clear. Exit: as StepObjectFrames. Clobbers zTemp00,
; zTemp10-11.
StepFramesFacingPlayerD:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
    lda a:$B60A,y
    sta z:zTemp10
    lda a:$B60B,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 117B:6B6 PickSpeedAndTurnFromTable
; @run 70:B6B6
; WHAT IT DOES: Sets the object's X and Y speeds from a table of four byte entries by its f3670,
; adds one to f3510, and reverses its X speed when the player is on its left.
; HOW: The table is at overlay address $B6E2 (X low, X high, Y low, Y high; loads run high byte
; first); SideOfPlayer; NegateVelocityX as a tail jump.
; WAITS: no.
; Entry: X object, D clear. Exit: A the side bit (0) or the new X speed's high byte after the
; negate, N, Z, C and V from it; Y the entry's offset; X kept. Clobbers zTemp00.
PickSpeedAndTurnFromTable:
    lda a:OBJ_3670,x
    asl a
    asl a
    tay
    lda a:$B6E3,y
    sta a:OBJ_VX_HI,x
    lda a:$B6E2,y
    sta a:OBJ_VX_LO,x
    lda a:$B6E5,y
    sta a:OBJ_VY_HI,x
    lda a:$B6E4,y
    sta a:OBJ_VY_LO,x
    inc a:OBJ_3510,x
    jsr VecSideOfPlayer
    and #$01
    beq PickSpeedAndTurnFromTable_at0
    jmp VecNegateVelocityX
PickSpeedAndTurnFromTable_at0:
    rts

; @routine 117C:09D WaitThenState2C
; @run 70:B89D
; WHAT IT DOES: A two-step wait like WaitThenState2: the first pass sets f34b8 to $1C and the step
; to 1; later passes animate the object facing the player (list index 2) and count the timer down;
; at zero the object enters state 2.
; HOW: StepFramesFacingPlayerD with Y 2, then a decrement.
; WAITS: no.
; Entry: X object, D clear. Exit: as WaitThenState2.
WaitThenState2C:
    lda a:OBJ_2800,x
    bne WaitThenState2C_at0
    lda #$1C
    sta a:OBJ_34B8,x
    inc a:OBJ_2800,x
    rts
WaitThenState2C_at0:
    ldy #$02
    jsr StepFramesFacingPlayerD
    dec a:OBJ_34B8,x
    bne WaitThenState2C_at1
    lda #$02
    sta a:OBJ_27A8,x
WaitThenState2C_at1:
    rts

; @routine 117C:0BB BecomeType9CWhenAnimationEndsB
; @run 70:B8BB
; WHAT IT DOES: Animates the object (list 8, facing the player) and, when the animation list has
; ended, turns it into an object of type $9C in state 0.
; HOW: StepFramesFacingPlayerD with Y 8; the list ending decides.
; WAITS: no.
; Entry: X object, D clear. Exit: as BecomeType9CWhenAnimationEnds.
BecomeType9CWhenAnimationEndsB:
    ldy #$08
    jsr StepFramesFacingPlayerD
    bne BecomeType9CWhenAnimationEndsB_at0
    lda #$9C
    sta a:OBJ_2750,x
    stz a:OBJ_27A8,x
BecomeType9CWhenAnimationEndsB_at0:
    rts

; @routine 117C:0F5 LoadRecordStartAngle40B
; @run 70:B8F5
; WHAT IT DOES: LoadRecordStartAngle40, in this stage's overlay: starts the object from its record,
; moves it by its velocity, and sets its angle's high byte to $40.
; HOW: LoadObjectRecord, ApplyVelocity, one store.
; WAITS: no.
; Entry, exit: as LoadRecordStartAngle40.
LoadRecordStartAngle40B:
    jsr VecLoadObjectRecord
    jsr VecApplyVelocity
    lda #$40
    sta a:OBJ_3568,x
    rts

; @routine 117C:3B1 FrameOneBWhenFallingElseAnimate
; @run 70:BBB1
; WHAT IT DOES: An object that is not rising (Y speed high byte positive) takes frame $1B; one
; that is rising animates along the list at overlay address $BBC7.
; HOW: Tests the speed's sign; StepObjectFrames as a tail jump.
; WAITS: no.
; Entry: X object, D clear. Exit: A $1B and N, Z from it when falling; else as StepObjectFrames.
; Clobbers zTemp10-11.
FrameOneBWhenFallingElseAnimate:
    lda a:OBJ_VY_HI,x
    bmi FrameOneBWhenFallingElseAnimate_at0
    lda #$1B
    sta a:OBJ_2A68,x
    rts
FrameOneBWhenFallingElseAnimate_at0:
    lda #$C7
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 117C:684 FaceThePlayerInSomeStatesB
; @run 70:BE84
; WHAT IT DOES: FaceThePlayerInSomeStates in this stage's overlay: turns the object to face the
; player when a table at overlay address $BE95 says its state is one that does.
; HOW: As FaceThePlayerInSomeStates.
; WAITS: no.
; Entry, exit: as FaceThePlayerInSomeStates.
FaceThePlayerInSomeStatesB:
    ldy a:OBJ_27A8,x
    lda a:AnimationSounds+7,y
    beq FaceThePlayerInSomeStatesB_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
FaceThePlayerInSomeStatesB_at0:
    rts

; @routine 117D:3B6 AnimateAlongListC3C2
; @run 71:C3B6
; WHAT IT DOES: Steps the object's animation along the list at overlay address $C3C2.
; HOW: The pointer into zTemp10-11, then StepObjectFrames (a call, then a return).
; WAITS: no.
; Entry: X object, D clear. Exit: as StepObjectFrames. Clobbers zTemp10-11.
AnimateAlongListC3C2:
    lda #$C2
    sta z:zTemp10
    lda #$C3
    sta z:zTemp11
    jsr VecStepObjectFrames
    rts

; @routine 117D:41F SwingOnCosineB
; @run 71:C41F
; WHAT IT DOES: Gives the object a Y speed of its life_timer's angle's cosine (speed $00A0),
; moves it, and turns life_timer on by 2.
; HOW: ScaleByCosine with zTemp00-01 set to $00A0, ApplyVelocity.
; WAITS: no.
; Entry: X object, D clear. Exit: A the new life_timer, N, Z, C and V from the add of 2; Y 0.
; Clobbers zTemp00-01, zTemp10-12, zProduct0-2, zMultiplierLo-Hi.
SwingOnCosineB:
    lda #$A0
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_34B8,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VY_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocity
    lda a:OBJ_34B8,x
    clc
    adc #$02
    sta a:OBJ_34B8,x
    rts

; @routine 117C:04A TurnOnRandomBeatWalkC
; @run 70:B84A
; @stop B87B
; WHAT IT DOES: A walk that turns on a random beat: at its first step it turns to the player's side,
; takes an X speed from a table at overlay address $B87B (two bytes by side), picks life_timer
; at random from eight bytes at $B87F, stops the animation and moves on; after that it runs
; TurnOnRandomBeatWalkFollowC.
; HOW: The shared body; the two tables lie between the two parts and stay data.
; WAITS: no.
; Entry: X object, D clear. Exit: first step: A the timer, N, Z from the new step, C and V from
; NextRandom, Y its index; later as TurnOnRandomBeatWalkFollowC.
TurnOnRandomBeatWalkC:
    lda a:OBJ_2800,x
    bne TurnOnRandomBeatWalkFollowC
    jsr VecSideOfPlayer
    and #$01
    asl a
    tay
    lda a:$B87B,y
    sta a:OBJ_VX_HI,x
    lda a:$B87B+1,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$07
    tay
    lda a:$B87F,y
    sta a:OBJ_34B8,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    inc a:OBJ_3510,x
    inc a:OBJ_2800,x
    rts

; @routine 117C:087 TurnOnRandomBeatWalkFollowC
; @run 70:B887
; WHAT IT DOES: The later steps of TurnOnRandomBeatWalkC: animates the object facing the player (list 2),
; counts life_timer down and moves it along X; at zero it enters state 5 with step 0.
; HOW: StepFramesFacingPlayerD, a decrement, ApplyVelocityX as a tail jump.
; WAITS: no.
; Entry: X object, D clear. Exit: as the animation step, the decrement and ApplyVelocityX leave them
; (A 5 and N, Z clear at zero).
TurnOnRandomBeatWalkFollowC:
    ldy #$02
    jsr StepFramesFacingPlayerD
    dec a:OBJ_34B8,x
    beq TurnOnRandomBeatWalkFollowC_done
    jmp VecApplyVelocityX
TurnOnRandomBeatWalkFollowC_done:
    lda #$05
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
    rts

; @routine 117C:219 StartFlyerAtRandom
; @run 70:BA19
; WHAT IT DOES: Starts the object from its record with flash flags $8F, places it at random
; (PlaceAtRandomOnEdge), gives it a random f36c8 and a f35c0 from four bytes at overlay address
; $BA49 by two random bits, turns it to face the player and gives it a Y speed of $FF40.
; HOW: LoadObjectRecord, PlaceAtRandomOnEdge, NextRandom twice, SideOfPlayer.
; WAITS: no.
; Entry: X object, D clear. Exit: A $40 and N, Z clear; C from the side compare; V from the
; random numbers; Y the table index.
StartFlyerAtRandom:
    jsr VecLoadObjectRecord
    lda #$8F
    sta a:OBJ_2B18,x
    jsr PlaceAtRandomOnEdge
    jsr VecNextRandom
    sta a:OBJ_36C8,x
    jsr VecNextRandom
    and #$03
    tay
    lda a:$BA49,y
    sta a:OBJ_35C0,x
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
    lda #$FF
    sta a:OBJ_VY_HI,x
    lda #$40
    sta a:OBJ_VY_LO,x
    rts

; @routine 117C:251 PlaceAtRandomOnEdge
; @run 70:BA51
; WHAT IT DOES: Puts the object at a random place chosen from a table of position pairs at overlay
; address $BA8A: the row group depends on which way the scroll moves (zTemp00 $10, $20 moving right,
; 0 moving left), the row on three random bits; its Y low byte comes from four bytes at $BABA by two
; more random bits; its Y high byte is cleared.
; HOW: NextRandom twice.
; WAITS: no.
; Entry: X object, D clear. Exit: A the Y byte, N, Z from it; C, V from the second NextRandom;
; Y its index. Clobbers zTemp00.
PlaceAtRandomOnEdge:
    lda #$10
    sta z:zTemp00
    lda a:ScrollSpeedX
    beq PlaceAtRandomOnEdge_at0
    bmi PlaceAtRandomOnEdge_at1
    lda #$20
    sta z:zTemp00
    bra PlaceAtRandomOnEdge_at0
PlaceAtRandomOnEdge_at1:
    stz z:zTemp00
PlaceAtRandomOnEdge_at0:
    jsr VecNextRandom
    and #$07
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:$BA8B,y
    sta a:OBJ_X_HI,x
    lda a:$BA8A,y
    sta a:OBJ_X_LO,x
    stz a:OBJ_Y_HI,x
    jsr VecNextRandom
    and #$03
    tay
    lda a:$BABA,y
    sta a:OBJ_Y_LO,x
    rts

; @routine 117C:2BE SwoopOnCosineC
; @run 70:BABE
; WHAT IT DOES: Unless the object in slot $54 shows frame 4: gives the object an X speed of
; its f36c8's angle's cosine (speed $00A0), moves it along X, advances f36c8, moves it by its
; velocity, and when it is on the first screen down and under $E0 moves it on a state.
; HOW: ScaleByCosine, ApplyVelocityX, ApplyVelocity.
; WAITS: no.
; Entry: X object, D clear. Exit: as the last test, add or compare left them; Y 0 after the scale.
SwoopOnCosineC:
    lda a:$2ABC
    cmp #$04
    beq SwoopOnCosine_at0
    lda #$A0
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_36C8,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VX_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VX_LO,x
    jsr VecApplyVelocityX
    inc a:OBJ_36C8,x
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    bne SwoopOnCosine_at0
    lda a:OBJ_Y_LO,x
    cmp #$E0
    bcs SwoopOnCosine_at0
    inc a:OBJ_27A8,x
SwoopOnCosine_at0:
    rts

; @routine 117C:609 FlashFallAnimateThenRemove
; @run 70:BE09
; WHAT IT DOES: Sets the object's flash flags to $8B, moves it along Y, and animates it along the list
; at overlay address $BE22; when the list has ended the object removes itself.
; HOW: ApplyVelocityY, StepObjectFrames; ClearObject as a tail jump.
; WAITS: no.
; Entry: X object, D clear. Exit: as StepObjectFrames, V from ApplyVelocityY. Clobbers zTemp10-11.
FlashFallAnimateThenRemove:
    lda #$8B
    sta a:OBJ_2B18,x
    jsr VecApplyVelocityY
    lda #$22
    sta z:zTemp10
    lda #$BE
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne FlashFallAnimateThenRemove_at0
    jmp VecClearObject
FlashFallAnimateThenRemove_at0:
    rts

; @routine 117C:6F1 StartAndMakeNeighbourB
; @run 70:BEF1
; WHAT IT DOES: Starts the object from its record, empties the slot after it and starts an object
; of type $2C there at its place, and sets its own life_timer to $50.
; HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace.
; WAITS: no.
; Entry: X object, D clear. Exit: A $50 and N, Z clear; C from the record; Y the neighbour's slot.
StartAndMakeNeighbourB:
    jsr VecLoadObjectRecord
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$2C
    jsr VecStartObjectAtPlace
    lda #$50
    sta a:OBJ_34B8,x
    rts

; @routine 117C:705 EnterNextStateWhenHitOrPlayerNear
; @run 70:BF05
; WHAT IT DOES: When the object has taken a hit, or the player is within $68 pixels along X and $18
; along Y, sets f3670 to 1 and moves the object on to its next state.
; HOW: DistanceXToPlayer and DistanceYToPlayer compared.
; WAITS: no.
; Entry: X object, D clear. Exit: A, N, Z, C from the last test; V from the last distance;
; or A 1 and N, Z from the new state.
EnterNextStateWhenHitOrPlayerNear:
    lda a:OBJ_33B0,x
    bne EnterNextStateWhenHitOrPlayerNear_at0
    jsr VecDistanceXToPlayer
    cmp #$68
    bcs EnterNextStateWhenHitOrPlayerNear_at1
    jsr VecDistanceYToPlayer
    cmp #$18
    bcs EnterNextStateWhenHitOrPlayerNear_at1
EnterNextStateWhenHitOrPlayerNear_at0:
    lda #$01
    sta a:OBJ_3670,x
    inc a:OBJ_27A8,x
EnterNextStateWhenHitOrPlayerNear_at1:
    rts

; @routine 117B:53B BurstWhenFlaggedHitB
; @run 70:B53B
; WHAT IT DOES: When the object is not yet in state 6 and bit 7 of its status flags is set, plays
; sound $8B, spawns its six pieces (SpawnSixPiecesB), stops its f3250 and its animation and enters
; state 6.
; HOW: Two tests, SoundCommand, SpawnSixPiecesB.
; WAITS: no.
; Entry: X object, D clear. Exit: A 6 and N, Z clear, C and V as the piece spawn left them when it
; burst; otherwise A, N, Z, C from the failing test; X kept.
BurstWhenFlaggedHitB:
    lda a:OBJ_27A8,x
    cmp #$06
    beq BurstWhenFlaggedHitB_at0
    lda a:OBJ_31A0,x
    and #$80
    beq BurstWhenFlaggedHitB_at0
    lda #$8B
    jsr VecSoundCommand
    jsr SpawnSixPiecesB
    stz a:OBJ_3250,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$06
    sta a:OBJ_27A8,x
BurstWhenFlaggedHitB_at0:
    rts

; @routine 117B:560 SpawnSixPiecesB
; @run 70:B560
; WHAT IT DOES: Starts up to six objects of type $24 at the object's place, each from a free slot
; from $30, giving each its life_timer, f3510 and speeds from a four byte entry of a table at
; overlay address $B596.
; HOW: The body SpawnSixPiecesA has, with this type and table.
; WAITS: no.
; Entry: X object, D clear. Exit: as SpawnSixPiecesA.
SpawnSixPiecesB:
    cly
SpawnSixPiecesB_at1:
    sty z:zHitSlot
    jsr VecFindFreeSlotFrom30
    bcc SpawnSixPiecesB_at0
    lda #$24
    jsr VecStartObjectAtPlace
    stx z:zRichterObject
    lda z:zHitSlot
    asl a
    asl a
    tax
    lda a:$B596,x
    sta a:OBJ_34B8,y
    lda a:$B597,x
    sta a:OBJ_3510,y
    lda a:$B598,x
    sta a:OBJ_VX_HI,y
    lda a:$B599,x
    sta a:OBJ_VY_HI,y
    ldx z:zRichterObject
    ldy z:zHitSlot
    iny
    cpy #$06
    bcc SpawnSixPiecesB_at1
SpawnSixPiecesB_at0:
    rts

; @routine 117C:001 SoundEveryFourthFrameC
; @run 70:B801
; WHAT IT DOES: Plays sound $22 on the frames when the low two bits of the object's f3618 are zero.
; HOW: One test and the call.
; WAITS: no.
; Entry: X object. Exit: as SoundEveryFourthFrame.
SoundEveryFourthFrameC:
    lda a:OBJ_3618,x
    and #$03
    bne SoundEveryFourthFrameC_at0
    lda #$22
    jsr VecSoundCommand
SoundEveryFourthFrameC_at0:
    rts

; @routine 117D:3CB StartFromScreenEdgeByPlayer
; @run 71:C3CB
; WHAT IT DOES: Loads the record, then puts the object at one edge of the screen heading across it:
; when the player's X low byte is $C0 or more, or between $40 and $BF while the player faces
; left, on the left edge (X $FFE0, speed $0140, facing left set); otherwise on the right (X $0120,
; speed -$0140). Its Y is the player's less 4.
; HOW: LoadObjectRecord, two compares, stores, a 16-bit subtract.
; WAITS: no.
; Entry: X object, D clear. Exit: A the high byte of the new Y, N, Z, C, V from its subtract; Y 11.
StartFromScreenEdgeByPlayer:
    jsr VecLoadObjectRecord
    lda a:OBJ_X_LO
    cmp #$40
    bcc StartFromScreenEdgeByPlayer_at0
    cmp #$C0
    bcs StartFromScreenEdgeByPlayer_at1
    lda a:OBJ_2AC0
    beq StartFromScreenEdgeByPlayer_at0
StartFromScreenEdgeByPlayer_at1:
    lda #$01
    sta a:OBJ_2AC0,x
    lda #$FF
    sta a:OBJ_X_HI,x
    lda #$E0
    sta a:OBJ_X_LO,x
    lda #$01
    sta a:OBJ_VX_HI,x
    lda #$40
    sta a:OBJ_VX_LO,x
    bra StartFromScreenEdgeByPlayer_at2
StartFromScreenEdgeByPlayer_at0:
    lda #$01
    sta a:OBJ_X_HI,x
    lda #$20
    sta a:OBJ_X_LO,x
    lda #$FE
    sta a:OBJ_VX_HI,x
    lda #$C0
    sta a:OBJ_VX_LO,x
StartFromScreenEdgeByPlayer_at2:
    lda a:OBJ_Y_LO
    sec
    sbc #$04
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI
    sbc #$00
    sta a:OBJ_Y_HI,x
    rts

; @routine 1173:1D6 StartPieceInSlotsFrom28
; @run 6C:91D6
; WHAT IT DOES: When the player's Y low byte is below $70, it advances its f3568 and starts an
; object at its place in the first free slot of $28 to $2F, of the type a table at overlay
; address $9204 gives by the low three bits of f3568; nothing happens when the eight slots are
; taken. Then (and when the player is low) it sets its f3510 to $A0.
; HOW: A search for a slot of type 0, ClearObjectInY, StartObjectAtPlace. When no slot is free the
; routine ends at once, without the timer: that arm ran in no script, so the draft stops there and
; liftcmp does not judge it.
; WAITS: no.
; Entry: X object, D clear. Exit: A $A0, N set, Z clear, C and V as StartObjectAtPlace left them,
; Y the slot; when the player is low, A $A0 and C, N, Z from the compare of the player's Y with $70.
StartPieceInSlotsFrom28:
    lda a:OBJ_Y_LO
    cmp #$70
    bcs StartPieceInSlotsFrom28_at0
    inc a:OBJ_3568,x
    ldy #$28
StartPieceInSlotsFrom28_at2:
    lda a:OBJ_2750,y
    beq StartPieceInSlotsFrom28_at1
    iny
    cpy #$30
    bcc StartPieceInSlotsFrom28_at2
    rts
StartPieceInSlotsFrom28_at1:
    jsr VecClearObjectInY
    phy
    lda a:OBJ_3568,x
    and #$07
    tay
    lda a:$9204,y
    ply
    jsr VecStartObjectAtPlace
StartPieceInSlotsFrom28_at0:
    lda #$A0
    sta a:OBJ_3510,x
    rts

; @routine 1173:20C SpawnType2AThenRestartTimer
; @run 6C:920C
; WHAT IT DOES: When its next_kind is 4, or the player's Y low byte is $70 or more, it starts an
; object of type $2A at its place from a free slot from $30 (and does nothing more when there is
; none). Then, and when neither held, it sets its f3510 to $C0.
; HOW: Two tests, FindFreeSlotFrom30, StartObjectAtPlace.
; WAITS: no.
; Entry: X object, D clear. Exit: A $C0 and N set, Z clear, C and Y as the last call left them;
; with no free slot, as FindFreeSlotFrom30 left them.
SpawnType2AThenRestartTimer:
    lda a:OBJ_3040,x
    cmp #$04
    beq SpawnType2AThenRestartTimer_at0
    lda a:OBJ_Y_LO
    cmp #$70
    bcc SpawnType2AThenRestartTimer_at1
SpawnType2AThenRestartTimer_at0:
    jsr VecFindFreeSlotFrom30
    bcc SpawnType2AThenRestartTimer_at2
    lda #$2A
    jsr VecStartObjectAtPlace
SpawnType2AThenRestartTimer_at1:
    lda #$C0
    sta a:OBJ_3510,x
SpawnType2AThenRestartTimer_at2:
    rts

; @routine 117B:63D PickStateByPlayerDistanceC
; @run 70:B63D
; WHAT IT DOES: PickStateByPlayerDistance in this stage's overlay, with its state table at
; overlay address $B663.
; HOW: The shared body. The two early arms (an object off the screen across, or down) ran in no
; script: the draft stops there, so liftcmp does not judge them.
; WAITS: no.
; Entry, exit: as PickStateByPlayerDistance.
PickStateByPlayerDistanceC:
    stz a:OBJ_2800,x
    lda a:OBJ_X_HI,x
    beq PickStateByPlayerDistanceC_at0
    lda #$03
    sta a:OBJ_27A8,x
    rts
PickStateByPlayerDistanceC_at0:
    lda a:OBJ_Y_HI,x
    beq PickStateByPlayerDistanceC_at1
    lda #$04
    sta a:OBJ_27A8,x
    rts
PickStateByPlayerDistanceC_at1:
    jsr VecPlayerDistanceSum
    and #$0F
    tay
    lda a:$B663,y
    sta a:OBJ_27A8,x
    rts

; @routine 117B:737 AnimateMoveThenSetUpNextStepB
; @run 70:B737
; WHAT IT DOES: Animates the object facing the player (list 4), moves it, and counts life_timer
; down; at zero it plays sound $8B, picks a speed and turn (PickSpeedAndTurnFromTable), sets f36c8
; to $58 plus six bits of PlayerDistanceSum, stops its animation and advances its step.
; HOW: StepFramesFacingPlayerD, ApplyVelocity, a decrement, then the calls named.
; WAITS: no.
; Entry: X object, C in (kept to the sum), D clear. Exit: as AnimateMoveThenSetUpNextStep.
AnimateMoveThenSetUpNextStepB:
    ldy #$04
    jsr StepFramesFacingPlayerD
    jsr VecApplyVelocity
    dec a:OBJ_34B8,x
    bne OrbitStepForward
    lda #$8B
    jsr VecSoundCommand
    jsr PickSpeedAndTurnFromTable
    jsr VecPlayerDistanceSum
    and #$3F
    clc
    adc #$58
    sta a:OBJ_36C8,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    inc a:OBJ_2800,x
    rts

; @routine 1173:26A KnockedUpWhenHitB
; @run 6C:926A
; WHAT IT DOES: KnockedUpWhenHit for a state below 3, entering state 3.
; HOW: The shared body. Only the first test ran in a script: the draft stops after it, so liftcmp
; does not judge the rest.
; WAITS: no.
; Entry, exit: as KnockedUpWhenHit.
KnockedUpWhenHitB:
    lda a:OBJ_31A0,x
    and #$80
    beq KnockedUpWhenHitB_at0
    lda a:OBJ_27A8,x
    cmp #$03
    bcs KnockedUpWhenHitB_at0
    stz a:OBJ_3250,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$03
    sta a:OBJ_2A10,x
    stz a:OBJ_2A68,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$FE
    sta a:OBJ_VY_LO,x
    lda #$03
    sta a:OBJ_27A8,x
KnockedUpWhenHitB_at0:
    rts

; @routine 117C:1EA KnockedUpWhenHit
; @run 70:B9EA
; WHAT IT DOES: When bit 7 of the object's status flags is set and its state is below 9, it stops its
; f3250 and its animation, takes sprite set 3 and frame 0, a Y speed of -$0102 and enters state 9.
; HOW: Two tests and the stores.
; WAITS: no.
; Entry: X object, D clear. Exit: A 9 and N, Z clear when it was knocked up; otherwise A, N, Z, C from
; the failing test. X, Y, V kept.
KnockedUpWhenHit:
    lda a:OBJ_31A0,x
    and #$80
    beq KnockedUpWhenHit_at0
    lda a:OBJ_27A8,x
    cmp #$09
    bcs KnockedUpWhenHit_at0
    stz a:OBJ_3250,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$03
    sta a:OBJ_2A10,x
    stz a:OBJ_2A68,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$FE
    sta a:OBJ_VY_LO,x
    lda #$09
    sta a:OBJ_27A8,x
KnockedUpWhenHit_at0:
    rts

; @routine 117C:6CB SetHitboxFromFrameRowsC
; @run 70:BECB
; WHAT IT DOES: Gives the object a hit rectangle from a table: the row is picked by its frame less $29 through a
; byte table at overlay address $C2B9, and the row's four bytes (at $C2D2, four to a row) are its X offset, Y
; offset, half width and half height.
; HOW: Two table reads, four stores.
; WAITS: no.
; Entry: X object, D clear. Exit: A the half height, N, Z from it, C from the row's shift, V from the subtract
; of $29, Y the row's offset.
SetHitboxFromFrameRowsC:
    lda a:OBJ_2A68,x
    sec
    sbc #$29
    tay
    lda a:$C2B9,y
    asl a
    asl a
    tay
    lda a:$C2D2,y
    sta a:OBJ_3930,x
    lda a:$C2D3,y
    sta a:OBJ_39E0,x
    lda a:$C2D4,y
    sta a:OBJ_3988,x
    lda a:$C2D5,y
    sta a:OBJ_3A38,x
    rts

; @routine 117D:277 FollowPreviousPiece
; @run 71:C277
; WHAT IT DOES: In state 0 it loads its record. Later it takes the place of the object one slot before it,
; that object's f3250 and facing, and a hit rectangle from a table of four byte rows at overlay address $C2EE
; picked by that object's frame less $29 (a row beyond 18 is the 18th).
; HOW: CopyObjectPlaceBack and table reads. The row past the table's end ran in no script: the draft
; stops there, so liftcmp does not judge it.
; WAITS: no.
; Entry: X object, D clear. Exit: state 0: as LoadObjectRecord; otherwise A the facing, N, Z from it, C 0, V from
; the subtract of $29, Y the row's offset.
FollowPreviousPiece:
    lda a:OBJ_27A8,x
    bne FollowPreviousPiece_at0
    jmp VecLoadObjectRecord
FollowPreviousPiece_at0:
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    lda a:$324F,x
    sta a:OBJ_3250,x
    lda a:$2A67,x
    sec
    sbc #$29
    cmp #$13
    bcc FollowPreviousPiece_at1
    lda #$13
FollowPreviousPiece_at1:
    asl a
    asl a
    tay
    lda a:$C2EE,y
    sta a:OBJ_3930,x
    lda a:$C2EF,y
    sta a:OBJ_39E0,x
    lda a:$C2F0,y
    sta a:OBJ_3988,x
    lda a:$C2F1,y
    sta a:OBJ_3A38,x
    lda a:$2ABF,x
    sta a:OBJ_2AC0,x
    rts

; @routine 1173:12C LoadObjectRecordFromTable
; @run 6C:912C
; WHAT IT DOES: LoadObjectRecord for the objects of this overlay: the twelve byte record of the object's kind is found
; through a table of addresses at overlay address $AD37 (kind 1 first) and copied into the object's fields; its hit
; rectangle offsets are cleared and its state advanced.
; HOW: The pointer into zTemp10-11 and twelve reads through it, as LoadObjectRecord does; the table is the
; overlay's own, not the one at the record pointer.
; WAITS: no.
; Entry: X object, D clear. Exit: A the record's last byte, Y 11, N and Z from the new state, C bit 3 of the
; packed byte; X, V kept.
LoadObjectRecordFromTable:
    lda a:OBJ_2750,x
    dec a
    asl a
    tay
    lda a:$AD37,y
    sta z:zTemp10
    lda a:$AD38,y
    sta z:zTemp11
    cly
    lda (zTemp10),y
    sta a:OBJ_3358,x
    iny
    lda (zTemp10),y
    sta a:OBJ_3300,x
    iny
    lda (zTemp10),y
    sta a:OBJ_32A8,x
    iny
    lda (zTemp10),y
    and #$0F
    sta a:OBJ_3250,x
    lda (zTemp10),y
    lsr a
    lsr a
    lsr a
    lsr a
    sta a:OBJ_FIELDS_2EE0,x
    iny
    lda (zTemp10),y
    sta a:OBJ_3988,x
    iny
    lda (zTemp10),y
    sta a:OBJ_3A38,x
    iny
    lda (zTemp10),y
    sta a:OBJ_2A10,x
    iny
    lda (zTemp10),y
    sta a:OBJ_2A68,x
    iny
    lda (zTemp10),y
    sta a:OBJ_30F0,x
    iny
    lda (zTemp10),y
    sta a:OBJ_3148,x
    iny
    lda (zTemp10),y
    sta a:OBJ_31A0,x
    iny
    lda (zTemp10),y
    sta a:OBJ_31F8,x
    stz a:OBJ_3930,x
    stz a:OBJ_39E0,x
    inc a:OBJ_27A8,x
    rts

; @routine 1173:2E0 SwingOnCosineThenLeapAtPlayer
; @run 6C:92E0
; WHAT IT DOES: Steps the frame list at overlay address $9346, gives itself a Y speed of the cosine of its life_timer
; angle (speed $0040), moves along Y, counts the angle up and moves along X. When the player is nearer than $60
; along X, one time in 128 (a random byte below 2) it picks a leap from a table of four byte rows at overlay address
; $934F by three random bits (X low, X high, Y low, Y high), reverses the X speed when it faces left and moves on
; a state.
; HOW: StepObjectFrames, ScaleByCosine, ApplyVelocityY, ApplyVelocityX, DistanceXToPlayer, NextRandom twice,
; NegateVelocityX.
; WAITS: no.
; Entry: X object, Y kept to the frame step, D clear. Exit: as the last test, add or call left them: after a leap
; A, N, Z from the new state.
SwingOnCosineThenLeapAtPlayer:
    lda #$93
    sta z:zTemp11
    lda #$46
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda #$40
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_34B8,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VY_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocityY
    inc a:OBJ_34B8,x
    jsr VecApplyVelocityX
    jsr VecDistanceXToPlayer
    cmp #$60
    bcs SwingOnCosineThenLeapAtPlayer_at0
    jsr VecNextRandom
    cmp #$02
    bcs SwingOnCosineThenLeapAtPlayer_at0
    jsr VecNextRandom
    and #$07
    asl a
    asl a
    tay
    lda a:$934F,y
    sta a:OBJ_VX_LO,x
    lda a:$9350,y
    sta a:OBJ_VX_HI,x
    lda a:$9351,y
    sta a:OBJ_VY_LO,x
    lda a:$9352,y
    sta a:OBJ_VY_HI,x
    lda a:OBJ_2AC0,x
    beq SwingOnCosineThenLeapAtPlayer_at1
    jsr VecNegateVelocityX
SwingOnCosineThenLeapAtPlayer_at1:
    inc a:OBJ_27A8,x
SwingOnCosineThenLeapAtPlayer_at0:
    rts

; @routine 117C:15D LaunchWithSoundThenDrift
; @run 70:B95D
; WHAT IT DOES: In state 0 it plays sound $25, loads its record, sets its Y speed (high byte) to $F5, moves along Y
; and takes an X speed from a table of two words at overlay address $B992 by its facing. In any other state it steps
; the frame list at overlay address $B996 and moves along X.
; HOW: SoundCommand, LoadObjectRecord, ApplyVelocityY; or StepObjectFrames and ApplyVelocityX as a tail jump.
; WAITS: no.
; Entry: X object, Y kept to the frame step, D clear. Exit: state 0: A the X speed's low byte, N, Z from it, C 0,
; V from ApplyVelocityY, Y the facing index; later A, N, Z, C, V from ApplyVelocityX, Y the step's.
LaunchWithSoundThenDrift:
    lda a:OBJ_27A8,x
    bne LaunchWithSoundThenDrift_at0
    lda #$25
    jsr VecSoundCommand
    jsr VecLoadObjectRecord
    lda #$F5
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda a:OBJ_2AC0,x
    asl a
    tay
    lda a:$B993,y
    sta a:OBJ_VX_HI,x
    lda a:$B992,y
    sta a:OBJ_VX_LO,x
    rts
LaunchWithSoundThenDrift_at0:
    lda #$96
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    jmp VecApplyVelocityX

; @routine 1173:299 StartAtRandomSpeedAndHeight
; @run 6C:9299
; WHAT IT DOES: Loads the record (LoadObjectRecordFromTable), takes a random life_timer, an X speed and X position
; from a table of four byte rows at overlay address $92D4 by one random bit, a Y low byte from a table at $92DC by
; two more random bits, and its facing from the side of the player.
; HOW: NextRandom twice, SideOfPlayer.
; WAITS: no.
; Entry: X object, D clear. Exit: A the side bit and N, Z from it, C from SideOfPlayer's compare (or the second
; random's add), V from the random; Y the table index of the Y low byte.
StartAtRandomSpeedAndHeight:
    jsr LoadObjectRecordFromTable
    jsr VecNextRandom
    sta a:OBJ_34B8,x
    and #$01
    asl a
    asl a
    tay
    lda a:$92D5,y
    sta a:OBJ_VX_HI,x
    lda a:$92D4,y
    sta a:OBJ_VX_LO,x
    lda a:$92D7,y
    sta a:OBJ_X_HI,x
    lda a:$92D6,y
    sta a:OBJ_X_LO,x
    jsr VecNextRandom
    and #$03
    tay
    lda a:$92DC,y
    sta a:OBJ_Y_LO,x
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
    rts

; @routine 117B:5AE ChargeWhenPlayerNearB
; @run 70:B5AE
; WHAT IT DOES: Animates the object facing the player (list 0). On the first screen across and down, when the player
; is under $78 pixels away along X it stops its animation, keeps bit 7 of its flags, takes an X speed (high byte) from
; a table at overlay address $B5F3 by its facing, a Y speed of 2, sets f36c8 to $48 plus five bits of PlayerDistanceSum
; and enters state 4 with step 2.
; HOW: StepFramesFacingPlayerD, DistanceXToPlayer, PlayerDistanceSum.
; WAITS: no.
; Entry: X object, D clear. Exit: A 2 and N, Z clear, C and V from the add of $48 when it charged; else as the
; animation step, the position test or the distance compare left them.
ChargeWhenPlayerNearB:
    cly
    jsr StepFramesFacingPlayerD
    lda a:OBJ_X_HI,x
    ora a:OBJ_Y_HI,x
    bne ChargeWhenPlayerNearB_at0
    jsr VecDistanceXToPlayer
    cmp #$78
    bcs ChargeWhenPlayerNearB_at0
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda a:OBJ_FLAGS_2F38,x
    and #$80
    sta a:OBJ_FLAGS_2F38,x
    ldy a:OBJ_2AC0,x
    lda a:$B5F3,y
    sta a:OBJ_VX_HI,x
    lda #$02
    sta a:OBJ_VY_HI,x
    jsr VecPlayerDistanceSum
    and #$1F
    clc
    adc #$48
    sta a:OBJ_36C8,x
    lda #$04
    sta a:OBJ_27A8,x
    lda #$02
    sta a:OBJ_2800,x
ChargeWhenPlayerNearB_at0:
    rts
