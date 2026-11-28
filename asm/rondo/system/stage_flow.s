; @routine 024D:009 StageVec8009_024D
; @run 6B:8009
; WHAT IT DOES: Goes to ClearStageBitRowsFour.
; HOW: One jump.
; WAITS: where the routine at $92D9 waits; the jump adds nothing to the stack.
; Entry, exit: as the routine at $92D9.
StageVec8009_024D:
    jmp ClearStageBitRowsFour

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

; @routine 024D:3E9 StageFallStepNext
; @run 6B:83E9
; WHAT IT DOES: Counts the fall's step on.
; HOW: INC zFallStep.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the new step. A, X, Y, C and V kept.
StageFallStepNext:
    inc z:zFallStep
    rts

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

