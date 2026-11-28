; @routine 024D:2E5 StageEntranceStep
; @run 6B:82E5
; WHAT IT DOES: A frame of the player's entrance: the music started (not in stage $10; $FE for stage $87, else $FF), the entrance run, and once play has begun the stage's objects set up.
; HOW: Sound commands, then StageEntranceByKind; in game sub-mode 1 the objects are cleared and SetUpStageObjects follows.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageEntranceStep:
    lda z:zStage
    cmp #$10
    beq StageEntranceStep_at1
    cmp #$87
    beq StageEntranceStep_at0
    lda #$FF
    jsr VecSoundCommand
    bra StageEntranceStep_at1
StageEntranceStep_at0:
    lda #$FE
    jsr VecSoundCommand
StageEntranceStep_at1:
    jsr StageEntranceByKind
    lda z:zGameSubMode
    cmp #$01
    beq StageEntranceStep_at2
    rts
StageEntranceStep_at2:
    jsr MapOverlayBanks5And6
    jsr MariaVecClearObjectsBut
    jmp SetUpStageObjects

; @routine 024D:30E StageEntranceByKind
; @run 6B:830E
; WHAT IT DOES: Goes to the entrance zField87 names.
; HOW: A jump through the table at $8315.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageEntranceKinds = $8315 ; the entrance by zField87
StageEntranceByKind:
    lda z:zField87
    asl a
    tax
    jmp (StageEntranceKinds,x)

; @routine 024D:400 StageClearedByKind
; @run 6B:8400
; WHAT IT DOES: A frame of the cleared stage: the frame counted, then the way out zClearKind names.
; HOW: INC zWaveCounter, a jump through the table at $8409.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageClearedKinds = $8409 ; the way out by zClearKind
StageClearedByKind:
    inc z:zWaveCounter
    lda z:zClearKind
    asl a
    tax
    jmp (StageClearedKinds,x)

; @routine 024D:458 StageClearedWalkByStep
; @run 6B:8458
; WHAT IT DOES: The walk-out way: the scroll and stage frame run, then the step zField85 names.
; HOW: Two calls, a jump through the table at $8465.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageClearedWalkSteps = $8465 ; the walk-out's step by zField85
StageClearedWalkByStep:
    jsr StageScrollStep
    jsr StepStageFrame
    lda z:zField85
    asl a
    tax
    jmp (StageClearedWalkSteps,x)

; @routine 024D:46F StageClearedWaitEmpty
; @run 6B:846F
; WHAT IT DOES: Runs the player and objects until no slot from $50 down to 1 holds a kind below $80, then clears the attack marks and goes to the next step.
; HOW: A walk of OBJ_KIND.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageClearedWaitEmpty:
    jsr StageShowStartFrame
    jsr StagePlayerObjects
    jsr StageCameraStep
    jsr StepScrollAndRunObjects
    ldx #$50
StageClearedWaitEmpty_at0:
    lda a:OBJ_KIND,x
    bmi StageClearedWaitEmpty_at1
    bne StageClearedWaitScroll_at0
StageClearedWaitEmpty_at1:
    dex
    bne StageClearedWaitEmpty_at0
    stz a:HitAttackFlags
    inc z:zField85
    rts

; @routine 024D:4D1 StageClearedWalkToStop
; @run 6B:84D1
; WHAT IT DOES: Walks the player back to the route's mark; there, clears the walk, sets zField8A to 2, starts Maria and goes on (StageShowAndStep).
; HOW: The route's step taken from the player's X, compared with WalkStopXByRoute.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageClearedWalkToStop:
    jsr StepScrollAndRunObjects
    lda #$E0
    sta a:OBJ_PIECE_PLACE
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:WalkStopXByRoute+4,y
    sta a:OBJ_SWOOP_SPEED_LO
    jsr StagePlayerFrame
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:OBJ_X_LO
    sec
    sbc a:WalkPlayerStepByRoute,y
    sta a:OBJ_X_LO
    cmp a:WalkStopXByRoute+2,y
    beq StageClearedWalkToStop_at0
    rts
StageClearedWalkToStop_at0:
    rmb7 z:zField8C
    stz a:OBJ_PIECE_PLACE
    stz a:OBJ_SWOOP_SPEED_LO
    lda #$02
    sta z:zField8A

; @routine 024D:513 StageClearedNextRoom
; @run 6B:8513
; WHAT IT DOES: Scrolls the next room in with the player carried along; at the screen's edge changes section and music, loads the section's lists, runs and entry, sets up the scroll again where the section needs it, and returns to play.
; HOW: The route's scroll step each frame; on arrival StageSectionNext, the stage's loads, and game sub-mode 1.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageClearedNextRoom:
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:WalkScrollStepByRoute,y
    sta a:ScrollStepXHi
    stz a:ScrollStepXLo
    lda a:OBJ_X_LO
    clc
    adc a:WalkPlayerStepByRoute,y
    sta a:OBJ_X_LO
    jsr StageCameraStep
    jsr StepScrollAndRunObjects
    lda a:ScrollXLo
    beq StageClearedNextRoom_at0
    rts
StageClearedNextRoom_at0:
    lda z:zField8C
    cmp #$07
    bcs StageClearedNextRoom_at1
    jsr StageSectionNext
    sta z:zField86
    bra StageClearedNextRoom_at4
StageClearedNextRoom_at1:
    inc z:zField83
    lda z:zStage
    cmp #$83
    beq StageClearedNextRoom_at4
    cmp #$89
    beq StageClearedNextRoom_at4
    cmp #$0C
    beq StageClearedNextRoom_at2
    cmp #$8C
    beq StageClearedNextRoom_at3
    cmp #$8E
    beq StageClearedNextRoom_at4
    smb4 z:zField8E
    lda #$FE
    jsr VecSoundCommand
    bra StageClearedNextRoom_at4
StageClearedNextRoom_at2:
    lda z:zField92
    cmp #$04
    beq StageClearedNextRoom_at3
    lda #$1C
    clc
    adc z:zCharacter
    jsr VecAdpcmLoad
StageClearedNextRoom_at3:
    lda #$5B
    sta z:zFadeSound
    jsr VecSoundCommand
StageClearedNextRoom_at4:
    smb1 z:zField8E
    lda z:zField8C
    cmp #$07
    bcs StageClearedNextRoom_at5
    jsr StartStageLists
StageClearedNextRoom_at5:
    jsr ExpandStageVramRuns
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088
    lda z:zField8E
    pha
    jsr LoadStageEntry
    pla
    sta z:zField8E
    lda z:zRasterEffect
    cmp #$0E
    beq StageClearedNextRoom_at6
    cmp #$0F
    bne StageClearedNextRoom_at7
StageClearedNextRoom_at6:
    jsr SetUpStageScrollTwice
    bra StageClearedNextRoom_at9
StageClearedNextRoom_at7:
    lda a:CameraMode
    cmp #$0D
    beq StageClearedNextRoom_at8
    cmp #$11
    bne StageClearedNextRoom_at9
StageClearedNextRoom_at8:
    jsr MapOverlayBank7
    jsr StageSetUpScroll
    jsr LoadStageEntry
StageClearedNextRoom_at9:
    rmb1 z:zField8E
    lda z:zField8C
    cmp #$09
    bcs StageClearedNextRoom_at10
    cmp #$07
    bcs StageClearedNextRoom_at11
StageClearedNextRoom_at10:
    jsr SetUpStageObjects
StageClearedNextRoom_at11:
    stz z:zField8C
    stz z:zPaused
    lda #$01
    sta z:zGameSubMode
    rts

; @routine 024D:5DB StageClearedFinishByStep
; @run 6B:85DB
; WHAT IT DOES: The third way out: the stage frame and fade timer run, then the step zField85 names.
; HOW: A jump through the table at $85E8.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageClearedFinishSteps = $85E8 ; the step by zField85
StageClearedFinishByStep:
    jsr StepStageFrame
    jsr StageFadeTimer
    lda z:zField85
    asl a
    tax
    jmp (StageClearedFinishSteps,x)

; @routine 024D:5EE StageClearedScrollToEnd
; @run 6B:85EE
; WHAT IT DOES: Scrolls on a pixel a frame with the player walking against it until screen 7; there stops the scroll, plays sound $8C and goes to the next step.
; HOW: The scroll's X step set, the player's X counted down.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageClearedScrollToEnd:
    jsr StageShowStartFrame
    jsr StagePlayerObjects
    lda #$01
    sta a:ScrollStepXHi
    stz a:ScrollStepXLo
    dec a:OBJ_X_LO
    jsr StageCameraStep
    jsr StepScrollAndRunObjects
    lda a:ScrollXHi
    cmp #$07
    beq StageClearedScrollToEnd_at0
    rts
StageClearedScrollToEnd_at0:
    jsr StageScrollSpeedsOff
    stz a:ScrollStepXHi
    stz a:ScrollStepXLo+2
    lda #$8C
    jsr VecSoundCommand
    inc z:zField85
    rts

; @routine 024D:61E StageClearedWaitObject22
; @run 6B:861E
; WHAT IT DOES: Runs the player and objects until object $22 reaches state 4 with its OBJ_FALL_LO at zero, then starts the walk on screen.
; HOW: Two tests of slot $22.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageClearedWaitObject22:
    jsr StagePlayerObjects
    jsr StepScrollAndRunObjects
    lda a:OBJ_STATE+34
    cmp #$04
    bne StageClearedWaitObject22_at0
    lda a:OBJ_FALL_LO+34
    bne StageClearedWaitObject22_at0
    lda #$20
    sta a:OBJ_SWOOP_SPEED_LO
    inc z:zField85
StageClearedWaitObject22_at0:
    rts

; @routine 024D:638 StageClearedWalkOnScreen
; @run 6B:8638
; WHAT IT DOES: Walks the player on to X $F0, then changes section ($8654).
; HOW: The player's frame with the walk's mark set.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageClearedWalkOnScreen:
    lda #$B8
    sta a:OBJ_PIECE_PLACE
    jsr StagePlayerFrame
    jsr VecPlayerAndCameraFrame
    jsr MariaVecRunObjects
    jsr StageCameraStep
    jsr StepScrollAndRunObjects
    lda a:OBJ_X_LO
    cmp #$F0
    beq StageNextSectionAfterWalk
    rts

; @routine 024D:654 StageNextSectionAfterWalk
; @run 6B:8654
; WHAT IT DOES: After a walk: a 2 frame display delay, the walk cleared, the next section, back to play.
; HOW: Stores and StageSectionNext.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageNextSectionAfterWalk:
    lda #$02
    sta z:zDisplayDelay
    stz a:OBJ_PIECE_PLACE
    stz a:HitAttackFlags
    stz a:OBJ_SWOOP_SPEED_LO
    jsr StageSectionNext
    stz z:zClearKind
    stz a:OBJ_FRAME
    rts

; @routine 024D:675 StageClearedDoorByStep
; @run 6B:8675
; WHAT IT DOES: The boss door way: the scroll and stage frame run, then the step zField85 names.
; HOW: A jump through the table at $8682.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageClearedDoorSteps = $8682 ; the step by zField85
StageClearedDoorByStep:
    jsr StageScrollStep
    jsr StepStageFrame
    lda z:zField85
    asl a
    tax
    jmp (StageClearedDoorSteps,x)

; @routine 024D:69D StageClearedWalkOut
; @run 6B:869D
; WHAT IT DOES: Walks the player out of the stage to the route's mark; there ends the stage: from zField8C $0F up chooses the next section and entrance and goes to the death step's reload ($823D); at $0B or $0C starts a list channel and changes section; else ends the mode.
; HOW: The route's step added to the player's 16-bit X.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageClearedWalkOut:
    jsr StepScrollAndRunObjects
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:WalkStopXByRoute+4,y
    sta a:OBJ_SWOOP_SPEED_LO
    sta a:OBJ_PIECE_PLACE
    jsr StagePlayerFrame
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:WalkScrollStepByRoute,y
    clc
    adc a:OBJ_X_LO
    sta a:OBJ_X_LO
    bne StageClearedWalkOut_at0
    lda a:WalkScrollStepByRoute,y
    clc
    adc a:OBJ_X_HI
    sta a:OBJ_X_HI
StageClearedWalkOut_at0:
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:OBJ_X_LO
    cmp a:WalkStopXByRoute+6,y
    bne StageClearedWalkOut_at1
    lda z:zField8C
    cmp #$0F
    bcs StageClearedWalkOut_at2
    cmp #$0B
    beq StageClearedWalkOut_at3
    cmp #$0C
    beq StageClearedWalkOut_at3
    inc z:zModeDone
StageClearedWalkOut_at1:
    rts
StageClearedWalkOut_at2:
    dec a
    lsr a
    lsr a
    lsr a
    lsr a
    sta z:zField83
    lda z:zField8C
    dec a
    and #$01
    ora #$02
    sta z:zField87
    jmp StageSubStepDeath
StageClearedWalkOut_at3:
    lda #$66
    ldx #$05
    jsr VecStartListChannelKind2
    jmp StageNextSectionAfterWalk

; @routine 024D:70A StageClearedLeaveScene
; @run 6B:870A
; WHAT IT DOES: Fades the music, clears the objects; with zField8C's low bits zero changes section, else leaves by route (StageLeaveByRoute).
; HOW: Sound $FF, MariaVecClearObjectsBut, a test of zField8C.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageClearedLeaveScene:
    lda #$FF
    jsr VecSoundCommand
    jsr MapOverlayBanks5And6
    jsr MariaVecClearObjectsBut
    lda z:zField8C
    and #$7F
    bne StageLeaveByRoute
    stz z:zField8C
    jmp StageNextSectionAfterWalk

; @routine 024D:7A1 StageDeathByStep
; @run 6B:87A1
; WHAT IT DOES: A frame of the death: but for step 3 the camera and objects run slowed (TimeSlowMode 3) with their hit tests; then the step zField80 names.
; HOW: A jump through the table at $87C7.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageDeathSteps = $87C7 ; the death's step by zField80
StageDeathByStep:
    inc z:zWaveCounter
    jsr StepStageFrame
    lda z:zField80
    cmp #$03
    beq StageDeathByStep_at0
    lda #$03
    sta a:TimeSlowMode
    jsr StageCameraStep
    jsr StepScrollAndRunObjects
    jsr HitTestAllObjects
    stz a:TimeSlowMode
StageDeathByStep_at0:
    jsr MapOverlayBanks5And6
    lda z:zField80
    asl a
    tax
    jmp (StageDeathSteps,x)

; @routine 024D:7D5 StageDeathLeapStart
; @run 6B:87D5
; WHAT IT DOES: Maria's death leap and its sound, then the next step ($87DB).
; HOW: Two calls into her overlay.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageDeathLeapStart:
    jsr MariaVecDeathLeap
    jsr MariaVecAnimationSound

; @routine 024D:7DB StageNextDeathStep
; @run 6B:87DB
; WHAT IT DOES: Goes to the death's next step.
; HOW: INC zField80.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageNextDeathStep:
    inc z:zField80
    rts

; @routine 024D:7DE StageDeathFall
; @run 6B:87DE
; WHAT IT DOES: Runs Maria's death objects, with the animation's sound when the frame changes; with zField89 set, the next step.
; HOW: The frame kept across the run and compared.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
StageDeathFall:
    lda a:OBJ_FRAME
    pha
    jsr MariaVecDeathObjects
    pla
    cmp a:OBJ_FRAME
    beq StageDeathFall_at0
    jsr MariaVecAnimationSound
StageDeathFall_at0:
    lda z:zField89
    bne StageNextDeathStep
    rts
