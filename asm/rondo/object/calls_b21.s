; @routine 072C:708 FlyPatternUnlessHeld072C
; @run 70:BF08
; WHAT IT DOES: Not hit-tested; while slot $10 allows, flies the pattern for OBJ_WORK_D, cleared below the first screen.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FlyPatternUnlessHeld072CVxHi = $B46E ; vx high
FlyPatternUnlessHeld072CVxLo = $B52E ; vx low
FlyPatternUnlessHeld072CVyHi = $B5EE ; vy high
FlyPatternUnlessHeld072CVyLo = $B6AE ; vy low
FlyPatternUnlessHeld072CPatterns = $BE24 ; speed index
FlyPatternUnlessHeld072C:
    stz a:OBJ_HITBOX_ON,x
    lda a:OBJ_WORK_D+$10
    and #$03
    bne FlyPatternUnlessHeld072C_at0
    lda #$4E
    sta z:zTemp10
    lda #$BF
    sta z:zTemp11
    jsr VecStepObjectFrames
    ldy a:OBJ_WORK_D,x
    lda a:FlyPatternUnlessHeld072CPatterns,y
    clc
    adc #$04
    tay
    lda a:FlyPatternUnlessHeld072CVxHi,y
    sta a:OBJ_VX_HI,x
    lda a:FlyPatternUnlessHeld072CVxLo,y
    sta a:OBJ_VX_LO,x
    lda a:FlyPatternUnlessHeld072CVyHi,y
    sta a:OBJ_VY_HI,x
    lda a:FlyPatternUnlessHeld072CVyLo,y
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocity
    jsr BurstAtHeightE0
    lda a:OBJ_Y_HI,x
    beq FlyPatternUnlessHeld072C_at0
    jmp VecClearObject
FlyPatternUnlessHeld072C_at0:
    rts

; @routine 088A:56A RunAndRecordPath088A
; @run 70:AD6A
; WHAT IT DOES: Runs the overlay's checks and state, records the place in two 64-byte paths, and starts the list channel for the frame.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RunAndRecordPath088ABeforeX = $BB65 ; one before the X path
RunAndRecordPath088APathX = $BB66 ; X path
RunAndRecordPath088ABeforeY = $BBA5 ; one before the Y path
RunAndRecordPath088APathY = $BBA6 ; Y path
RunAndRecordPath088A:
    inc a:OBJ_WORK_J,x
    jsr FaceThePlayerInSomeStatesC
    jsr ToState11WhenHit088A
    jsr StateSixteenWhenHit088A
    jsr SetHitBoxFromFrameTable
    jsr JumpByStateTable088A661
    ldy #$3F
RunAndRecordPath088A_at0:
    lda a:RunAndRecordPath088ABeforeX,y
    sta a:RunAndRecordPath088ABeforeX+1,y
    lda a:RunAndRecordPath088ABeforeY,y
    sta a:RunAndRecordPath088ABeforeY+1,y
    dey
    bne RunAndRecordPath088A_at0
    lda a:OBJ_X_LO,x
    sta a:RunAndRecordPath088ABeforeX+1
    lda a:OBJ_Y_LO,x
    sta a:RunAndRecordPath088ABeforeY+1
    lda a:OBJ_WORK_L,x
    bne RunAndRecordPath088A_at1
    phx
    lda a:OBJ_FRAME,x
    sec
    sbc #$01
    clc
    adc #$85
    ldx #$05
    jsr VecStartListChannelKind2
    plx
RunAndRecordPath088A_at1:
    rts

; @routine 09EB:239 StartParts09EB
; @run 70:B239
; WHAT IT DOES: Starts five kind $2B and eight kind $2C parts, places itself below the screen, waits $E0, state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartParts09EB:
    ldy #$20
StartParts09EB_at0:
    jsr VecClearObjectInY
    lda #$2B
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    iny
    cpy #$25
    bcc StartParts09EB_at0
StartParts09EB_at1:
    jsr VecClearObjectInY
    lda #$2C
    jsr VecStartObjectAtPlace
    iny
    cpy #$2D
    bcc StartParts09EB_at1
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_X_HI,x
    lda #$C0
    sec
    sbc a:ScrollXLo
    sta a:OBJ_X_LO,x
    lda #$01
    sta a:OBJ_Y_HI,x
    lda #$80
    sta a:OBJ_Y_LO,x
    lda #$E0
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
    rts

; @routine 101B:483 SplitInTwo101B
; @run 70:B483
; WHAT IT DOES: Replaced by two kind $2D halves, each with its argument, wait and speeds.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SplitInTwo101BArgs = $B4C8 ; start argument of each
SplitInTwo101BWaits = $B4CA ; OBJ_WORK_D
SplitInTwo101BVxHi = $B4CC ; vx high
SplitInTwo101BVxLo = $B4CE ; vx low
SplitInTwo101BVyHi = $B4D0 ; vy high
SplitInTwo101BVyLo = $B4D2 ; vy low
SplitInTwo101B:
    cly
SplitInTwo101B_at1:
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc SplitInTwo101B_at0
    lda #$2D
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    phx
    ldx z:zBurstCount
    lda a:SplitInTwo101BArgs,x
    sta a:OBJ_START_ARG,y
    lda a:SplitInTwo101BWaits,x
    sta a:OBJ_WORK_D,y
    lda a:SplitInTwo101BVxHi,x
    sta a:OBJ_VX_HI,y
    lda a:SplitInTwo101BVxLo,x
    sta a:OBJ_VX_LO,y
    lda a:SplitInTwo101BVyHi,x
    sta a:OBJ_VY_HI,y
    lda a:SplitInTwo101BVyLo,x
    sta a:OBJ_VY_LO,y
    plx
    ldy z:zBurstCount
    iny
    cpy #$02
    bcc SplitInTwo101B_at1
SplitInTwo101B_at0:
    jmp VecClearObject

; @routine 101B:783 WakeBeforeLevelX101B
; @run 70:B783
; WHAT IT DOES: When the player's level X is in $200-$2ED: wakes, hit-tested, leaping by the side the player is on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WakeBeforeLevelX101BSpeeds = $B7C8 ; vx by side
WakeBeforeLevelX101B:
    lda a:OBJ_X_LO
    clc
    adc a:ScrollXLo
    sta z:zTemp00
    lda a:OBJ_X_HI
    adc a:ScrollXHiScreen
    cmp #$02
    bne WakeBeforeLevelX101B_at0
    lda z:zTemp00
    cmp #$EE
    bcs WakeBeforeLevelX101B_at0
    jsr VecRecordObjectEvent
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda a:OBJ_LOOP_FLAGS,x
    ora #$40
    sta a:OBJ_LOOP_FLAGS,x
    jsr VecSideOfPlayer
    and #$01
    asl a
    tay
    lda a:WakeBeforeLevelX101BSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:WakeBeforeLevelX101BSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda #$FC
    sta a:OBJ_VY_HI,x
    inc a:OBJ_SUBSTATE,x
WakeBeforeLevelX101B_at0:
    rts

; @routine 101E:151 WalkByTable101E
; @run 71:C951
; WHAT IT DOES: Animates and moves across at the speed OBJ_WORK_I picks, turning round at a wall.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WalkByTable101ESpeeds = $C995 ; vx by OBJ_WORK_I
WalkByTable101E:
    lda #$99
    sta z:zTemp10
    lda #$C9
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_WORK_I,x
    asl a
    tay
    lda a:WalkByTable101ESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:WalkByTable101ESpeeds,y
    sta a:OBJ_VX_LO,x
    jsr VecApplyVelocityX
    lda #$08
    sta z:zProduct1
    lda #$02
    jsr VecProbeTerrain
    lda a:OBJ_VX_HI,x
    rol a
    rol a
    and #$01
    eor #$01
    asl a
    tay
    lda a:TileHit,y
    ora a:TileHit+1,y
    and #$02
    bne WalkByTable101E_at0
    rts
WalkByTable101E_at0:
    jsr VecNegateVelocityX
    jmp VecApplyVelocityX

; @routine 10CB:72F SeedWhileLeader10CB
; @run 70:B72F
; WHAT IT DOES: Runs the list at $B777 throwing kind $27 at frame $4F; when slot 0 reaches state 9, substate 5.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SeedWhileLeader10CB:
    lda a:OBJ_STATE
    cmp #$09
    bne SeedWhileLeader10CB_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_SUBSTATE,x
    rts
SeedWhileLeader10CB_at0:
    lda #$77
    sta z:zTemp10
    lda #$B7
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq SeedWhileLeader10CB_at1
    bcc SeedWhileLeader10CB_at2
    lda a:OBJ_FRAME,x
    cmp #$4F
    bne SeedWhileLeader10CB_at2
    ldy #$30
SeedWhileLeader10CB_at4:
    lda a:OBJ_KIND,y
    beq SeedWhileLeader10CB_at3
    iny
    cpy #$34
    bcc SeedWhileLeader10CB_at4
    rts
SeedWhileLeader10CB_at3:
    jsr VecClearObjectInY
    lda #$27
    jsr VecStartObjectAtPlace
    lda #$1E
    jsr VecSoundCommand
    rts
SeedWhileLeader10CB_at1:
    inc a:OBJ_SUBSTATE,x
SeedWhileLeader10CB_at2:
    rts

; @routine 10CB:7A2 ThrowByFrame10CB
; @run 70:B7A2
; WHAT IT DOES: Runs the list at $B7E6, throwing kind $2B at frame $58 and $2C (with sound $E5) at $5C; at its end substate 6.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ThrowByFrame10CB:
    lda #$E6
    sta z:zTemp10
    lda #$B7
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq ThrowByFrame10CB_at0
    bcc ThrowByFrame10CB_at1
    lda a:OBJ_FRAME,x
    cmp #$58
    beq ThrowByFrame10CB_at2
    cmp #$5C
    bne ThrowByFrame10CB_at1
    lda #$E5
    jsr VecSoundCommand
    ldy #$11
    jsr VecClearObjectInY
    lda #$2C
    jsr VecStartObjectAtPlace
    rts
ThrowByFrame10CB_at2:
    ldy #$30
    jsr VecClearObjectInY
    lda #$2B
    jsr VecStartObjectAtPlace
    rts
ThrowByFrame10CB_at0:
    stz a:OBJ_VY_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_WORK_G,x
    lda #$06
    sta a:OBJ_SUBSTATE,x
ThrowByFrame10CB_at1:
    rts

; @routine 10CC:550 ListSeedFreeSlot10CC
; @run 70:BD50
; WHAT IT DOES: At step 1 of every fourth count starts a kind $28 object in a free slot of $13-$1F; at the list's end substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListSeedFreeSlot10CC:
    lda #$97
    sta z:zTemp10
    lda #$BD
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq ListSeedFreeSlot10CC_at0
    lda a:OBJ_ANIM_STEP,x
    cmp #$01
    bne ListSeedFreeSlot10CC_at1
    lda a:OBJ_WORK_D,x
    and #$03
    bne ListSeedFreeSlot10CC_at1
    ldy #$13
ListSeedFreeSlot10CC_at3:
    lda a:OBJ_KIND,y
    beq ListSeedFreeSlot10CC_at2
    iny
    cpy #$20
    bcc ListSeedFreeSlot10CC_at3
    rts
ListSeedFreeSlot10CC_at2:
    lda #$28
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda #$01
    sta a:OBJ_START_ARG,y
    rts
ListSeedFreeSlot10CC_at0:
    lda #$01
    sta a:OBJ_WORK_L,x
    lda #$B0
    sta a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
ListSeedFreeSlot10CC_at1:
    rts

; @routine 117F:07E StartSixSegments117F
; @run 71:D07E
; WHAT IT DOES: Health 3; starts six numbered kind $3E segments two slots on, lags 8 apart.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartSixSegments117F:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    lda #$F0
    sta a:OBJ_WORK_D,x
    lda #$03
    sta a:OBJ_HEALTH,x
    txa
    tay
    iny
    iny
    lda #$06
    sta z:zTemp00
    lda #$03
    sta z:zTemp01
    stz z:zTemp02
StartSixSegments117F_at0:
    jsr VecClearObjectInY
    lda #$3E
    jsr VecStartObjectAtPlace
    txa
    sta a:OBJ_WORK_L,y
    lda z:zTemp01
    sta a:OBJ_WORK_D,y
    clc
    adc #$08
    sta z:zTemp01
    lda z:zTemp02
    sta a:OBJ_WORK_M,y
    inc z:zTemp02
    iny
    dec z:zTemp00
    bne StartSixSegments117F_at0
    rts

; @routine 12DB:16D HoldThenShoot12DB
; @run 70:B16D
; WHAT IT DOES: Holds the player and the scroll's limit at X $380; throws kind $25 at step 9; at the list's end a wait of $40 and state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
HoldThenShoot12DB:
    lda #$03
    sta a:OBJ_WORK_E
    lda #$02
    sta a:PlayerStartPose
    stz a:OBJ_WORK_F
    lda #$03
    sta a:ScrollXMaxHi
    lda #$80
    sta a:ScrollXMaxHi+1
    lda #$B4
    sta z:zTemp10
    lda #$B1
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq HoldThenShoot12DB_at0
    bcc HoldThenShoot12DB_at1
    lda a:OBJ_ANIM_STEP,x
    cmp #$09
    bne HoldThenShoot12DB_at1
    ldy #$50
    jsr VecClearObjectInY
    lda #$25
    jsr VecStartObjectAtPlace
    rts
HoldThenShoot12DB_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$40
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
HoldThenShoot12DB_at1:
    rts

; @routine 12DC:440 WalkByTable12DC
; @run 70:BC40
; WHAT IT DOES: Animates and moves across at the speed OBJ_WORK_I picks, turning round at a wall.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WalkByTable12DCSpeeds = $BC84 ; vx by OBJ_WORK_I
WalkByTable12DC:
    lda #$88
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_WORK_I,x
    asl a
    tay
    lda a:WalkByTable12DCSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:WalkByTable12DCSpeeds,y
    sta a:OBJ_VX_LO,x

; @routine 12DD:69D BoxByLeader12DD
; @run 71:C69D
; WHAT IT DOES: Rides slot $10 with a box by its frame ($43-$48); no box otherwise.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BoxByLeader12DDBoxes = $C6E1 ; hit box by the leader's frame
BoxByLeader12DD:
    lda a:OBJ_STATE,x
    bne BoxByLeader12DD_at0
    jmp VecLoadObjectRecord
BoxByLeader12DD_at0:
    lda a:OBJ_FRAME+$10
    cmp #$43
    bcc BoxByLeader12DD_at1
    cmp #$49
    bcs BoxByLeader12DD_at1
    sec
    sbc #$43
    asl a
    asl a
    tay
    lda a:BoxByLeader12DDBoxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:BoxByLeader12DDBoxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:BoxByLeader12DDBoxes+2,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:BoxByLeader12DDBoxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    ldy #$10
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_FACING_LEFT+$10
    sta a:OBJ_FACING_LEFT,x
    rts
BoxByLeader12DD_at1:
    stz a:OBJ_HITBOX_HALF_W,x
    stz a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 12DD:7D6 LoadShotByArg12DD
; @run 71:C7D6
; WHAT IT DOES: Loads at one of the places the start argument names, X mirrored while OBJ_WORK_D is set.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadShotByArg12DDPlaces = $C81D ; place by argument
LoadShotByArg12DD:
    jsr VecLoadObjectRecord
    lda a:OBJ_HITBOX_ON,x
    ora #$80
    sta a:OBJ_HITBOX_ON,x
    lda #$FC
    sta a:OBJ_VY_HI,x
    lda #$00
    sta a:OBJ_VY_LO,x
    lda #$24
    sta a:OBJ_WORK_E,x
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    tay
    lda a:LoadShotByArg12DDPlaces+1,y
    sta a:OBJ_X_HI,x
    lda a:LoadShotByArg12DDPlaces,y
    sta a:OBJ_X_LO,x
    lda a:LoadShotByArg12DDPlaces+3,y
    sta a:OBJ_Y_HI,x
    lda a:LoadShotByArg12DDPlaces+2,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_WORK_D,x
    beq LoadShotByArg12DD_at0
    lda a:OBJ_X_LO,x
    eor #$FF
    inc a
    sta a:OBJ_X_LO,x
LoadShotByArg12DD_at0:
    rts

; @routine 12DE:33B WalkByTable12DE
; @run 71:CB3B
; WHAT IT DOES: On each new frame of the list moves across at the speed OBJ_WORK_E picks, turning round at a wall.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WalkByTable12DESpeeds = $CB81 ; vx by OBJ_WORK_E
WalkByTable12DE:
    lda #$85
    sta z:zTemp10
    lda #$CB
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc MoveBounceByTable12DE+$32

; @routine 13FB:525 WalkByTable13FB
; @run 70:B525
; WHAT IT DOES: On each new frame of the list moves across at the speed OBJ_WORK_J picks, turning round at a wall.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WalkByTable13FBSpeeds = $B579 ; vx by OBJ_WORK_J
WalkByTable13FB:
    lda #$6C
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc WalkByTable13FB_at0
    lda a:OBJ_WORK_J,x
    asl a
    tay
    lda a:WalkByTable13FBSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:WalkByTable13FBSpeeds,y
    sta a:OBJ_VX_LO,x
    jsr VecApplyVelocityX
    lda #$04
    sta z:zProduct1
    lda #$02
    jsr VecProbeTerrain
    lda a:OBJ_VX_HI,x
    rol a
    rol a
    and #$01
    eor #$01
    asl a
    tay
    lda a:TileHit,y
    ora a:TileHit+1,y
    and #$02
    bne WalkByTable13FB_at1
WalkByTable13FB_at0:
    rts
WalkByTable13FB_at1:
    jsr VecNegateVelocityX
    jmp VecApplyVelocityX

; @routine 161B:646 SpawnWhileWaiting161B
; @run 70:B646
; WHAT IT DOES: Throws kind $34 pieces while OBJ_WORK_H counts, then shows itself, waits again and becomes hit-tested, state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnWhileWaiting161B:
    lda a:OBJ_SUBSTATE,x
    bne SpawnWhileWaiting161B_at0
    lda a:OBJ_WORK_H,x
    and #$07
    bne SpawnWhileWaiting161B_at1
    jsr VecFindFreeSlotFrom30
    bcc SpawnWhileWaiting161B_at1
    lda #$34
    jsr VecStartObjectAtPlace
    jsr VecNextRandom
    and #$07
    sta a:OBJ_START_ARG,y
SpawnWhileWaiting161B_at1:
    dec a:OBJ_WORK_H,x
    bne SpawnWhileWaiting161B_at2
    lda #$20
    sta a:OBJ_WORK_H,x
    lda #$0E
    sta a:OBJ_FRAME,x
    lda #$10
    sta a:OBJ_SPRITE_ATTR,x
    inc a:OBJ_SUBSTATE,x
SpawnWhileWaiting161B_at2:
    rts
SpawnWhileWaiting161B_at0:
    dec a:OBJ_WORK_H,x
    bne SpawnWhileWaiting161B_at2
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    stz a:OBJ_SPRITE_ATTR,x
    inc a:OBJ_STATE,x
    rts

; @routine 161D:71F ListLunge161D
; @run 71:C71F
; WHAT IT DOES: Lunges during steps 7-10 of the list (sounds at 8); at its end state 3.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListLunge161DSpeeds = $C789 ; vx by facing
ListLunge161D:
    lda #$66
    sta z:zTemp10
    lda #$C7
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq ListLunge161D_at0
    lda a:OBJ_ANIM_STEP,x
    cmp #$07
    bcc ListLunge161D_at1
    cmp #$0B
    bcs ListLunge161D_at1
    cmp #$08
    bne ListLunge161D_at2
    lda #$87
    jsr VecSoundCommand
    lda #$1A
    jsr VecSoundIfOnFirstScreen
ListLunge161D_at2:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:ListLunge161DSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:ListLunge161DSpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$0C
    sta z:zProduct1
    jmp BounceAtWalls
ListLunge161D_at0:
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ListLunge161D_at1:
    rts

; @routine 16CC:086 LoadWithLimbs16CC
; @run 70:B886
; WHAT IT DOES: Loads above the screen, counted in slot $10, with five kind $24 and two kind $25 limbs.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadWithLimbs16CC:
    jsr VecLoadObjectRecord
    inc a:OBJ_WORK_L+$10
    stz a:OBJ_X_HI,x
    lda #$A0
    sta a:OBJ_X_LO,x
    lda #$FF
    sta a:OBJ_Y_HI,x
    lda #$A0
    sta a:OBJ_Y_LO,x
    lda #$02
    sta a:OBJ_VY_HI,x
    lda #$60
    sta a:OBJ_VY_LO,x
    ldy #$11
LoadWithLimbs16CC_at0:
    jsr VecClearObjectInY
    lda #$24
    jsr VecStartObjectAtPlace
    iny
    cpy #$16
    bcc LoadWithLimbs16CC_at0
    iny
    jsr VecClearObjectInY
    lda #$25
    jsr VecStartObjectAtPlace
    iny
    jsr VecClearObjectInY
    lda #$25
    jsr VecStartObjectAtPlace
    rts
