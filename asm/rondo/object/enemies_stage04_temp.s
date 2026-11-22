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
    dec a:OBJ_3510,x
    bne CountdownThenState4_at1
CountdownThenState4_at0:
    lda #$04
    sta a:OBJ_27A8,x
CountdownThenState4_at1:
    rts

; @routine 07DB:785 ChangeToType9B
; @run 70:B785
; WHAT IT DOES: Turns the object into one of type $9B, in state 0.
; HOW: Stores $9B in its type byte (OBJ_2750) and zero in its state.
; WAITS: no.
; Entry: X object. Exit: A $9B, N set, Z clear; X, Y and C kept.
ChangeToType9B:
    lda #$9B
    sta a:OBJ_2750,x
    stz a:OBJ_27A8,x
    rts

; @routine 07DB:7B1 EnterState6IfFlagged
; @run 70:B7B1
; WHAT IT DOES: Sends the object to state 6 when bit 7 of its f31a0 is set.
; HOW: Reads f31a0 once and masks bit 7; writes the state only when it is set.
; WAITS: no.
; Entry: X object. Exit: A the masked bit, or 6 when entered; N, Z from it; X, Y and C kept.
EnterState6IfFlagged:
    lda a:OBJ_31A0,x
    and #$80
    beq EnterState6IfFlagged_at0
    lda #$06
    sta a:OBJ_27A8,x
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
    dec a:OBJ_34B8,x
    bne CountdownStopThenState4_at0
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$04
    sta a:OBJ_27A8,x
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
    lda a:OBJ_2750+$4D
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
    sta a:OBJ_3568,x
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
    sta a:OBJ_34B8,x
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
StepFramesFacingPlayer:
    lda a:OBJ_27A8,x
    cmp #$02
    bcc StepFramesFacingPlayer_at1
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
StepFramesFacingPlayer_at1:
    lda a:$B409,y
    sta z:zTemp10
StepFramesFacingPlayer_at0:
    lda a:$B40A,y
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
    lda a:OBJ_2800,x
    bne WaitThenState2_at1
    lda #$1C
    sta a:OBJ_34B8,x
    inc a:OBJ_2800,x
    rts
WaitThenState2_at1:
    ldy #$02
    jsr StepFramesFacingPlayer
    dec a:OBJ_34B8,x
    bne WaitThenState2_at2
    lda #$02
WaitThenState2_at0:
    sta a:OBJ_27A8,x
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
    sta a:OBJ_2750,x
    stz a:OBJ_27A8,x
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
WalkWhileFrameChanges:
    cly
    jsr StepFramesFacingPlayerAlways
    bcc WalkWhileFrameChanges_at0
    ldy a:OBJ_3510,x
    lda a:$B804,y
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
StepFramesFacingPlayerAlways:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
StepFramesFacingPlayerAlways_at0:
    lda a:$B82C,y
    sta z:zTemp10
StepFramesFacingPlayerAlways_at1:
    lda a:$B82D,y
    sta z:zTemp11
    jmp VecStepObjectFrames

