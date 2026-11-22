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
