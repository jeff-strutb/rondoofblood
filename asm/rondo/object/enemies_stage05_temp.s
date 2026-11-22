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
