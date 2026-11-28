; @routine 024D:009 StageVec8009_024D
; @run 6B:8009
; WHAT IT DOES: Goes to ClearStageBitRowsFour.
; HOW: One jump.
; WAITS: where the routine at $92D9 waits; the jump adds nothing to the stack.
; Entry, exit: as the routine at $92D9.
StageVec8009_024D:
    jmp ClearStageBitRowsFour

; @routine 024D:02D StageStart
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start: the game state wiped, its lists, VRAM runs, set-up, objects and player, then the
; music by stage: with zField8E's bit 0 clear StageStartPlain; else the secret stages StageStartMusicByStage, stage $0C
; StageStartMusicKeep, stage $10 the boss music, others the card's play-again.
; HOW: The calls in turn; zField82 indexes SectionOrder for zField86; BBR0 and compares of zStage.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as the music routine, or as the play-again.
StageStart:
    jsr VecResetGameState
    jsr StartStageLists
    jsr ExpandStageVramRuns
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01
    jsr SpawnStageObjectsRows34
    jsr SetUpStageObjects
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:SectionOrder,y
    sta z:zField86
    jsr ResetStageVariables024D
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 024D:06A StageStartBossMusic
; @run 6B:806A
; WHAT IT DOES: The boss music, then the start's end.
; HOW: BSR StageStartMusicBoss, BRA to StageStartDone.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as StageStartDone, the rest as the music left them.
StageStartBossMusic:
    bsr StageStartMusicBoss
    bra StageStartDone

; @routine 024D:06E StageStartMusicBoss
; @run 6B:806E
; WHAT IT DOES: The music stopped and the sound $5B kept and sent, with zField8E's bit 3 set.
; HOW: SMB3, the stop vector, BRA to StageStartSound.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as StageStartSound.
StageStartMusicBoss:
    smb3 z:zField8E
    jsr VecStopSound
    lda #$5B
    bra StageStartSound

; @routine 024D:077 StageStartMusicKeep
; @run 6B:8077
; @stop 8079
; WHAT IT DOES: StageStartMusicByStage with zField8E's bit 3 set, so the fade's end sends the sound.
; HOW: SMB3, running into StageStartMusicByStage.
; WAITS: no.
; Entry: A zStage. Exit: as StageStartMusicByStage.
StageStartMusicKeep:
    smb3 z:zField8E

; @routine 024D:079 StageStartMusicByStage
; @run 6B:8079
; WHAT IT DOES: For a stage of $80 or more: stage $80 itself nothing; else the music stopped and the sound $5A.
; HOW: CMP #$80; JSR the stop vector; running into StageStartSound.
; WAITS: no.
; Entry: A zStage. Exit: stage $80: N and Z from the compare, C 1; else as StageStartSound.
StageStartMusicByStage:
    cmp #$80
    beq StageStartDone_at0
    jsr VecStopSound
    lda #$5A

; @routine 024D:082 StageStartSound
; @run 6B:8082
; WHAT IT DOES: Keeps the stage's start sound as zFadeSound and sends it.
; HOW: STA zFadeSound, the tail JMP to the sound hook.
; WAITS: no.
; Entry: A the sound, X and Y the caller's. Exit: as SoundCommand.
StageStartSound:
    sta z:zFadeSound
    jmp VecSoundCommand

; @routine 024D:087 StageStartPlain
; @run 6B:8087
; @stop 8093
; WHAT IT DOES: The start with zField8E's bit 0 clear: stage $10 the boss music, stage $0C the by-stage music, others
; none; then the start's end.
; HOW: CPs of zStage read once; the BSR to StageStartMusicKeep.
; WAITS: no.
; Entry: nothing. Exit: as StageStartDone, the rest as the music left them.
StageStartPlain:
    lda z:zStage
    cmp #$10
    beq StageStartBossMusic
    cmp #$0C
    bne StageStartDone
    bsr StageStartMusicKeep

; @routine 024D:093 StageStartDone
; @run 6B:8093
; WHAT IT DOES: The stage start's end: sub-mode 5, the forced pad's frames counted down, the player's start pose set.
; HOW: Stores and a DEC.
; WAITS: no.
; Entry: nothing. Exit: A 1, N and Z from it; X, Y, C and V kept.
StageStartDone:
    lda #$05
    sta z:zGameSubMode
    dec a:OBJ_WORK_E
    lda #$01
    sta a:PlayerStartPose
StageStartDone_at0:
    rts

; @routine 024D:0BF StageCameraStep
; @run 6B:80BF
; WHAT IT DOES: The camera's frame with the overlay's bank 7 mapped.
; HOW: MapOverlayBank7, then the tail JMP to $A08D, the C of the routine loaded there.
; WAITS: no.
; Entry: nothing. Exit: as CameraStep, entered with A the bank, N, Z, C and V from the map's add.
StageCameraStep:
    jsr MapOverlayBank7
    jmp CameraStep

; @routine 024D:0C5 StageScrollSpeedsOff
; @run 6B:80C5
; WHAT IT DOES: Stops the layers' scroll: maps the overlay's bank 7 and runs its ClearScrollSpeeds.
; HOW: MapOverlayBank7, then the tail JMP to $A067, the C of the routine loaded there.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as ClearScrollSpeeds leaves them, entered with A the bank and N, Z, C and V
; from the map's add.
StageScrollSpeedsOff:
    jsr MapOverlayBank7
    jmp ClearScrollSpeeds

; @routine 024D:178 StageFadeTimer
; @run 6B:8178
; WHAT IT DOES: The fade's countdown, two a frame. When the count reaches zero on the first step the fade is running
; (zFadeActive 1) and, unless zField8E's bits 3 or 4 are set, the card starts its CD fade of type $0C; when it
; reaches zero on the second the mark is cleared and, unless bit 4 is set, the sound zFadeSound goes out (bit 3 set)
; or the CD fade of type 0.
; HOW: Two DECs of zFadeCount with a test after each; BBS4 and BBS3 of zField8E, each a read of it; the sound hook
; at $401B or the card's cd_fade, reached by a tail JMP.
; WAITS: no.
; Entry: X and Y the caller's. Exit: count zero: A 0, N and Z from it; else N and Z from the last DEC or test, A the
; value the last test loaded; after the sound or the fade, as they leave them. X and Y kept.
StageFadeTimer:
    lda z:zFadeCount
    beq StagePauseToggle_at0
    dec z:zFadeCount
    beq StageFadeTimer_at0
    dec z:zFadeCount
    stz z:zFadeActive
    bbs4 zField8E,StagePauseToggle_at0
    bbs3 zField8E,StageFadeTimer_at1
    cla
    jmp BiosCdFade
StageFadeTimer_at1:
    lda z:zFadeSound
    jmp VecSoundCommand
StageFadeTimer_at0:
    lda #$01
    sta z:zFadeActive
    lda z:zField8E
    and #$18
    bne StagePauseToggle_at0
    lda #$0C
    jmp BiosCdFade

; @routine 024D:1A2 StagePauseToggle
; @run 6B:81A2
; WHAT IT DOES: Pauses the stage when RUN is pressed, unless the fade is running, the display is still off, or
; zPaused's bit 1 holds the pause.
; HOW: zFadeActive XOR 1 must not be zero; BiosJoyTrigger's bit 3; zDisplayDelay zero; BBS1 of zPaused (a read);
; StagePauseSet.
; WAITS: no.
; Entry: nothing. Exit: A the last value loaded or computed, N and Z from it; or as StagePauseSet. X, Y, C and V kept.
StagePauseToggle:
    lda z:zFadeActive
    eor #$01
    beq StagePauseToggle_at0
    lda a:BiosJoyTrigger
    and #$08
    beq StagePauseToggle_at0
    lda z:zDisplayDelay
    bne StagePauseToggle_at0
    bbs1 zPaused,StagePauseToggle_at0
    jsr StagePauseSet
StagePauseToggle_at0:
    rts

; @routine 024D:23D StageSubStepDeath
; @run 6B:823D
; WHAT IT DOES: Sub-step for the player's death: sub-mode 2 with zField80 and zField89 cleared, the scroll stopped,
; the forced presses and the fall's step cleared; when the time-slowing mode is set it is cleared and the fade
; starts again from 2.
; HOW: Stores; StageScrollSpeedsOff; TimeSlowMode; the tail JMP to StageFadeTimer.
; WAITS: no.
; Entry: X and Y the caller's. Exit: A TimeSlowMode (0), N and Z from it, the rest as StageScrollSpeedsOff left them;
; else as StageFadeTimer.
StageSubStepDeath:
    lda #$02
    sta z:zGameSubMode
    stz z:zField80
    stz z:zField89
    jsr StageScrollSpeedsOff
    stz a:PadForcedPress
    stz a:PadForcedPressB
    stz z:zFallStep
    lda a:TimeSlowMode
    bne StageSubStepDeath_at0
    rts
StageSubStepDeath_at0:
    stz a:TimeSlowMode
    lda #$02
    sta z:zFadeCount
    jmp StageFadeTimer

; @routine 024D:260 StageChooseClearKind
; @run 6B:8260
; WHAT IT DOES: Chooses how the cleared stage plays out (zClearKind) from zField8C: 0 gives 2, negative 4, else 1 for
; 1, 2, $0D, $0E and 7 up to $0A but $0B and $0C, and 3 otherwise; then sub-mode 3 with the scroll stopped and the
; forced presses and time-slowing cleared.
; HOW: CMP chain on zField8C read once; StageScrollSpeedsOff by the C at $80C5.
; WAITS: no.
; Entry: nothing. Exit: as the call at $80C5 and the stores leave them; A, N and Z as that left them.
StageChooseClearKind:
    lda z:zField8C
    beq StageChooseClearKind_at0
    bmi StageChooseClearKind_at1
    ldx #$01
    stx a:HitAttackFlags
    cmp #$03
    bcc StageChooseClearKind_at2
    cmp #$07
    bcc StageChooseClearKind_at3
    cmp #$0B
    beq StageChooseClearKind_at3
    cmp #$0C
    beq StageChooseClearKind_at3
    cmp #$0F
    bcs StageChooseClearKind_at3
StageChooseClearKind_at2:
    lda #$01
    bra StageChooseClearKind_at4
StageChooseClearKind_at3:
    lda #$03
    bra StageChooseClearKind_at4
StageChooseClearKind_at1:
    lda #$04
    bra StageChooseClearKind_at4
StageChooseClearKind_at0:
    lda #$02
StageChooseClearKind_at4:
    sta z:zClearKind
    lda #$03
    sta z:zGameSubMode
    stz z:zField85
    jsr StageScrollSpeedsOff
    stz a:PadForcedPress
    stz a:PadForcedPressB
    stz a:TimeSlowMode
    rts

; @routine 024D:2A2 StageSubStepFadeOut
; @run 6B:82A2
; WHAT IT DOES: Sub-step to the fade: sub-mode 4 from its first step, zField89 cleared, the scroll stopped.
; HOW: Stores, then the tail JMP to $80C5 (StageScrollSpeedsOff here), the C of the routine loaded there.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as StageScrollSpeedsOff.
StageSubStepFadeOut:
    lda #$04
    sta z:zGameSubMode
    stz z:zField80
    stz z:zField89
    jmp StageScrollSpeedsOff

; @routine 024D:2AD StageSubStepFadeOutLate
; @run 6B:82AD
; WHAT IT DOES: Sub-step to the fade at its step 5: sub-mode 4, zField80 5, zField89 cleared, the scroll stopped.
; HOW: Stores, then the tail JMP to $80C5 (StageScrollSpeedsOff here), the C of the routine loaded there.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as StageScrollSpeedsOff.
StageSubStepFadeOutLate:
    lda #$04
    sta z:zGameSubMode
    lda #$05
    sta z:zField80
    stz z:zField89
    jmp StageScrollSpeedsOff

; @routine 024D:2BA StageSubStepTally
; @run 6B:82BA
; WHAT IT DOES: Sub-step to the tally: sub-mode 6 with zField80 and zField89 cleared, the scroll stopped, the forced
; presses and the fall's step cleared, the player's work bytes F cleared and E $40; when the time-slowing mode is
; set it is cleared and the fade starts again from 2.
; HOW: Stores; StageScrollSpeedsOff; TimeSlowMode; the tail JMP to StageFadeTimer.
; WAITS: no.
; Entry: X and Y the caller's. Exit: A TimeSlowMode (0), N and Z from it, the rest as StageScrollSpeedsOff left them;
; else as StageFadeTimer.
StageSubStepTally:
    lda #$06
    sta z:zGameSubMode
    stz z:zField80
    stz z:zField89
    jsr StageScrollSpeedsOff
    stz a:PadForcedPress
    stz a:PadForcedPressB
    stz z:zFallStep
    stz a:OBJ_WORK_F
    lda #$40
    sta a:OBJ_WORK_E
    lda a:TimeSlowMode
    bne StageSubStepTally_at0
    rts
StageSubStepTally_at0:
    stz a:TimeSlowMode
    lda #$02
    sta z:zFadeCount
    jmp StageFadeTimer

; @routine 024D:31D StageForcedWalk
; @run 6B:831D
; WHAT IT DOES: A sub-mode 3 start (zField87 0): the stage set up (InitStage), sub-mode 1, and the player walked by a
; forced pad away from the side he faces: $18 frames when zField8C is set (it is cleared).
; HOW: JSR InitStage; zGameSubMode 1; OBJ_PAD_FORCE_FRAMES; facing XOR 1 picks the pad from ForcedWalkPads.
; WAITS: where InitStage waits.
; Entry: nothing. Exit: A the pad, N and Z from it, X the table index; Y, C and V as InitStage left them.
StageForcedWalk:
    jsr InitStage
    lda #$01
    sta z:zGameSubMode
    lda z:zField8C
    beq StageForcedWalk_at0
    stz z:zField8C
    lda #$18
    sta a:OBJ_WORK_E
StageForcedWalk_at0:
    lda a:OBJ_FACING_LEFT
    eor #$01
    and #$01
    tax
    lda a:ForcedWalkPads,x
    sta a:OBJ_WORK_F
    rts

; @routine 024D:340 StageForcedWalkB
; @run 6B:8340
; WHAT IT DOES: A sub-mode 3 start (zField87 1): as StageForcedWalk, the same bytes at their own address.
; HOW: As StageForcedWalk.
; WAITS: where InitStage waits.
; Entry, exit: as StageForcedWalk.
StageForcedWalkB:
    jsr InitStage
    lda #$01
    sta z:zGameSubMode
    lda z:zField8C
    beq StageForcedWalkB_at0
    stz z:zField8C
    lda #$18
    sta a:OBJ_WORK_E
StageForcedWalkB_at0:
    lda a:OBJ_FACING_LEFT
    eor #$01
    and #$01
    tax
    lda a:ForcedWalkPads,x
    sta a:OBJ_WORK_F
    rts

; @routine 024D:361 StageFallRight
; @run 6B:8361
; @stop 8373
; WHAT IT DOES: The fall to the right (zField87 2), a step a frame: the first sets the player's speed $0140 and runs
; into StageFallPose; the second StageFallRightLand; the third StageFallRightDone.
; HOW: zFallStep decremented twice with BEQs.
; WAITS: as the step taken.
; Entry: nothing. Exit: as the step taken.
StageFallRight:
    lda z:zFallStep
    dec a
    beq StageFallRightLand
    dec a
    beq StageFallRightDone
    lda #$40
    sta a:OBJ_VX_LO
    lda #$01
    sta a:OBJ_VX_HI

; @routine 024D:373 StageFallPose
; @run 6B:8373
; WHAT IT DOES: The fall's first step: the player's animation 0 set and its sound, then the step counted on.
; HOW: MapOverlayBanks5And6, $A018 with A 0, $A012, BRA to the INC of zFallStep.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the new step; the rest as $A012 left them.
StageFallPose:
    jsr MapOverlayBanks5And6
    lda #$00
    jsr MariaVecSetAnimation
    jsr MariaVecAnimationSound
    bra StageFallRightLand_at0

; @routine 024D:380 StageFallRightLand
; @run 6B:8380
; WHAT IT DOES: The fall to the right's middle step: the player idle, his animation sound, and until his X passes $10
; on a screen past the first the step counts on; else his landing.
; HOW: MapOverlayBanks5And6, $A01B, $A012; OBJ_X_HI and OBJ_X_LO; the tail JMP to $A01E with X 0, or INC zFallStep.
; WAITS: no.
; Entry: nothing. Exit: counting: N and Z from the new step; else as $A01E.
StageFallRightLand:
    jsr MapOverlayBanks5And6
    jsr MariaVecAnimationTick
    jsr MariaVecAnimationSound
    lda a:OBJ_X_HI
    beq StageFallRightLand_at1
    lda a:OBJ_X_LO
    cmp #$10
    bcs StageFallRightLand_at0
StageFallRightLand_at1:
    clx
    jmp MariaVecObjectApplyVelocity
StageFallRightLand_at0:
    inc z:zFallStep
    rts

; @routine 024D:39C StageFallRightDone
; @run 6B:839C
; WHAT IT DOES: The last step of the fall to the right: the stage set up again and the player walked on by pad $20.
; HOW: JSR InitStage; sub-mode 1, zField8C cleared; $20 frames of pad $20.
; WAITS: where InitStage waits.
; Entry: nothing. Exit: A $20, N and Z from it; X, Y, C and V as InitStage left them.
StageFallRightDone:
    jsr InitStage
    lda #$01
    sta z:zGameSubMode
    stz z:zField8C
    lda #$20
    sta a:OBJ_WORK_E
    lda #$20
    sta a:OBJ_WORK_F
    rts

; @routine 024D:3B0 StageFallLeft
; @run 6B:83B0
; @stop 83D0
; WHAT IT DOES: The fall to the left (zField87 3): the second step StageFallLeftLand, the third StageFallLeftDone; the
; first, with the object in slot $50 of kind $30, counts the step on twice, else sets the player's speed -$0140, poses
; him (StageFallPose) and runs into StageFallLeftLand.
; HOW: zFallStep decremented twice; OBJ_KIND of slot $50; BSR and BRA to StageFallStepNext; JSR StageFallPose.
; WAITS: as the step taken.
; Entry: nothing. Exit: as the step taken.
StageFallLeft:
    lda z:zFallStep
    dec a
    beq StageFallLeftLand
    dec a
    beq StageFallLeftDone
    lda a:OBJ_KIND+80
    cmp #$30
    bne StageFallLeft_at0
    bsr StageFallStepNext
    bra StageFallStepNext
StageFallLeft_at0:
    lda #$C0
    sta a:OBJ_VX_LO
    lda #$FE
    sta a:OBJ_VX_HI
    jsr StageFallPose

; @routine 024D:3D0 StageFallLeftLand
; @run 6B:83D0
; WHAT IT DOES: The fall to the left's middle step: as StageFallRightLand, counting on while his X is below $F0.
; HOW: As StageFallRightLand.
; WAITS: no.
; Entry, exit: as StageFallRightLand.
StageFallLeftLand:
    jsr MapOverlayBanks5And6
    jsr MariaVecAnimationTick
    jsr MariaVecAnimationSound
    lda a:OBJ_X_HI
    beq StageFallLeftLand_at0
    lda a:OBJ_X_LO
    cmp #$F0
    bcc StageFallStepNext
StageFallLeftLand_at0:
    clx
    jmp MariaVecObjectApplyVelocity

; @routine 024D:3E9 StageFallStepNext
; @run 6B:83E9
; WHAT IT DOES: Counts the fall's step on.
; HOW: INC zFallStep.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the new step. A, X, Y, C and V kept.
StageFallStepNext:
    inc z:zFallStep
    rts

; @routine 024D:3EC StageFallLeftDone
; @run 6B:83EC
; WHAT IT DOES: The last step of the fall to the left: as StageFallRightDone with pad $80.
; HOW: As StageFallRightDone.
; WAITS: where InitStage waits.
; Entry: nothing. Exit: A $80, N and Z from it; X, Y, C and V as InitStage left them.
StageFallLeftDone:
    jsr InitStage
    lda #$01
    sta z:zGameSubMode
    stz z:zField8C
    lda #$20
    sta a:OBJ_WORK_E
    lda #$80
    sta a:OBJ_WORK_F
    rts

; @routine 024D:413 StageRestartInPlace
; @run 6B:8413
; WHAT IT DOES: Sets the stage up anew in place: the screen cleared, its lists, VRAM runs, set-up, objects and the
; player's start, then sub-mode 1 with zField86 from zField83.
; HOW: ClearScreenState, StartStageLists, ExpandStageVramRuns, InitStage, SpawnStageObjectsRows01, zField8A 2,
; MapOverlayBanks5And6, $A003, SetUpStageObjects.
; WAITS: where its calls wait.
; Entry: nothing. Exit: A zField83, N and Z from it; X, Y, C and V as SetUpStageObjects left them.
StageRestartInPlace:
    jsr VecClearScreenState
    jsr StartStageLists
    jsr ExpandStageVramRuns
    jsr InitStage
    jsr SpawnStageObjectsRows01
    lda #$02
    sta z:zField8A
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SetUpStageObjects
    stz z:zFadeActive
    stz z:zPaused
    lda #$01
    sta z:zGameSubMode
    lda z:zField83
    sta z:zField86
    rts

; @routine 024D:448 StageShowStartFrame
; @run 6B:8448
; WHAT IT DOES: The player's frame by character set, then his animation sound.
; HOW: MapOverlayBanks5And6; OBJ_FRAME of slot 0 from StartFrameByCharacter; the tail JMP to $A012.
; WAITS: no.
; Entry: nothing. Exit: as $A012, entered with X zCharacter, A the frame, N and Z from it.
StageShowStartFrame:
    jsr MapOverlayBanks5And6
    ldx z:zCharacter
    lda a:StartFrameByCharacter,x
    sta a:OBJ_FRAME
    jmp MariaVecAnimationSound

; @routine 024D:4C7 StageClearedWaitScroll
; @run 6B:84C7
; WHAT IT DOES: A cleared stage's step: the scroll and the objects run, and once zField8C's bit 7 is set the next
; step comes.
; HOW: StepScrollAndRunObjects; zField8C negative counts zField85 on.
; WAITS: no.
; Entry: X the caller's. Exit: A zField8C, N and Z from it or from zField85's INC; X, Y, C and V as the objects left
; them.
StageClearedWaitScroll:
    jsr StepScrollAndRunObjects
    lda z:zField8C
    bpl StageClearedWaitScroll_at0
    inc z:zField85
StageClearedWaitScroll_at0:
    rts

; @routine 024D:66A StageSectionNext
; @run 6B:866A
; WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from SectionOrder by it.
; HOW: INC zField82, read again as the index, one table read, one store.
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
StageSectionNext:
    inc z:zField82
    lda z:zField82
    tay
    lda a:SectionOrder,y
    sta z:zField83
    rts

; @routine 024D:695 StageClearedScrollOn
; @run 6B:8695
; WHAT IT DOES: A cleared stage's step: the scroll and the objects run, zField8C's bit 7 is cleared and the next step
; comes.
; HOW: StepScrollAndRunObjects; RMB7 of zField8C; INC zField85.
; WAITS: no.
; Entry: X the caller's. Exit: N and Z from zField85's INC; A, X, Y, C and V as the objects left them.
StageClearedScrollOn:
    jsr StepScrollAndRunObjects
    rmb7 z:zField8C
    inc z:zField85
    rts

; @routine 024E:6D4 StagePauseSet
; @run 6B:8ED4
; WHAT IT DOES: Pauses the stage. While the opening sequence's toggle is set, or the stage is paused already, the
; pause is held (zPaused bits 0 and 1 set); otherwise zPaused is 1 and the opening sequence's frame count is cleared.
; HOW: StageSequenceToggle, BBS0 of zPaused (a read), STZ StageSequenceDelay; one store to zPaused.
; WAITS: no.
; Entry: nothing. Exit: A zPaused's new value, N and Z from it. X, Y, C and V kept.
StagePauseSet:
    lda a:StageSequenceToggle
    bne StagePauseSet_at0
    bbs0 zPaused,StagePauseSet_at0
    stz a:StageSequenceDelay
    lda #$01
    bra StagePauseSet_at1
StagePauseSet_at0:
    lda z:zPaused
    ora #$03
StagePauseSet_at1:
    sta z:zPaused
    rts

