; @routine 051B:0FE LoadOrFallAlongListByTimer
; @run 70:B0FE
; @stop B125
; WHAT IT DOES: In state 0 loads the object's record and takes its two speeds from the four-byte rows of the table at $B125 by OBJ_34B8. Later it goes on as StepListByTimerWithGravity.
; HOW: LoadObjectRecord, table reads (bank 70); a tail jump into StepListByTimerWithGravity.
; WAITS: no.
; Entry: X object. Exit: state 0: the last table byte, Y the row; else as StepListByTimerWithGravity.
LoadOrFallAlongListByTimer:
    lda a:OBJ_27A8,x
    bne StepListByTimerWithGravity
    jsr VecLoadObjectRecord
    lda a:OBJ_34B8,x
    asl a
    asl a
    tay
    lda a:$B126,y
    sta a:OBJ_VX_HI,x
    lda a:$B125,y
    sta a:OBJ_VX_LO,x
    lda a:$B128,y
    sta a:OBJ_VY_HI,x
    lda a:$B127,y
    sta a:OBJ_VY_LO,x
    rts

; @routine 051B:135 StepListByTimerWithGravity
; @run 70:B135
; WHAT IT DOES: Steps the animation list the table at $B14D gives for OBJ_34B8, applies gravity without a limit and moves the object.
; HOW: Two table reads (bank 70), StepObjectFrames, ApplyGravityUnlimited, a tail jump into ApplyVelocity.
; WAITS: no.
; Entry: X object, Y overwritten. Exit: as ApplyVelocity.
StepListByTimerWithGravity:
    lda a:OBJ_34B8,x
    asl a
    tay
    lda a:$B14D,y
    sta z:zTemp10
    lda a:$B14E,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 051B:23D LoadRecordA04
; @run 70:B23D
; WHAT IT DOES: Loads the object's record.
; HOW: LoadObjectRecord.
; WAITS: no.
; Entry: X object. Exit: as LoadObjectRecord.
LoadRecordA04:
    jsr VecLoadObjectRecord
    rts

; @routine 051B:241 StartPhaseWhenScrolled
; @run 70:B241
; WHAT IT DOES: Once the level's X position high byte is 3 or more it clears the object in slot $41, clears the wave counter, sets OBJ_34B8 to $5B, sets bit 1 of the hit attack flags and zTemp $3C to 1, and goes on to the next state.
; HOW: A compare, ClearObjectInY, stores, an INC.
; WAITS: no.
; Entry: X object. Exit: the compare, or the new state's N and Z (A 1).
StartPhaseWhenScrolled:
    lda a:ScrollXHiScreen
    cmp #$03
    bcc StartPhaseWhenScrolled_at0
    ldy #$41
    jsr VecClearObjectInY
    stz z:zWaveCounter
    lda #$5B
    sta a:OBJ_34B8,x
    lda a:HitAttackFlags
    ora #$02
    sta a:HitAttackFlags
    lda #$01
    sta z:$3C
    inc a:OBJ_27A8,x
StartPhaseWhenScrolled_at0:
    rts

; @routine 051B:30D StateSixOnStatusBit7
; @run 70:B30D
; WHAT IT DOES: An object in a state under 6 whose OBJ_31A0 has bit 7 set goes to state 6 and clears OBJ_2800.
; HOW: A compare, a test, two stores.
; WAITS: no.
; Entry: X object. Exit: the compare, the test, or A 6.
StateSixOnStatusBit7:
    lda a:OBJ_27A8,x
    cmp #$06
    bcs StateSixOnStatusBit7_at0
    lda a:OBJ_31A0,x
    and #$80
    beq StateSixOnStatusBit7_at0
    lda #$06
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
StateSixOnStatusBit7_at0:
    rts

; @routine 051B:324 LoadThenStateByKindByte
; @run 70:B324
; WHAT IT DOES: Loads the object's record and sets OBJ_2800 to 1; then by OBJ_3040: zero leaves it; positive goes to state 5 with OBJ_2800 0; negative clears OBJ_3250 and bit 6 of OBJ_30F0 and goes to state 4 with OBJ_2800 0.
; HOW: LoadObjectRecord, tests, stores.
; WAITS: no.
; Entry: X object. Exit: A the state, or OBJ_3040 when zero.
LoadThenStateByKindByte:
    jsr VecLoadObjectRecord
    lda #$01
    sta a:OBJ_2800,x
    lda a:OBJ_3040,x
    beq LoadThenStateByKindByte_at0
    bmi LoadThenStateByKindByte_at1
    lda #$05
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
LoadThenStateByKindByte_at0:
    rts
LoadThenStateByKindByte_at1:
    stz a:OBJ_3250,x
    lda a:OBJ_30F0,x
    and #$BF
    sta a:OBJ_30F0,x
    lda #$04
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
    rts

; @routine 051B:368 StepListB37FThenNext
; @run 70:B368
; WHAT IT DOES: Steps the animation list at $B37F; when it ends, the object's animation step and timer are cleared and it goes to the next OBJ_2800.
; HOW: StepObjectFrames, then stores and an increment.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step, or the new OBJ_2800's N and Z when the list ended.
StepListB37FThenNext:
    lda #$7F
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListB37FThenNext_at0
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    inc a:OBJ_2800,x
StepListB37FThenNext_at0:
    rts

; @routine 051B:392 StepListB3B8ThenLand
; @run 70:B392
; WHAT IT DOES: Steps the animation list at $B3B8, then asks StopOnLanding with a probe height of 4; on landing the object gets an X speed of $FEE0, clears its animation step and timer and goes to the next OBJ_2800.
; HOW: StepObjectFrames, StopOnLanding, stores, an INC.
; WAITS: no.
; Entry: X object, Y the probe's column kept unless the step sets it, C passed on. Exit: the mover's registers, then the new OBJ_2800's N and Z when landed.
StepListB3B8ThenLand:
    lda #$B8
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$04
    jsr VecStopOnLanding
    bcc StepListB3B8ThenLand_at0
    lda #$FE
    sta a:OBJ_VX_HI,x
    lda #$E0
    sta a:OBJ_VX_LO,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    inc a:OBJ_2800,x
StepListB3B8ThenLand_at0:
    rts

; @routine 051B:3C3 StepListB37FUnlessFar
; @run 70:B3C3
; WHAT IT DOES: When the object's X high byte is not zero, or when the animation list at $B37F ends, clears its animation step and timer, goes on to the next state and clears OBJ_2800.
; HOW: StepObjectFrames, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the test or the step, or the new state's N and Z.
StepListB37FUnlessFar:
    lda a:OBJ_X_HI,x
    bne StepListB37FUnlessFar_at0
    lda #$7F
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListB37FUnlessFar_at1
StepListB37FUnlessFar_at0:
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    inc a:OBJ_27A8,x
    stz a:OBJ_2800,x
StepListB37FUnlessFar_at1:
    rts

; @routine 051B:4BF SetHitBox0C18
; @run 70:B4BF
; WHAT IT DOES: Sets the object's hit box: X offset 0, half width $0C, half height $18.
; HOW: Three stores.
; WAITS: no.
; Entry: X object. Exit: A $18, N and Z from it.
SetHitBox0C18:
    stz a:OBJ_3930,x
    lda #$0C
    sta a:OBJ_3988,x
    lda #$18
    sta a:OBJ_3A38,x
    rts

; @routine 051B:5CD StartPieceAtNextSlot
; @run 70:B5CD
; WHAT IT DOES: Clears the object in the slot after this one, starts a kind $32 object there at this object's place and goes on to the next OBJ_2800.
; HOW: ClearObjectInY, StartObjectAtPlace, an INC.
; WAITS: no.
; Entry: X object. Exit: A the last byte copied, Y the slot after; N and Z from the new OBJ_2800.
StartPieceAtNextSlot:
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$32
    jsr VecStartObjectAtPlace
    inc a:OBJ_2800,x
    rts

; @routine 051B:648 StepListB66BToState2
; @run 70:B648
; WHAT IT DOES: Steps the animation list at $B66B; when it ends the object gets an X speed of $FEE0, clears OBJ_35C0 and OBJ_2800 and goes to state 2.
; HOW: StepObjectFrames, stores.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step, or A 2 when the list ended.
StepListB66BToState2:
    lda #$6B
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListB66BToState2_at0
    lda #$FE
    sta a:OBJ_VX_HI,x
    lda #$E0
    sta a:OBJ_VX_LO,x
    stz a:OBJ_35C0,x
    lda #$02
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
StepListB66BToState2_at0:
    rts

; @routine 051B:690 DrawTileScriptByHighBits
; @run 70:B690
; WHAT IT DOES: Draws the tile script of group 0 numbered by bits 4 to 6 of OBJ_3040 (as 0 to 7) and goes on to the next OBJ_2800.
; HOW: DrawTileScript, an INC.
; WAITS: no.
; Entry: X object. Exit: the tile script's registers, then N and Z from the new OBJ_2800.
DrawTileScriptByHighBits:
    lda a:OBJ_3040,x
    and #$70
    lsr a
    lsr a
    lsr a
    lsr a
    cly
    jsr VecDrawTileScript
    inc a:OBJ_2800,x
    rts

; @routine 051B:6ED StartFourPiecesWithCount
; @run 70:B6ED
; WHAT IT DOES: Starts up to four kind $33 objects at this object's place, each in a free slot from $30, and gives each its number (0 to 3) as OBJ_34B8.
; HOW: A loop of FindFreeSlotFrom30, StartObjectAtPlace and a counter in zTemp00.
; WAITS: no.
; Entry: X object. Exit: the last compare, or as FindFreeSlotFrom30 when no slot was free.
StartFourPiecesWithCount:
    stz z:zTemp00
StartFourPiecesWithCount_at1:
    jsr VecFindFreeSlotFrom30
    bcc StartFourPiecesWithCount_at0
    lda #$33
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_34B8,y
    inc z:zTemp00
    lda z:zTemp00
    cmp #$04
    bcc StartFourPiecesWithCount_at1
StartFourPiecesWithCount_at0:
    rts

; @routine 051B:707 LandThenFaceAndRun
; @run 70:B707
; WHAT IT DOES: Asks StopOnLanding with a probe height of 4; on landing the object faces the player (bit 0 of SideOfPlayer), takes an X speed from the table at $B3BF by that side, clears OBJ_35C0 and OBJ_2800 and goes to state 2.
; HOW: StopOnLanding, SideOfPlayer, table reads (bank 70), stores.
; WAITS: no.
; Entry: X object, Y the probe's column, C passed on. Exit: the mover's registers, then A 2 when landed.
LandThenFaceAndRun:
    lda #$04
    jsr VecStopOnLanding
    bcc LandThenFaceAndRun_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
    asl a
    tay
    lda a:$B3C0,y
    sta a:OBJ_VX_HI,x
    lda a:$B3BF,y
    sta a:OBJ_VX_LO,x
    stz a:OBJ_35C0,x
    lda #$02
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
LandThenFaceAndRun_at0:
    rts

; @routine 051B:76A LoadRecordB04
; @run 70:B76A
; WHAT IT DOES: Loads the object's record.
; HOW: LoadObjectRecord.
; WAITS: no.
; Entry: X object. Exit: as LoadObjectRecord.
LoadRecordB04:
    jsr VecLoadObjectRecord
    rts
; @routine 051B:7AC StartJumpUpLeft
; @run 70:B7AC
; WHAT IT DOES: Sets the object's speed to $FE80 in X and $FC80 in Y (high bytes $FE and $FC) and goes on to the next state.
; HOW: Stores and an INC.
; WAITS: no.
; Entry: X object. Exit: A $80, N and Z from the new state.
StartJumpUpLeft:
    lda #$FE
    sta a:OBJ_VX_HI,x
    lda #$80
    sta a:OBJ_VX_LO,x
    lda #$FC
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    inc a:OBJ_27A8,x
    rts

; @routine 051B:7C4 StepListB819
; @run 70:B7C4
; WHAT IT DOES: Steps the animation list at $B819, then goes on into the code that follows it ($B7CF).
; HOW: StepObjectFrames; the code after it is entered by a tail jump.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the step, then the code at $B7CF.
StepListB819:
    lda #$19
    sta z:zTemp10
    lda #$B8
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$06
    jsr VecStopOnLanding
    php
    and #$02
    beq StepListB819_at0
    plp
    lda a:OBJ_31A0,x
    ora #$80
    sta a:OBJ_31A0,x
    rts
StepListB819_at0:
    plp
    bcc StepListB819_at1
    lda a:OBJ_34B8,x
    asl a
    asl a
    tay
    lda a:$B83B,y
    sta a:OBJ_VX_HI,x
    lda a:$B83A,y
    sta a:OBJ_VX_LO,x
    lda a:$B83D,y
    sta a:OBJ_VY_HI,x
    lda a:$B83C,y
    sta a:OBJ_VY_LO,x
    lda a:OBJ_34B8,x
    cmp #$05
    bcs StepListB819_at1
    lda a:OBJ_34B8,x
    bne StepListB819_at2
    lda #$2C
    jsr VecSoundCommand
StepListB819_at2:
    inc a:OBJ_34B8,x
StepListB819_at1:
    rts

; @routine 051D:4E8 CopyScrollToTimer
; @run 71:C4E8
; WHAT IT DOES: Sets the object's OBJ_34B8 to the scroll's X fraction of slot 4 and then to its X low of slot 4 (the second store stays).
; HOW: Two reads and two stores.
; WAITS: no.
; Entry: X object. Exit: A the scroll X low, N and Z from it.
CopyScrollToTimer:
    lda a:ScrollXFrac+4
    sta a:OBJ_34B8,x
    lda a:ScrollXLo+4
    sta a:OBJ_34B8,x
    rts

; @routine 051E:109 RandomTimer10
; @run 71:C909
; WHAT IT DOES: Sets the object's timer OBJ_34B8 to a random number from 0 to 15 plus 16.
; HOW: NextRandom.
; WAITS: no.
; Entry: X object. Exit: A the timer, N, Z, C and V from the add.
RandomTimer10:
    jsr VecNextRandom
    and #$0F
    clc
    adc #$10
    sta a:OBJ_34B8,x
    rts
