; @routine 09EA:55A FaceByCue
; @run 70:AD5A
; WHAT IT DOES: While the screen is not scrolling along X, and in a state the table at $AD70 marks, the object
; faces the way the player is (bit 0 of SideOfPlayer into its facing).
; HOW: Reads ScrollSpeedX and the table by state, then SideOfPlayer.
; WAITS: no.
; Entry: X object. Exit: A and N, Z from the last load or AND; Y the state when the table was read.
FaceByCue:
    lda a:ScrollSpeedX
    bne FaceByCue_at0
    ldy a:OBJ_27A8,x
    lda a:$AD70,y
    beq FaceByCue_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
FaceByCue_at0:
    rts

; @routine 09EA:664 StartTwinSpawner
; @run 70:AE64
; WHAT IT DOES: Loads the object's record, clears OBJ_3618, sets its wait to $60 and sets bit 2 of zField8E; then
; clears objects $11 and $12 and starts a kind $22 and a kind $27 at this object's place in them.
; HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace; the last is a tail jump.
; WAITS: no.
; Entry: X object. Exit: A the last byte copied, N and Z from it, Y $12, C from the record.
StartTwinSpawner:
    jsr VecLoadObjectRecord
    stz a:OBJ_3618,x
    lda #$60
    sta a:OBJ_3510,x
    lda z:zField8E
    ora #$04
    sta z:zField8E
    ldy #$11
    jsr VecClearObjectInY
    lda #$22
    jsr VecStartObjectAtPlace
    ldy #$12
    jsr VecClearObjectInY
    lda #$27
    jmp VecStartObjectAtPlace

; @routine 09EA:689 WalkTowardPlayer
; @run 70:AE89
; WHAT IT DOES: PickNextAction and WalkSideways, then the facing byte becomes the walk direction and, if the player is
; within $40 along X, the object enters state 2.
; HOW: DistanceXToPlayer.
; Untraced: the state 2 arm (AE9C).
; WAITS: no.
; Entry: X object. Exit: the last compare, A the distance.
WalkTowardPlayer:
    jsr PickNextAction
    jsr WalkSideways
    lda a:OBJ_2AC0,x
    sta a:OBJ_3568,x
    jsr VecDistanceXToPlayer
    cmp #$40
    bcs WalkTowardPlayer_at0
    lda #$02
    sta a:OBJ_27A8,x
WalkTowardPlayer_at0:
    rts

; @routine 09EA:6A2 WalkSideways
; @run 70:AEA2
; WHAT IT DOES: Steps the walk animation (list $AEC3), sets the X speed to $FFC0 (left), reverses it when the object's
; direction byte OBJ_3568 is set, and moves with WalkAndTurnAtEdges.
; HOW: StepObjectFrames, NegateVelocityX, WalkAndTurnAtEdges.
; WAITS: no.
; Entry: X object. Exit: as WalkAndTurnAtEdges.
WalkSideways:
    lda #$C3
    sta z:zTemp10
    lda #$AE
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$FF
    sta a:OBJ_VX_HI,x
    lda #$C0
    sta a:OBJ_VX_LO,x
    lda a:OBJ_3568,x
    beq WalkSideways_at0
    jsr VecNegateVelocityX
WalkSideways_at0:
    jsr WalkAndTurnAtEdges
    rts

; @routine 09EA:6CC WalkAndTurnAtEdges
; @run 70:AECC
; WHAT IT DOES: Moves the object along X; when its X plus the scroll reaches an edge (page 3 from $30, or any other
; page below $48) it turns round: the X speed is reversed and the move made again.
; HOW: ApplyVelocityX, the add of ScrollXLo and ScrollXHiScreen, then the compare; the turn is NegateVelocityX and a
; tail jump into ApplyVelocityX.
; Untraced: both arms that turn, and the page 3 branch (AEE9 and AEEF).
; WAITS: no.
; Entry: X object. Exit: no turn: A the low byte, N, Z, C from its compare; turn: as ApplyVelocityX.
WalkAndTurnAtEdges:
    jsr VecApplyVelocityX
    lda a:OBJ_X_LO,x
    clc
    adc a:ScrollXLo
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    adc a:ScrollXHiScreen
    cmp #$03
    beq WalkAndTurnAtEdges_at0
    lda z:zTemp00
    cmp #$48
    bcc WalkAndTurnAtEdges_at1
WalkAndTurnAtEdges_at2:
    rts
WalkAndTurnAtEdges_at0:
    lda z:zTemp00
    cmp #$30
    bcc WalkAndTurnAtEdges_at2
WalkAndTurnAtEdges_at1:
    jsr VecNegateVelocityX
    jmp VecApplyVelocityX

; @routine 09EA:6F5 PickNextAction
; @run 70:AEF5
; WHAT IT DOES: While its wait OBJ_3510 runs, counts it down. At zero it picks a random state (from the pair of bytes
; at $AF28), plays the sound the pair's second byte names, resets its animation and picks a new wait from the table
; at $AF24 by a random 0 to 3.
; HOW: NextRandom twice, SoundCommand.
; WAITS: no.
; Entry: X object, Y kept unless the picks set it. Exit: waiting: A the old wait, N and Z from the new; else A the
; new wait, Y its index, C and V from the second random.
PickNextAction:
    lda a:OBJ_3510,x
    beq PickNextAction_at0
    dec a:OBJ_3510,x
    rts
PickNextAction_at0:
    jsr VecNextRandom
    and #$01
    asl a
    tay
    lda a:$AF28,y
    sta a:OBJ_27A8,x
    lda a:$AF29,y
    jsr VecSoundCommand
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    jsr VecNextRandom
    and #$03
    tay
    lda a:$AF24,y
    sta a:OBJ_3510,x
    rts
; @routine 09EA:72C WalkAndPick
; @run 70:AF2C
; WHAT IT DOES: WalkSideways; the walk direction OBJ_3568 becomes the facing turned round; when the player is $80 or more away along X the object enters state 1; then
; PickNextAction (a tail jump).
; HOW: WalkSideways, DistanceXToPlayer, PickNextAction.
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: as PickNextAction.
WalkAndPick:
    jsr WalkSideways
    lda a:OBJ_2AC0,x
    eor #$01
    sta a:OBJ_3568,x
    jsr VecDistanceXToPlayer
    cmp #$80
    bcc WalkAndPick_at0
    lda #$01
    sta a:OBJ_27A8,x
WalkAndPick_at0:
    jmp PickNextAction

; @routine 09EA:746 StepShootOnCue
; @run 70:AF46
; WHAT IT DOES: Steps the animation list at $AF89; when it ends the object enters state 2; on a new frame with animation step 3 it plays sound $26; then PlayCueOnFrame runs;
; on frame $0A with OBJ_35C0's low three bits clear it starts a kind $24 object at its place in the first free slot from $30, facing as it does.
; HOW: StepObjectFrames, SoundCommand, PlayCueOnFrame, FindFreeSlotFrom30, StartObjectAtPlace.
; Untraced: one byte.
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
StepShootOnCue:
    lda #$AF
    sta z:zTemp11
    lda #$89
    sta z:zTemp10
    jsr VecStepObjectFrames
    beq StepShootOnCue_at0
    bcc StepShootOnCue_at1
    lda a:OBJ_3408,x
    cmp #$03
    bne StepShootOnCue_at1
    lda #$26
    jsr VecSoundCommand
StepShootOnCue_at1:
    jsr PlayCueOnFrame
    lda a:OBJ_2A68,x
    cmp #$0A
    bne StepShootOnCue_at2
    lda a:OBJ_35C0,x
    and #$07
    bne StepShootOnCue_at2
    jsr VecFindFreeSlotFrom30
    bcc StepShootOnCue_at2
    lda #$24
    jsr VecStartObjectAtPlace
    lda a:OBJ_2AC0,x
    sta a:OBJ_2AC0,y
    rts
StepShootOnCue_at0:
    lda #$02
    sta a:OBJ_27A8,x
StepShootOnCue_at2:
    rts

; @routine 09EA:792 PlayCueOnFrame
; @run 70:AF92
; @stop AFB9
; WHAT IT DOES: An object showing frame 8 or $0A starts a script when its animation timer equals one of the cue
; counts in a table at $AFB9 (frame 8: the first eight; frame $0A: the next fourteen); the script is $80 plus the
; cue's index.
; HOW: A scan of the table with the timer, ending in StartCueScript.
; Untraced: the table (AFB9 to AFCF) is data the traced run only read.
; WAITS: no.
; Entry: X object. Exit: no cue: A and flags of the last compare; cue: as StartCueScript.
PlayCueOnFrame:
    lda a:OBJ_2A68,x
    cmp #$08
    beq PlayCueOnFrame_at0
    cmp #$0A
    beq PlayCueOnFrame_at1
    rts
PlayCueOnFrame_at0:
    cly
    lda #$08
    sta z:zTemp00
    bra PlayCueOnFrame_at2
PlayCueOnFrame_at1:
    ldy #$08
    lda #$16
    sta z:zTemp00
PlayCueOnFrame_at2:
    lda a:OBJ_3460,x
    cmp a:$AFB9,y
    beq StartCueScript
    iny
    cpy z:zTemp00
    bcc PlayCueOnFrame_at2
    rts

; @routine 09EA:7D0 StartCueScript
; @run 70:AFD0
; WHAT IT DOES: Starts the script slot 5 with the script $80 plus Y (the last part of PlayCueOnFrame).
; HOW: X is pushed and pulled around StartScriptSlotKind4.
; WAITS: no.
; Entry: X object, Y the cue's index. Exit: A and Y the script, N and Z from X, C and V from the add of $80.
StartCueScript:
    phx
    tya
    clc
    adc #$80
    ldx #$05
    jsr VecStartScriptSlotKind4
    plx
    rts

; @routine 09EA:7DC StepFlyingCue
; @run 70:AFDC
; WHAT IT DOES: Steps the animation list at $AFFE; when it ends the object enters state 2; on a new frame, at animation
; step $1A, sound $1A plays.
; HOW: StepObjectFrames, SoundCommand.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
StepFlyingCue:
    lda #$AF
    sta z:zTemp11
    lda #$FE
    sta z:zTemp10
    jsr VecStepObjectFrames
    beq StepFlyingCue_at1
    bcc StepFlyingCue_at2
    lda a:OBJ_3408,x
    cmp #$1A
    bne StepFlyingCue_at2
    lda #$1A
StepFlyingCue_at0:
    jsr VecSoundCommand
    rts
StepFlyingCue_at1:
    lda #$02
    sta a:OBJ_27A8,x
StepFlyingCue_at2:
    rts
; @routine 09EB:03B StepThenQuarterWait
; @run 70:B03B
; WHAT IT DOES: Steps the animation list at $B060; when it ends the object's wait OBJ_3510 is quartered, it enters state 2
; and its timer is raised to at least 3.
; HOW: StepObjectFrames, two LSRs of memory, a compare.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: list running: as the step; ended: the last compare or load.
StepThenQuarterWait:
    lda #$B0
    sta z:zTemp11
    lda #$60
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne StepThenQuarterWait_at0
    lsr a:OBJ_3510,x
    lsr a:OBJ_3510,x
    lda #$02
    sta a:OBJ_27A8,x
    lda a:OBJ_3A90,x
    cmp #$03
    bcs StepThenQuarterWait_at0
    lda #$03
    sta a:OBJ_3A90,x
StepThenQuarterWait_at0:
    rts

; @routine 09EB:067 StartShotRing
; @run 70:B067
; WHAT IT DOES: Clears objects $11 and $12, starts ten objects of kind $28 at this object's place in slots $18 to $27, each
; with this object's facing; then resets the animation, puts the object off the left of the screen (X from $C0 less the scroll,
; Y $FFC0), stops it and enters its next state.
; HOW: ClearObjectInY and StartObjectAtPlace in a loop.
; WAITS: no.
; Entry: X object. Exit: A $C0, Y $28, N and Z from the new state, C and V from the subtract of the scroll.
StartShotRing:
    ldy #$11
    jsr VecClearObjectInY
    iny
    jsr VecClearObjectInY
    ldy #$18
StartShotRing_at0:
    jsr VecClearObjectInY
    lda #$28
    jsr VecStartObjectAtPlace
    lda a:OBJ_2AC0,x
    sta a:OBJ_2AC0,y
    iny
    cpy #$28
    bcc StartShotRing_at0
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    stz a:OBJ_X_HI,x
    lda #$C0
    sec
    sbc a:ScrollXLo
    sta a:OBJ_X_LO,x
    lda #$FF
    sta a:OBJ_Y_HI,x
    lda #$C0
    sta a:OBJ_Y_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_2A68,x
    inc a:OBJ_27A8,x
    rts

; @routine 09EB:0AE EnterStateWhenSlotClear
; @run 70:B0AE
; WHAT IT DOES: The object enters its next state once object $26's kind is 0 (the one it waits for is gone).
; HOW: A read of that kind and an INC.
; WAITS: no.
; Entry: X object. Exit: A the kind read, N and Z from it or from the new state.
EnterStateWhenSlotClear:
    lda a:$2776
    bne EnterStateWhenSlotClear_at0
    inc a:OBJ_27A8,x
EnterStateWhenSlotClear_at0:
    rts

; @routine 09EB:0B7 BurstOnLanding
; @run 70:B0B7
; WHAT IT DOES: A thing that falls (list $B101, unlimited gravity) and bursts when it reaches Y $0040 on the first page: sound
; $A1, script slot 3 started, kind $29 objects started in slots $12 to $19, a sideways speed of $0140 and the next state.
; HOW: StepObjectFrames, ApplyGravityUnlimited, ApplyVelocityY, SoundCommand, StartSoleScriptSlot, StartObjectAtPlace.
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
BurstOnLanding:
    lda #$B1
    sta z:zTemp11
    lda #$01
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jsr VecApplyVelocityY
    lda a:OBJ_Y_HI,x
    bne BurstOnLanding_at0
    lda a:OBJ_Y_LO,x
    cmp #$40
    bcc BurstOnLanding_at0
    lda #$A1
    jsr VecSoundCommand
    phx
    cla
    ldx #$03
    jsr VecStartSoleScriptSlot
    plx
    ldy #$12
BurstOnLanding_at1:
    lda #$29
    jsr VecStartObjectAtPlace
    iny
    cpy #$1A
    bcc BurstOnLanding_at1
    lda #$01
    sta a:OBJ_VX_HI,x
    lda #$40
    sta a:OBJ_VX_LO,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    inc a:OBJ_27A8,x
BurstOnLanding_at0:
    rts

; @routine 09EB:16F InWindowRange
; @run 70:B16F
; WHAT IT DOES: Says whether the object is inside the page range the stage keeps its things alive in: the object's X plus the scroll,
; high byte 3 or more with the low byte $C0 or more, or high byte 1 with the low byte under $C0.
; HOW: The add of ScrollXLo and ScrollXHiScreen and two compares.
; WAITS: no.
; Entry: X object. Exit: C set when inside; A the last load, N and Z from the last compare, V from the add.
InWindowRange:
    lda a:OBJ_X_LO,x
    clc
    adc a:ScrollXLo
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    adc a:ScrollXHiScreen
    cmp #$03
    bcs InWindowRange_at0
    cmp #$01
    bne InWindowRange_at1
    lda z:zTemp00
    cmp #$C0
    bcs InWindowRange_at1
InWindowRange_at2:
    sec
    rts
InWindowRange_at0:
    lda z:zTemp00
    cmp #$C0
    bcs InWindowRange_at2
InWindowRange_at1:
    clc
    rts

; @routine 09EB:196 StartSplitterHigh
; @run 70:B196
; WHAT IT DOES: Starts a falling splitter at Y low $48: its X speed is picked from a table at $B1BF by two bits of the
; player's distance sum, OBJ_3670 is cleared, OBJ_2800 moves on, and when the object is on the player's left its X speed is
; reversed.
; HOW: PlayerDistanceSum, SideOfPlayer, NegateVelocityX.
; Untraced: the reversal (B1BB).
; WAITS: no.
; Entry: X object, C passed to PlayerDistanceSum. Exit: the last load, AND or subtract.
StartSplitterHigh:
    jsr VecPlayerDistanceSum
    and #$03
    asl a
    tay
    lda a:$B1C0,y
    sta a:OBJ_VX_HI,x
    lda a:$B1BF,y
    sta a:OBJ_VX_LO,x
    lda #$48
    sta a:OBJ_Y_LO,x
    stz a:OBJ_3670,x
    inc a:OBJ_2800,x
    jsr VecSideOfPlayer
    and #$01
    beq StartSplitterHigh_at0
    jsr VecNegateVelocityX
StartSplitterHigh_at0:
    rts

; @routine 09EB:1E8 StartSplitterLow
; @run 70:B1E8
; WHAT IT DOES: StartSplitterHigh with the Y low byte $58.
; HOW: As StartSplitterHigh.
; Untraced: the reversal (B20D).
; WAITS: no.
; Entry and exit: as StartSplitterHigh.
StartSplitterLow:
    jsr VecPlayerDistanceSum
    and #$03
    asl a
    tay
    lda a:$B1C0,y
    sta a:OBJ_VX_HI,x
    lda a:$B1BF,y
    sta a:OBJ_VX_LO,x
    lda #$58
    sta a:OBJ_Y_LO,x
    stz a:OBJ_3670,x
    inc a:OBJ_2800,x
    jsr VecSideOfPlayer
    and #$01
    beq StartSplitterLow_at0
    jsr VecNegateVelocityX
StartSplitterLow_at0:
    rts

; @routine 09EB:21A MaybeShootAhead
; @run 70:B21A
; WHAT IT DOES: Every 32nd count of the object's OBJ_35C0, with a chance of three in four, starts a kind $2A object at its place in
; the first free slot from $30 and gives it the object's facing.
; HOW: NextRandom, FindFreeSlotFrom30, StartObjectAtPlace.
; Untraced: one byte.
; WAITS: no.
; Entry: X object. Exit: the last load, AND or INC.
MaybeShootAhead:
    lda a:OBJ_35C0,x
    and #$1F
    bne MaybeShootAhead_at0
    jsr VecNextRandom
    and #$03
    beq MaybeShootAhead_at0
    jsr VecFindFreeSlotFrom30
    bcc MaybeShootAhead_at0
    lda #$2A
    jsr VecStartObjectAtPlace
    lda a:OBJ_2AC0,x
    sta a:OBJ_2AC0,y
MaybeShootAhead_at0:
    rts
; @routine 09EB:51E PickWaterLevel
; @run 70:B51E
; WHAT IT DOES: Sets the water the stage wants (RasterWaterWanted and LevelYHi) to a two-byte pair from a table at $B531 picked by
; two bits of the wave counter.
; HOW: Reads the wave counter and the table (bank 70).
; WAITS: no.
; Entry: nothing. Exit: A the second byte, Y the index, N and Z from A, C 0.
PickWaterLevel:
    lda z:zWaveCounter
    and #$03
    asl a
    tay
    lda a:$B531,y
    sta a:RasterWaterWanted
    lda a:$B532,y
    sta a:LevelYHi
    rts

; @routine 09EB:55F WaitForPage2
; @run 70:B55F
; WHAT IT DOES: When the level position's high byte is 2, the object's timer is set to $40 and it enters its next state; the
; object in slot $10 also plays sound $A2.
; HOW: A compare of ScrollXHiScreen, an INC, SoundCommand.
; WAITS: no.
; Entry: X object, Y pushed by the sound driver. Exit: see the wrapper.
WaitForPage2:
    lda a:ScrollXHiScreen
    cmp #$02
    bne WaitForPage2_at0
    lda #$40
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8,x
    cpx #$10
    bne WaitForPage2_at0
    lda #$A2
    jsr VecSoundCommand
WaitForPage2_at0:
    rts

; @routine 09EB:5BB RiseUntilAboveGround
; @run 70:B5BB
; WHAT IT DOES: Moves the object, takes $0040 off its Y speed (it climbs), and when its Y position has passed from negative
; to a low byte under $80 it enters its next state.
; HOW: ApplyVelocity, a 16-bit subtract.
; WAITS: no.
; Entry: X object. Exit: the last load or INC.
RiseUntilAboveGround:
    jsr VecApplyVelocity
    lda a:OBJ_VY_LO,x
    sec
    sbc #$40
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    lda a:OBJ_Y_HI,x
    bpl RiseUntilAboveGround_at0
    lda a:OBJ_Y_LO,x
    bmi RiseUntilAboveGround_at0
    inc a:OBJ_27A8,x
RiseUntilAboveGround_at0:
    rts

; @routine 09EB:5DD WaitForSlot10State4
; @run 70:B5DD
; WHAT IT DOES: Once the object in slot $10 has reached state 4, this object's timer becomes $40 and it enters its next state.
; HOW: A compare of that state.
; WAITS: no.
; Entry: X object. Exit: the last compare, load or INC.
WaitForSlot10State4:
    lda a:$27B8
    cmp #$04
    bcc WaitForSlot10State4_at0
    lda #$40
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8,x
WaitForSlot10State4_at0:
    rts

; @routine 09EB:7A4 FollowPlayerHitbox
; @run 70:B7A4
; WHAT IT DOES: In state 0 loads the object's record; later it takes the player's place and sets its hitbox from a four-byte row of the
; table at $BC8B picked by the player's frame less 5.
; HOW: LoadObjectRecord (a tail jump), CopyObjectPlaceBack from slot $10, the table (bank 70).
; WAITS: no.
; Entry: X object. Exit: state 0: as LoadObjectRecord; else A the last table byte, Y the row, N and Z from A, C and V from the subtract.
FollowPlayerHitbox:
    lda a:OBJ_27A8,x
    bne FollowPlayerHitbox_at0
    jmp VecLoadObjectRecord
FollowPlayerHitbox_at0:
    ldy #$10
    jsr VecCopyObjectPlaceBack
    lda a:$2A78
    sec
    sbc #$05
    asl a
    asl a
    tay
    lda a:$BC8B,y
    sta a:OBJ_3930,x
    lda a:$BC8C,y
    sta a:OBJ_39E0,x
    lda a:$BC8D,y
    sta a:OBJ_3988,x
    lda a:$BC8E,y
    sta a:OBJ_3A38,x
    rts

; @routine 09EB:7EB StartNumberedPiece
; @run 70:B7EB
; WHAT IT DOES: Loads the object's record; its timer comes from a table at $B801 by the slot less $18; its frame is that number plus
; $12.
; HOW: LoadObjectRecord and an add.
; WAITS: no.
; Entry: X object. Exit: A the frame, N, Z, C and V from the add, Y the number.
StartNumberedPiece:
    jsr VecLoadObjectRecord
    txa
    sec
    sbc #$18
    tay
    lda a:SoundEffectTable+6,y
    sta a:OBJ_34B8,x
    tya
    clc
    adc #$12
    sta a:OBJ_2A68,x
    rts
; @routine 09EC:01A RiseAndClear
; @run 70:B81A
; WHAT IT DOES: Moves the object, takes $0040 off its Y speed (it climbs) and clears it once its Y position has passed from negative
; to a low byte under $80.
; HOW: ApplyVelocity, a 16-bit subtract, a tail jump into ClearObject.
; WAITS: no.
; Entry: X object. Exit: the last load or subtract.
RiseAndClear:
    jsr VecApplyVelocity
    lda a:OBJ_VY_LO,x
    sec
    sbc #$40
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    lda a:OBJ_Y_HI,x
    bpl RiseAndClear_at0
    lda a:OBJ_Y_LO,x
    bmi RiseAndClear_at0
    jmp VecClearObject
RiseAndClear_at0:
    rts

; @routine 09EC:059 RaiseStateAfterSlot10
; @run 70:B859
; WHAT IT DOES: Once the object in slot $10 is in state $0A or later, an object below state 3 enters state 3.
; HOW: Two compares.
; Untraced: the whole arm after the first compare (B864 to B86C).
; WAITS: no.
; Entry: X object. Exit: the last compare or load.
RaiseStateAfterSlot10:
    lda a:$27B8
    cmp #$0A
    bcc RaiseStateAfterSlot10_at0
    lda a:OBJ_27A8,x
    cmp #$03
    bcs RaiseStateAfterSlot10_at0
    lda #$03
    sta a:OBJ_27A8,x
RaiseStateAfterSlot10_at0:
    rts

; @routine 09EC:06D StartChainLink
; @run 70:B86D
; WHAT IT DOES: Loads the object's record, sets OBJ_3510 to $41 and its timer from a table at $B888 by the slot less $12; the object in slot
; $19 also moves to its next frame.
; HOW: LoadObjectRecord and an INC.
; WAITS: no.
; Entry: X object. Exit: slot $19: N and Z of the new frame; else the compare of X with $19.
StartChainLink:
    jsr VecLoadObjectRecord
    lda #$41
    sta a:OBJ_3510,x
    txa
    sec
    sbc #$12
    tay
    lda a:$B888,y
    sta a:OBJ_34B8,x
    cpx #$19
    bne TurnOnRandomBeatWalkFollowC
    inc a:OBJ_2A68,x
    rts

; @routine 09EC:090 ChainCountdown
; @run 70:B890
; WHAT IT DOES: A link of the chain follows the one before it (ChainFollowsPrevious); its timer counts down and at zero it enters its next state.
; HOW: ChainFollowsPrevious, DEC, INC.
; WAITS: no.
; Entry: X object. Exit: as ChainFollowsPrevious, then N and Z of the count or of the new state.
ChainCountdown:
    jsr ChainFollowsPrevious
    dec a:OBJ_34B8,x
    bne ChainCountdown_at0
    inc a:OBJ_27A8,x
ChainCountdown_at0:
    rts

; @routine 09EC:09C ChainSwings
; @run 70:B89C
; WHAT IT DOES: A link follows the one before it, takes slot $10's facing, and swings: the cosine of its angle OBJ_35C0 (multiplier $0200) is added to OBJ_3568 and OBJ_3510 as
; a 16-bit pair, and a 16-bit step from a table at $B8E4 (by OBJ_3670 and 7) is added to OBJ_3618 and OBJ_35C0; a carry out of that goes up one on OBJ_3670.
; HOW: ChainFollowsPrevious, ScaleByCosine, table reads (bank 70).
; WAITS: no.
; Entry: X object. Exit: the last add or INC.
ChainSwings:
    jsr ChainFollowsPrevious
    lda a:$2AD0
    sta a:OBJ_2AC0,x
    lda #$02
    sta z:zTemp01
    lda #$00
    sta z:zTemp00
    lda a:OBJ_35C0,x
    jsr VecScaleByCosine
    lda a:OBJ_3568,x
    clc
    adc z:SpriteFlipMask
    sta a:OBJ_3568,x
    lda a:OBJ_3510,x
    adc z:zProduct2
    sta a:OBJ_3510,x
    lda a:OBJ_3670,x
    and #$07
    asl a
    tay
    lda a:OBJ_3618,x
    clc
    adc a:$B8E4,y
    sta a:OBJ_3618,x
    lda a:OBJ_35C0,x
    adc a:$B8E5,y
    sta a:OBJ_35C0,x
    bcc ChainSwings_at0
    inc a:OBJ_3670,x
ChainSwings_at0:
    rts

; @routine 09EC:0F4 ChainFollowsPrevious
; @run 70:B8F4
; WHAT IT DOES: A link of a chain: object $12 takes the place of slot $10 and moves sideways by a table byte ($B923) picked by that
; slot's facing, with Y speed 8; any other link takes the place of the link before it and takes its speed from its angle OBJ_3510 with a
; length of $0A00; both then move.
; HOW: CopyObjectPlaceBack, SetVelocityFromAngle, and a tail jump into ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as ApplyVelocity, Y the table index or the slot before.
ChainFollowsPrevious:
    cpx #$12
    bne ChainFollowsPrevious_at0
    ldy #$10
    jsr VecCopyObjectPlaceBack
    ldy a:$2AD0
    lda a:$B923,y
    sta a:OBJ_VX_HI,x
    lda #$08
    sta a:OBJ_VY_HI,x
    jmp VecApplyVelocity
ChainFollowsPrevious_at0:
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    stz z:zTemp00
    lda #$0A
    sta z:zTemp01
    lda a:OBJ_3510,x
    jsr VecSetVelocityFromAngle
    jmp VecApplyVelocity

; @routine 09EC:1A5 StartKickedPiece
; @run 70:B9A5
; WHAT IT DOES: Loads the object's record, gives it an X speed from a table at $B9BD by its facing and a Y speed of $10, moves it once and
; stops its Y speed.
; HOW: LoadObjectRecord, ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as ApplyVelocity with Y the facing.
StartKickedPiece:
    jsr VecLoadObjectRecord
    ldy a:OBJ_2AC0,x
    lda a:$B9BD,y
    sta a:OBJ_VX_HI,x
    lda #$10
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    stz a:OBJ_VY_HI,x
    rts

; @routine 09EC:203 StepThenClear
; @run 70:BA03
; WHAT IT DOES: Steps the animation list at $BA14; when it ends the object is cleared.
; HOW: StepObjectFrames and a tail jump into ClearObject.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the step.
StepThenClear:
    lda #$BA
    sta z:zTemp11
    lda #$14
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne StepThenClear_at0
    jmp VecClearObject
StepThenClear_at0:
    rts
; @routine 09EC:7BC JumpByStateTable
; @run 70:BFBC
; WHAT IT DOES: Goes to the code a table at $BFCE names for the object's state.
; HOW: The table's two bytes go to zTemp10-11 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable:
    lda a:OBJ_27A8,x
    asl a
    tay
    lda a:$BFCE,y
    sta z:zTemp10
    lda a:$BFCF,y
    sta z:zTemp11
    jmp (zTemp10+$2000)

; @routine 09EC:7F2 SwayThenReact
; @run 70:BFF2
; WHAT IT DOES: Steps the animation by phase, sets the sway speed, bobs and moves, then reacts at the window edge.
; HOW: StepListByPhase, SetSwaySpeed, BobAndMove, a tail jump into ReactAtWindowEdge.
; WAITS: no.
; Entry: X object. Exit: as ReactAtWindowEdge.
SwayThenReact:
    jsr StepListByPhase
    jsr SetSwaySpeed
    jsr BobAndMove
    jmp ReactAtWindowEdge

; @routine 09EC:7FE SwayOrAdvance
; @run 70:BFFE
; WHAT IT DOES: Counts the object's angle byte down; at zero it goes on as EnterNextAndReset, otherwise it sways (SwayStep).
; HOW: DEC, then the tail calls.
; WAITS: no.
; Entry: X object. Exit: as EnterNextAndReset or BobAndMove.
SwayOrAdvance:
    dec a:OBJ_3568,x
    beq EnterNextAndReset
SwayOrAdvance_at0:
    jsr StepListByPhase
    jsr SetSwaySpeed
    jmp BobAndMove

; @routine 09ED:00C SwayThenFlip
; @run 71:C00C
; WHAT IT DOES: Counts the angle byte down; at zero it flips the side, goes on as EnterNextAndReset and sets OBJ_3510 to 1; otherwise it sways.
; HOW: DEC, FlipSide, EnterNextAndReset, or SwayStep.
; WAITS: no.
; Entry: X object. Exit: at zero A 1 from the load; else as BobAndMove.
SwayThenFlip:
    dec a:OBJ_3568,x
    beq SwayThenFlip_at0
    bne SwayOrAdvance_at0
SwayThenFlip_at0:
    jsr FlipSide
    jsr EnterNextAndReset
    lda #$01
    sta a:OBJ_3510,x
    rts

; @routine 09ED:01F SwayThenRestart
; @run 71:C01F
; WHAT IT DOES: Counts the angle byte down; at zero it goes on as EnterNextAndReset, puts the object in state 1 and OBJ_3510 at 0; otherwise it sways.
; HOW: DEC, EnterNextAndReset, or SwayStep.
; WAITS: no.
; Entry: X object. Exit: at zero A 1 from the load; else as BobAndMove.
SwayThenRestart:
    dec a:OBJ_3568,x
    beq SwayThenRestart_at0
    bne SwayOrAdvance_at0
SwayThenRestart_at0:
    jsr EnterNextAndReset
    lda #$01
    sta a:OBJ_27A8,x
    stz a:OBJ_3510,x
    rts

; @routine 09ED:032 ClearAnimation
; @run 71:C032
; WHAT IT DOES: Restarts the object's animation: step and timer 0.
; HOW: Two stores.
; WAITS: no.
; Entry: X object. Exit: every register and flag kept.
ClearAnimation:
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    rts

; @routine 09ED:039 StepListByPhase
; @run 71:C039
; WHAT IT DOES: The object faces as its life timer says and steps the animation list that a table at $C103 names for its OBJ_3510.
; HOW: Reads the list's address into zTemp10-11; a tail jump into StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, Y kept unless the step sets it (the index otherwise).
StepListByPhase:
    lda a:OBJ_34B8,x
    sta a:OBJ_2AC0,x
    lda a:OBJ_3510,x
    asl a
    tay
    lda a:$C103,y
    sta z:zTemp10
    lda a:$C104,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 09ED:051 ReactAtWindowEdge
; @run 71:C051
; WHAT IT DOES: When the object's X plus the scroll is at the edge its side (life timer 0 or 1) names ($E0 or more for 0, under $20 for 1) and it
; is in state 1, it goes on as EnterNextAndReset.
; HOW: ScrollXLo and two compares; the last step runs on into the next routine's bytes.
; Untraced: one byte (the fall-through branch).
; WAITS: no.
; Entry: X object. Exit: the last compare or the load; Y the side.
ReactAtWindowEdge:
    lda a:OBJ_X_LO,x
    clc
    adc a:ScrollXLo
    ldy a:OBJ_34B8,x
    beq ReactAtWindowEdge_at0
    cmp #$20
    bcc ReactAtWindowEdge_at1
    rts
ReactAtWindowEdge_at0:
    cmp #$E0
    bcs ReactAtWindowEdge_at1
    rts
ReactAtWindowEdge_at1:
    lda a:OBJ_27A8,x
    cmp #$01
    bne EnterNextAndReset_at0

; @routine 09ED:06E EnterNextAndReset
; @run 71:C06E
; WHAT IT DOES: Moves the object on one state and its OBJ_3510 on one, restarts its animation and gives it a countdown of $20 in
; its angle byte.
; HOW: Two INCs, ClearAnimation, a store.
; WAITS: no.
; Entry: X object. Exit: A $20, N and Z from it.
EnterNextAndReset:
    inc a:OBJ_27A8,x
    inc a:OBJ_3510,x
    jsr ClearAnimation
    lda #$20
    sta a:OBJ_3568,x
EnterNextAndReset_at0:
    rts

; @routine 09ED:07D FlipSide
; @run 71:C07D
; WHAT IT DOES: Flips bit 0 of the object's life timer (the side it works on).
; HOW: An EOR.
; WAITS: no.
; Entry: X object. Exit: A the new value, N and Z from it.
FlipSide:
    lda a:OBJ_34B8,x
    eor #$01
    sta a:OBJ_34B8,x
    rts

; @routine 09ED:086 BobAndMove
; @run 71:C086
; WHAT IT DOES: Makes the object bob: OBJ_3618 counts up and its sine (scaled by 3FF) added to OBJ_35C0 is its Y low byte; between
; Y $B0 and the right place on screen (X from $80 to $9F once the scroll is added) a random wobble of -1 to 2 is added; then it moves.
; HOW: ScaleBySine, NextRandom, a tail jump into ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as ApplyVelocity, Y 0 as the sine's exit leaves it.
BobAndMove:
    lda #$FF
    sta z:zTemp00
    lda #$03
    sta z:zTemp01
    inc a:OBJ_3618,x
    lda a:OBJ_3618,x
    jsr VecScaleBySine
    lda z:zProduct2
    clc
    adc a:OBJ_35C0,x
    sta a:OBJ_Y_LO,x
    cmp #$B0
    bcs BobAndMove_at0
    lda a:OBJ_X_LO,x
    clc
    adc a:ScrollXLo
    cmp #$80
    bcc BobAndMove_at0
    cmp #$A0
    bcs BobAndMove_at0
    jsr VecNextRandom
    and #$03
    dec a
    clc
    adc a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,x
BobAndMove_at0:
    jmp VecApplyVelocity

; @routine 09ED:0C3 SetSwaySpeed
; @run 71:C0C3
; WHAT IT DOES: Sets the object's X speed from a word of a table at $C0DE, picked by OBJ_3510 times 4 plus life timer times 2.
; HOW: Two shifts, an add with carry, two table reads (bank 71).
; WAITS: no.
; Entry: X object. Exit: A the high byte, Y the index, N and Z from A, C and V from the add.
SetSwaySpeed:
    lda a:OBJ_3510,x
    asl a
    asl a
    sta z:zTemp10
    lda a:OBJ_34B8,x
    asl a
    adc z:zTemp10
    tay
    lda a:$C0DE,y
    sta a:OBJ_VX_LO,x
    lda a:$C0DF,y
    sta a:OBJ_VX_HI,x
    rts
