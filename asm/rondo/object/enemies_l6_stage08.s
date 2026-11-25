; @routine 088A:31C LoadThenSpeedF0
; @run 70:AB1C
; WHAT IT DOES: Loads the object's record and sets its X speed high to $F0.
; HOW: LoadObjectRecord.
; WAITS: no.
; Entry: X object. Exit: A $F0, N set, Z clear; C and Y from the record.
LoadThenSpeedF0:
    jsr VecLoadObjectRecord
    lda #$F0
    sta a:OBJ_VX_HI,x
    rts

; @routine 088A:325 NextStateOnScrollY
; @run 70:AB25
; WHAT IT DOES: Goes on to the next state when the scroll's Y high byte is not zero.
; HOW: A read and an INC.
; WAITS: no.
; Entry: X object. Exit: the load, or the new state's N and Z.
NextStateOnScrollY:
    lda a:ScrollYHi
    beq NextStateOnScrollY_at0
    inc a:OBJ_27A8,x
NextStateOnScrollY_at0:
    rts

; @routine 088A:359 StartThenFallAlongList
; @run 70:AB59
; WHAT IT DOES: In state 0 loads the object's record. Later it steps the animation list at $AB72, applies gravity without a limit and moves the object.
; HOW: LoadObjectRecord, StepObjectFrames, ApplyGravityUnlimited, ApplyVelocity.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: state 0: as LoadObjectRecord; else as ApplyVelocity.
StartThenFallAlongList:
    lda a:OBJ_27A8,x
    bne StartThenFallAlongList_at0
    jmp VecLoadObjectRecord
StartThenFallAlongList_at0:
    lda #$AB
    sta z:zTemp11
    lda #$72
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 088A:39B FrameBySideThenTimerOnScroll
; @run 70:AB9B
; WHAT IT DOES: Sets the object's frame to $25 when its X low is under $C0, else 0; once the scroll's Y high byte is not zero it sets OBJ_34B8 to $C0 and goes on to the next state.
; HOW: Stores, compares, an INC.
; WAITS: no.
; Entry: X object. Exit: the scroll's load, or the new state's N and Z (A $C0).
FrameBySideThenTimerOnScroll:
    stz a:OBJ_2A68,x
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcs FrameBySideThenTimerOnScroll_at0
    lda #$25
    sta a:OBJ_2A68,x
FrameBySideThenTimerOnScroll_at0:
    lda a:ScrollYHi
    beq FrameBySideThenTimerOnScroll_at1
    lda #$C0
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8,x
FrameBySideThenTimerOnScroll_at1:
    rts

; @routine 088A:400 StepListACB2ThenNextOnScreen
; @run 70:AC00
; WHAT IT DOES: Steps the animation list at $ACB2, moves the object along Y and goes on to the next state once its Y position is on the first screen (high byte not zero and low byte under $C0).
; HOW: StepObjectFrames, ApplyVelocityY, compares, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the last compare or load, or the new state's N and Z.
StepListACB2ThenNextOnScreen:
    lda #$B2
    sta z:zTemp10
    lda #$AC
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocityY
    lda a:OBJ_Y_HI,x
    beq StepListACB2ThenNextOnScreen_at0
    lda a:OBJ_Y_LO,x
    cmp #$C0
    bcs StepListACB2ThenNextOnScreen_at0
    inc a:OBJ_27A8,x
StepListACB2ThenNextOnScreen_at0:
    rts

; @routine 088A:41E StartListSoundThenSpawn20
; @run 70:AC1E
; WHAT IT DOES: Starts the script list $83 on channel 4, starts a kind $20 object in slot $10 at this object's place and clears this object.
; HOW: StartListChannelKind2, StartObjectAtPlace, a tail jump into ClearObject.
; WAITS: no.
; Entry: X object. Exit: as ClearObject.
StartListSoundThenSpawn20:
    phx
    lda #$83
    ldx #$04
    jsr VecStartListChannelKind2
    plx
    ldy #$10
    lda #$20
    jsr VecStartObjectAtPlace
    jmp VecClearObject

; @routine 088A:4B9 StartThreeNumberedObjects
; @run 70:ACB9
; WHAT IT DOES: Clears the objects in slots $16 to $18, starts a kind $28 object in each at the place of the object in slot $3E, gives each sprite set 7, its number (0 to 2) as OBJ_34B8 and the frame number plus $47, and then clears the object in slot $3E.
; HOW: A loop of ClearObjectInY and StartObjectAtPlace, ClearObject.
; WAITS: no.
; Entry: X object (kept). Exit: the last compare of Y with $19; A the last frame.
StartThreeNumberedObjects:
    phx
    ldx #$3E
    ldy #$16
    stz z:zTemp00
StartThreeNumberedObjects_at0:
    jsr VecClearObjectInY
    lda #$28
    jsr VecStartObjectAtPlace
    lda #$07
    sta a:OBJ_2A10,y
    lda z:zTemp00
    sta a:OBJ_34B8,y
    clc
    adc #$47
    sta a:OBJ_2A68,y
    inc z:zTemp00
    iny
    cpy #$19
    bcc StartThreeNumberedObjects_at0
    jsr VecClearObject
    plx
    rts

; @routine 088A:4FC LoadThenSpeedFromTable
; @run 70:ACFC
; WHAT IT DOES: Loads the object's record, takes its X speed from the two-byte entries of the table at $AD21 by OBJ_34B8, its timer from the table at $AD29 and its frame as OBJ_34B8 plus $47.
; HOW: LoadObjectRecord, table reads (bank 70), an add.
; WAITS: no.
; Entry: X object. Exit: A the frame, N, Z, C and V from the add; Y OBJ_34B8.
LoadThenSpeedFromTable:
    jsr VecLoadObjectRecord
    lda a:OBJ_34B8,x
    asl a
    tay
    lda a:$AD22,y
    sta a:OBJ_VX_HI,x
    lda a:$AD21,y
    sta a:OBJ_VX_LO,x
    ldy a:OBJ_34B8,x
    lda a:$AD29,y
    sta a:OBJ_3510,x
    tya
    clc
    adc #$47
    sta a:OBJ_2A68,x
    rts

; @routine 088A:52D CountThenNextStateF3510
; @run 70:AD2D
; WHAT IT DOES: Counts OBJ_3510 down; at zero it sets it to $30 and goes on to the next state.
; HOW: DEC, a store, INC.
; WAITS: no.
; Entry: X object. Exit: the count, or the new state's N and Z (A $30).
CountThenNextStateF3510:
    dec a:OBJ_3510,x
    bne CountThenNextStateF3510_at0
    lda #$30
    sta a:OBJ_3510,x
    inc a:OBJ_27A8,x
CountThenNextStateF3510_at0:
    rts

; @routine 088A:53B FallAndBlinkThenClear
; @run 70:AD3B
; WHAT IT DOES: Adds $0008 to the object's Y speed and moves the object. Its frame is 0, or on every other frame of the wave counter OBJ_34B8 plus $47. Counts OBJ_3510 down and clears the object at zero.
; HOW: A 16-bit add, ApplyVelocity, a test of zTemp wave bit 0, an add, DEC, ClearObject.
; WAITS: no.
; Entry: X object. Exit: the count, or as ClearObject.
FallAndBlinkThenClear:
    lda a:OBJ_VY_LO,x
    clc
    adc #$08
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    stz a:OBJ_2A68,x
    lda z:zWaveCounter
    and #$01
    beq FallAndBlinkThenClear_at0
    lda a:OBJ_34B8,x
    clc
    adc #$47
    sta a:OBJ_2A68,x
FallAndBlinkThenClear_at0:
    dec a:OBJ_3510,x
    bne FallAndBlinkThenClear_at1
    jsr VecClearObject
FallAndBlinkThenClear_at1:
    rts

; @routine 088A:699 LoadThenStartFiveFromTable
; @run 70:AE99
; WHAT IT DOES: Loads the object's record, clears OBJ_3778 and its Y speed, sets OBJ_34B8 to $30 and bit 2 of zField8E, copies its X and Y low bytes into all 64 entries of the two tables at $BB66 and $BBA6, then clears the objects in slots $11 to $15 and starts in each an object whose kind the table at $AEDA gives, at this object's place.
; HOW: LoadObjectRecord, two loops, ClearObjectInY, StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: the last compare of Y with $16.
LoadThenStartFiveFromTable:
    jsr VecLoadObjectRecord
    stz a:OBJ_3778,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$30
    sta a:OBJ_34B8,x
    lda z:zField8E
    ora #$04
    sta z:zField8E
    cly
LoadThenStartFiveFromTable_at0:
    lda a:OBJ_X_LO,x
    sta a:RichterWallPushBits+1,y
    lda a:OBJ_Y_LO,x
    sta a:$BBA6,y
    iny
    cpy #$40
    bcc LoadThenStartFiveFromTable_at0
    ldy #$11
LoadThenStartFiveFromTable_at1:
    jsr VecClearObjectInY
    phy
    tya
    sec
    sbc #$11
    tay
    lda a:$AEDA,y
    ply
    jsr VecStartObjectAtPlace
    iny
    cpy #$16
    bcc LoadThenStartFiveFromTable_at1
    rts

; @routine 088A:71E PlaySoundTwo
; @run 70:AF1E
; WHAT IT DOES: Plays sound 2.
; HOW: SoundCommand.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: see abi_sound_end.
PlaySoundTwo:
    lda #$02
    jsr VecSoundCommand
    rts

; @routine 088A:724 SetFrameThenCountToNext
; @run 70:AF24
; WHAT IT DOES: Sets the object's frame (OBJ_2A68) to $0C and counts OBJ_3568 down; at zero it sets it to $10 and goes on to the next state.
; HOW: A store, DEC, a store, INC.
; WAITS: no.
; Entry: X object. Exit: the count, or the new state's N and Z (A $10).
SetFrameThenCountToNext:
    lda #$0C
    sta a:OBJ_2A68,x
    dec a:OBJ_3568,x
    bne SetFrameThenCountToNext_at0
    lda #$10
    sta a:OBJ_3568,x
    inc a:OBJ_27A8,x
SetFrameThenCountToNext_at0:
    rts

; @routine 088A:765 StepListByIndex088A
; @run 70:AF65
; WHAT IT DOES: Steps the animation list whose address the table at $AF72 gives for the index in Y.
; HOW: Two table reads (bank 70), a tail jump into StepObjectFrames.
; WAITS: no.
; Entry: X object, Y the table index. Exit: as StepObjectFrames.
StepListByIndex088A:
    lda a:$AF72,y
    sta z:zTemp10
    lda a:$AF73,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 088B:084 StartFallWithFrame21
; @run 70:B084
; WHAT IT DOES: Zeroes the object's X speed, sets its Y speed high to $FD and its frame to $21 and goes on to the next OBJ_2800.
; HOW: Stores and an INC.
; WAITS: no.
; Entry: X object. Exit: A $21, N and Z from the new OBJ_2800.
StartFallWithFrame21:
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    lda #$21
    sta a:OBJ_2A68,x
    inc a:OBJ_2800,x
    rts

; @routine 088B:098 FallThenSoundAtPeak
; @run 70:B098
; WHAT IT DOES: Applies gravity and moves the object; once its Y speed high byte is not negative it sends sound $9A and goes on to the next OBJ_2800.
; HOW: ApplyGravity, ApplyVelocity, SoundCommand, an INC.
; WAITS: no.
; Entry: X object, Y kept for the sound's push. Exit: see the wrapper.
FallThenSoundAtPeak:
    jsr VecApplyGravity
    jsr VecApplyVelocity
    lda a:OBJ_VY_HI,x
    bmi FallThenSoundAtPeak_at0
    lda #$9A
    jsr VecSoundCommand
    inc a:OBJ_2800,x
FallThenSoundAtPeak_at0:
    rts

; @routine 088B:21D TurnAroundInState9
; @run 70:B21D
; WHAT IT DOES: Sets the object's state to 9 and reverses its X speed (a tail jump into NegateVelocityX).
; HOW: A store, NegateVelocityX.
; WAITS: no.
; Entry: X object. Exit: as NegateVelocityX.
TurnAroundInState9:
    lda #$09
    sta a:OBJ_27A8,x
    jmp VecNegateVelocityX

; @routine 088B:225 FallBackInState9
; @run 70:B225
; WHAT IT DOES: Sets the object's state to 9 and reverses its Y speed (a tail jump into NegateVelocityY).
; HOW: A store, NegateVelocityY.
; WAITS: no.
; Entry: X object. Exit: as NegateVelocityY.
FallBackInState9:
    lda #$09
    sta a:OBJ_27A8,x
    jmp VecNegateVelocityY

; @routine 088B:240 SpeedBySideOfScreen
; @run 70:B240
; WHAT IT DOES: Sets OBJ_3510 to 2, takes the X speed high from the two-byte table at $B259 by bit 7 of the object's X low and clears the X speed low, then moves the object.
; HOW: Two ROLs, a table read (bank 70), a tail jump into ApplyVelocity.
; WAITS: no.
; Entry: X object, C passed into the first ROL. Exit: as ApplyVelocity, Y the table index.
SpeedBySideOfScreen:
    lda #$02
    sta a:OBJ_3510,x
    lda a:OBJ_X_LO,x
    rol a
    rol a
    and #$01
    tay
    lda a:$B259,y
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    jmp VecApplyVelocity

; @routine 088B:6C5 ClearFrame
; @run 70:B6C5
; WHAT IT DOES: Sets the object's frame to 0.
; HOW: A store.
; WAITS: no.
; Entry: X object. Exit: all kept.
ClearFrame:
    stz a:OBJ_2A68,x
    rts
; @routine 088B:6C9 PlaceFromTableCopySlot10
; @run 70:B6C9
; WHAT IT DOES: Puts the object at the X and Y low bytes the tables at $BB66 and $BBA6 give for the row the table at $B736 names by OBJ_3510, copies the sprite set, frame and facing of the object in slot $10 and sets its flash flags to $10.
; HOW: Table reads (bank 70), copies.
; WAITS: no.
; Entry: X object. Exit: A $10, N and Z from it; Y the row.
PlaceFromTableCopySlot10:
    ldy a:OBJ_3510,x
    lda a:$B736,y
    tay
    lda a:RichterWallPushBits+1,y
    sta a:OBJ_X_LO,x
    lda a:$BBA6,y
    sta a:OBJ_Y_LO,x
    lda a:$2A20
    sta a:OBJ_2A10,x
    lda a:$2A78
    sta a:OBJ_2A68,x
    lda a:$2AD0
    sta a:OBJ_2AC0,x
    lda #$10
    sta a:OBJ_2B18,x
    rts

; @routine 088B:705 PlaceFromTableThenStepList
; @run 70:B705
; WHAT IT DOES: Puts the object at the X and Y low bytes the tables at $BB66 and $BBA6 give for the row the table at $B73A names by OBJ_3510, sets sprite set 7, copies the facing of the object in slot $10 and steps the animation list the table at $B73E gives for OBJ_3510.
; HOW: Table reads (bank 70), StepObjectFrames.
; WAITS: no.
; Entry: X object, Y overwritten. Exit: as StepObjectFrames.
PlaceFromTableThenStepList:
    ldy a:OBJ_3510,x
    lda a:$B73A,y
    tay
    lda a:RichterWallPushBits+1,y
    sta a:OBJ_X_LO,x
    lda a:$BBA6,y
    sta a:OBJ_Y_LO,x
    lda #$07
    sta a:OBJ_2A10,x
    lda a:$2AD0
    sta a:OBJ_2AC0,x
    lda a:OBJ_3510,x
    asl a
    tay
    lda a:$B73E,y
    sta z:zTemp10
    lda a:$B73F,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    rts

; @routine 088B:777 SetTimersOf12To15
; @run 70:B777
; WHAT IT DOES: Sets OBJ_34B8 of the objects 18 to 21 to 1.
; HOW: A loop of Y from 18.
; WAITS: no.
; Entry: nothing. Exit: A 1, Y 22, the compare of Y with 22.
SetTimersOf12To15:
    ldy #$12
SetTimersOf12To15_at0:
    lda #$01
    sta a:OBJ_34B8,y
    iny
    cpy #$16
    bcc SetTimersOf12To15_at0
    rts
; @routine 088C:43C StartThenWaitForField83
; @run 70:BC3C
; WHAT IT DOES: In state 0 loads the object's record and sets frame 2 and sprite set 6. In state 1, once zField83 is not zero, it sets the scroll Y high to 1, frame $46, sprite set 7, X low $80, Y low $58 and goes on to the next state.
; HOW: LoadObjectRecord, a DEC, stores, an INC.
; WAITS: no.
; Entry: X object. Exit: the DEC, the load of zField83, or the new state's N and Z (A $58).
StartThenWaitForField83:
    lda a:OBJ_27A8,x
    bne StartThenWaitForField83_at0
    jsr VecLoadObjectRecord
    lda #$02
    sta a:OBJ_2A68,x
    lda #$06
    sta a:OBJ_2A10,x
StartThenWaitForField83_at0:
    dec a
    bne StartThenWaitForField83_at1
    lda z:zField83
    beq StartThenWaitForField83_at1
    lda #$01
    sta a:ScrollYHi
    lda #$46
    sta a:OBJ_2A68,x
    lda #$07
    sta a:OBJ_2A10,x
    lda #$80
    sta a:OBJ_X_LO,x
    lda #$58
    sta a:OBJ_Y_LO,x
    inc a:OBJ_27A8,x
StartThenWaitForField83_at1:
    rts
