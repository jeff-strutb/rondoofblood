; TEMPORARY: the stage 05 overlay's object routines whose enemy kind is not yet identified, kept
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
; TEMPORARY: the stage 05 overlay's object routines whose enemy kind is not yet identified, kept
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
; TEMPORARY: the stage 05 overlay's object routines whose enemy kind is not yet identified, kept
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
; TEMPORARY: the stage 05 overlay's object routines whose enemy kind is not yet identified, kept
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
; TEMPORARY: the stage 05 overlay's object routines whose enemy kind is not yet identified, kept
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
; TEMPORARY: the stage 05 overlay's object routines whose enemy kind is not yet identified, kept
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
; TEMPORARY: the stage 05 overlay's object routines whose enemy kind is not yet identified, kept
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
; TEMPORARY: the stage 05 overlay's object routines whose enemy kind is not yet identified, kept
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
; TEMPORARY: the stage 05 overlay's object routines whose enemy kind is not yet identified, kept
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
; TEMPORARY: the stage 05 overlay's object routines whose enemy kind is not yet identified, kept
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
; TEMPORARY: the stage 05 overlay's object routines whose enemy kind is not yet identified, kept
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
; TEMPORARY: the stage 05 overlay's object routines whose enemy kind is not yet identified, kept
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
; @routine 0938:7F2 SelectRasterStage1
; @run 6F:BFF2
; WHAT IT DOES: Sets the stage value the raster effect tests to 1.
; HOW: One store into zRasterStage.
; WAITS: no.
; Entry: nothing. Exit: A 1, N and Z clear; X, Y, C and V kept.
SelectRasterStage1:
    lda #$01
    sta z:zRasterStage
    rts

; @routine 093C:10A SetHitboxDxFromFrame
; @run 70:B90A
; WHAT IT DOES: Sets the object's f3250 to 2 and its hit rectangle's X offset from a table by
; its frame, for frames 1 to 6; any other frame clears both.
; HOW: The table is the byte list at overlay address $B928, indexed by frame minus one. Frame 0
; is not in range of the first test's subtract and reads the table at index $FF.
; WAITS: no.
; Entry: X object. Exit: A the table byte, or the frame; N, Z, C from the last compare or
; subtract; Y the table index when read; X kept.
SetHitboxDxFromFrame:
    lda #$02
    sta a:OBJ_3250,x
    lda a:OBJ_2A68,x
    cmp #$07
    bcs SetHitboxDxFromFrame_at0
    sec
    sbc #$01
    tay
    lda a:$B928,y
    sta a:OBJ_3930,x
    rts
SetHitboxDxFromFrame_at0:
    stz a:OBJ_3250,x
    stz a:OBJ_3930,x
    rts

; @routine 093C:374 PhaseTimerToState1
; @run 70:BB74
; WHAT IT DOES: Counts the object's f3720 down. While it runs, the object's animation is held
; at its start and its step byte is 1; at zero the object enters state 1 with frame 1, or
; frame 4 when its f36c8 is set.
; HOW: One decrement; each branch then stores its constants.
; WAITS: no.
; Entry: X object. Exit: A and N, Z from the last value loaded; X, Y, C and V kept.
PhaseTimerToState1:
    dec a:OBJ_3720,x
    beq PhaseTimerToState1_at0
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$01
    sta a:OBJ_2800,x
    rts
PhaseTimerToState1_at0:
    lda #$01
    sta a:OBJ_27A8,x
    lda #$01
    sta a:OBJ_2A68,x
    lda a:OBJ_36C8,x
    beq PhaseTimerToState1_at1
    lda #$04
    sta a:OBJ_2A68,x
PhaseTimerToState1_at1:
    rts

; @routine 093C:586 SetHitboxFromFrameRows
; @run 70:BD86
; WHAT IT DOES: Sets the object's whole hit rectangle (X offset, Y offset, half width, half
; height) from a four byte row of a table, the row chosen by the frame minus $13.
; HOW: The table is at overlay address $BDA8.
; WAITS: no.
; Entry: X object. Exit: A the last byte loaded; N, Z from it; C bit 6 of the frame minus $13
; (the last shift); Y the row's offset; X kept.
SetHitboxFromFrameRows:
    lda a:OBJ_2A68,x
    sec
    sbc #$13
    asl a
    asl a
    tay
    lda a:$BDA8,y
    sta a:OBJ_3930,x
    lda a:$BDA9,y
    sta a:OBJ_39E0,x
    lda a:$BDAA,y
    sta a:OBJ_3988,x
    lda a:$BDAB,y
    sta a:OBJ_3A38,x
    rts

; @routine 093D:2B7 EnterState7OnFlagOrYSignChange
; @run 71:C2B7
; WHAT IT DOES: Sends the object to state 7 when bit 7 of its status flags is set, or when its
; Y position's high byte is not zero and the sign of the next byte (the middle byte) is the
; opposite of the high byte's.
; HOW: Tests the flag, then the high byte, then the middle byte against it. The arms past a
; non-zero high byte ran in no script: the draft stops there and liftcmp judges only the flag
; and zero tests; the rest waits for coverage (PROCEDURE section 6).
; WAITS: no.
; Entry: X object. Exit: A and N, Z from the last load; X, Y, C and V kept.
EnterState7OnFlagOrYSignChange:
    lda a:OBJ_31A0,x
    and #$80
    bne EnterState7OnFlagOrYSignChange_at0
    lda a:OBJ_Y_HI,x
    beq EnterState7OnFlagOrYSignChange_at1
    bmi EnterState7OnFlagOrYSignChange_at2
    lda a:OBJ_Y_LO,x
    bpl EnterState7OnFlagOrYSignChange_at1
    bra EnterState7OnFlagOrYSignChange_at0
EnterState7OnFlagOrYSignChange_at2:
    lda a:OBJ_Y_LO,x
    bmi EnterState7OnFlagOrYSignChange_at1
EnterState7OnFlagOrYSignChange_at0:
    lda #$07
    sta a:OBJ_27A8,x
EnterState7OnFlagOrYSignChange_at1:
    rts

; @routine 093D:2D7 SetHitboxFromFrameRowsIndirect
; @run 71:C2D7
; WHAT IT DOES: Sets the object's whole hit rectangle from a four byte row of a table, the row
; named by a second table indexed by the frame minus $27.
; HOW: The index table is at overlay address $C6E6, the rows at $C6FF.
; WAITS: no.
; Entry: X object. Exit: A the last byte loaded; N, Z from it; C bit 6 of the row number; Y the
; row's offset; X kept.
SetHitboxFromFrameRowsIndirect:
    lda a:OBJ_2A68,x
    sec
    sbc #$27
    tay
    lda a:$C6E6,y
    asl a
    asl a
    tay
    lda a:$C6FF,y
    sta a:OBJ_3930,x
    lda a:$C700,y
    sta a:OBJ_39E0,x
    lda a:$C701,y
    sta a:OBJ_3988,x
    lda a:$C702,y
    sta a:OBJ_3A38,x
    rts

; @routine 093D:43A LaunchHopByF3510
; @run 71:C43A
; WHAT IT DOES: Launches the object in a hop: X speed from a two byte table by the object's f3510,
; Y speed -3.00 (up), frame $36, f35c0 set to 8, and its step byte advanced.
; HOW: The table is at overlay address $C45C, indexed by f3510 (0 or 1).
; WAITS: no.
; Entry: X object. Exit: A 8; N, Z from the step byte's new value; Y the f3510 byte; X and C
; kept.
LaunchHopByF3510:
    ldy a:OBJ_3510,x
    lda a:$C45C,y
    sta a:OBJ_VX_HI,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_LO,x
    lda #$36
    sta a:OBJ_2A68,x
    lda #$08
    sta a:OBJ_35C0,x
    inc a:OBJ_2800,x
    rts

; @routine 093D:5C3 CountdownStopThenState3
; @run 71:C5C3
; WHAT IT DOES: Counts the object's f35c0 timer down and, when it reaches zero, stops its
; animation and sends it to state 3.
; HOW: One decrement of the timer; the rest only at zero.
; WAITS: no.
; Entry: X object. Exit: A and N, Z from the timer, or A 3 at zero; X, Y and C kept.
CountdownStopThenState3:
    dec a:OBJ_35C0,x
    bne CountdownStopThenState3_at0
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$03
    sta a:OBJ_27A8,x
CountdownStopThenState3_at0:
    rts

; @routine 093B:0F6 StartWithFrame1A
; @run 70:B0F6
; WHAT IT DOES: The first time the object runs (state 0), starts it from its record and gives it
; frame $1A; later states leave it alone.
; HOW: Tests the state, then LoadObjectRecord and one store.
; WAITS: no.
; Entry: X object. Exit: A the state, or $1A after the start; N, Z from it; C and Y from the
; record load when it ran; X and V kept.
StartWithFrame1A:
    lda a:OBJ_27A8,x
    bne StartWithFrame1A_at0
    jsr VecLoadObjectRecord
    lda #$1A
    sta a:OBJ_2A68,x
StartWithFrame1A_at0:
    rts

; @routine 093C:180 ThrowByNextKind
; @run 70:B980
; WHAT IT DOES: Moves the object one step along a velocity built from its f3040 byte (bit 0 picks
; the X speed from a two byte table, the low nibble times $10, negated, is the Y speed), then
; clears both speeds again.
; HOW: The table is at overlay address $B9A3. ApplyVelocity moves it between the two sets of
; stores.
; WAITS: no.
; Entry: X object. Exit: A, N, Z, C and V from ApplyVelocity's add; Y bit 0 of f3040; X kept.
; Clobbers zTemp10.
ThrowByNextKind:
    lda a:OBJ_3040,x
    and #$01
    tay
    lda a:$B9A3,y
    sta a:OBJ_VX_HI,x
    lda a:OBJ_3040,x
    asl a
    asl a
    asl a
    asl a
    eor #$FF
    inc a
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VY_HI,x
    rts

; @routine 093C:23E AnimateThenState1
; @run 70:BA3E
; WHAT IT DOES: Steps the object's animation along one of two lists picked by bit 0 of its f3618;
; when the list has ended, adds one to f3618 and sends the object to state 1.
; HOW: The two list pointers are at overlay address $BA5D. StepObjectFrames's result says whether
; the list ended.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames (Y the list's offset when the step did not change
; it); when the list ended, A 1, N clear, Z clear. X and V kept. Clobbers zTemp10-11.
AnimateThenState1:
    lda a:OBJ_3618,x
    and #$01
    asl a
    tay
    lda a:$BA5D,y
    sta z:zTemp10
    lda a:$BA5E,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne AnimateThenState1_at0
    inc a:OBJ_3618,x
    lda #$01
    sta a:OBJ_27A8,x
AnimateThenState1_at0:
    rts

; @routine 093C:338 AnimateThenNextStep
; @run 70:BB38
; WHAT IT DOES: Steps the object's animation along the list that its f3670 and f36c8 pick (two
; pointers a table row, f3670 times two plus f36c8 as the byte offset); when the list has ended
; the step byte goes up by one.
; HOW: The pointers are at overlay address $BB54. V is left by the offset's add.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, and when the list ended N, Z from the new step byte
; with A 0; V from the offset's add; Y the offset when the step did not change it. Clobbers
; zTemp10-11.
AnimateThenNextStep:
    lda a:OBJ_3670,x
    asl a
    clc
    adc a:OBJ_36C8,x
    tay
    lda a:RichterColumnOffsets+7,y
    sta z:zTemp10
    lda a:RichterColumnOffsets+8,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne AnimateThenNextStep_at0
    inc a:OBJ_2800,x
AnimateThenNextStep_at0:
    rts

; @routine 093C:48F FollowParentYAndLeave
; @run 70:BC8F
; WHAT IT DOES: Copies the Y position's middle byte of the object whose slot is in this object's
; f3568 (its parent) into this object's, and removes this object when the parent is in state 4.
; HOW: ClearObject as a tail jump.
; WAITS: no.
; Entry: X object. Exit: A the parent's state, N, Z, C from its compare with 4; Y the parent's
; slot; X and V kept.
FollowParentYAndLeave:
    ldy a:OBJ_3568,x
    lda a:OBJ_Y_LO,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_27A8,y
    cmp #$04
    bne FollowParentYAndLeave_at0
    jmp VecClearObject
FollowParentYAndLeave_at0:
    rts

; @routine 093D:579 AnimateThenNextStepB
; @run 71:C579
; WHAT IT DOES: Steps the object's animation along the list at overlay address $C590; when the list
; has ended, restarts it and adds one to the step byte.
; HOW: The list pointer goes into zTemp10-11; StepObjectFrames says whether the list ended.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, and at the end N, Z from the new step byte; A 0;
; X and V kept. Clobbers zTemp10-11.
AnimateThenNextStepB:
    lda #$C5
    sta z:zTemp11
    lda #$90
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne AnimateThenNextStepB_at0
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    inc a:OBJ_2800,x
AnimateThenNextStepB_at0:
    rts

; @routine 093D:78C CountdownThenFallOrBecomeType9B
; @run 71:C78C
; WHAT IT DOES: Counts the object's f3510 down; while it runs, pulls the object down with gravity
; and moves it by its velocity; at zero it turns into an object of type $9B in state 0.
; HOW: ApplyGravityUnlimited, then ApplyVelocity as a tail jump.
; WAITS: no.
; Entry: X object. Exit: as ApplyVelocity while counting; at zero A $9B, N set, Z clear (C, V
; kept); X and Y kept. Clobbers zTemp10.
CountdownThenFallOrBecomeType9B:
    dec a:OBJ_3510,x
    bne CountdownThenFallOrBecomeType9B_at0
    lda #$9B
    sta a:OBJ_2750,x
    stz a:OBJ_27A8,x
    rts
CountdownThenFallOrBecomeType9B_at0:
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 093C:1A5 PickTimersAtRandom
; @run 70:B9A5
; WHAT IT DOES: Gives the object a pair of timers (f3510 and life_timer) from a table of four
; pairs, the pair picked by a random number.
; HOW: NextRandom, two bits of it doubled into an index into the table at overlay address $B9B9.
; WAITS: no.
; Entry: X object, D clear. Exit: A the second timer, N and Z from it; C 0 (the doubling); V as
; NextRandom's last add left it; Y the table offset; X kept.
PickTimersAtRandom:
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:$B9B9,y
    sta a:OBJ_3510,x
    lda a:$B9BA,y
    sta a:OBJ_34B8,x
    rts

; @routine 093C:1C1 PickAngleAtRandom
; @run 70:B9C1
; WHAT IT DOES: Gives the object an angle (f35c0 and f3568) from a table of four pairs, the pair
; picked by a random number.
; HOW: As PickTimersAtRandom, with the table at overlay address $B9D5.
; WAITS: no.
; Entry: X object, D clear. Exit: as PickTimersAtRandom.
PickAngleAtRandom:
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:$B9D5,y
    sta a:OBJ_35C0,x
    lda a:$B9D6,y
    sta a:OBJ_3568,x
    rts

; @routine 093C:4A3 AnimateMoveAndSag
; @run 70:BCA3
; WHAT IT DOES: Steps the object's animation along the list its life_timer picks (a table of
; pointers at overlay address $BCD1), moves it by its velocity, and while life_timer is 1 adds
; $18/256 to its Y speed.
; HOW: StepObjectFrames, ApplyVelocity, then a 16-bit add through the Y speed bytes.
; WAITS: no.
; Entry: X object, D clear. Exit: A, N, Z, C from the compare with 1, or from the last add
; (the Y speed's high byte); V from ApplyVelocity or that add; Y the list's offset unless the
; animation step changed it; X kept. Clobbers zTemp10-11.
AnimateMoveAndSag:
    lda a:OBJ_34B8,x
    asl a
    tay
    lda a:$BCD1,y
    sta z:zTemp10
    lda a:$BCD2,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_34B8,x
    cmp #$01
    bne AnimateMoveAndSag_at0
    lda a:OBJ_VY_LO,x
    clc
    adc #$18
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
AnimateMoveAndSag_at0:
    rts

; @routine 093C:6E9 StepFramesFacingPlayerC
; @run 70:BEE9
; WHAT IT DOES: Turns the object to face the player and steps its animation along the list whose
; pointer is the pair of bytes at overlay address $BEFE plus Y.
; HOW: SideOfPlayer, the pointer into zTemp10-11, then StepObjectFrames as a tail jump.
; WAITS: no.
; Entry: X object, Y the list's offset, D clear. Exit: as StepObjectFrames. Clobbers zTemp00,
; zTemp10-11.
StepFramesFacingPlayerC:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
    lda a:$BEFE,y
    sta z:zTemp10
    lda a:$BEFF,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 093D:282 FaceThePlayerInSomeStates
; @run 71:C282
; WHAT IT DOES: Turns the object to face the player when a table at overlay address $C293 says its
; state is one that does.
; HOW: Reads the table by the state byte; non-zero runs SideOfPlayer into the facing byte.
; WAITS: no.
; Entry: X object, D clear. Exit: A the table byte or the side bit; N, Z from it; C from
; SideOfPlayer's last compare when it ran; Y the state; X kept. Clobbers zTemp00.
FaceThePlayerInSomeStates:
    ldy a:OBJ_27A8,x
    lda a:$C293,y
    beq FaceThePlayerInSomeStates_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
FaceThePlayerInSomeStates_at0:
    rts

; @routine 093D:4AB NextPhaseAtRandom
; @run 71:C4AB
; WHAT IT DOES: Steps the object to its next phase: counts f3568 up, sets life_timer from a four
; byte table by it, turns f3510 to the side the player is on, and picks a step byte of 1 or 2 at
; random.
; HOW: A table at overlay address $C4CE, SideOfPlayer, NextRandom.
; WAITS: no.
; Entry: X object, D clear. Exit: A the new step byte, N and Z from it, C and V clear; Y the table
; offset; X kept. Clobbers zTemp00.
NextPhaseAtRandom:
    inc a:OBJ_3568,x
    lda a:OBJ_3568,x
    and #$03
    tay
    lda a:$C4CE,y
    sta a:OBJ_34B8,x
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_3510,x
    jsr VecNextRandom
    and #$01
    clc
    adc #$01
    sta a:OBJ_2800,x
    rts

; @routine 093B:4C3 ShowOnScreenAndWrapX
; @run 70:B4C3
; WHAT IT DOES: The first time the object runs (state 0) starts it from its record. After that
; it sets frame 2, hides it (frame 0) when its screen X is past the first seven screens and
; the low sum is negative, and, while the scroll moves, wraps the object's X by $80 across the
; page boundary at either end of the level (X high $FF with low under $E0 to high 1, or high 1
; with low from $20 up to high $FF), by the scroll's direction.
; HOW: Adds the scroll's X low and screen to the object's X; tests ScrollSpeedX's sign.
; WAITS: no.
; Entry: X object, D clear. Exit: as LoadObjectRecord in state 0; otherwise A, N, Z, C and V
; from the last compare, load or add; X, Y kept. Clobbers zTemp00 (in states but 0).
ShowOnScreenAndWrapX:
    lda a:OBJ_27A8,x
    bne ShowOnScreenAndWrapX_at0
    jmp VecLoadObjectRecord
ShowOnScreenAndWrapX_at0:
    lda #$02
    sta a:OBJ_2A68,x
    lda a:OBJ_X_LO,x
    clc
    adc a:ScrollXLo
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    adc a:ScrollXHiScreen
    cmp #$07
    bcc ShowOnScreenAndWrapX_at1
    lda z:zTemp00
    bpl ShowOnScreenAndWrapX_at1
    stz a:OBJ_2A68,x
ShowOnScreenAndWrapX_at1:
    lda a:ScrollSpeedX
    beq ShowOnScreenAndWrapX_at2
    bmi ShowOnScreenAndWrapX_at3
    lda a:OBJ_X_HI,x
    cmp #$FF
    bne ShowOnScreenAndWrapX_at2
    lda a:OBJ_X_LO,x
    cmp #$E0
    bcs ShowOnScreenAndWrapX_at2
    clc
    adc #$80
    sta a:OBJ_X_LO,x
    lda #$01
    sta a:OBJ_X_HI,x
ShowOnScreenAndWrapX_at2:
    rts
ShowOnScreenAndWrapX_at3:
    lda a:OBJ_X_HI,x
    cmp #$01
    bne ShowOnScreenAndWrapX_at2
    lda a:OBJ_X_LO,x
    cmp #$20
    bcc ShowOnScreenAndWrapX_at2
    clc
    adc #$80
    sta a:OBJ_X_LO,x
    lda #$FF
    sta a:OBJ_X_HI,x
    rts

; @routine 093C:28D ChooseNextMoveWhenFacingAway
; @run 70:BA8D
; WHAT IT DOES: When the side the player is on matches the object's facing (compared with bit 0
; of f3618), picks a phase and timer at random from two tables and advances the step; otherwise it
; enters state 1.
; HOW: SideOfPlayer; NextRandom twice; tables at overlay addresses $BAD5, $BAE5 and $BAF1.
; WAITS: no.
; Entry: X object, D clear. Exit: A the last byte loaded, N and Z from the step byte's new
; value (when picking); C and V from the second add; Y the last table index; X kept. Clobbers
; zTemp00, zTemp08.
ChooseNextMoveWhenFacingAway:
    lda a:OBJ_2AC0,x
    eor a:OBJ_3618,x
    and #$01
    sta z:zTemp08
    jsr VecSideOfPlayer
    and #$01
    cmp z:zTemp08
    beq ChooseNextMoveWhenFacingAway_at0
    lda #$01
    sta a:OBJ_27A8,x
    rts
ChooseNextMoveWhenFacingAway_at0:
    jsr VecNextRandom
    and #$0F
    tay
    lda a:$BAD5,y
    sta a:OBJ_3670,x
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    and #$03
    clc
    adc z:zTemp00
    tay
    lda a:$BAE5,y
    sta a:OBJ_3720,x
    lda a:OBJ_3618,x
    and #$01
    tay
    lda a:$BAF1,y
    sta a:OBJ_36C8,x
    inc a:OBJ_2800,x
    rts

; @routine 093C:6AB StartChargeWhenPlayerNear
; @run 70:BEAB
; WHAT IT DOES: Animates the object facing the player; while the object is on the first screen
; and under $78 pixels from the player along X, it stops its animation, keeps only bit 7 of its
; flags, sets its Y speed's high byte to 3, picks a timer ($44 plus 7 random bits) in f36c8 and
; enters state 4 with step 2.
; HOW: StepFramesFacingPlayerC (Y 0), the high bytes ORed, DistanceXToPlayer, NextRandom.
; WAITS: no.
; Entry: X object, D clear. Exit: A 2 and N, Z clear, C 0, V from the add of the timer when it
; started; otherwise as the last compare, OR or animation step left them; Y 0 or as the step
; left it; X kept.
StartChargeWhenPlayerNear:
    cly
    jsr StepFramesFacingPlayerC
    lda a:OBJ_X_HI,x
    ora a:OBJ_Y_HI,x
    bne StartChargeWhenPlayerNear_at0
    jsr VecDistanceXToPlayer
    cmp #$78
    bcs StartChargeWhenPlayerNear_at0
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda a:OBJ_FLAGS_2F38,x
    and #$80
    sta a:OBJ_FLAGS_2F38,x
    lda #$03
    sta a:OBJ_VY_HI,x
    jsr VecNextRandom
    and #$7F
    clc
    adc #$44
    sta a:OBJ_36C8,x
    lda #$04
    sta a:OBJ_27A8,x
    lda #$02
    sta a:OBJ_2800,x
StartChargeWhenPlayerNear_at0:
    rts

; @routine 093D:191 WaitThenState2B
; @run 71:C191
; WHAT IT DOES: A two-step wait like WaitThenState2: the first pass sets f34b8 to $1C and the step
; to 1; later passes animate the object facing the player (list index 2) and count the timer down;
; at zero the object enters state 2.
; HOW: StepFramesFacingPlayerC with Y 2, then a decrement.
; WAITS: no.
; Entry: X object, D clear. Exit: as WaitThenState2.
WaitThenState2B:
    lda a:OBJ_2800,x
    bne WaitThenState2B_at0
    lda #$1C
    sta a:OBJ_34B8,x
    inc a:OBJ_2800,x
    rts
WaitThenState2B_at0:
    ldy #$02
    jsr StepFramesFacingPlayerC
    dec a:OBJ_34B8,x
    bne WaitThenState2B_at1
    lda #$02
    sta a:OBJ_27A8,x
WaitThenState2B_at1:
    rts

; @routine 093C:1E4 CountTimerPairDownOrRepick
; @run 70:B9E4
; WHAT IT DOES: Counts the sixteen bit timer (f3510 low, life_timer high) down by one; when it is
; already zero, picks new timers at random (PickTimersAtRandom), stops the animation and enters
; state 2.
; HOW: The two bytes are ORed to test for zero; the decrement is a subtract with the borrow carried.
; WAITS: no.
; Entry: X object, D clear. Exit: A the new high byte and N, Z from it, C and V from the borrow
; subtract; or A 2 and N, Z clear with C, V and Y as the random pick left them.
CountTimerPairDownOrRepick:
    lda a:OBJ_34B8,x
    ora a:OBJ_3510,x
    bne CountTimerPairDownOrRepick_at0
    jsr PickTimersAtRandom
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$02
    sta a:OBJ_27A8,x
    rts
CountTimerPairDownOrRepick_at0:
    lda a:OBJ_3510,x
    sec
    sbc #$01
    sta a:OBJ_3510,x
    lda a:OBJ_34B8,x
    sbc #$00
    sta a:OBJ_34B8,x
    rts

; @routine 093C:20D CountAnglePairDownOrRepick
; @run 70:BA0D
; WHAT IT DOES: On the first screen across only: counts the sixteen bit value f35c0 (low) and
; f3568 (high) down by one; when it is already zero, picks a new angle at random
; (PickAngleAtRandom), stops the animation, clears the step and enters state 3.
; HOW: As CountTimerPairDownOrRepick.
; WAITS: no.
; Entry: X object, D clear. Exit: as CountTimerPairDownOrRepick, or A the X high byte and N, Z from
; it when off the first screen.
CountAnglePairDownOrRepick:
    lda a:OBJ_X_HI,x
    bne CountAnglePairDownOrRepick_at0
    lda a:OBJ_3568,x
    ora a:OBJ_35C0,x
    bne CountAnglePairDownOrRepick_at1
    jsr PickAngleAtRandom
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    stz a:OBJ_2800,x
    lda #$03
    sta a:OBJ_27A8,x
    rts
CountAnglePairDownOrRepick_at1:
    lda a:OBJ_35C0,x
    sec
    sbc #$01
    sta a:OBJ_35C0,x
    lda a:OBJ_3568,x
    sbc #$00
    sta a:OBJ_3568,x
CountAnglePairDownOrRepick_at0:
    rts

; @routine 093C:54C AnimateHitboxThenExpire
; @run 70:BD4C
; WHAT IT DOES: Animates the object along the list at overlay address $BD63, sets its hit
; rectangle from the frame (SetHitboxFromFrameRows), and counts f3510 down; at zero the object
; removes itself.
; HOW: StepObjectFrames, SetHitboxFromFrameRows, a decrement, ClearObject as a tail jump.
; WAITS: no.
; Entry: X object, D clear. Exit: N, Z from the timer; A, C, V, Y as SetHitboxFromFrameRows left
; them; X kept. Clobbers zTemp10-11.
AnimateHitboxThenExpire:
    lda #$BD
    sta z:zTemp11
    lda #$63
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr SetHitboxFromFrameRows
    dec a:OBJ_3510,x
    bne AnimateHitboxThenExpire_at0
    jmp VecClearObject
AnimateHitboxThenExpire_at0:
    rts

; @routine 093D:33A WalkWhileFrameChangesB
; @run 71:C33A
; WHAT IT DOES: Animates the object along the list whose pointer is the pair of bytes at overlay
; address $C356 plus Y and, on the frames where the picture changed, gives it an X speed from a
; table at $C35A by its f3510 and moves it along X.
; HOW: StepObjectFrames, then ApplyVelocityX.
; WAITS: no.
; Entry: X object, Y the list's offset, D clear. Exit: after a change as ApplyVelocityX, Y f3510;
; otherwise as StepObjectFrames; X kept. Clobbers zTemp10-11.
WalkWhileFrameChangesB:
    lda a:$C356,y
    sta z:zTemp10
    lda a:$C357,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc WalkWhileFrameChangesB_at0
    ldy a:OBJ_3510,x
    lda a:$C35A,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
WalkWhileFrameChangesB_at0:
    rts

; @routine 093D:53D LeapAwayFromPlayerTable
; @run 71:C53D
; WHAT IT DOES: Advances the step and launches the object: its X speed from a table at overlay
; address $C569 by the player's distance in 32 pixel steps, Y speed -4.5 ($FB80); when its
; f3510 is not zero the X speed is then reversed.
; HOW: DistanceXToPlayer shifted to five bits and doubled, NegateVelocityX.
; WAITS: no.
; Entry: X object, D clear. Exit: A, N, Z from f3510 (when zero) or the negated speed's high byte
; with C and V from its add; Y the table offset.
LeapAwayFromPlayerTable:
    inc a:OBJ_2800,x
    jsr VecDistanceXToPlayer
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    asl a
    tay
    lda a:$C56A,y
    sta a:OBJ_VX_HI,x
    lda a:$C569,y
    sta a:OBJ_VX_LO,x
    lda #$FB
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda a:OBJ_3510,x
    beq LeapAwayFromPlayerTable_at0
    jsr VecNegateVelocityX
LeapAwayFromPlayerTable_at0:
    rts

; @routine 093D:7A0 StartThenBlinkAndExpire
; @run 71:C7A0
; WHAT IT DOES: The first time (state 0) starts the object from its record and sets f3510 to
; $10. After that it counts f3510 down, removing the object at zero; otherwise it shows frame 0,
; or its life_timer as the frame on every fourth count.
; HOW: LoadObjectRecord; a decrement; ClearObject as a tail jump.
; WAITS: no.
; Entry: X object, D clear. Exit: as the last store, load, AND or decrement left them; Y 11 after
; the record start.
StartThenBlinkAndExpire:
    lda a:OBJ_27A8,x
    bne StartThenBlinkAndExpire_at0
    jsr VecLoadObjectRecord
    lda #$10
    sta a:OBJ_3510,x
    rts
StartThenBlinkAndExpire_at0:
    dec a:OBJ_3510,x
    bne StartThenBlinkAndExpire_at1
    jmp VecClearObject
StartThenBlinkAndExpire_at1:
    stz a:OBJ_2A68,x
    lda a:OBJ_3510,x
    and #$03
    bne StartThenBlinkAndExpire_at2
    lda a:OBJ_34B8,x
    sta a:OBJ_2A68,x
StartThenBlinkAndExpire_at2:
    rts

; @routine 093C:731 PickStateByPlayerDistanceB
; @run 70:BF31
; WHAT IT DOES: PickStateByPlayerDistance in this stage's overlay, with its state table at
; overlay address $BF57.
; HOW: The shared body.
; WAITS: no.
; Entry, exit: as PickStateByPlayerDistance.
PickStateByPlayerDistanceB:
    stz a:OBJ_2800,x
    lda a:OBJ_X_HI,x
    beq PickStateByPlayerDistanceB_at0
    lda #$03
    sta a:OBJ_27A8,x
    rts
PickStateByPlayerDistanceB_at0:
    lda a:OBJ_Y_HI,x
    beq PickStateByPlayerDistanceB_at1
    lda #$04
    sta a:OBJ_27A8,x
    rts
PickStateByPlayerDistanceB_at1:
    jsr VecPlayerDistanceSum
    and #$0F
    tay
    lda a:$BF57,y
    sta a:OBJ_27A8,x
    rts

; @routine 093C:7AA PickSpeedAndTurnFromTableC
; @run 70:BFAA
; WHAT IT DOES: PickSpeedAndTurnFromTable in this stage's overlay, with its table at overlay
; address $BFD6.
; HOW: The shared body.
; WAITS: no.
; Entry, exit: as PickSpeedAndTurnFromTable.
PickSpeedAndTurnFromTableC:
    lda a:OBJ_3670,x
    asl a
    asl a
    tay
    lda a:$BFD7,y
    sta a:OBJ_VX_HI,x
    lda a:$BFD6,y
    sta a:OBJ_VX_LO,x
    lda a:$BFD9,y
    sta a:OBJ_VY_HI,x
    lda a:$BFD8,y
    sta a:OBJ_VY_LO,x
    inc a:OBJ_3510,x
    jsr VecSideOfPlayer
    and #$01
    beq PickSpeedAndTurnFromTableC_at0
    jmp VecNegateVelocityX
PickSpeedAndTurnFromTableC_at0:
    rts

; @routine 093D:13E TurnOnRandomBeatWalkB
; @run 71:C13E
; @stop C16F
; WHAT IT DOES: A walk that turns on a random beat: at its first step it turns to the player's side,
; takes an X speed from a table at overlay address $C16F (two bytes by side), picks life_timer
; at random from eight bytes at $C173, stops the animation and moves on; after that it runs
; TurnOnRandomBeatWalkFollowB.
; HOW: The shared body; the two tables lie between the two parts and stay data.
; WAITS: no.
; Entry: X object, D clear. Exit: first step: A the timer, N, Z from the new step, C and V from
; NextRandom, Y its index; later as TurnOnRandomBeatWalkFollowB.
TurnOnRandomBeatWalkB:
    lda a:OBJ_2800,x
    bne TurnOnRandomBeatWalkFollowB
    jsr VecSideOfPlayer
    and #$01
    asl a
    tay
    lda a:$C16F,y
    sta a:OBJ_VX_HI,x
    lda a:$C16F+1,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$07
    tay
    lda a:$C173,y
    sta a:OBJ_34B8,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    inc a:OBJ_3510,x
    inc a:OBJ_2800,x
    rts

; @routine 093D:17B TurnOnRandomBeatWalkFollowB
; @run 71:C17B
; WHAT IT DOES: The later steps of TurnOnRandomBeatWalkB: animates the object facing the player (list 2),
; counts life_timer down and moves it along X; at zero it enters state 5 with step 0.
; HOW: StepFramesFacingPlayerC, a decrement, ApplyVelocityX as a tail jump.
; WAITS: no.
; Entry: X object, D clear. Exit: as the animation step, the decrement and ApplyVelocityX leave them
; (A 5 and N, Z clear at zero).
TurnOnRandomBeatWalkFollowB:
    ldy #$02
    jsr StepFramesFacingPlayerC
    dec a:OBJ_34B8,x
    beq TurnOnRandomBeatWalkFollowB_done
    jmp VecApplyVelocityX
TurnOnRandomBeatWalkFollowB_done:
    lda #$05
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
    rts

; @routine 093C:5C4 StartThenAnimateAndExpire
; @run 70:BDC4
; WHAT IT DOES: The first time (state 0) starts the object from its record, sets f2a10 to its
; life_timer plus 3 and f3510 to $28. After that it animates along the list whose pointer its
; life_timer picks (a table at overlay address $BDF8) and counts f3510 down; at zero the object
; removes itself, otherwise it goes on as FollowParentYAndLeave.
; HOW: LoadObjectRecord; StepObjectFrames; the last two as tail jumps.
; WAITS: no.
; Entry: X object, D clear. Exit: as the last step that ran left them.
StartThenAnimateAndExpire:
    lda a:OBJ_27A8,x
    bne StartThenAnimateAndExpire_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_34B8,x
    clc
    adc #$03
    sta a:OBJ_2A10,x
    lda #$28
    sta a:OBJ_3510,x
    rts
StartThenAnimateAndExpire_at0:
    lda a:OBJ_34B8,x
    asl a
    tay
    lda a:$BDF8,y
    sta z:zTemp10
    lda a:$BDF9,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_3510,x
    bne StartThenAnimateAndExpire_at1
    jmp VecClearObject
StartThenAnimateAndExpire_at1:
    jmp FollowParentYAndLeave

; @routine 093D:2FD StartAndMakeNeighbour
; @run 71:C2FD
; WHAT IT DOES: Starts the object from its record, empties the slot after it and starts an object
; of type $2D there at its place, sets its own f3670 to 1 and its life_timer to $50.
; HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace.
; WAITS: no.
; Entry: X object, D clear. Exit: A $50 and N, Z clear; C from the record; Y the neighbour's slot;
; X kept.
StartAndMakeNeighbour:
    jsr VecLoadObjectRecord
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$2D
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_3670,x
    lda #$50
    sta a:OBJ_34B8,x
    rts

; @routine 093D:51F SpawnShotFromFrame
; @run 71:C51F
; WHAT IT DOES: When the object's frame is $2B or more, starts an object of type $35 at its place
; in a free slot from $30, facing as this one does, with this frame in its life_timer.
; HOW: FindFreeSlotFrom30, StartObjectAtPlace, three copies.
; WAITS: no.
; Entry: X object, D clear. Exit: A the frame (the last byte copied), N, Z from it; C as the slot
; search left it; Y the slot; X kept.
SpawnShotFromFrame:
    lda a:OBJ_2A68,x
    cmp #$2B
    bcc SpawnShotFromFrame_at0
    jsr VecFindFreeSlotFrom30
    bcc SpawnShotFromFrame_at0
    lda #$35
    jsr VecStartObjectAtPlace
    lda a:OBJ_2AC0,x
    sta a:OBJ_2AC0,y
    lda a:OBJ_2A68,x
    sta a:OBJ_34B8,y
SpawnShotFromFrame_at0:
    rts

; @routine 093D:76B StartWithFrameAndFace
; @run 71:C76B
; @stop C787
; WHAT IT DOES: The first time (state 0) starts the object from its record, gives it a frame from
; a table at overlay address $C787 by its life_timer, and turns its X speed round when the player is
; on its left; later states go on as CountdownThenFallOrBecomeType9B.
; HOW: LoadObjectRecord, SideOfPlayer, NegateVelocityX.
; WAITS: no.
; Entry: X object, D clear. Exit: as the last call or test left them; Y the life_timer used as the
; table index.
StartWithFrameAndFace:
    lda a:OBJ_27A8,x
    bne CountdownThenFallOrBecomeType9B
    jsr VecLoadObjectRecord
    ldy a:OBJ_34B8,x
    lda a:$C787,y
    sta a:OBJ_2A68,x
    jsr VecSideOfPlayer
    and #$01
    beq StartWithFrameAndFace_at0
    jsr VecNegateVelocityX
StartWithFrameAndFace_at0:
    rts
