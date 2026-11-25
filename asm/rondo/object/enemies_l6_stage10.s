; @routine 10CB:28D LoadThenStartTwoScripts
; @run 70:B28D
; WHAT IT DOES: Loads the object's record, puts it at Y $FFC0 and starts the scripts $82 in slot 5 and $AF in slot 8.
; HOW: LoadObjectRecord, StartScriptSlotKind1 twice.
; WAITS: no.
; Entry: X object. Exit: A and Y the last script, N and Z from the object index.
LoadThenStartTwoScripts:
    jsr VecLoadObjectRecord
    lda #$FF
    sta a:OBJ_Y_HI,x
    lda #$C0
    sta a:OBJ_Y_LO,x
    phx
    lda #$82
    ldx #$05
    jsr VecStartScriptSlotKind1
    lda #$AF
    ldx #$08
    jsr VecStartScriptSlotKind1
    plx
    rts

; @routine 10CB:2CB ClearSlot4EUnlessCamera18
; @run 70:B2CB
; WHAT IT DOES: Unless the camera mode is $18, clears the object in slot $4E, sets OBJ_3510 to $40 and goes on to the next OBJ_2800.
; HOW: A compare, ClearObjectInY, a store, an INC.
; WAITS: no.
; Entry: X object. Exit: the compare, or the new OBJ_2800's N and Z (A $40).
ClearSlot4EUnlessCamera18:
    lda a:CameraMode
    cmp #$18
    beq ClearSlot4EUnlessCamera18_at0
    ldy #$4E
    jsr VecClearObjectInY
    lda #$40
    sta a:OBJ_3510,x
    inc a:OBJ_2800,x
ClearSlot4EUnlessCamera18_at0:
    rts

; @routine 10CB:2E0 CountThenSoleScriptAndSound
; @run 70:B2E0
; WHAT IT DOES: Counts OBJ_3510 down; at zero it starts the sole script 0 in slot 3, sends sound $E4, sets its flags to $81, OBJ_3510 to $60 and zTemp $9E to 4 and goes on to the next OBJ_2800.
; HOW: DEC, StartSoleScriptSlot, SoundCommand, stores, an INC.
; WAITS: no.
; Entry: X object. Exit: see the wrapper.
CountThenSoleScriptAndSound:
    dec a:OBJ_3510,x
    bne CountThenSoleScriptAndSound_at0
    phx
    cla
    ldx #$03
    jsr VecStartSoleScriptSlot
    plx
    lda #$E4
    jsr VecSoundCommand
    lda #$81
    sta a:OBJ_FLAGS_2F38,x
    lda #$60
    sta a:OBJ_3510,x
    lda #$04
    sta z:zRasterStage
    inc a:OBJ_2800,x
CountThenSoleScriptAndSound_at0:
    rts

; @routine 10CB:3BE CountScriptsThenStepList
; @run 70:B3BE
; WHAT IT DOES: Runs StartScriptWhenCountInTable, then steps the animation list at $B3D2; when it ends the object goes to the next OBJ_2800.
; HOW: StartScriptWhenCountInTable, StepObjectFrames, an INC.
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: the step, or the new OBJ_2800's N and Z when the list ended.
CountScriptsThenStepList:
    jsr StartScriptWhenCountInTable
    lda #$D2
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne CountScriptsThenStepList_at0
    inc a:OBJ_2800,x
CountScriptsThenStepList_at0:
    rts

; @routine 10CB:3ED StartScriptWhenCountInTable
; @run 70:B3ED
; WHAT IT DOES: While OBJ_3568 is not negative it counts it up and looks its old value up in the eight-byte table at $B40D; when found at index n it starts script $98 plus n in slot 5.
; HOW: A search loop, StartScriptSlotKind1.
; WAITS: no.
; Entry: X object. Exit: the compare, or the script start's registers.
StartScriptWhenCountInTable:
    lda a:OBJ_3568,x
    bmi StartScriptWhenCountInTable_at0
    inc a:OBJ_3568,x
    cly
StartScriptWhenCountInTable_at2:
    cmp a:$B40D,y
    beq StartScriptWhenCountInTable_at1
    iny
    cpy #$08
    bcc StartScriptWhenCountInTable_at2
StartScriptWhenCountInTable_at0:
    rts
StartScriptWhenCountInTable_at1:
    tya
    clc
    adc #$98
    phx
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    rts

; @routine 10CB:415 RiseThenPlaceWithSound
; @run 70:B415
; WHAT IT DOES: Moves the object along Y by its speed and takes $0010 off its Y speed. When its Y high byte is $FE it sends sound $E7, sets its flags to $81, frame $1D, puts it at X $0000, Y $FFE0, gives it X speed $0400 and Y speed $0600 (high bytes) and goes on to the next OBJ_2800.
; HOW: ApplyVelocityY, a 16-bit subtract, SoundCommand, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept for the sound's push. Exit: see the wrapper.
RiseThenPlaceWithSound:
    jsr VecApplyVelocityY
    lda a:OBJ_VY_LO,x
    sec
    sbc #$10
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    lda a:OBJ_Y_HI,x
    cmp #$FE
    bne RiseThenPlaceWithSound_at0
    lda #$E7
    jsr VecSoundCommand
    lda #$80
    sta a:OBJ_FLAGS_2F38,x
    lda #$1D
    sta a:OBJ_2A68,x
    stz a:OBJ_X_HI,x
    lda #$00
    sta a:OBJ_X_LO,x
    lda #$FF
    sta a:OBJ_Y_HI,x
    lda #$E0
    sta a:OBJ_Y_LO,x
    lda #$06
    sta a:OBJ_VY_HI,x
    lda #$04
    sta a:OBJ_VX_HI,x
    inc a:OBJ_2800,x
RiseThenPlaceWithSound_at0:
    rts

; @routine 10CB:45F RiseThenCueMusic
; @run 70:B45F
; WHAT IT DOES: Moves the object along Y and takes $0030 off its Y speed. Once the speed's high byte is negative it steps the animation list at $B4AA; when that ends it sets bit 4 of zField8E, sends sound $FF, clears bits 3 and 4, plays the music again (CdPlayAgain), sets bit 2, clears zTemp $9E and OBJ_3250 to 2, clears its animation step and timer, goes on to the next state and clears OBJ_2800.
; HOW: ApplyVelocity, a 16-bit subtract, StepObjectFrames, SoundCommand, CdPlayAgain, stores.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
RiseThenCueMusic:
    jsr VecApplyVelocity
    lda a:OBJ_VY_LO,x
    sec
    sbc #$30
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    bpl RiseThenCueMusic_at0
    lda #$AA
    sta z:zTemp10
    lda #$B4
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne RiseThenCueMusic_at0
    smb4 z:zField8E
    lda #$FF
    jsr VecSoundCommand
    rmb3 z:zField8E
    rmb4 z:zField8E
    jsr VecCdPlayAgain
    lda z:zField8E
    ora #$04
    sta z:zField8E
    stz z:zRasterStage
    lda #$02
    sta a:OBJ_3250,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    inc a:OBJ_27A8,x
    stz a:OBJ_2800,x
RiseThenCueMusic_at0:
    rts

; @routine 10CB:5AB SpawnInFreeSlotEvery30
; @run 70:B5AB
; WHAT IT DOES: Counts OBJ_37D0 down; at zero it sets it to $1E, looks for an empty slot from $18 to $1F, clears it and starts a kind $26 object there at this object's place.
; HOW: DEC, a search loop, ClearObjectInY, StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: the count, the compare, or the start's registers.
SpawnInFreeSlotEvery30:
    lda a:OBJ_37D0,x
    beq SpawnInFreeSlotEvery30_at0
    dec a:OBJ_37D0,x
    rts
SpawnInFreeSlotEvery30_at0:
    lda #$1E
    sta a:OBJ_37D0,x
    ldy #$18
    lda a:OBJ_2750,y
    beq SpawnInFreeSlotEvery30_at1
    iny
    cpy #$20
    bcc RiseUntilAboveGround
    rts
SpawnInFreeSlotEvery30_at1:
    jsr VecClearObjectInY
    lda #$26
    jsr VecStartObjectAtPlace
    rts

; @routine 10CB:680 StepListB697ThenStopY
; @run 70:B680
; WHAT IT DOES: Steps the animation list at $B697; when it ends, the object's Y speed is cleared and it goes to the next OBJ_2800.
; HOW: StepObjectFrames, then stores and an increment.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step, or the new OBJ_2800's N and Z when the list ended.
StepListB697ThenStopY:
    lda #$97
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListB697ThenStopY_at0
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_2800,x
StepListB697ThenStopY_at0:
    rts

; @routine 10CB:6B6 RiseThenPlaceAndDrop
; @run 70:B6B6
; WHAT IT DOES: Moves the object along Y by its speed and takes $0018 off its Y speed. When its Y high byte is $FE it puts the object at X $0070, Y $FFC0, gives it X speed $0030 and Y speed $0400, sets its flags to $81 and zTemp $9E to 4 and goes on to the next OBJ_2800.
; HOW: ApplyVelocityY, a 16-bit subtract, stores, an INC.
; WAITS: no.
; Entry: X object. Exit: the compare, or the new OBJ_2800's N and Z (A 4).
RiseThenPlaceAndDrop:
    jsr VecApplyVelocityY
    lda a:OBJ_VY_LO,x
    sec
    sbc #$18
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    lda a:OBJ_Y_HI,x
    cmp #$FE
    bne RiseThenPlaceAndDrop_at0
    lda #$81
    sta a:OBJ_FLAGS_2F38,x
    lda #$04
    sta z:zRasterStage
    stz a:OBJ_X_HI,x
    lda #$70
    sta a:OBJ_X_LO,x
    lda #$FF
    sta a:OBJ_Y_HI,x
    lda #$C0
    sta a:OBJ_Y_LO,x
    lda #$00
    sta a:OBJ_VX_HI,x
    lda #$30
    sta a:OBJ_VX_LO,x
    lda #$04
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_2800,x
RiseThenPlaceAndDrop_at0:
    rts

; @routine 10CB:702 StepListB728ThenRiseSlowly
; @run 70:B702
; WHAT IT DOES: Steps the animation list at $B728; while it runs the object moves and its Y speed loses $0010 a frame; when it ends the object goes to the next OBJ_2800.
; HOW: StepObjectFrames, ApplyVelocity, a 16-bit subtract, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the subtract's, or the new OBJ_2800's N and Z.
StepListB728ThenRiseSlowly:
    lda #$28
    sta z:zTemp10
    lda #$B7
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq StepListB728ThenRiseSlowly_at0
    jsr VecApplyVelocity
    lda a:OBJ_VY_LO,x
    sec
    sbc #$10
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    rts
StepListB728ThenRiseSlowly_at0:
    inc a:OBJ_2800,x
    rts

; @routine 10CC:74F LoadThenFallAndFlash
; @run 70:BF4F
; WHAT IT DOES: In state 0 loads the object's record and sets flags 1, frame $59, X speed $00C0, Y speed $0030 and OBJ_3510 $34. Later it adds $0008 to its Y speed, moves, sets its flash flags to $10 while OBJ_3510 is under $10, counts OBJ_3510 down and clears the object at zero.
; HOW: LoadObjectRecord, a 16-bit add, ApplyVelocity, DEC, ClearObject.
; WAITS: no.
; Entry: X object. Exit: the record's registers and A $34 in state 0; else the count.
LoadThenFallAndFlash:
    lda a:OBJ_27A8,x
    bne StartDriftingPiece
    jsr VecLoadObjectRecord
    lda #$01
    sta a:OBJ_FLAGS_2F38,x
    lda #$59
    sta a:OBJ_2A68,x
    lda #$00
    sta a:OBJ_VX_HI,x
    lda #$C0
    sta a:OBJ_VX_LO,x
    lda #$00
    sta a:OBJ_VY_HI,x
    lda #$30
    sta a:OBJ_VY_LO,x
    lda #$34
    sta a:OBJ_3510,x
    rts
    lda a:OBJ_VY_LO,x
    clc
    adc #$08
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda a:OBJ_3510,x
    cmp #$10
    bcs LoadThenFallAndFlash_at0
    lda #$10
    sta a:OBJ_2B18,x
LoadThenFallAndFlash_at0:
    dec a:OBJ_3510,x
    bne LoadThenFallAndFlash_at1
    jmp VecClearObject
LoadThenFallAndFlash_at1:
    rts

; @routine 10CD:0C6 StepListC103ThenPickAim
; @run 71:C0C6
; WHAT IT DOES: Steps the animation list at $C103; when it ends the object picks OBJ_34B8: half the time the angle to the player, otherwise a random $00 to $7F plus the byte the table at $C088 gives for the player's side; it then clears OBJ_3510 and OBJ_3568, sets OBJ_3250 to 2 and goes on to the next state.
; HOW: StepObjectFrames, NextRandom, SideOfPlayer, AngleToPlayer, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step, or the new state's N and Z (A 2) when the list ended.
StepListC103ThenPickAim:
    lda #$03
    sta z:zTemp10
    lda #$C1
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListC103ThenPickAim_at0
    jsr VecNextRandom
    and #$01
    beq StepListC103ThenPickAim_at1
    jsr VecSideOfPlayer
    and #$01
    tay
    jsr VecNextRandom
    and #$7F
    clc
    adc a:$C088,y
    sta a:OBJ_34B8,x
    bra StepListC103ThenPickAim_at2
StepListC103ThenPickAim_at1:
    jsr VecAngleToPlayer
    sta a:OBJ_34B8,x
StepListC103ThenPickAim_at2:
    stz a:OBJ_3510,x
    stz a:OBJ_3568,x
    lda #$02
    sta a:OBJ_3250,x
    inc a:OBJ_27A8,x
StepListC103ThenPickAim_at0:
    rts

; @routine 10CD:126 StepListC170ThenMoveAlongAngle
; @run 71:C126
; WHAT IT DOES: Steps the animation list at $C170, sets the object's speed from its angle OBJ_34B8 with the length OBJ_3510:OBJ_3568, moves it and adds $000C to the length. When the object in slot $10 is in state 6 or more, or this object's OBJ_31A0 has bit 7 set, it clears its animation step and timer and goes on to the next state (and writes 0 to the byte at $0002 plus its slot).
; HOW: StepObjectFrames, SetVelocityFromAngle, ApplyVelocity, a 16-bit add, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the last test, or the new state's N and Z.
StepListC170ThenMoveAlongAngle:
    lda #$70
    sta z:zTemp10
    lda #$C1
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_3568,x
    sta z:zTemp00
    lda a:OBJ_3510,x
    sta z:zTemp01
    lda a:OBJ_34B8,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_3568,x
    clc
    adc #$0C
    sta a:OBJ_3568,x
    lda a:OBJ_3510,x
    adc #$00
    sta a:OBJ_3510,x
    lda a:$27B8
    cmp #$06
    bcs WalkAndAttackB
    lda a:OBJ_31A0,x
    and #$80
    beq StepListC170ThenMoveAlongAngle_at0
    stz a:VDC_DATA_LO,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    inc a:OBJ_27A8,x
StepListC170ThenMoveAlongAngle_at0:
    rts

; @routine 10CD:181 StepListC192ThenClear
; @run 71:C181
; WHAT IT DOES: Steps the animation list at $C192; when it ends the object is cleared.
; HOW: StepObjectFrames, a tail jump into ClearObject.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the step.
StepListC192ThenClear:
    lda #$92
    sta z:zTemp10
    lda #$C1
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne WaitThenState2B
    jmp VecClearObject
    rts
; @routine 10CD:334 StartFirstObjectWhenCamera2
; @run 71:C334
; WHAT IT DOES: When the level's Y high byte is 2, the scroll Y low is under $30 and the first object's state and OBJ_2800 are both 0, it sets that object's OBJ_3510 to $40 and OBJ_3568 to $10 and this object goes to the next state.
; HOW: Compares and stores, an INC.
; WAITS: no.
; Entry: X object. Exit: the last compare or load, or the new state's N and Z (A $10).
StartFirstObjectWhenCamera2:
    lda a:ScrollYHiScreen
    cmp #$02
    bne StartFirstObjectWhenCamera2_at0
    lda a:ScrollYLo
    cmp #$30
    bcs StartFirstObjectWhenCamera2_at0
    lda a:OBJ_27A8
    ora a:OBJ_2800
    bne StartFirstObjectWhenCamera2_at0
    lda #$40
    sta a:OBJ_3510
    lda #$10
    sta a:OBJ_3568
    inc a:OBJ_27A8,x
StartFirstObjectWhenCamera2_at0:
    rts
