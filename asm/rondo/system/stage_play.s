; @routine 024D:0AB StagePlayerFrame
; @run 6B:80AB
; WHAT IT DOES: Runs the player program's frame: maps the banks 5 and 6 above zRamBaseBank and goes to the vector at
; $A000 of the program loaded there.
; HOW: MapOverlayBanks5And6, then the tail JMP to $A000.
; WAITS: where the player's frame waits.
; Entry: X and Y the caller's. Exit: as the player's frame leaves them, entered with A the second bank, N and Z from
; it, C and V from the map's add.
StagePlayerFrame:
    jsr MapOverlayBanks5And6
    jmp PlayerProgramFrame

; @routine 024D:0B1 StagePlayerObjects
; @run 6B:80B1
; WHAT IT DOES: Runs the player program's objects: maps the banks 5 and 6 above zRamBaseBank and goes to the vector at
; $A009 of the program loaded there.
; HOW: MapOverlayBanks5And6, then the tail JMP to $A009.
; WAITS: where the player's objects wait.
; Entry: X and Y the caller's. Exit: as the player's objects leave them, entered with A the second bank, N and Z from
; it, C and V from the map's add.
StagePlayerObjects:
    jsr MapOverlayBanks5And6
    jmp PlayerProgramObjects

; @routine 024D:0E8 StageDemoPadStep
; @run 6B:80E8
; WHAT IT DOES: While a demo runs, maps its pad record (the banks 8 and 9 above zRamBaseBank, into MPR5 and MPR6) and
; plays its frame's pad back.
; HOW: zGameRunning zero ends at once, at the RTS before it ($80E7); else the ADC, TAM #$20, INC A, TAM #$40, and the
; BRA into StageDemoPadPlay.
; WAITS: no.
; Entry: nothing. Exit: no demo: A 0, Z 1, the rest kept. Else as StageDemoPadPlay, entered with A the second bank, N
; and Z from it, C and V from the add.
StageDemoPadStep:
    lda z:zGameRunning
    beq ResetStageVariables024D_at0
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    bra StageDemoPadPlay

; @routine 024D:0F8 StageDemoPadRecord
; @run 6B:80F8
; WHAT IT DOES: Records a demo's pad, one frame, in the form StageDemoPadPlay plays back: while the pad is the one the
; pair at zScriptPtr holds its count is counted on; a new pad, or a count of $FF, starts the next pair with a count of
; 1. The second pad's SELECT ends the record (zModeDone counted on).
; HOW: LDA [zScriptPtr] against $FF; BiosJoyTrigger2 bit 2; the pad compared with [zScriptPtr],1; BSRs to
; StageDemoPadRecordNext.
; WAITS: no.
; Nothing is known to reach it: no JSR, JMP or table of the overlay names $80F8, and the BRA before it goes past it.
; Entry: the record mapped. Exit: the count counted on: A the count, N and Z from it, Y 1 when the pad was compared. A
; new pair: A the pad, Y 1, N and Z from it. The record over: as StageDemoPadRecordNext's end, then zModeDone's INC.
; X kept.
StageDemoPadRecord:
    lda (zScriptPtr)
    cmp #$FF
    bne StageDemoPadRecord_at0
    bsr StageDemoPadRecordNext
StageDemoPadRecord_at0:
    lda a:BiosJoyTrigger2
    and #$04
    bne StageDemoPadRecord_end
    lda (zScriptPtr)
    beq StageDemoPadRecord_at1
    ldy #$01
    lda a:BiosPadState
    cmp (zScriptPtr),y
    bne StageDemoPadRecord_at2
    lda (zScriptPtr)
    inc a
    sta (zScriptPtr)
    rts
StageDemoPadRecord_at2:
    bsr StageDemoPadRecordNext
StageDemoPadRecord_at1:
    lda #$01
    sta (zScriptPtr)
    tay
    lda a:BiosPadState
    sta (zScriptPtr),y
    rts
StageDemoPadRecord_end:
    bsr StageDemoPadRecordNext
    bra StageDemoPadDone

; @routine 024D:12B StageDemoPadRecordNext
; @run 6B:812B
; WHAT IT DOES: Starts a demo record's next pair: zScriptPtr steps by 2 and the pair there is cleared; a pointer that
; reaches $E000 ends the record (zModeDone counted on) instead.
; HOW: ADC #2 to zScriptPtr's low byte; on its carry the high byte's INC and a CMP of it with $E0. CLA, STA
; [zScriptPtr], LDY #1, STA [zScriptPtr],Y.
; WAITS: no.
; Nothing is known to reach it but StageDemoPadRecord, and nothing to reach that.
; Entry: nothing. Exit: the pair cleared: A 0, Y 1, N 0 and Z 0, C and V from the add (C from the CMP when the high byte
; stepped). The record over: A $E0, C 1, N and Z from zModeDone's INC, V from the add, Y kept. X kept.
StageDemoPadRecordNext:
    lda z:zScriptPtr
    clc
    adc #$02
    sta z:zScriptPtr
    bcc StageDemoPadRecordNext_at0
    inc z:zScriptPtrHi
    lda z:zScriptPtrHi
    cmp #$E0
    bne StageDemoPadRecordNext_at0
StageDemoPadDone:
    inc z:zModeDone
    rts
StageDemoPadRecordNext_at0:
    cla
    sta (zScriptPtr)
    ldy #$01
    sta (zScriptPtr),y
    rts

; @routine 024D:147 StageDemoPadPlay
; @run 6B:8147
; WHAT IT DOES: Plays a demo's pad record back, one frame: the record (at zScriptPtr) is pairs of a count of frames
; and a pad. The pad goes to BiosPadState, its buttons not held the frame before to BiosJoyTrigger; when its count runs
; out the next pair comes. A count of zero ends the demo: zModeDone is counted on.
; HOW: zDemoPadFrames is the count left; at zero the pair's count is read through zScriptPtr. The pad is read at
; [zScriptPtr],1; zDemoPadLast keeps the pad given, its complement ANDed with the new one for the new presses. DEC
; zDemoPadFrames; at zero zScriptPtr steps by 2, its high byte counted on when the low carries.
; WAITS: no.
; Entry: the record mapped (StageDemoPadStep). Exit: the record over: A 0, N and Z from zModeDone's INC. A pad given
; with frames left: A the pad, Y 1, N and Z from the DEC. The pair's last frame: A zScriptPtr's new low byte, Y 1, C and
; V from the add, N and Z from it, or from the high byte's INC when the low carried (C then 1). X kept; V kept but where
; the pointer steps.
StageDemoPadPlay:
    lda z:zDemoPadFrames
    bne StageDemoPadPlay_pad
    lda (zScriptPtr)
    beq StageDemoPadDone
    sta z:zDemoPadFrames
StageDemoPadPlay_pad:
    ldy #$01
    lda (zScriptPtr),y
    sta a:BiosPadState
    lda z:zDemoPadLast
    eor #$FF
    and a:BiosPadState
    sta a:BiosJoyTrigger
    lda a:BiosPadState
    sta z:zDemoPadLast
    dec z:zDemoPadFrames
    beq StageDemoPadPlay_at0
    rts
StageDemoPadPlay_at0:
    lda z:zScriptPtr
    clc
    adc #$02
    sta z:zScriptPtr
    bcc StageDemoPadPlay_at1
    inc z:zScriptPtrHi
StageDemoPadPlay_at1:
    rts

; @routine 024D:1BA StageSubStepPlay
; @run 6B:81BA
; @stop 822D
; WHAT IT DOES: The stage's play sub-step, one frame: the demo's pad, the fade's countdown and the pause. Paused, the
; paused frame runs instead. Else: the wave counter steps; the player's slow timer counts down (script 9 in slot 7 at
; its end) and while it runs the layers' scroll steps are cleared and the player runs on even counts only; the
; player's frame, and unless a sub-step is asked for, the player and the camera (PlayerAndCameraFrame), the camera step
; and the player's objects (or the walk to the middle while zField8E's bit 7 is set), the scroll and the objects, the
; hit tests, the stage's frame and the player's move. Last, the sub-step asked for in zSubStep (cleared) runs from the
; table at $822D.
; HOW: Three JSRs into the overlay; BBR0 of zPaused; INC zWaveCounter; PlayerSlowTimerLo ORA Hi, two DECs; four STZs
; of the scroll steps; TST #1 of the timer's low byte twice; the JSRs; zSubStep doubled indexes the table, whose word
; is jumped through.
; WAITS: where its callees wait; the handler is reached by a jump.
; Entry: X and Y the caller's. Exit: as the handler leaves them, entered with A and X twice zSubStep, N and Z from it,
; C its bit 7.
StageSubStepPlay:
    jsr StageDemoPadStep
    jsr StageFadeTimer
    jsr StagePauseToggle
    bbr0 zPaused,StageSubStepPlay_run
    jmp StagePausedStep
StageSubStepPlay_run:
    inc z:zWaveCounter
    lda a:PlayerSlowTimerLo
    ora a:PlayerSlowTimerHi
    beq StageSubStepPlay_at0
    dec a:PlayerSlowTimerLo
    bne StageSubStepPlay_at1
    dec a:PlayerSlowTimerHi
    bne StageSubStepPlay_at1
    lda #$09
    ldx #$07
    jsr VecStartScriptSlotKind1
StageSubStepPlay_at1:
    stz a:ScrollStepXHi
    stz a:ScrollStepXLo
    stz a:ScrollStepYHi
    stz a:ScrollStepYLo
    tst #$01,a:PlayerSlowTimerLo
    bne StageSubStepPlay_at2
StageSubStepPlay_at0:
    jsr StagePlayerFrame
    lda z:zSubStep
    bne StageSubStepPlay_next
    jsr VecPlayerAndCameraFrame
StageSubStepPlay_at2:
    lda z:zField8E
    bpl StageSubStepPlay_at3
    jsr StagePlayerToCentre
    bra StageSubStepPlay_at4
StageSubStepPlay_at3:
    jsr StageCameraStep
    tst #$01,a:PlayerSlowTimerLo
    bne StageSubStepPlay_at4
    jsr StagePlayerObjects
StageSubStepPlay_at4:
    jsr StepScrollAndRunObjects
    jsr HitTestAllObjects
    jsr StepStageFrame
    jsr MapOverlayBanks5And6
    jsr PlayerProgramMoveAndPush
StageSubStepPlay_next:
    lda z:zSubStep
    stz z:zSubStep
    asl a
    tax
    jmp (StagePlayHandlers,x)

; @routine 024D:72C StagePlayerToCentre
; @run 6B:872C
; WHAT IT DOES: The walk that brings the player to the screen's middle while zField8E's bit 7 is set: his pad held
; forced for $80 frames and his objects run; standing (state 0 or 1, step 0) he is shown in his start pose and moved 4
; pixels a frame towards X $80 while the layers scroll 4 the other way and the camera steps. Within 2 of $80 the walk
; ends: zField8E's bit 7, the forced frames and the start pose cleared.
; HOW: OBJ_SUBSTATE and OBJ_STATE of slot 0; StagePlayerObjects; OBJ_X_LO plus 2 masked with $FC (before the move) and
; $F8 (after it) against $80; ScrollStepXHi $FC or 4 by the side of $80 he is on, ScrollStepXLo 0; StageCameraStep.
; WAITS: no.
; Entry: nothing. Exit: not standing, or standing and his objects run: as StagePlayerObjects leaves them. Moved: A his
; X plus 2 masked with $F8, N, Z and C from the CMP with $80, V from the add; X and Y as the camera step left them. At
; the middle: A $80, Z 1, C 1, V from the last add.
StagePlayerToCentre:
    lda a:OBJ_SUBSTATE
    bne StagePlayerToCentre_at0
    lda a:OBJ_STATE
    cmp #$02
    bcc StagePlayerToCentre_at1
StagePlayerToCentre_at0:
    lda #$80
    sta a:OBJ_PAD_FORCE_FRAMES
    stz a:OBJ_PAD_FORCED
    jsr StagePlayerObjects
    rts
StagePlayerToCentre_at1:
    lda #$80
    sta a:OBJ_PAD_FORCE_FRAMES
    stz a:OBJ_PAD_FORCED
    lda #$01
    sta a:PlayerStartPose
    jsr StagePlayerObjects
    lda a:OBJ_X_LO
    clc
    adc #$02
    and #$FC
    cmp #$80
    beq StagePlayerToCentre_at2
    lda a:OBJ_X_LO
    bmi StagePlayerToCentre_at3
    lda #$FC
    sta a:ScrollStepXHi
    stz a:ScrollStepXLo
    lda a:OBJ_X_LO
    clc
    adc #$04
    sta a:OBJ_X_LO
    bra StagePlayerToCentre_at4
StagePlayerToCentre_at3:
    lda #$04
    sta a:ScrollStepXHi
    stz a:ScrollStepXLo
    lda a:OBJ_X_LO
    clc
    adc #$FC
    sta a:OBJ_X_LO
StagePlayerToCentre_at4:
    jsr StageCameraStep
    lda a:OBJ_X_LO
    clc
    adc #$02
    and #$F8
    cmp #$80
    bne StagePlayerToCentre_end
StagePlayerToCentre_at2:
    rmb7 z:zField8E
    stz a:OBJ_PAD_FORCE_FRAMES
    stz a:PlayerStartPose
StagePlayerToCentre_end:
    rts

; @routine 024E:6AE StagePausedIdle
; @run 6B:8EAE
; WHAT IT DOES: Returns: the paused frame's end while the pause screen is not up.
; HOW: One RTS.
; WAITS: no.
; Entry: anything. Exit: all registers and flags kept.
StagePausedIdle:
    rts

; @routine 024E:6EA StagePausedStep
; @run 6B:8EEA
; WHAT IT DOES: The paused game's frame: nothing until the pause screen is up (zPaused bit 1), then its sequence.
; HOW: BBR1 of zPaused to the RTS at $8EAE; else on into StepStageSequence.
; WAITS: as StepStageSequence.
; Entry: X and Y the caller's. Exit: bit 1 clear: all kept. Else as StepStageSequence leaves them.
StagePausedStep:
    bbr1 zPaused,StagePausedIdle
