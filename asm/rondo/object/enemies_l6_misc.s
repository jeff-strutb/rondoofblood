; @routine 051F:454 WaitForPressThenAnimate
; @run 71:D454
; WHAT IT DOES: In state 0 loads the object's record and sets sprite set 6. Later, with OBJ_2800 0, it waits until the first object is at Y low $AA and X low $CC to $D3 and the pad's new presses have bit 4; then it moves to the next OBJ_2800, clears its animation step and timer and takes the first object's X low and Y low plus $D8 as its place. With OBJ_2800 not 0 it sets the first object's OBJ_3510 to $C0 and OBJ_3568 to $10 and steps the animation list at $D443; sound $3B when a frame changes; when the list ends it clears OBJ_2800, its frame and the first object's OBJ_3510.
; HOW: LoadObjectRecord, StepObjectFrames, SoundCommand, stores.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
WaitForPressThenAnimate:
    lda a:OBJ_27A8,x
    bne WaitForPressThenAnimate_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_2A10,x
    rts
WaitForPressThenAnimate_at0:
    lda a:OBJ_2800,x
    bne WaitForPressThenAnimate_at1
    lda a:OBJ_Y_LO
    cmp #$AA
    bne WaitForPressThenAnimate_at2
    lda a:OBJ_X_LO
    cmp #$D4
    bcs WaitForPressThenAnimate_at2
    lda a:OBJ_X_LO
    cmp #$CC
    bcc WaitForPressThenAnimate_at2
    lda z:zPadPressed
    and #$10
    beq WaitForPressThenAnimate_at2
    inc a:OBJ_2800,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda a:OBJ_X_LO
    sta a:OBJ_X_LO,x
    lda a:OBJ_Y_LO
    clc
    adc #$D8
    sta a:OBJ_Y_LO,x
WaitForPressThenAnimate_at2:
    rts
WaitForPressThenAnimate_at1:
    lda #$C0
    sta a:OBJ_3510
    lda #$10
    sta a:OBJ_3568
    lda #$43
    sta z:zTemp10
    lda #$D4
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne WaitForPressThenAnimate_at3
    stz a:OBJ_2800,x
    stz a:OBJ_2A68,x
    stz a:OBJ_3510
    rts
WaitForPressThenAnimate_at3:
    bcc WaitForPressThenAnimate_at4
    lda #$3B
    jsr VecSoundCommand
WaitForPressThenAnimate_at4:
    rts

; @routine 072C:5A7 LoadRecordA06
; @run 70:BDA7
; WHAT IT DOES: Loads the object's record.
; HOW: LoadObjectRecord.
; WAITS: no.
; Entry: X object. Exit: as LoadObjectRecord.
LoadRecordA06:
    jsr VecLoadObjectRecord
    rts

; @routine 092E:2D1 ClearHitsTaken
; @run 6B:8AD1
; WHAT IT DOES: Sets the object's hits taken (OBJ_33B0) to 0.
; HOW: A store.
; WAITS: no.
; Entry: X object. Exit: all kept.
ClearHitsTaken:
    stz a:OBJ_33B0,x
    rts
; @routine 0E0B:099 HalveDirectionWhenHurt
; @run 70:B099
; WHAT IT DOES: Unless the object is in state 6, halves its OBJ_3618 once its hits taken reach 5.
; HOW: Two compares and a memory shift right.
; WAITS: no.
; Entry: X object. Exit: the last compare, or the shift's N, Z and C.
HalveDirectionWhenHurt:
    lda a:OBJ_27A8,x
    cmp #$06
    beq HalveDirectionWhenHurt_at0
    lda a:OBJ_33B0,x
    cmp #$05
    bcc HalveDirectionWhenHurt_at0
    lsr a:OBJ_3618,x
HalveDirectionWhenHurt_at0:
    rts

; @routine 0E0B:0C2 SetHitBoxFromTable
; @run 70:B0C2
; WHAT IT DOES: When OBJ_3250 is not zero, takes the object's hit box (four bytes: OBJ_3930, OBJ_39E0, OBJ_3988, OBJ_3A38) from the table at $BEEC, four bytes a row, the row being the frame less one.
; HOW: A pointer built in zTemp00-01 by two adds of twice the row, four reads through it.
; WAITS: no.
; Entry: X object. Exit: A the last byte read, Y 3; or the test of OBJ_3250.
SetHitBoxFromTable:
    lda a:OBJ_3250,x
    beq SetHitBoxFromTable_at0
    lda #$EC
    sta z:zTemp00
    lda #$BE
    sta z:zTemp01
    lda a:OBJ_2A68,x
    sec
    sbc #$01
    asl a
    sta z:zTemp02
    lda z:zTemp00
    clc
    adc z:zTemp02
    sta z:zTemp00
    lda z:zTemp01
    adc #$00
    sta z:zTemp01
    lda z:zTemp00
    clc
    adc z:zTemp02
    sta z:zTemp00
    lda z:zTemp01
    adc #$00
    sta z:zTemp01
    cly
    lda (zTemp00),y
    sta a:OBJ_3930,x
    iny
    lda (zTemp00),y
    sta a:OBJ_39E0,x
    iny
    lda (zTemp00),y
    sta a:OBJ_3988,x
    iny
    lda (zTemp00),y
    sta a:OBJ_3A38,x
SetHitBoxFromTable_at0:
    rts

; @routine 0E0B:10B LoadThenSpawn24
; @run 70:B10B
; WHAT IT DOES: Loads the object's record, sets OBJ_3618 to $80, clears the object in slot $11 and starts a kind $24 object there at this object's place.
; HOW: LoadObjectRecord, ClearObjectInY, a tail jump into StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: A the last byte copied, Y $11.
LoadThenSpawn24:
    jsr VecLoadObjectRecord
    lda #$80
    sta a:OBJ_3618,x
    ldy #$11
    jsr VecClearObjectInY
    lda #$24
    jmp VecStartObjectAtPlace

; @routine 0E0B:184 FallThenStopForRaster
; @run 70:B184
; WHAT IT DOES: Moves the object along Y by its speed and counts OBJ_34B8 down; at zero it clears the raster effect and its argument, clears the object's Y speed and goes
; on to the next OBJ_2800.
; HOW: ApplyVelocityY, DEC, stores, INC.
; WAITS: no.
; Entry: X object. Exit: the count, or the new OBJ_2800's N and Z.
FallThenStopForRaster:
    jsr VecApplyVelocityY
    dec a:OBJ_34B8,x
    bne FallThenStopForRaster_at0
    stz z:zRasterArg
    stz z:zRasterEffect
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_2800,x
FallThenStopForRaster_at0:
    rts

; @routine 0E0B:19A StepListB1BAThenSound
; @run 70:B19A
; WHAT IT DOES: Steps the animation list at $B1BA; when it ends it sends sound $46, sets sprite set 4 and frame 1 and goes on to the next OBJ_2800.
; HOW: StepObjectFrames, SoundCommand, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
StepListB1BAThenSound:
    lda #$BA
    sta z:zTemp10
    lda #$B1
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListB1BAThenSound_at0
    lda #$46
    jsr VecSoundCommand
    lda #$04
    sta a:OBJ_2A10,x
    lda #$01
    sta a:OBJ_2A68,x
    inc a:OBJ_2800,x
StepListB1BAThenSound_at0:
    rts

; @routine 0E0B:237 StepListB25CThenStartList
; @run 70:B237
; WHAT IT DOES: Steps the animation list at $B25C; when it ends it starts the script list $82 on channel 5, sets bit 2 of zField8E, sets OBJ_3250 to 2 and goes on to the next state.
; HOW: StepObjectFrames, StartListChannelKind2, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step, or the new state's N and Z when the list ended.
StepListB25CThenStartList:
    lda #$5C
    sta z:zTemp10
    lda #$B2
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListB25CThenStartList_at0
    phx
    lda #$82
    ldx #$05
    jsr VecStartListChannelKind2
    plx
    lda z:zField8E
    ora #$04
    sta z:zField8E
    lda #$02
    sta a:OBJ_3250,x
    inc a:OBJ_27A8,x
StepListB25CThenStartList_at0:
    rts

; @routine 0E0B:34D CountDownThenState6
; @run 70:B34D
; WHAT IT DOES: Counts OBJ_3618 down; at zero the object goes to state 6, clears OBJ_2800 and takes a new OBJ_3618 from a table at $B36B by the low two bits of OBJ_35C0.
; HOW: DEC, stores, a table read (bank 70).
; WAITS: no.
; Entry: X object. Exit: the count; at zero, the table byte as loaded (Y its index).
CountDownThenState6:
    lda a:OBJ_3618,x
    beq CountDownThenState6_at0
    dec a:OBJ_3618,x
    rts
CountDownThenState6_at0:
    lda #$06
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
    lda a:OBJ_35C0,x
    and #$03
    tay
    lda a:$B36B,y
    sta a:OBJ_3618,x
    rts

; @routine 0E0B:51C StepListB53CToState5
; @run 70:B51C
; WHAT IT DOES: Steps the animation list at $B53C; when it ends it clears the raster effect and its argument and the animation step and timer, and the object goes to state 5 with OBJ_2800 0.
; HOW: StepObjectFrames, stores.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step, or A 5 when the list ended.
StepListB53CToState5:
    lda #$3C
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne BurstWhenFlaggedHitB
    stz z:zRasterArg
    stz z:zRasterEffect
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$05
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
    rts

; @routine 0E0B:5DD NextPhaseUnlessKind25
; @run 70:B5DD
; WHAT IT DOES: Goes on to the next OBJ_2800 unless the object in slot $50 is of kind $25.
; HOW: A compare and an INC.
; WAITS: no.
; Entry: X object. Exit: the compare, or the new OBJ_2800's N and Z.
NextPhaseUnlessKind25:
    lda a:$27A0
    cmp #$25
    beq NextPhaseUnlessKind25_at0
    inc a:OBJ_2800,x
NextPhaseUnlessKind25_at0:
    rts

; @routine 0E0B:5E8 StepListB5FEToState4
; @run 70:B5E8
; WHAT IT DOES: Steps the animation list at $B5FE; when it ends, the object goes to state 4 and OBJ_2800 0.
; HOW: StepObjectFrames, then stores and an increment.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step, or A 4 when the list ended.
StepListB5FEToState4:
    lda #$FE
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ClampSpeedXToTwo
    lda #$04
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
    rts

; @routine 0E0C:128 LoadThenRiseFast
; @run 70:B928
; WHAT IT DOES: Loads the object's record, moves it up once by a Y speed high of $D8, then sets OBJ_3510 to $30 and the Y speed to $FA00.
; HOW: LoadObjectRecord, ApplyVelocityY, stores.
; WAITS: no.
; Entry: X object. Exit: A 0, Z set.
LoadThenRiseFast:
    jsr VecLoadObjectRecord
    lda #$D8
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda #$30
    sta a:OBJ_3510,x
    lda #$FA
    sta a:OBJ_VY_HI,x
    lda #$00
    sta a:OBJ_VY_LO,x
    rts

; @routine 0E0C:187 StepListB9B7ThenAimAtPlayer
; @run 70:B987
; WHAT IT DOES: Steps the animation list at $B9B7; when it ends the object takes the angle to the player as OBJ_34B8, sets its speed along it with length $0180, clears its animation step and timer, sets OBJ_3510 to $C0 and goes on to the next state.
; HOW: StepObjectFrames, AngleToPlayer, SetVelocityFromAngle, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step, or the new state's N and Z (A $C0) when the list ended.
StepListB9B7ThenAimAtPlayer:
    lda #$B7
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListB9B7ThenAimAtPlayer_at0
    jsr VecAngleToPlayer
    sta a:OBJ_34B8,x
    lda #$80
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_34B8,x
    jsr VecSetVelocityFromAngle
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$C0
    sta a:OBJ_3510,x
    inc a:OBJ_27A8,x
StepListB9B7ThenAimAtPlayer_at0:
    rts

; @routine 0E0C:1C2 TurnTowardPlayerAndMove
; @run 70:B9C2
; WHAT IT DOES: Runs TurnTowardPlayerOrStepList, sets the object's speed from its angle OBJ_34B8 with length $01C0 and moves it, then turns the angle toward the angle to the player (step 2) with the low bit cleared; counts OBJ_3510 down and at zero sets it to $F0 and goes on to the next state.
; HOW: TurnTowardPlayerOrStepList, SetVelocityFromAngle, ApplyVelocity, AngleToPlayer, TurnAngleToward, DEC, an INC.
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: the count, or the new state's N and Z (A $F0).
TurnTowardPlayerAndMove:
    jsr TurnTowardPlayerOrStepList
    lda #$C0
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_34B8,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    jsr VecAngleToPlayer
    sta z:zTemp02
    lda #$02
    sta z:zTemp00
    lda a:OBJ_34B8,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    and #$FE
    sta a:OBJ_34B8,x
    dec a:OBJ_3510,x
    bne TurnTowardPlayerAndMove_at0
    lda #$F0
    sta a:OBJ_3510,x
    inc a:OBJ_27A8,x
TurnTowardPlayerAndMove_at0:
    rts

; @routine 0E0C:201 TurnTowardPlayerOrStepList
; @run 70:BA01
; WHAT IT DOES: While OBJ_3568 counts down the object waits, turning round when it reaches zero. At zero it compares its facing with the player's side: when they differ it sets OBJ_3568 to 5 and frame $2C, otherwise it steps the animation list at $B9FA.
; HOW: DEC, EOR, SideOfPlayer, StepObjectFrames.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the last instruction's registers.
TurnTowardPlayerOrStepList:
    lda a:OBJ_3568,x
    beq RichterItemSecondCosts
    dec a:OBJ_3568,x
    bne TurnTowardPlayerOrStepList_at0
    lda a:OBJ_2AC0,x
    eor #$01
    sta a:OBJ_2AC0,x
TurnTowardPlayerOrStepList_at0:
    rts
    jsr VecSideOfPlayer
    and #$01
    cmp a:OBJ_2AC0,x
    beq TurnTowardPlayerOrStepList_at1
    lda #$05
    sta a:OBJ_3568,x
    lda #$2C
    sta a:OBJ_2A68,x
    rts
TurnTowardPlayerOrStepList_at1:
    lda #$FA
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    rts

; @routine 0E0C:412 CopyPlaceAndHitBoxOfSlot10
; @run 70:BC12
; WHAT IT DOES: In state 0 loads the object's record. Later it takes the place of the object in slot $10 and its facing; if that object's OBJ_3250 is not zero it takes the hit box (four bytes a row from the table at $C024, the row being that object's frame less one; the half width plus 8), else it clears its half width and half height.
; HOW: LoadObjectRecord, CopyObjectPlaceBack, a pointer built in zTemp00-01, four reads through it.
; WAITS: no.
; Entry: X object. Exit: see the code: the last read or the clears.
CopyPlaceAndHitBoxOfSlot10:
    lda a:OBJ_27A8,x
    bne CopyPlaceAndHitBoxOfSlot10_at0
    jmp VecLoadObjectRecord
CopyPlaceAndHitBoxOfSlot10_at0:
    ldy #$10
    jsr VecCopyObjectPlaceBack
    lda a:$2AD0
    sta a:OBJ_2AC0,x
    lda a:$3260
    beq CopyPlaceAndHitBoxOfSlot10_at1
    lda #$24
    sta z:zTemp00
    lda #$C0
    sta z:zTemp01
    lda a:$2A78
    sec
    sbc #$01
    asl a
    sta z:zTemp02
    lda z:zTemp00
    clc
    adc z:zTemp02
    sta z:zTemp00
    lda z:zTemp01
    adc #$00
    sta z:zTemp01
    lda z:zTemp00
    clc
    adc z:zTemp02
    sta z:zTemp00
    lda z:zTemp01
    adc #$00
    sta z:zTemp01
    cly
    lda (zTemp00),y
    sta a:OBJ_3930,x
    iny
    lda (zTemp00),y
    sta a:OBJ_39E0,x
    iny
    lda (zTemp00),y
    clc
    adc #$08
    sta a:OBJ_3988,x
    iny
    lda (zTemp00),y
    sta a:OBJ_3A38,x
    rts
CopyPlaceAndHitBoxOfSlot10_at1:
    stz a:OBJ_3988,x
    stz a:OBJ_3A38,x
    rts

; @routine 12DF:038 RandomTimerC0
; @run 71:D038
; WHAT IT DOES: Sets the object's timer OBJ_34B8 to a random number with bits 6 and 7 set.
; HOW: NextRandom.
; WAITS: no.
; Entry: X object. Exit: A the timer, N set, Z clear; C and V from the random number.
RandomTimerC0:
    jsr VecNextRandom
    ora #$C0
    sta a:OBJ_34B8,x
    rts
