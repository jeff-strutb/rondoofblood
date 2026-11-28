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
    inc a:OBJ_STATE,x
NextStateOnScrollY_at0:
    rts

; @routine 088A:32E DrawStepCountThenClear
; @run 70:AB2E
; WHAT IT DOES: On every 32nd frame of the wave counter it draws the tile script of group 0 numbered OBJ_START_ARG, starts a kind $25 object in a free slot from $30 at its place and moves; it counts OBJ_START_ARG up and clears the object when it reaches 4.
; HOW: DrawTileScript, FindFreeSlotFrom30, StartObjectAtPlace, ApplyVelocity, an INC, ClearObject.
; WAITS: no.
; Entry: X object. Exit: the compare, or as ClearObject.
DrawStepCountThenClear:
    lda z:zWaveCounter
    and #$1F
    bne DrawStepCountThenClear_at0
    lda a:OBJ_START_ARG,x
    clc
    adc #$00
    cly
    jsr VecDrawTileScript
    jsr VecFindFreeSlotFrom30
    bcc DrawStepCountThenClear_at1
    lda #$25
    jsr VecStartObjectAtPlace
DrawStepCountThenClear_at1:
    jsr VecApplyVelocity
    inc a:OBJ_START_ARG,x
    lda a:OBJ_START_ARG,x
    cmp #$04
    bcc DrawStepCountThenClear_at0
    jmp VecClearObject
DrawStepCountThenClear_at0:
    rts

; @routine 088A:359 StartThenFallAlongList
; @run 70:AB59
; WHAT IT DOES: In state 0 loads the object's record. Later it steps the animation list at $AB72, applies gravity without a limit and moves the object.
; HOW: LoadObjectRecord, StepObjectFrames, ApplyGravityUnlimited, ApplyVelocity.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: state 0: as LoadObjectRecord; else as ApplyVelocity.
StartThenFallAlongList:
    lda a:OBJ_STATE,x
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
; WHAT IT DOES: Sets the object's frame to $25 when its X low is under $C0, else 0; once the scroll's Y high byte is not zero it sets OBJ_WORK_D to $C0 and goes on to the next state.
; HOW: Stores, compares, an INC.
; WAITS: no.
; Entry: X object. Exit: the scroll's load, or the new state's N and Z (A $C0).
FrameBySideThenTimerOnScroll:
    stz a:OBJ_FRAME,x
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcs FrameBySideThenTimerOnScroll_at0
    lda #$25
    sta a:OBJ_FRAME,x
FrameBySideThenTimerOnScroll_at0:
    lda a:ScrollYHi
    beq FrameBySideThenTimerOnScroll_at1
    lda #$C0
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
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
    inc a:OBJ_STATE,x
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
; WHAT IT DOES: Clears the objects in slots $16 to $18, starts a kind $28 object in each at the place of the object in slot $3E, gives each sprite set 7, its number (0 to 2) as OBJ_WORK_D and the frame number plus $47, and then clears the object in slot $3E.
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
    sta a:OBJ_SPRITE_SET,y
    lda z:zTemp00
    sta a:OBJ_WORK_D,y
    clc
    adc #$47
    sta a:OBJ_FRAME,y
    inc z:zTemp00
    iny
    cpy #$19
    bcc StartThreeNumberedObjects_at0
    jsr VecClearObject
    plx
    rts

; @routine 088A:4FC LoadThenSpeedFromTable
; @run 70:ACFC
; WHAT IT DOES: Loads the object's record, takes its X speed from the two-byte entries of the table at $AD21 by OBJ_WORK_D, its timer from the table at $AD29 and its frame as OBJ_WORK_D plus $47.
; HOW: LoadObjectRecord, table reads (bank 70), an add.
; WAITS: no.
; Entry: X object. Exit: A the frame, N, Z, C and V from the add; Y OBJ_WORK_D.
LoadThenSpeedFromTable:
    jsr VecLoadObjectRecord
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:$AD22,y
    sta a:OBJ_VX_HI,x
    lda a:$AD21,y
    sta a:OBJ_VX_LO,x
    ldy a:OBJ_WORK_D,x
    lda a:$AD29,y
    sta a:OBJ_WORK_E,x
    tya
    clc
    adc #$47
    sta a:OBJ_FRAME,x
    rts

; @routine 088A:52D CountThenNextStateF3510
; @run 70:AD2D
; WHAT IT DOES: Counts OBJ_WORK_E down; at zero it sets it to $30 and goes on to the next state.
; HOW: DEC, a store, INC.
; WAITS: no.
; Entry: X object. Exit: the count, or the new state's N and Z (A $30).
CountThenNextStateF3510:
    dec a:OBJ_WORK_E,x
    bne CountThenNextStateF3510_at0
    lda #$30
    sta a:OBJ_WORK_E,x
    inc a:OBJ_STATE,x
CountThenNextStateF3510_at0:
    rts

; @routine 088A:53B FallAndBlinkThenClear
; @run 70:AD3B
; WHAT IT DOES: Adds $0008 to the object's Y speed and moves the object. Its frame is 0, or on every other frame of the wave counter OBJ_WORK_D plus $47. Counts OBJ_WORK_E down and clears the object at zero.
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
    stz a:OBJ_FRAME,x
    lda z:zWaveCounter
    and #$01
    beq FallAndBlinkThenClear_at0
    lda a:OBJ_WORK_D,x
    clc
    adc #$47
    sta a:OBJ_FRAME,x
FallAndBlinkThenClear_at0:
    dec a:OBJ_WORK_E,x
    bne FallAndBlinkThenClear_at1
    jsr VecClearObject
FallAndBlinkThenClear_at1:
    rts

; @routine 088A:63F SetHitBoxFromFrameTable
; @run 70:AE3F
; WHAT IT DOES: Takes the object's hit box (OBJ_HITBOX_DX, OBJ_HITBOX_DY, OBJ_HITBOX_HALF_W, OBJ_HITBOX_HALF_H) from the four-byte row of the table at $B8F7 that its frame less one names.
; HOW: Table reads (bank 70).
; WAITS: no.
; Entry: X object. Exit: A the last byte read, Y the row offset.
SetHitBoxFromFrameTable:
    lda a:OBJ_FRAME,x
    sec
    sbc #$01
    asl a
    asl a
    tay
    lda a:$B8F7,y
    sta a:OBJ_HITBOX_DX,x
    lda a:$B8F8,y
    sta a:OBJ_HITBOX_DY,x
    lda a:$B8F9,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:SoundMusicTable,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 088A:699 LoadThenStartFiveFromTable
; @run 70:AE99
; WHAT IT DOES: Loads the object's record, clears OBJ_WORK_L and its Y speed, sets OBJ_WORK_D to $30 and bit 2 of zField8E, copies its X and Y low bytes into all 64 entries of the two tables at $BB66 and $BBA6, then clears the objects in slots $11 to $15 and starts in each an object whose kind the table at $AEDA gives, at this object's place.
; HOW: LoadObjectRecord, two loops, ClearObjectInY, StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: the last compare of Y with $16.
LoadThenStartFiveFromTable:
    jsr VecLoadObjectRecord
    stz a:OBJ_WORK_L,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$30
    sta a:OBJ_WORK_D,x
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
; WHAT IT DOES: Sets the object's frame (OBJ_FRAME) to $0C and counts OBJ_WORK_F down; at zero it sets it to $10 and goes on to the next state.
; HOW: A store, DEC, a store, INC.
; WAITS: no.
; Entry: X object. Exit: the count, or the new state's N and Z (A $10).
SetFrameThenCountToNext:
    lda #$0C
    sta a:OBJ_FRAME,x
    dec a:OBJ_WORK_F,x
    bne SetFrameThenCountToNext_at0
    lda #$10
    sta a:OBJ_WORK_F,x
    inc a:OBJ_STATE,x
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

; @routine 088B:044 CheckPlayerThenStepListIndex08
; @run 70:B044
; WHAT IT DOES: With OBJ_SUBSTATE 0 it goes on to OBJ_SUBSTATE 1 and, when the player is $50 or more away along X, goes to state 6. Later it steps the animation list StepListByIndex088A gives for index 8 and when that ends clears its animation step and timer and goes to state 5. (The state 6 arm ran in no script; it is written from the decoded bytes.)
; HOW: DistanceXToPlayer, StepListByIndex088A, stores.
; WAITS: no.
; Entry: X object. Exit: the compare, the step's, or A 5 / 6.
CheckPlayerThenStepListIndex08:
    lda a:OBJ_SUBSTATE,x
    bne CheckPlayerThenStepListIndex08_at0
    inc a:OBJ_SUBSTATE,x
    jsr VecDistanceXToPlayer
    cmp #$50
    bcc CheckPlayerThenStepListIndex08_at1
    lda #$06
    sta a:OBJ_STATE,x
CheckPlayerThenStepListIndex08_at1:
    rts
CheckPlayerThenStepListIndex08_at0:
    ldy #$08
    jsr StepListByIndex088A
    bne CheckPlayerThenStepListIndex08_at2
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_STATE,x
CheckPlayerThenStepListIndex08_at2:
    rts

; @routine 088B:084 StartFallWithFrame21
; @run 70:B084
; WHAT IT DOES: Zeroes the object's X speed, sets its Y speed high to $FD and its frame to $21 and goes on to the next OBJ_SUBSTATE.
; HOW: Stores and an INC.
; WAITS: no.
; Entry: X object. Exit: A $21, N and Z from the new OBJ_SUBSTATE.
StartFallWithFrame21:
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    lda #$21
    sta a:OBJ_FRAME,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 088B:098 FallThenSoundAtPeak
; @run 70:B098
; WHAT IT DOES: Applies gravity and moves the object; once its Y speed high byte is not negative it sends sound $9A and goes on to the next OBJ_SUBSTATE.
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
    inc a:OBJ_SUBSTATE,x
FallThenSoundAtPeak_at0:
    rts

; @routine 088B:0AC StepListIndex0AThenSpawn
; @run 70:B0AC
; WHAT IT DOES: Steps the animation list StepListByIndex088A gives for index $0A. When the list ends it clears its animation step and timer and goes to state 2. While it runs, on a changed frame $24 it starts a kind $23 object in a free slot from $30 at its place.
; HOW: StepListByIndex088A, FindFreeSlotFrom30, StartObjectAtPlace, stores.
; WAITS: no.
; Entry: X object. Exit: the step's, the compare, or the start's registers.
StepListIndex0AThenSpawn:
    ldy #$0A
    jsr StepListByIndex088A
    beq StepListIndex0AThenSpawn_at0
    bcc StepListIndex0AThenSpawn_at1
    lda a:OBJ_FRAME,x
    cmp #$24
    bne StepListIndex0AThenSpawn_at1
    jsr VecFindFreeSlotFrom30
    bcc StepListIndex0AThenSpawn_at0
    lda #$23
    jsr VecStartObjectAtPlace
    rts
StepListIndex0AThenSpawn_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$02
    sta a:OBJ_STATE,x
StepListIndex0AThenSpawn_at1:
    rts

; @routine 088B:0E8 SpeedFromRandomRow
; @run 70:B0E8
; WHAT IT DOES: Takes the object's two speeds (four bytes: X low and high, Y low and high) from the row of the table at $B109 that a random number from 0 to 7 picks.
; HOW: NextRandom, table reads (bank 70).
; WAITS: no.
; Entry: X object. Exit: A the last byte read, Y the row offset, C clear.
SpeedFromRandomRow:
    jsr VecNextRandom
    and #$07
    asl a
    asl a
    tay
    lda a:$B10A,y
    sta a:OBJ_VX_HI,x
    lda a:$B109,y
    sta a:OBJ_VX_LO,x
    lda a:$B10C,y
    sta a:OBJ_VY_HI,x
    lda a:$B10B,y
    sta a:OBJ_VY_LO,x
    rts

; @routine 088B:21D TurnAroundInState9
; @run 70:B21D
; WHAT IT DOES: Sets the object's state to 9 and reverses its X speed (a tail jump into NegateVelocityX).
; HOW: A store, NegateVelocityX.
; WAITS: no.
; Entry: X object. Exit: as NegateVelocityX.
TurnAroundInState9:
    lda #$09
    sta a:OBJ_STATE,x
    jmp VecNegateVelocityX

; @routine 088B:225 FallBackInState9
; @run 70:B225
; WHAT IT DOES: Sets the object's state to 9 and reverses its Y speed (a tail jump into NegateVelocityY).
; HOW: A store, NegateVelocityY.
; WAITS: no.
; Entry: X object. Exit: as NegateVelocityY.
FallBackInState9:
    lda #$09
    sta a:OBJ_STATE,x
    jmp VecNegateVelocityY

; @routine 088B:240 SpeedBySideOfScreen
; @run 70:B240
; WHAT IT DOES: Sets OBJ_WORK_E to 2, takes the X speed high from the two-byte table at $B259 by bit 7 of the object's X low and clears the X speed low, then moves the object.
; HOW: Two ROLs, a table read (bank 70), a tail jump into ApplyVelocity.
; WAITS: no.
; Entry: X object, C passed into the first ROL. Exit: as ApplyVelocity, Y the table index.
SpeedBySideOfScreen:
    lda #$02
    sta a:OBJ_WORK_E,x
    lda a:OBJ_X_LO,x
    rol a
    rol a
    and #$01
    tay
    lda a:$B259,y
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    jmp VecApplyVelocity

; @routine 088B:25B NearPlayerThenPlaceOrState11
; @run 70:B25B
; WHAT IT DOES: When the player is within $60 along X and $A0 along Y the object goes on as SpeedBySideOfScreen; otherwise it goes to state $0B.
; HOW: DistanceXToPlayer, DistanceYToPlayer, a tail jump into SpeedBySideOfScreen, a store.
; WAITS: no.
; Entry: X object. Exit: as SpeedBySideOfScreen, or A $0B.
NearPlayerThenPlaceOrState11:
    jsr VecDistanceXToPlayer
    cmp #$60
    bcs NearPlayerThenPlaceOrState11_at0
    jsr VecDistanceYToPlayer
    cmp #$A0
    bcc SpeedBySideOfScreen
NearPlayerThenPlaceOrState11_at0:
    lda #$0B
    sta a:OBJ_STATE,x
    rts

; @routine 088B:285 SoundThenAimRandomLength
; @run 70:B285
; WHAT IT DOES: Sends sound $2B, then sets the object's speed along the angle to the player with a length from a table at $B2A7 by a random 0 to 3, and goes on to the next OBJ_SUBSTATE.
; HOW: SoundCommand, AngleToPlayer, NextRandom, SetVelocityFromAngle, an INC.
; WAITS: no.
; Entry: X object, Y kept for the sound's push. Exit: see the wrapper.
SoundThenAimRandomLength:
    lda #$2B
    jsr VecSoundCommand
    jsr VecAngleToPlayer
    pha
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:$B2A7,y
    sta z:zTemp00
    lda a:$B2A8,y
    sta z:zTemp01
    pla
    jsr VecSetVelocityFromAngle
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 088B:6C5 ClearFrame
; @run 70:B6C5
; WHAT IT DOES: Sets the object's frame to 0.
; HOW: A store.
; WAITS: no.
; Entry: X object. Exit: all kept.
ClearFrame:
    stz a:OBJ_FRAME,x
    rts
; @routine 088B:6C9 PlaceFromTableCopySlot10
; @run 70:B6C9
; WHAT IT DOES: Puts the object at the X and Y low bytes the tables at $BB66 and $BBA6 give for the row the table at $B736 names by OBJ_WORK_E, copies the sprite set, frame and facing of the object in slot $10 and sets its flash flags to $10.
; HOW: Table reads (bank 70), copies.
; WAITS: no.
; Entry: X object. Exit: A $10, N and Z from it; Y the row.
PlaceFromTableCopySlot10:
    ldy a:OBJ_WORK_E,x
    lda a:$B736,y
    tay
    lda a:RichterWallPushBits+1,y
    sta a:OBJ_X_LO,x
    lda a:$BBA6,y
    sta a:OBJ_Y_LO,x
    lda a:$2A20
    sta a:OBJ_SPRITE_SET,x
    lda a:$2A78
    sta a:OBJ_FRAME,x
    lda a:$2AD0
    sta a:OBJ_FACING_LEFT,x
    lda #$10
    sta a:OBJ_SPRITE_ATTR,x
    rts

; @routine 088B:705 PlaceFromTableThenStepList
; @run 70:B705
; WHAT IT DOES: Puts the object at the X and Y low bytes the tables at $BB66 and $BBA6 give for the row the table at $B73A names by OBJ_WORK_E, sets sprite set 7, copies the facing of the object in slot $10 and steps the animation list the table at $B73E gives for OBJ_WORK_E.
; HOW: Table reads (bank 70), StepObjectFrames.
; WAITS: no.
; Entry: X object, Y overwritten. Exit: as StepObjectFrames.
PlaceFromTableThenStepList:
    ldy a:OBJ_WORK_E,x
    lda a:$B73A,y
    tay
    lda a:RichterWallPushBits+1,y
    sta a:OBJ_X_LO,x
    lda a:$BBA6,y
    sta a:OBJ_Y_LO,x
    lda #$07
    sta a:OBJ_SPRITE_SET,x
    lda a:$2AD0
    sta a:OBJ_FACING_LEFT,x
    lda a:OBJ_WORK_E,x
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
; WHAT IT DOES: Sets OBJ_WORK_D of the objects 18 to 21 to 1.
; HOW: A loop of Y from 18.
; WAITS: no.
; Entry: nothing. Exit: A 1, Y 22, the compare of Y with 22.
SetTimersOf12To15:
    ldy #$12
SetTimersOf12To15_at0:
    lda #$01
    sta a:OBJ_WORK_D,y
    iny
    cpy #$16
    bcc SetTimersOf12To15_at0
    rts
; @routine 088B:7AF FollowSlot10WithHitBox
; @run 70:B7AF
; WHAT IT DOES: In state 0 loads the object's record. Later it takes the place, facing and OBJ_HITBOX_ON of the object in slot $10 and a hit box from the four-byte row of the table at $B9DF that slot's frame less one names; with a zero half width and half height it clears OBJ_HITBOX_ON. When the object in slot $10 is no longer of kind $20 this object is cleared.
; HOW: LoadObjectRecord, CopyObjectPlaceBack, table reads (bank 70), ClearObject.
; WAITS: no.
; Entry: X object. Exit: the compare with $20, or as ClearObject.
FollowSlot10WithHitBox:
    lda a:OBJ_STATE,x
    bne FollowSlot10WithHitBox_at0
    jmp VecLoadObjectRecord
FollowSlot10WithHitBox_at0:
    ldy #$10
    jsr VecCopyObjectPlaceBack
    lda a:$2A78
    sec
    sbc #$01
    asl a
    asl a
    tay
    lda a:$B9DF,y
    sta a:OBJ_HITBOX_DX,x
    lda a:$B9E0,y
    sta a:OBJ_HITBOX_DY,x
    lda a:$B9E1,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:$B9E2,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:$2AD0
    sta a:OBJ_FACING_LEFT,x
    lda a:$3260
    sta a:OBJ_HITBOX_ON,x
    lda a:OBJ_HITBOX_HALF_W,x
    ora a:OBJ_HITBOX_HALF_H,x
    bne FollowSlot10WithHitBox_at1
    stz a:OBJ_HITBOX_ON,x
FollowSlot10WithHitBox_at1:
    lda a:$2760
    cmp #$20
    beq FollowSlot10WithHitBox_at2
    jmp VecClearObject
FollowSlot10WithHitBox_at2:
    rts

; @routine 088C:43C StartThenWaitForField83
; @run 70:BC3C
; WHAT IT DOES: In state 0 loads the object's record and sets frame 2 and sprite set 6. In state 1, once zField83 is not zero, it sets the scroll Y high to 1, frame $46, sprite set 7, X low $80, Y low $58 and goes on to the next state.
; HOW: LoadObjectRecord, a DEC, stores, an INC.
; WAITS: no.
; Entry: X object. Exit: the DEC, the load of zField83, or the new state's N and Z (A $58).
StartThenWaitForField83:
    lda a:OBJ_STATE,x
    bne StartThenWaitForField83_at0
    jsr VecLoadObjectRecord
    lda #$02
    sta a:OBJ_FRAME,x
    lda #$06
    sta a:OBJ_SPRITE_SET,x
StartThenWaitForField83_at0:
    dec a
    bne StartThenWaitForField83_at1
    lda z:zField83
    beq StartThenWaitForField83_at1
    lda #$01
    sta a:ScrollYHi
    lda #$46
    sta a:OBJ_FRAME,x
    lda #$07
    sta a:OBJ_SPRITE_SET,x
    lda #$80
    sta a:OBJ_X_LO,x
    lda #$58
    sta a:OBJ_Y_LO,x
    inc a:OBJ_STATE,x
StartThenWaitForField83_at1:
    rts
