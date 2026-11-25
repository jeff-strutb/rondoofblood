; @routine 051A:7C5 LoadThenBreakIntoThrownPiece
; @run 70:AFC5
; @stop B018
; WHAT IT DOES: In state 0 loads the object's record and sets sprite set 7, frame 1 and half height $16. Later it steps the animation list at $B018; once its hits taken reach 5 it records its event, becomes the kind in OBJ_3040 with OBJ_3A90 $10 and state 0 and starts a kind $9C object in a free slot from $50 at its place, with Y speed $FA00 for one move along Y.
; HOW: LoadObjectRecord, StepObjectFrames, RecordObjectEvent, FindFreeSlotFrom50, StartObjectAtPlace, ApplyVelocityY.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the last move, or the records' registers.
LoadThenBreakIntoThrownPiece:
    lda a:OBJ_27A8,x
    bne LoadThenBreakIntoThrownPiece_at0
    jsr VecLoadObjectRecord
    lda #$07
    sta a:OBJ_2A10,x
    lda #$01
    sta a:OBJ_2A68,x
    lda #$16
    sta a:OBJ_3A38,x
    rts
LoadThenBreakIntoThrownPiece_at0:
    lda #$B0
    sta z:zTemp11
    lda #$18
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_33B0,x
    cmp #$05
    bcc LoadThenBreakIntoThrownPiece_at1
    jsr VecRecordObjectEvent
    lda a:OBJ_3040,x
    sta a:OBJ_2750,x
    lda #$10
    sta a:OBJ_3A90,x
    stz a:OBJ_27A8,x
    jsr VecFindFreeSlotFrom50
    bcc LoadThenBreakIntoThrownPiece_at1
    lda #$9C
    jsr VecStartObjectAtPlace
    lda #$FA
    sta a:OBJ_VY_HI,y
    sxy
    jsr VecApplyVelocityY
    stz a:OBJ_VY_HI,x
    sxy
LoadThenBreakIntoThrownPiece_at1:
    rts

; @routine 051B:02F LoadThenStepListBreakAtFive
; @run 70:B02F
; WHAT IT DOES: In state 0 loads the object's record and sets sprite set 7 and frame 7. Later it steps the animation list at $B075; once its hits taken reach 5 it sends sound $82, records its event, becomes the kind in OBJ_3040 with OBJ_3A90 $10 and state 0, and starts a kind $9B object in a free slot from $50 at its place. (The part after the compare ran in no script; it is written from the decoded bytes.)
; HOW: LoadObjectRecord, StepObjectFrames, SoundCommand, RecordObjectEvent, FindFreeSlotFrom50, StartObjectAtPlace.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
LoadThenStepListBreakAtFive:
    lda a:OBJ_27A8,x
    bne LoadThenStepListBreakAtFive_at0
    jsr VecLoadObjectRecord
    lda #$07
    sta a:OBJ_2A10,x
    lda #$07
    sta a:OBJ_2A68,x
    rts
LoadThenStepListBreakAtFive_at0:
    lda #$B0
    sta z:zTemp11
    lda #$75
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_33B0,x
    cmp #$05
    bcc LoadThenStepListBreakAtFive_at1
    lda #$82
    jsr VecSoundCommand
    jsr VecRecordObjectEvent
    lda a:OBJ_3040,x
    sta a:OBJ_2750,x
    lda #$10
    sta a:OBJ_3A90,x
    stz a:OBJ_27A8,x
    jsr VecFindFreeSlotFrom50
    bcc LoadThenStepListBreakAtFive_at1
    lda #$9B
    jmp VecStartObjectAtPlace
LoadThenStepListBreakAtFive_at1:
    rts

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

; @routine 051B:264 SetFirstObjectTimersThenCount
; @run 70:B264
; WHAT IT DOES: Clears the wave counter and sets the first object's OBJ_3510 to 4 and OBJ_3568 to $20 and the byte at $3BF0 to 2; counts OBJ_34B8 down and at zero sets zTemp $8C to $80, zTemp $38 to 3 and clears the hit attack flags.
; HOW: Stores, DEC.
; WAITS: no.
; Entry: X object. Exit: the count, or A 3 at zero.
SetFirstObjectTimersThenCount:
    stz z:zWaveCounter
    lda #$04
    sta a:OBJ_3510
    lda #$02
    sta a:PlayerByte3BF0
    lda #$20
    sta a:OBJ_3568
    dec a:OBJ_34B8,x
    bne SetFirstObjectTimersThenCount_at0
    lda #$80
    sta z:zField8C
    lda #$03
    sta z:zSubStep
    stz a:HitAttackFlags
SetFirstObjectTimersThenCount_at0:
    rts

; @routine 051B:286 LoadThenSpawnWhileScrolling
; @run 70:B286
; WHAT IT DOES: In state 0 loads the object's record and looks for a free pair of slots. Later, while the screen scrolls right, it counts the 16-bit timer OBJ_34B8:OBJ_3510 down; at zero it looks in slots $30, $32 and $34 for one whose two kind bytes are zero, clears that object, starts a kind $22 object there at its place and sets its timer from a four-entry table at $B2DF by a random number.
; HOW: LoadObjectRecord, a 16-bit subtract, a search loop, ClearObjectInY, StartObjectAtPlace, NextRandom.
; WAITS: no.
; Entry: X object. Exit: the last test or load.
LoadThenSpawnWhileScrolling:
    lda a:OBJ_27A8,x
    bne LoadThenSpawnWhileScrolling_at0
    jsr VecLoadObjectRecord
    bra LoadThenSpawnWhileScrolling_at1
    rts
LoadThenSpawnWhileScrolling_at0:
    lda a:ScrollSpeedX
    beq LoadThenSpawnWhileScrolling_at2
    bmi LoadThenSpawnWhileScrolling_at2
    lda a:OBJ_34B8,x
    ora a:OBJ_3510,x
    beq LoadThenSpawnWhileScrolling_at1
    lda a:OBJ_3510,x
    sec
    sbc #$01
    sta a:OBJ_3510,x
    lda a:OBJ_34B8,x
    sbc #$00
    sta a:OBJ_34B8,x
    rts
LoadThenSpawnWhileScrolling_at1:
    ldy #$30
LoadThenSpawnWhileScrolling_at4:
    lda a:OBJ_2750,y
    ora a:OBJ_2750+1,y
    beq LoadThenSpawnWhileScrolling_at3
    iny
    iny
    cpy #$36
    bcc LoadThenSpawnWhileScrolling_at4
    rts
LoadThenSpawnWhileScrolling_at3:
    jsr VecClearObjectInY
    lda #$22
    jsr VecStartObjectAtPlace
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:$B2DF,y
    sta a:OBJ_3510,x
    lda a:$B2E0,y
    sta a:OBJ_34B8,x
LoadThenSpawnWhileScrolling_at2:
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

; @routine 051B:3E2 WalkTurnOrJumpOnLanding
; @run 70:B3E2
; WHAT IT DOES: When OBJ_3568's low five bits are zero and the player is within $24 along X and $1C along Y, the object faces the player's side (bit 0 of SideOfPlayer) in OBJ_35C0, clears its animation step and timer and goes to state 3. Otherwise it steps the animation list at $B45E and, on a changed frame, asks MoveAndLand with a probe height of 4: a wall hit gives it a Y speed of $FC80 and X speed of $FF00 and state 1; a floor stop clears its speeds; both clear the animation step and timer, with OBJ_2800 0 or 1.
; HOW: DistanceXToPlayer, DistanceYToPlayer, SideOfPlayer, StepObjectFrames, MoveAndLand, stores.
; WAITS: no.
; Entry: X object, Y the column kept unless the step sets it. Exit: see the wrapper.
WalkTurnOrJumpOnLanding:
    lda a:OBJ_3568,x
    and #$1F
    bne WalkTurnOrJumpOnLanding_at0
    jsr VecDistanceXToPlayer
    cmp #$24
    bcs WalkTurnOrJumpOnLanding_at0
    jsr VecDistanceYToPlayer
    cmp #$1C
    bcs WalkTurnOrJumpOnLanding_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_35C0,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$03
    sta a:OBJ_27A8,x
    rts
WalkTurnOrJumpOnLanding_at0:
    lda #$5E
    sta z:zTemp10
    lda #$B4
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc WalkTurnOrJumpOnLanding_at1
    lda #$04
    jsr VecMoveAndLand
    bcs WalkTurnOrJumpOnLanding_at2
    and #$01
    beq WalkTurnOrJumpOnLanding_at3
WalkTurnOrJumpOnLanding_at1:
    rts
WalkTurnOrJumpOnLanding_at2:
    lda #$FC
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda #$FF
    sta a:OBJ_VX_HI,x
    lda #$00
    sta a:OBJ_VX_LO,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$01
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
    rts
WalkTurnOrJumpOnLanding_at3:
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$01
    sta a:OBJ_27A8,x
    lda #$01
    sta a:OBJ_2800,x
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

; @routine 051B:514 ResetAnimThenNextPhase
; @run 70:B514
; WHAT IT DOES: Clears OBJ_3250 and the animation step and timer, sets OBJ_34B8 to 4 and goes on to the next OBJ_2800.
; HOW: Stores and an INC.
; WAITS: no.
; Entry: X object. Exit: A 4, N and Z from the new OBJ_2800.
ResetAnimThenNextPhase:
    stz a:OBJ_3250,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$04
    sta a:OBJ_34B8,x
    inc a:OBJ_2800,x
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

; @routine 051B:5DC WalkWhileNearPlayer
; @run 70:B5DC
; WHAT IT DOES: Counts toward the end of a walk: when the player is within $60 along X and $20 along Y, or when OBJ_3568's low nibble is zero and OBJ_3040 counts down to zero, it clears its animation step and timer and goes on to the next OBJ_2800. Otherwise it flips OBJ_35C0 when OBJ_3568 is zero, steps the animation list at $B62F and, on a changed frame, takes its X speed from the table at $B644 by OBJ_35C0 and moves along X.
; HOW: DistanceXToPlayer, DistanceYToPlayer, tests, StepObjectFrames, ApplyVelocityX.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the last test or step.
WalkWhileNearPlayer:
    jsr VecDistanceXToPlayer
    cmp #$60
    bcs WalkWhileNearPlayer_at0
    jsr VecDistanceYToPlayer
    cmp #$20
    bcc WalkWhileNearPlayer_at1
WalkWhileNearPlayer_at0:
    lda a:OBJ_3568,x
    and #$0F
    bne WalkWhileNearPlayer_at2
    dec a:OBJ_3040,x
    bne WalkWhileNearPlayer_at2
WalkWhileNearPlayer_at1:
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    inc a:OBJ_2800,x
    rts
WalkWhileNearPlayer_at2:
    lda a:OBJ_3568,x
    bne WalkWhileNearPlayer_at3
    lda a:OBJ_35C0,x
    eor #$01
    sta a:OBJ_35C0,x
WalkWhileNearPlayer_at3:
    lda #$2F
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc SoundReadPair
    lda a:OBJ_35C0,x
    asl a
    tay
    lda a:$B644,y
    sta a:OBJ_VX_LO,x
    lda a:$B645,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
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
; @routine 051B:76E FollowBlockWhileState5
; @run 70:B76E
; WHAT IT DOES: When the object before this one in the table is not in state 5 it sets bit 7 of OBJ_31A0 and stops. Otherwise it takes that object's place (CopyObjectPlaceBack), sets its speed to $0C00 in X and $E400 in Y (X $F600 and Y $DC00 and the next state when the object before it has frame $15), moves once and clears the speeds. (The first part ran in no script; it is written from the bytes.)
; HOW: CopyObjectPlaceBack, stores, ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: the last clear (all registers as ApplyVelocity left them but A, N and Z).
FollowBlockWhileState5:
    lda a:$27A7,x
    cmp #$05
    beq FollowBlockWhileState5_at0
    lda a:OBJ_31A0,x
    ora #$80
    sta a:OBJ_31A0,x
    rts
FollowBlockWhileState5_at0:
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    lda #$0C
    sta a:OBJ_VX_HI,x
    lda #$E4
    sta a:OBJ_VY_HI,x
    lda a:$2A67,x
    cmp #$15
    bne SoundCommandLengths
    lda #$F6
    sta a:OBJ_VX_HI,x
    lda #$DC
    sta a:OBJ_VY_HI,x
    inc a:OBJ_27A8,x
    jsr VecApplyVelocity
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VY_HI,x
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

; @routine 051E:090 LoadThenSinkAlongList
; @run 71:C890
; WHAT IT DOES: In state 0 loads the object's record, sets OBJ_2FE8 to $80, keeps its Y low in OBJ_34B8 and gives OBJ_3408 a random $00 to $0F. Each frame it adds $FE80 to its Y position (fraction and low byte) and steps the animation list at $C8D0; when the list ends it puts its Y low back from OBJ_34B8.
; HOW: LoadObjectRecord, NextRandom, a 16-bit add, StepObjectFrames.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step, or the load of OBJ_34B8 when the list ended.
LoadThenSinkAlongList:
    lda a:OBJ_27A8,x
    bne LoadThenSinkAlongList_at0
    jsr VecLoadObjectRecord
    lda #$80
    sta a:OBJ_2FE8,x
    lda a:OBJ_Y_LO,x
    sta a:OBJ_34B8,x
    jsr VecNextRandom
    and #$0F
    sta a:OBJ_3408,x
LoadThenSinkAlongList_at0:
    lda a:OBJ_Y_FRAC,x
    clc
    adc #$80
    sta a:OBJ_Y_FRAC,x
    lda a:OBJ_Y_LO,x
    adc #$FE
    sta a:OBJ_Y_LO,x
    lda #$D0
    sta z:zTemp10
    lda #$C8
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne LoadThenSinkAlongList_at1
    lda a:OBJ_34B8,x
    sta a:OBJ_Y_LO,x
LoadThenSinkAlongList_at1:
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
; @routine 051E:115 SpawnPointFromTables
; @run 71:C915
; WHAT IT DOES: Counts OBJ_34B8 down; at zero it gives itself a new timer (RandomTimer10), finds a free slot from $30 and there starts a kind $AF object with this object's OBJ_3040, placed from the table at $C99A by that byte and OBJ_3510's low three bits (four bytes a row: X low, X high, Y low, Y high; the X and Y low plus a random 0 to 7), and OBJ_3510 low two bits; then counts OBJ_3510 up.
; HOW: RandomTimer10, FindFreeSlotFrom30, table reads (bank 70), NextRandom twice.
; WAITS: no.
; Entry: X object. Exit: the new OBJ_3510's N and Z, Y the new slot; or the count.
SpawnPointFromTables:
    dec a:OBJ_34B8,x
    bne SpawnPointFromTables_at0
    bsr RandomTimer10
    jsr VecFindFreeSlotFrom30
    bcc SpawnPointFromTables_at0
    lda #$AF
    sta a:OBJ_2750,y
    phx
    lda a:OBJ_3040,x
    sta a:OBJ_3040,y
    sta z:zTemp10
    lda a:OBJ_3510,x
    and #$07
    asl a
    asl a
    ldx z:zTemp10
    clc
    adc a:$C97A,x
    tax
    lda a:$CA99,x
    sta a:OBJ_X_LO,y
    lda a:$CA9A,x
    sta a:OBJ_X_HI,y
    lda a:$CA9B,x
    sta a:OBJ_Y_LO,y
    lda a:$CA9C,x
    sta a:OBJ_Y_HI,y
    plx
    jsr VecNextRandom
    and #$07
    clc
    adc a:OBJ_X_LO,y
    sta a:OBJ_X_LO,y
    jsr VecNextRandom
    and #$07
    clc
    adc a:OBJ_Y_LO,y
    sta a:OBJ_Y_LO,y
    lda a:OBJ_3510,x
    and #$03
    sta a:OBJ_3510,y
    inc a:OBJ_3510,x
SpawnPointFromTables_at0:
    rts
