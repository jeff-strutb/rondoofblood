; TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
;
;
;
;
;
;
;
; TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
;
;
;
;
;
;
;
; TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
;
;
;
;
;
;
;
; TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
;
;
;
;
;
;
;
; TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
;
;
;
;
;
;
;
; TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
;
;
;
;
;
;
;
; TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
;
;
;
;
;
;
;
; TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
;
;
;
;
;
;
;
; TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
;
;
;
;
;
;
;
; TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
;
;
;
;
;
;
;
; TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
;
;
;
;
;
;
;
; TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
;
;
;
;
;
;
;
; TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
;
;
;
;
;
;
;
; TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
;
;
;
;
;
;
;
; TEMPORARY: the stage 04 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
;
;
;
;
;
;
;
; @routine 07DB:5FD ClampSpeedXToTwo
; @run 70:B5FD
; WHAT IT DOES: Keeps an object's X speed from passing 2 pixels a frame either way: a speed over
; +2 becomes +2.00 and one under -2 becomes -2.00.
; HOW: Reads the speed's high byte (twice, as the sign test and the compare each load it) and,
; past the limit, stores the limit in the high byte and zero in the low byte.
; WAITS: no.
; Entry: X object. Exit: A and N, Z, C from the last compare or load (C the compare's, also when the
; speed is clamped); X and Y kept.
ClampSpeedXToTwo:
    lda a:OBJ_VX_HI,x
    bmi ClampSpeedXToTwo_minus
    lda a:OBJ_VX_HI,x
    cmp #$02
    bcc ClampSpeedXToTwo_out
    lda #$02
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
ClampSpeedXToTwo_out:
    rts
ClampSpeedXToTwo_minus:
    lda a:OBJ_VX_HI,x
    cmp #$FE
    bcs ClampSpeedXToTwo_out
    lda #$FE
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    rts

; @routine 07DB:622 ThrowUpward
; @run 70:B622
; WHAT IT DOES: Throws an object upward at 3 pixels a frame, unless it is already rising slowly
; (its Y speed between -2.00 and 0 counted as a high byte of $FE or $FF).
; HOW: Reads the Y speed's high byte; below $FE it stores $FD there and zero in the low byte.
; WAITS: no.
; Entry: X object. Exit: A and N, Z, C from the last compare or load; X and Y kept.
ThrowUpward:
    lda a:OBJ_VY_HI,x
    cmp #$FE
ThrowUpward_at0:
    bcs ThrowUpward_at1
    lda #$FD
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
ThrowUpward_at1:
    rts

; @routine 07DB:71B CountdownThenState4
; @run 70:B71B
; WHAT IT DOES: Counts the object's f3510 timer down and, when it reaches zero, sends the
; object to state 4.
; HOW: One decrement of the timer; the state byte is written only at zero.
; WAITS: no.
; Entry: X object. Exit: A and N, Z from the timer, or A 4 at zero; X, Y and C kept.
CountdownThenState4:
    dec a:OBJ_WORK_E,x
    bne CountdownThenState4_at1
CountdownThenState4_at0:
    lda #$04
    sta a:OBJ_STATE,x
CountdownThenState4_at1:
    rts

; @routine 07DB:785 ChangeToType9B
; @run 70:B785
; WHAT IT DOES: Turns the object into one of type $9B, in state 0.
; HOW: Stores $9B in its type byte (OBJ_KIND) and zero in its state.
; WAITS: no.
; Entry: X object. Exit: A $9B, N set, Z clear; X, Y and C kept.
ChangeToType9B:
    lda #$9B
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts

; @routine 07DB:7B1 EnterState6IfFlagged
; @run 70:B7B1
; WHAT IT DOES: Sends the object to state 6 when bit 7 of its f31a0 is set.
; HOW: Reads f31a0 once and masks bit 7; writes the state only when it is set.
; WAITS: no.
; Entry: X object. Exit: A the masked bit, or 6 when entered; N, Z from it; X, Y and C kept.
EnterState6IfFlagged:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq EnterState6IfFlagged_at0
    lda #$06
    sta a:OBJ_STATE,x
EnterState6IfFlagged_at0:
    rts

; @routine 07DC:09D CountdownStopThenState4
; @run 70:B89D
; WHAT IT DOES: Counts the object's f34b8 timer down and, when it reaches zero, clears its f3408
; and f3460 and sends it to state 4.
; HOW: One decrement of the timer; the rest only at zero.
; WAITS: no.
; Entry: X object. Exit: A and N, Z from the timer, or A 4 at zero; X, Y and C kept.
CountdownStopThenState4:
    dec a:OBJ_WORK_D,x
    bne CountdownStopThenState4_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_STATE,x
CountdownStopThenState4_at0:
    rts

; @routine 07DF:349 RightOfObject4DWithin
; @run 71:D349
; WHAT IT DOES: Says, in the carry, whether the object stands to the right of the object in slot
; $4D by less than the distance in A, when slot $4D holds an object of type $AC.
; HOW: Stores A as the limit in zTemp00; checks slot $4D's type; subtracts the two X positions
; (low bytes into zTemp01, then high bytes); the answer is yes only when the high bytes are
; equal and the low difference is under the limit.
; WAITS: no.
; Entry: X object, A limit. Exit: C set for yes; A the last value compared; N, Z from it. Clobbers
; zTemp00-01. The arms after the type check ran in no script, so the draft stops there and
; liftcmp judges only the type check; the rest waits for coverage (PROCEDURE section 6).
RightOfObject4DWithin:
    sta z:zTemp00
    lda a:OBJ_KIND+$4D
    cmp #$AC
    bne RightOfObject4DWithin_at0
    lda a:OBJ_X_LO,x
    sec
    sbc a:OBJ_X_LO+$4D
    sta z:zTemp01
    lda a:OBJ_X_HI,x
    sbc a:OBJ_X_HI+$4D
    bne RightOfObject4DWithin_at0
    lda z:zTemp01
    cmp z:zTemp00
    bcs RightOfObject4DWithin_at0
    sec
    rts
RightOfObject4DWithin_at0:
    clc
    rts

; @routine 07DB:726 LoadRecordStartAngle40
; @run 70:B726
; WHAT IT DOES: Starts an object from its record, moves it by its velocity, and sets its angle's
; high byte to $40.
; HOW: LoadObjectRecord, ApplyVelocity, one store.
; WAITS: no.
; Entry: X object. Exit: A $40, N and Z clear; C and V from ApplyVelocity's add; Y 11; X kept.
LoadRecordStartAngle40:
    jsr VecLoadObjectRecord
    jsr VecApplyVelocity
LoadRecordStartAngle40_at0:
    lda #$40
    sta a:OBJ_WORK_F,x
    rts

; @routine 07DB:7BE LoadRecordTimer50
; @run 70:B7BE
; WHAT IT DOES: Starts an object from its record and sets its f34b8 timer to $50.
; HOW: LoadObjectRecord, one store.
; WAITS: no.
; Entry: X object. Exit: A $50, N and Z clear; C from the record; Y 11; X and V kept.
LoadRecordTimer50:
    jsr VecLoadObjectRecord
    lda #$50
    sta a:OBJ_WORK_D,x
    rts

; @routine 07DB:3ED StepFramesFacingPlayer
; @run 70:B3ED
; WHAT IT DOES: Steps the object's animation by one frame from the list whose pointer is the
; pair of bytes at overlay address $B409 plus Y, first turning the object to face the player
; (the facing byte takes bit 0 of the side the player is on) once the object is past state 1.
; HOW: SideOfPlayer, then the list pointer into zTemp10-11, then StepObjectFrames as a tail jump.
; WAITS: no.
; Entry: X object, Y the list's offset in the table. Exit: as StepObjectFrames (A $FF and C 1
; when the frame changed; A 0 and C 0 when the list ended; A $FF and C 0 when held; Y changed
; only in the first two cases); V and X kept. Clobbers zTemp00, zTemp10-11.
StepFramesFacingPlayerLists = $B409 ; a list's address by index
StepFramesFacingPlayer:
    lda a:OBJ_STATE,x
    cmp #$02
    bcc StepFramesFacingPlayer_at1
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
StepFramesFacingPlayer_at1:
    lda a:StepFramesFacingPlayerLists,y
    sta z:zTemp10
StepFramesFacingPlayer_at0:
    lda a:StepFramesFacingPlayerLists+1,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 07DB:6CE WaitThenState2
; @run 70:B6CE
; WHAT IT DOES: A two-step wait. First time through (step byte 0) it sets the object's f34b8
; timer to $1C and the step to 1. After that it animates the object facing the player with the
; list at index 2 and counts the timer down; at zero the object enters state 2.
; HOW: StepFramesFacingPlayer with Y 2, then a decrement of the timer.
; WAITS: no.
; Entry: X object. Exit: N, Z from the timer or A 2 at zero; C and Y as the animation step left
; them (Y 2 if it did not change it).
WaitThenState2:
    lda a:OBJ_SUBSTATE,x
    bne WaitThenState2_at1
    lda #$1C
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
    rts
WaitThenState2_at1:
    ldy #$02
    jsr StepFramesFacingPlayer
    dec a:OBJ_WORK_D,x
    bne WaitThenState2_at2
    lda #$02
WaitThenState2_at0:
    sta a:OBJ_STATE,x
WaitThenState2_at2:
    rts

; @routine 07DB:6EC BecomeType9CWhenAnimationEnds
; @run 70:B6EC
; WHAT IT DOES: Animates the object (list 8, facing the player) and, when the animation list has
; ended, turns it into an object of type $9C in state 0.
; HOW: StepFramesFacingPlayer with Y 8; Z from it says the list ended.
; WAITS: no.
; Entry: X object. Exit: as the animation step, or A $9C and N set, Z clear (C and Y from the step).
BecomeType9CWhenAnimationEnds:
    ldy #$08
    jsr StepFramesFacingPlayer
    bne BecomeType9CWhenAnimationEnds_at0
    lda #$9C
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
BecomeType9CWhenAnimationEnds_at0:
    rts

; @routine 07DB:7F1 WalkWhileFrameChanges
; @run 70:B7F1
; WHAT IT DOES: Animates the object facing the player and, on the frames where the picture
; changed, gives it an X speed from a two byte table by its f3510 and moves it along X.
; HOW: StepFramesFacingPlayerAlways with Y 0; when it says the frame changed, the speed byte
; at overlay address $B804 plus f3510 goes to the X speed's high byte and ApplyVelocityX runs.
; WAITS: no.
; Entry: X object. Exit: after a change, as ApplyVelocityX; otherwise as the animation step
; (A $FF, C 0, or A 0, C 0). Clobbers zTemp00, zTemp10-11, zTemp-bytes of the move.
WalkWhileFrameChangesVxHis = $B804 ; table WalkWhileFrameChanges reads
WalkWhileFrameChanges:
    cly
    jsr StepFramesFacingPlayerAlways
    bcc WalkWhileFrameChanges_at0
    ldy a:OBJ_WORK_E,x
    lda a:WalkWhileFrameChangesVxHis,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
WalkWhileFrameChanges_at0:
    rts

; @routine 07DC:017 StepFramesFacingPlayerAlways
; @run 70:B817
; WHAT IT DOES: StepFramesFacingPlayer without the state test: the object always turns to face
; the player, and the animation list is the pair of bytes at overlay address $B82C plus Y.
; HOW: SideOfPlayer, the list pointer into zTemp10-11, then StepObjectFrames as a tail jump.
; WAITS: no.
; Entry, exit: as StepFramesFacingPlayer.
StepFramesFacingPlayerAlwaysLists = $B82C ; a list's address by index
StepFramesFacingPlayerAlways:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
StepFramesFacingPlayerAlways_at0:
    lda a:StepFramesFacingPlayerAlwaysLists,y
    sta z:zTemp10
StepFramesFacingPlayerAlways_at1:
    lda a:StepFramesFacingPlayerAlwaysLists+1,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 07DB:336 SpawnSixPiecesA
; @run 70:B336
; WHAT IT DOES: Starts up to six objects of type $21 at the object's place, each from a free slot
; from $30, giving each its life_timer, f3510 and speeds from a four byte entry of a table at
; overlay address $B36C.
; HOW: A loop over pieces 0 to 5 (kept in zHitSlot, the object in zRichterObject while the
; table is read): FindFreeSlotFrom30, StartObjectAtPlace, the four stores; it stops when no slot
; is free.
; WAITS: no.
; Entry: X object, D clear. Exit: after six pieces A the last speed byte, N, Z, C from the compare
; of Y with 6, Y 6; when no slot was free as FindFreeSlotFrom30 left it. X kept. Clobbers
; zHitSlot, zRichterObject, zTemp10-11 (record reads), and the stage's cleared slots.
SpawnSixPiecesAWorkDs = $B36C ; table SpawnSixPiecesA reads
SpawnSixPiecesAVxHis = $B36E ; table SpawnSixPiecesA reads
SpawnSixPiecesA:
    cly
SpawnSixPiecesA_at1:
    sty z:zHitSlot
    jsr VecFindFreeSlotFrom30
    bcc SpawnSixPiecesA_at0
    lda #$21
    jsr VecStartObjectAtPlace
    stx z:zRichterObject
    lda z:zHitSlot
    asl a
    asl a
    tax
    lda a:SpawnSixPiecesAWorkDs,x
    sta a:OBJ_WORK_D,y
    lda a:SpawnSixPiecesAWorkDs+1,x
    sta a:OBJ_WORK_E,y
    lda a:SpawnSixPiecesAVxHis,x
    sta a:OBJ_VX_HI,y
    lda a:SpawnSixPiecesAVxHis+1,x
    sta a:OBJ_VY_HI,y
    ldx z:zRichterObject
    ldy z:zHitSlot
    iny
    cpy #$06
    bcc SpawnSixPiecesA_at1
SpawnSixPiecesA_at0:
    rts

; @routine 07DB:384 LoadRecordFacingAndRandomFrame
; @run 70:B384
; WHAT IT DOES: Starts the object from its record, makes its facing bit 0 of its next_kind, and
; gives its animation step a random value from 0 to 15.
; HOW: LoadObjectRecord, NextRandom masked to a nibble.
; WAITS: no.
; Entry: X object, D clear. Exit: A the random nibble, N, Z from it; C and V from NextRandom's
; last add; Y 11; X kept.
LoadRecordFacingAndRandomFrame:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    and #$01
    sta a:OBJ_FACING_LEFT,x
    jsr VecNextRandom
    and #$0F
    sta a:OBJ_ANIM_STEP,x
    rts

; @routine 07DB:4E7 PickSpeedAndTurnFromTableB
; @run 70:B4E7
; WHAT IT DOES: PickSpeedAndTurnFromTable in this stage's overlay: sets the object's X and Y
; speeds from a table of four byte entries (at overlay address $B513) by its f3670, adds one to
; f3510, and reverses its X speed when the player is on its left.
; HOW: SideOfPlayer; NegateVelocityX as a tail jump.
; WAITS: no.
; Entry, exit: as PickSpeedAndTurnFromTable.
PickSpeedAndTurnFromTableBVxLos = $B513 ; table PickSpeedAndTurnFromTableB reads
PickSpeedAndTurnFromTableBVyLos = $B515 ; table PickSpeedAndTurnFromTableB reads
PickSpeedAndTurnFromTableB:
    lda a:OBJ_WORK_I,x
    asl a
    asl a
    tay
    lda a:PickSpeedAndTurnFromTableBVxLos+1,y
    sta a:OBJ_VX_HI,x
    lda a:PickSpeedAndTurnFromTableBVxLos,y
    sta a:OBJ_VX_LO,x
    lda a:PickSpeedAndTurnFromTableBVyLos+1,y
    sta a:OBJ_VY_HI,x
    lda a:PickSpeedAndTurnFromTableBVyLos,y
    sta a:OBJ_VY_LO,x
    inc a:OBJ_WORK_E,x
    jsr VecSideOfPlayer
    and #$01
    beq PickSpeedAndTurnFromTableB_at0
    jmp VecNegateVelocityX
PickSpeedAndTurnFromTableB_at0:
    rts

; @routine 07DB:73B OrbitStepBack
; @run 70:B73B
; WHAT IT DOES: Moves the object on a circle: its velocity from the angle in angle_hi at speed
; 1.0 (SetVelocityFromAngle), moves it, and turns the angle back by 4; when the angle reaches
; $E0 it is set to $40 and the object moves on to its next state.
; HOW: zTemp00-01 = $0100, SetVelocityFromAngle, ApplyVelocity, a subtract.
; WAITS: no.
; Entry: X object, D clear. Exit: A, N, Z, C from the compare with $E0, or A $40 and N, Z from the
; new state; V from the subtract; X, Y kept (Y 0 from the scale).
OrbitStepBack:
    stz z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_WORK_F,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_F,x
    sec
    sbc #$04
    sta a:OBJ_WORK_F,x
    cmp #$E0
    bne OrbitStepBack_at0
    lda #$40
    sta a:OBJ_WORK_F,x
    inc a:OBJ_STATE,x
OrbitStepBack_at0:
    rts

; @routine 07DB:760 OrbitStepForward
; @run 70:B760
; WHAT IT DOES: OrbitStepBack the other way: the angle turns on by 4; at $A0 it is set to $40
; and the object goes back one state.
; HOW: As OrbitStepBack with an add and a decrement.
; WAITS: no.
; Entry, exit: as OrbitStepBack, the compare being with $A0.
OrbitStepForward:
    stz z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_WORK_F,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_F,x
    clc
    adc #$04
    sta a:OBJ_WORK_F,x
    cmp #$A0
    bne OrbitStepForward_at0
    lda #$40
    sta a:OBJ_WORK_F,x
    dec a:OBJ_STATE,x
OrbitStepForward_at0:
    rts

; @routine 07DF:331 StartTypeAndLaunchUp
; @run 71:D331
; WHAT IT DOES: Starts a new object of type $9C where this one is (the new slot is in Y), gives
; it a Y speed of -$10 high byte ($F0) and moves it once by it.
; HOW: StartObjectAtPlace, a store through Y, then ApplyVelocityY on the new object (X and Y
; swapped around it), and its Y speed high byte cleared again.
; WAITS: no.
; Entry: X object, Y new slot, D clear. Exit: A, N, Z, C, V from ApplyVelocityY; X and Y kept.
StartTypeAndLaunchUp:
    lda #$9C
    jsr VecStartObjectAtPlace
    lda #$F0
    sta a:OBJ_VY_HI,y
    sxy
    jsr VecApplyVelocityY
    stz a:OBJ_VY_HI,x
    sxy
StartTypeAndLaunchUp_end:
    rts

; @routine 07DC:0C8 AnimateThenFireOrWait
; @run 70:B8C8
; WHAT IT DOES: Animates the object facing the player (list 2). When the list has ended it
; stops the animation, enters state 3, counts f3568 up and sets its life_timer from a four byte
; table at overlay address $B806 by f3568's low two bits. When the frame changed to $32 it starts
; an object of type $23 at its place in a free slot from $30.
; HOW: StepFramesFacingPlayerAlways; FindFreeSlotFrom30; StartObjectAtPlace.
; WAITS: no.
; Entry: X object, D clear. Exit: as the last step that ran (the animation step, a compare, the
; slot search or the start); Y 2, the table index, or the slot.
AnimateThenFireOrWaitWorkDs = $B806 ; table AnimateThenFireOrWait reads
AnimateThenFireOrWait:
    ldy #$02
    jsr StepFramesFacingPlayerAlways
    beq AnimateThenFireOrWait_at0
    bcc AnimateThenFireOrWait_at1
    lda a:OBJ_FRAME,x
    cmp #$32
    bne AnimateThenFireOrWait_at1
    jsr VecFindFreeSlotFrom30
    bcc AnimateThenFireOrWait_at1
    lda #$23
    jsr VecStartObjectAtPlace
    rts
AnimateThenFireOrWait_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
    inc a:OBJ_WORK_F,x
    lda a:OBJ_WORK_F,x
    and #$03
    tay
    lda a:AnimateThenFireOrWaitWorkDs,y
    sta a:OBJ_WORK_D,x
AnimateThenFireOrWait_at1:
    rts

; @routine 07DC:16B BurstIntoSixPieces
; @run 70:B96B
; WHAT IT DOES: Replaces the object by up to six objects of type $24 started at its place in free
; slots from $30, each facing as this one does and with its life_timer, f3510 and both speed
; bytes pairs from an eight byte entry of a table at overlay address $B9B6; then removes the
; object.
; HOW: A loop over pieces 0 to 5 as SpawnSixPiecesA; ClearObject as a tail jump.
; WAITS: no.
; Entry: X object, D clear. Exit: as the last compare of the piece count (C 1, Z 1, A the last
; speed byte), or as FindFreeSlotFrom30 left it when no slot was free; Y 6 or $40; X kept.
BurstIntoSixPiecesWorkDs = $B9B6 ; table BurstIntoSixPieces reads
BurstIntoSixPiecesVxHis = $B9B8 ; table BurstIntoSixPieces reads
BurstIntoSixPiecesVyHis = $B9BA ; table BurstIntoSixPieces reads
BurstIntoSixPieces:
    cly
BurstIntoSixPieces_at1:
    sty z:zHitSlot
    jsr VecFindFreeSlotFrom30
    bcc BurstIntoSixPieces_at0
    lda #$24
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    stx z:zRichterObject
    lda z:zHitSlot
    asl a
    asl a
    asl a
    tax
    lda a:BurstIntoSixPiecesWorkDs,x
    sta a:OBJ_WORK_D,y
    lda a:BurstIntoSixPiecesWorkDs+1,x
    sta a:OBJ_WORK_E,y
    lda a:BurstIntoSixPiecesVxHis,x
    sta a:OBJ_VX_HI,y
    lda a:BurstIntoSixPiecesVxHis+1,x
    sta a:OBJ_VX_LO,y
    lda a:BurstIntoSixPiecesVyHis,x
    sta a:OBJ_VY_HI,y
    lda a:BurstIntoSixPiecesVyHis+1,x
    sta a:OBJ_VY_LO,y
    ldx z:zRichterObject
    ldy z:zHitSlot
    iny
    cpy #$06
    bcc BurstIntoSixPieces_at1
BurstIntoSixPieces_at0:
    jmp VecClearObject

; @routine 07DB:311 BurstWhenFlaggedHit
; @run 70:B311
; WHAT IT DOES: When the object is not yet in state 6 and bit 7 of its status flags is set, plays
; sound $8B (first screen only), spawns its six pieces (SpawnSixPiecesA), stops its f3250 and its
; animation and enters state 6.
; HOW: Two tests, SoundIfOnFirstScreen, SpawnSixPiecesA.
; WAITS: no.
; Entry: X object, D clear. Exit: A 6 and N, Z clear when it burst, C and V as the piece spawn left
; them; otherwise A, N, Z, C from the failing test; X kept; Y as the spawn left it.
BurstWhenFlaggedHit:
    lda a:OBJ_STATE,x
    cmp #$06
    beq BurstWhenFlaggedHit_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq BurstWhenFlaggedHit_at0
    lda #$8B
    jsr VecSoundIfOnFirstScreen
    jsr SpawnSixPiecesA
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
BurstWhenFlaggedHit_at0:
    rts

; @routine 07DB:398 ChargeWhenPlayerNear
; @run 70:B398
; WHAT IT DOES: Animates the object facing the player (list 0). When its next_kind bit 7 is
; clear and the player is under $68 pixels away along X, it stops its animation, keeps bit 7 of
; its flags, takes an X speed from a table by its facing (at overlay address $B3E7) and Y speed
; from a table by its next_kind (at $B3E9), sets f36c8 to $48 plus five bits of PlayerDistanceSum,
; enters state 4 with step 2.
; HOW: StepFramesFacingPlayer, DistanceXToPlayer, PlayerDistanceSum.
; WAITS: no.
; Entry: X object, D clear. Exit: A 2 and N, Z clear, C and V from the add of $48 when it
; charged; else as the animation step, the next_kind test or the distance compare left them.
ChargeWhenPlayerNearVxHis = $B3E7 ; table ChargeWhenPlayerNear reads
ChargeWhenPlayerNearVyLos = $B3E9 ; table ChargeWhenPlayerNear reads
ChargeWhenPlayerNear:
    cly
    jsr StepFramesFacingPlayer
    lda a:OBJ_START_ARG,x
    bmi ChargeWhenPlayerNear_at0
    jsr VecDistanceXToPlayer
    cmp #$68
    bcs ChargeWhenPlayerNear_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda a:OBJ_SCROLL_BAND,x
    and #$80
    sta a:OBJ_SCROLL_BAND,x
    ldy a:OBJ_FACING_LEFT,x
    lda a:ChargeWhenPlayerNearVxHis,y
    sta a:OBJ_VX_HI,x
    lda a:OBJ_START_ARG,x
    lsr a
    asl a
    tay
    lda a:ChargeWhenPlayerNearVyLos,y
    sta a:OBJ_VY_LO,x
    lda a:ChargeWhenPlayerNearVyLos+1,y
    sta a:OBJ_VY_HI,x
    jsr VecPlayerDistanceSum
    and #$1F
    clc
    adc #$48
    sta a:OBJ_WORK_J,x
    lda #$04
    sta a:OBJ_STATE,x
    lda #$02
    sta a:OBJ_SUBSTATE,x
ChargeWhenPlayerNear_at0:
    rts

; @routine 07DB:46E PickStateByPlayerDistance
; @run 70:B46E
; WHAT IT DOES: Clears the step byte, then: an object off the first screen across goes to state
; 3; one off it down goes to state 4; one on it takes a state from a table at overlay address
; $B494 indexed by the low nibble of PlayerDistanceSum.
; HOW: Two tests of the position's high bytes, then PlayerDistanceSum.
; WAITS: no.
; Entry: X object, C in (kept to the sum), D clear. Exit: A the state stored, N, Z from it; C and V
; from PlayerDistanceSum on the last path; Y the table index there. The second branch ran in no
; script: the draft stops there, so liftcmp does not judge it.
PickStateByPlayerDistanceStates = $B494 ; table PickStateByPlayerDistance reads
PickStateByPlayerDistance:
    stz a:OBJ_SUBSTATE,x
    lda a:OBJ_X_HI,x
    beq PickStateByPlayerDistance_at0
    lda #$03
    sta a:OBJ_STATE,x
    rts
PickStateByPlayerDistance_at0:
    lda a:OBJ_Y_HI,x
    beq PickStateByPlayerDistance_at1
    lda #$04
    sta a:OBJ_STATE,x
    rts
PickStateByPlayerDistance_at1:
    jsr VecPlayerDistanceSum
    and #$0F
    tay
    lda a:PickStateByPlayerDistanceStates,y
    sta a:OBJ_STATE,x
    rts

; @routine 07DB:568 AnimateMoveThenSetUpNextStep
; @run 70:B568
; WHAT IT DOES: Animates the object facing the player (list 4), moves it, and counts life_timer
; down; at zero it plays sound $8B (first screen only), picks a speed and turn
; (PickSpeedAndTurnFromTableB), sets f36c8 to $58 plus six bits of PlayerDistanceSum, stops its
; animation and advances its step.
; HOW: StepFramesFacingPlayer, ApplyVelocity, a decrement, then the calls named.
; WAITS: no.
; Entry: X object, C in (kept to the sum), D clear. Exit: while counting, N, Z from the timer, C and
; V from ApplyVelocity, Y as the animation step left it; at zero A the timer value, N, Z from the
; new step, C and V from the add of $58, Y from the speed pick.
AnimateMoveThenSetUpNextStep:
    ldy #$04
    jsr StepFramesFacingPlayer
    jsr VecApplyVelocity
    dec a:OBJ_WORK_D,x
    bne AnimateMoveThenSetUpNextStep_at0
    lda #$8B
    jsr VecSoundIfOnFirstScreen
    jsr PickSpeedAndTurnFromTableB
    jsr VecPlayerDistanceSum
    and #$3F
    clc
    adc #$58
    sta a:OBJ_WORK_J,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
AnimateMoveThenSetUpNextStep_at0:
    rts

; @routine 07DB:632 SoundEveryFourthFrameB
; @run 70:B632
; WHAT IT DOES: Plays sound $22, on the first screen only, on the frames when the low two bits of
; the object's f3618 are zero.
; HOW: One test and SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X object, D clear. Exit: A the masked bits and N, Z from them when no sound; else as
; SoundIfOnFirstScreen leaves it.
SoundEveryFourthFrameB:
    lda a:OBJ_WORK_H,x
    and #$03
    bne SoundEveryFourthFrameB_at0
    lda #$22
    jsr VecSoundIfOnFirstScreen
SoundEveryFourthFrameB_at0:
    rts

; @routine 07DB:67B TurnOnRandomBeatWalkA
; @run 70:B67B
; @stop B6AC
; WHAT IT DOES: A walk that turns on a random beat: at its first step it turns to the player's side,
; takes an X speed from a table at overlay address $B6AC (two bytes by side), picks life_timer
; at random from eight bytes at $B6B0, stops the animation and moves on; after that it runs
; TurnOnRandomBeatWalkFollowA.
; HOW: The shared body; the two tables lie between the two parts and stay data.
; WAITS: no.
; Entry: X object, D clear. Exit: first step: A the timer, N, Z from the new step, C and V from
; NextRandom, Y its index; later as TurnOnRandomBeatWalkFollowA.
TurnOnRandomBeatWalkAVxHis = $B6AC ; table TurnOnRandomBeatWalkA reads
TurnOnRandomBeatWalkAWorkDs = $B6B0 ; table TurnOnRandomBeatWalkA reads
TurnOnRandomBeatWalkA:
    lda a:OBJ_SUBSTATE,x
    bne TurnOnRandomBeatWalkFollowA
    jsr VecSideOfPlayer
    and #$01
    asl a
    tay
    lda a:TurnOnRandomBeatWalkAVxHis,y
    sta a:OBJ_VX_HI,x
    lda a:TurnOnRandomBeatWalkAVxHis+1,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$07
    tay
    lda a:TurnOnRandomBeatWalkAWorkDs,y
    sta a:OBJ_WORK_D,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 07DB:6B8 TurnOnRandomBeatWalkFollowA
; @run 70:B6B8
; WHAT IT DOES: The later steps of TurnOnRandomBeatWalkA: animates the object facing the player (list 2),
; counts life_timer down and moves it along X; at zero it enters state 5 with step 0.
; HOW: StepFramesFacingPlayer, a decrement, ApplyVelocityX as a tail jump.
; WAITS: no.
; Entry: X object, D clear. Exit: as the animation step, the decrement and ApplyVelocityX leave them
; (A 5 and N, Z clear at zero).
TurnOnRandomBeatWalkFollowA:
    ldy #$02
    jsr StepFramesFacingPlayer
    dec a:OBJ_WORK_D,x
    beq TurnOnRandomBeatWalkFollowA_done
    jmp VecApplyVelocityX
TurnOnRandomBeatWalkFollowA_done:
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 07DC:250 FallAnimatedThenBecomeKind9B
; @run 70:BA50
; WHAT IT DOES: In state 0 it loads its record and, when the player is on its left (bit 0 of
; SideOfPlayer), reverses its X speed. In any other state it counts f3510 down; at zero it becomes
; kind $9B in state 0. Until then it steps the frame list picked by life_timer from a table at
; overlay address $BA89 (two bytes each), applies gravity without a limit and moves.
; HOW: LoadObjectRecord, SideOfPlayer, NegateVelocityX; or a decrement; or StepObjectFrames,
; ApplyGravityUnlimited and ApplyVelocity as a tail jump. The reversal ran in no script: the draft
; stops there, so liftcmp does not judge it.
; WAITS: no.
; Entry: X object, D clear. Exit: state 0: as LoadObjectRecord, then SideOfPlayer's bit, and the
; reversal's add when it ran; counting out: A, N, Z from the decrement (A 0); moving: A, N, Z, C
; and V from ApplyVelocity, Y from the frame step.
FallAnimatedThenBecomeKind9BLists = $BA89 ; a list's address by index
FallAnimatedThenBecomeKind9B:
    lda a:OBJ_STATE,x
    bne FallAnimatedThenBecomeKind9B_at0
    jsr VecLoadObjectRecord
    jsr VecSideOfPlayer
    and #$01
    beq FallAnimatedThenBecomeKind9B_at1
    jsr VecNegateVelocityX
FallAnimatedThenBecomeKind9B_at1:
    rts
FallAnimatedThenBecomeKind9B_at0:
    dec a:OBJ_WORK_E,x
    bne FallAnimatedThenBecomeKind9B_at2
    lda #$9B
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts
FallAnimatedThenBecomeKind9B_at2:
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:FallAnimatedThenBecomeKind9BLists,y
    sta z:zTemp10
    lda a:FallAnimatedThenBecomeKind9BLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 07DC:1E6 StartLobTowardPlayer
; @run 70:B9E6
; @stop BA21
; WHAT IT DOES: In state 0 it loads its record and throws itself on an arc toward the player: an X speed from a
; table of word pairs at overlay address $BA21 by the player's distance along X (a pair per 32 pixels), the Y speed
; -$0480, reversed in X when the player is on its left. In any other state it runs StepLobArc.
; HOW: LoadObjectRecord, ApplyVelocityY, DistanceXToPlayer, SideOfPlayer, NegateVelocityX; the table lies between
; the two parts and stays data. The reversal ran in no script: the draft stops there, so liftcmp does not judge it.
; WAITS: no.
; Entry: X object, Y kept to the frame step, D clear. Exit: state 0: A the Y speed's low byte or the reversal's, N, Z
; from it, C from the table shift, V from the distance, Y the table index; later as StepLobArc.
StartLobTowardPlayer:
    lda a:OBJ_STATE,x
    bne StepLobArc
    jsr VecLoadObjectRecord
    lda #$F0
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    jsr VecDistanceXToPlayer
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    asl a
    tay
    lda a:RichterItemUseCosts+5,y
    sta a:OBJ_VX_HI,x
    lda a:RichterItemUseCosts+4,y
    sta a:OBJ_VX_LO,x
    lda #$FB
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    jsr VecSideOfPlayer
    and #$01
    beq StartLobTowardPlayer_at0
    jsr VecNegateVelocityX
StartLobTowardPlayer_at0:
    rts

; @routine 07DC:231 StepLobArc
; @run 70:BA31
; WHAT IT DOES: The later frames of the arc: steps the frame list at overlay address $B848, makes the Y speed
; grow by $24 and moves the object.
; HOW: StepObjectFrames, a sixteen bit add, ApplyVelocity as a tail jump.
; WAITS: no.
; Entry: X object, Y kept by the step, D clear. Exit: A, N, Z, C, V from ApplyVelocity; Y the step's.
StepLobArc:
    lda #$B8
    sta z:zTemp11
    lda #$48
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_VY_LO,x
    clc
    adc #$24
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jmp VecApplyVelocity
