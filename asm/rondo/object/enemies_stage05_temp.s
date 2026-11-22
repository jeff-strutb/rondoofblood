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
