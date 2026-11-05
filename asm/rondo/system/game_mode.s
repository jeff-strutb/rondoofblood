; @routine 0045:105 FrameModeDispatch
; @run 69:5105
; WHAT IT DOES: Runs this frame's game mode.
; HOW: Counts zFrameTick, then jumps through the table by zGameMode.
; Entry: nothing. Exit: as the handler leaves them.
FrameModeDispatch:
    inc z:zFrameTick
    lda z:zGameMode
    asl a
    tax
    jmp (GameModeHandlers,x)

; @routine 0045:124 ModeBoot
; @run 69:5124
; WHAT IT DOES: Mode 0: the boot overlay's frame, then the display off, the screen size
; set and on to the title; a negative zModeDone goes on to the title's sub-mode 3.
; HOW: Overlay frame, WaitVblankDisplayOff, the BIOS's screen size, SetGameMode.
; Entry: nothing. Exit: as SetGameMode, or with zGameSubMode 3 and A 3.
ModeBoot:
    jsr FrameOverlay8000
    lda z:zModeDone
    bne ModeBoot_at0
    rts
ModeBoot_at0:
    jsr WaitVblankDisplayOff
    lda #$04
    sta z:zDisplayDelay
    lda #$05
    jsr BiosExScrSiz
    stz z:zStage
    lda #$FF
    sta z:zCharacterLoaded
    stz z:zCharacter
    ldx z:zModeDone
    lda #$02
    jsr SetGameMode
    txa
    bpl ModeBoot_at1
    lda #$03
    sta z:zGameSubMode
ModeBoot_at1:
    rts

; @routine 0045:14F ModeLogo
; @run 69:514F
; WHAT IT DOES: Mode 1: the overlay's frame, then the stage's load.
; HOW: Overlay frame at its second entry; zField80 cleared; SetGameMode.
; Entry: nothing. Exit: as SetGameMode.
ModeLogo:
    jsr FrameOverlay8003
    lda z:zModeDone
    bne ModeLogo_at0
    rts
ModeLogo_at0:
    stz z:zField80
    lda #$04
    jmp SetGameMode

; @routine 0045:15E ModeAttract
; @run 69:515E
; WHAT IT DOES: Mode 2: the title's frame. The overlay's answer picks what comes next:
; zero runs the overlay's other entry; positive restarts the attract's load; negative
; starts the next demo stage, or on the first pass loads the title's scenes.
; HOW: The overlay at $8006, then zAttractStep; the five demo stages are a table.
; Entry: nothing. Exit: as what ran last.
ModeAttract:
    jsr FrameOverlay8006
    lda z:zModeDone
    bmi ModeAttract_at0
    bne ModeAttract_at1
    jmp FrameOverlay8009
ModeAttract_at1:
    lda #$01
    jmp SetGameMode
ModeAttract_at0:
    lda z:zAttractStep
    beq ModeAttract_at2
    jsr ResetScoreCounters
    stz z:zCharacter
    lda #$05
    jsr LoadSceneScript
    ldy z:zAttractStep
    lda a:AttractStages,y
    sta z:zStage
    jsr StageEnter
    inc z:zAttractStep
    lda z:zAttractStep
    cmp #$05
    bne ModeAttract_at3
    stz z:zAttractStep
ModeAttract_at3:
    inc z:zGameRunning
    jmp NextGameMode
ModeAttract_at2:
    inc z:zAttractStep
    stz z:zMusicWanted
    jsr VecResetGameState
    lda #$07
    jsr LoadSceneScript
    lda #$08
    jsr LoadSceneScript
    lda #$09
    jsr LoadSceneScript
    jsr AdpcmLoadFixed
    jmp NextGameMode

; @routine 0045:1B6 ModeAttractPlay
; @run 69:51B6
; WHAT IT DOES: Mode 3: a demo's title frames; Run or the overlay's end stops the demo,
; fades the CD and goes to mode 8.
; HOW: The BIOS's joypad byte, the overlay's frame, a sound fade, the CD fade.
; Entry: nothing. Exit: as what ran last.
ModeAttractPlay:
    lda a:BiosJoyTrigger
    and #$08
    bne ModeAttractPlay_at0
    jsr FrameOverlay8000
    lda z:zModeDone
    bne ModeAttractPlay_at0
    rts
ModeAttractPlay_at0:
    stz z:zGameRunning
    lda #$FE
    jsr VecSoundCommand
    lda #$0C
    jsr BiosCdFade
    lda #$40
    sta z:zWaitTimer
    jsr VecResetGameState
    lda #$08
    jmp SetGameMode

; @routine 0045:1DD ModeStageLoad
; @run 69:51DD
; WHAT IT DOES: Mode 4: the menu overlay's frame, then what zModeEvents asks for: the
; name entry, a new game from the menu, or the first stage.
; HOW: The overlay's fifth entry, then bits 3 and 4 of zModeEvents.
; Entry: nothing. Exit: as what ran last.
ModeStageLoad:
    jsr OverlayTerrainLookup
    lda z:zModeDone
    bne ModeStageLoad_at0
    rts
ModeStageLoad_at0:
    bbs3 zModeEvents,ModeStageLoadScreens
    bbs4 zModeEvents,ModeStageLoadContinue
    lda #$05
    jsr LoadSceneScript

; @routine 0045:1F0 StageEnterNext
; @run 69:51F0
; WHAT IT DOES: Enters the stage in zStage with a clean score count and goes on a mode.
; HOW: StageEnter, ResetScoreCounters, NextGameMode.
; Entry: nothing. Exit: as NextGameMode.
StageEnterNext:
    jsr StageEnter
    jsr ResetScoreCounters
    jmp NextGameMode

; @routine 0045:1F9 ModeStageLoadScreens
; @run 69:51F9
; WHAT IT DOES: The path of ModeStageLoad for bit 3 of zModeEvents: loads the screens'
; scenes and goes to mode 10.
; HOW: Clears the bit and the wanted ADPCM, ResetGameState, three scenes, the fixed ADPCM.
; Entry: from ModeStageLoad. Exit: as SetGameMode.
ModeStageLoadScreens:
    rmb3 z:zModeEvents
    stz z:zMusicWanted
    jsr VecResetGameState
    lda #$07
    jsr LoadSceneScript
    lda #$08
    jsr LoadSceneScript
    lda #$09
    jsr LoadSceneScript
    jsr AdpcmLoadFixed
    lda #$0A
    jmp SetGameMode

; @routine 0045:217 ModeStageLoadContinue
; @run 69:5217
; WHAT IT DOES: The path of ModeStageLoad for bit 4 of zModeEvents: enters the stage and
; goes to the intro mode.
; HOW: ResetScoreCounters, a scene, StageEnter, zGameRunning counted, SetGameMode.
; Entry: from ModeStageLoad. Exit: as SetGameMode.
ModeStageLoadContinue:
    jsr ResetScoreCounters
    lda #$05
    jsr LoadSceneScript
    jsr StageEnter
    inc z:zGameRunning
    lda #$03
    jmp SetGameMode

; @routine 0045:229 ModeStageDone
; @run 69:5229
; WHAT IT DOES: The stage's overlay finished with a positive answer: goes on a mode,
; except on the bonus stage, which marks it and, unless the save slot's flag is set,
; loads the ending's scenes and goes to mode 9.
; HOW: Compares zStage; StageClearBit in StagesVisited; the save's flag at $7D25.
; Entry: from ModeStage. Exit: as what ran last.
ModeStageDone:
    lda z:zStage
    cmp #$8A
    beq ModeStageDone_at0
ModeStageDone_at2:
    jmp NextGameMode
ModeStageDone_at0:
    jsr StageClearBit
    ora a:StagesVisited,x
    sta a:StagesVisited,x
    jsr VecResetGameState
    jsr VecSoundStopAll
    lda z:zSaveSlot
    bmi ModeStageDone_at1
    jsr SaveProgress
    ldy z:zSaveSlot
    ldx a:SaveSlotOffsets,y
    lda a:SaveScoreTotal+6,x
    bne ModeStageDone_at2
ModeStageDone_at1:
    lda #$FF
    sta z:zCharacterLoaded
    lda #$5C
    jsr LoadSceneScript
    lda #$5D
    jsr LoadSceneScript
    lda #$09
    jmp SetGameMode

; @routine 0045:265 ModeStage
; @run 69:5265
; WHAT IT DOES: Mode 5: a stage's frame. A negative answer goes to mode 7, a positive one
; to ModeStageDone. At zero, bits of zModeEvents ask for a boss's ADPCM and scene, a
; screen's scenes, or the stage's clear with the score saved.
; HOW: Bits 0, 1 and 2 of zModeEvents, each cleared as it is taken.
; Entry: nothing. Exit: as what ran last.
ModeStage:
    jsr FrameOverlay8000
    lda z:zModeDone
    bmi ModeStage_at0
    bne ModeStageDone
    bbr0 zModeEvents,ModeStage_at1
    rmb0 z:zModeEvents
    lda #$22
    clc
    adc z:zCharacter
    jsr AdpcmLoad
    lda #$5E
    jmp LoadSceneScript
ModeStage_at1:
    bbs1 zModeEvents,ModeStage_at2
    bbs2 zModeEvents,ModeStage_at3
    rts
ModeStage_at0:
    lda #$07
    jmp SetGameMode
ModeStage_at2:
    rmb1 z:zModeEvents
    jsr VecResetGameState
    lda #$FF
    sta z:zMusicWanted
    sta z:zCharacterLoaded
    lda z:zStage
    cmp #$04
    beq ModeStage_at4
    cmp #$07
    beq ModeStage_at5
    cmp #$09
    beq ModeStage_at6
    lda #$0E
    jsr LoadSceneScript
    lda #$0F
    jsr LoadSceneScript
    bra ModeStage_at7
ModeStage_at6:
    lda #$0C
    jsr LoadSceneScript
    lda #$0D
    jsr LoadSceneScript
    bra ModeStage_at7
ModeStage_at5:
    lda #$0A
    jsr LoadSceneScript
    lda #$0B
    jsr LoadSceneScript
    bra ModeStage_at7
ModeStage_at4:
    lda #$02
    jsr LoadSceneScript
    lda #$03
    jsr LoadSceneScript
ModeStage_at7:
    lda #$09
    jmp SetGameMode
ModeStage_at3:
    rmb2 z:zModeEvents
    bsr StageClearBit
    ora a:StagesVisited,x
    sta a:StagesVisited,x
    lda z:zSaveSlot
    bmi ModeStage_at8
    jsr SaveScore
ModeStage_at8:
    lda z:zCharacter
    asl a
    adc #$58
    jsr LoadSceneScript
    lda z:zCharacter
    asl a
    adc #$59
    jsr LoadSceneScript
    lda #$09
    jmp SetGameMode

; @routine 0045:2FE ModeStageCleared
; @run 69:52FE
; WHAT IT DOES: Mode 6: marks the stage visited and runs into StageClearedNext.
; HOW: StageClearBit into StagesVisited.
; Entry: nothing. Exit: as StageClearedNext.
ModeStageCleared:
    bsr StageClearBit
    ora a:StagesVisited,x
    sta a:StagesVisited,x

; @routine 0045:306 StageClearedNext
; @run 69:5306
; WHAT IT DOES: Enters the next stage, marks it and saves, and goes to the stage mode; if
; there is no next stage, back to the title's mode 0.
; HOW: StageEnter; zStage zero is the end. StageClearBit into StagesReached.
; Entry: nothing. Exit: as SetGameMode.
StageClearedNext:
    jsr StageEnter
    lda z:zStage
    beq StageClearedNext_at0
    bsr StageClearBit
    ora a:StagesReached,x
    sta a:StagesReached,x
    lda z:zSaveSlot
    bmi StageClearedNext_at1
    jsr SaveProgress
StageClearedNext_at1:
    lda #$05
    jmp SetGameMode
StageClearedNext_at0:
    cla
    jmp SetGameMode

; @routine 0045:325 StageClearBit
; @run 69:5325
; WHAT IT DOES: Finds zStage's bit in the visited tables: the byte in X, the bit in A.
; HOW: Bits 2-4 of the stage give the byte, five further on for the second character;
; bits 0-1 the bit; a stage with bit 7 set takes the high nibble.
; Entry: nothing. Exit: A the bit, X the byte, Y zStage, N and Z from Y or the last shift;
; C from the last shift or the add; V from the add. Clobbers zTemp00.
StageClearBit:
    cla
    ldx z:zCharacter
    beq StageClearBit_at0
    lda #$05
StageClearBit_at0:
    sta z:zTemp00
    lda z:zStage
    lsr a
    lsr a
    and #$07
    clc
    adc z:zTemp00
    tax
    lda z:zStage
    and #$03
    tay
    lda #$01
StageClearBit_at2:
    dey
    bmi StageClearBit_at1
    asl a
    bra StageClearBit_at2
StageClearBit_at1:
    ldy z:zStage
    bpl StageClearBit_at3
    asl a
    asl a
    asl a
    asl a
StageClearBit_at3:
    rts

; @routine 0045:34E ModeDead
; @run 69:534E
; WHAT IT DOES: Mode 7: the continue screen's frame. Then counts a continue used, in
; BCD up to 999, saves, and goes back into the stage, or at the last continue to the
; title. A stage with bit 7 set restarts from zStageRequested.
; HOW: Maps the overlay's bank; the continues at $3F41-$3F42; $3F24 picks what follows.
; Entry: nothing. Exit: as what ran last.
ModeDead:
    lda #$0D
    clc
    adc z:zRamBaseBank
    tam #$10
    jsr FrameOverlay8000
    lda z:zModeDone
    bne ModeDead_at0
    rts
ModeDead_at0:
    sed
    lda a:ContinuesLo
    clc
    adc #$01
    sta a:ContinuesLo
    cld
    bcc ModeDead_at1
    inc a:ContinuesHi
ModeDead_at1:
    lda a:ContinuesHi
    cmp #$0A
    bne ModeDead_at2
    lda #$99
    sta a:ContinuesLo
    lda #$09
    sta a:ContinuesHi
ModeDead_at2:
    lda z:zSaveSlot
    bmi ModeDead_at3
    jsr SaveScore
ModeDead_at3:
    lda a:ContinueChoice
    beq ModeDead_at4
    dec a
    beq ModeDead_at5
    lda z:zCharacter
    eor #$01
    sta z:zCharacter
ModeDead_at4:
    lda z:zStage
    bpl ModeDead_at6
    lda z:zStageRequested
    and #$7F
ModeDead_at6:
    ora #$40
    sta z:zStage
    jsr ResetScoreCounters
    lda z:zStage
    cmp #$43
    bne ModeDead_at7
    lda #$42
    sta z:zStage
ModeDead_at7:
    lda #$04
    jsr SetGameMode
    jmp StageEnterNext
ModeDead_at5:
    jsr VecResetGameState

; @routine 0045:3B7 ModeBackToTitle
; @run 69:53B7
; WHAT IT DOES: Goes back to the title: forgets what is loaded, loads the menu's scenes
; and ADPCM, and sets mode 0.
; HOW: Two scenes, an ADPCM, SetGameMode.
; Entry: nothing. Exit: as SetGameMode.
ModeBackToTitle:
    lda #$FF
    sta z:zMusicWanted
    sta z:zCharacterLoaded
    lda #$00
    sta z:zStage
    lda #$04
    jsr LoadSceneScript
    lda #$06
    jsr LoadSceneScript
    lda #$24
    jsr AdpcmLoad
    cla
    jmp SetGameMode

; @routine 0045:3D7 SaveProgress
; @run 69:53D7
; WHAT IT DOES: Writes the character's progress to the save slot in zSaveSlot.
; HOW: Reads the backup file to RAM, copies into the slot, writes it back.
; Entry: nothing. Exit: as BackupWrite.
SaveProgress:
    jsr VecBackupRead
    ldy z:zSaveSlot
    ldx a:SaveSlotOffsets,y
    bra SaveScore_at0

; @routine 0045:3E1 SaveScore
; @run 69:53E1
; WHAT IT DOES: As SaveProgress, and first adds the stage's score to the slot's total, in
; BCD, stopping at 999999.
; HOW: A 24-bit BCD add from zScoreAddLo-Hi.
; Entry: nothing. Exit: as BackupWrite.
SaveScore:
    jsr VecBackupRead
    ldy z:zSaveSlot
    ldx a:SaveSlotOffsets,y
    sed
    lda z:zScoreAddLo
    clc
    adc a:SaveScoreTotal,x
    sta a:SaveScoreTotal,x
    lda z:zScoreAddHi
    adc a:SaveScoreTotal+1,x
    sta a:SaveScoreTotal+1,x
    cla
    adc a:SaveScoreTotal+2,x
    sta a:SaveScoreTotal+2,x
    bcc SaveScore_at1
    lda #$99
    sta a:SaveScoreTotal,x
    sta a:SaveScoreTotal+1,x
    sta a:SaveScoreTotal+2,x
SaveScore_at1:
    cld
SaveScore_at0:
    lda a:ProgressCharacter
    sta a:SaveBuffer,x
    lda a:ContinuesLo
    sta a:SaveContinues,x
    lda a:ContinuesHi
    sta a:SaveContinues+1,x
    cly
SaveScore_at2:
    lda a:StagesReached,y
    sta a:SaveBuffer+1,x
    inx
    iny
    cpy #$14
    bne SaveScore_at2
    jmp VecBackupWrite

; @routine 0045:432 ModeGameOverWait
; @run 69:5432
; WHAT IT DOES: Mode 8: waits out zWaitTimer, stops the sound, and goes back to the title;
; with bit 4 of zModeEvents, to the stage's load from the start.
; HOW: Counts down; SoundStopAll; ModeBackToTitle.
; Entry: nothing. Exit: on waiting, A kept, N and Z from the count; else as what ran last.
ModeGameOverWait:
    dec z:zWaitTimer
    beq ModeGameOverWait_at0
    rts
ModeGameOverWait_at0:
    jsr VecSoundStopAll
    bbs4 zModeEvents,ModeGameOverWait_at1
ModeGameOverWait_at2:
    jmp ModeBackToTitle
ModeGameOverWait_at1:
    rmb4 z:zModeEvents
    jsr ModeBackToTitle
    stz z:zCharacter
    stz z:zField80
    lda #$04
    jmp SetGameMode

; @routine 0045:44E ModeScreen
; @run 69:544E
; WHAT IT DOES: Mode 9: a screen's frame between stages, then what follows it: back to
; the title from stage $10; from the bonus stage the save's flag and the next stage;
; otherwise the stage's load again with the player's stock restored.
; HOW: Overlay frame, then zStage.
; Entry: nothing. Exit: as what ran last.
ModeScreen:
    jsr FrameOverlay8000
    lda z:zModeDone
    bne ModeScreen_at0
    rts
ModeScreen_at0:
    jsr VecResetGameState
    lda #$05
    jsr LoadSceneScript
    lda z:zStage
    cmp #$10
    beq ModeGameOverWait_at2
    cmp #$8A
    beq ModeScreen_at1
    jsr StageLoad
    stz z:zField99
    lda #$05
    sta z:zField95
    lda #$03
    sta z:zField8A
    lda #$01
    sta z:zField8E
    lda z:zFieldA3
    sta z:zField83
    lda #$05
    jsr SetGameMode
    rts
ModeScreen_at1:
    lda z:zSaveSlot
    bmi ModeScreen_at2
    jsr VecBackupRead
    ldy z:zSaveSlot
    ldx a:SaveSlotOffsets,y
    lda #$01
    sta a:SaveScoreTotal+6,x
    jsr VecBackupWrite
ModeScreen_at2:
    jmp StageClearedNext

; @routine 0045:49A ModeEnding
; @run 69:549A
; WHAT IT DOES: Mode 10: the ending's frame, then the stage in zStage.
; HOW: Overlay frame, ResetGameState, a scene, StageEnter, ResetScoreCounters, SetGameMode.
; Entry: nothing. Exit: as what ran last.
ModeEnding:
    jsr FrameOverlay8000
    lda z:zModeDone
    bne ModeEnding_at0
    rts
ModeEnding_at0:
    stz z:zGameRunning
    jsr VecResetGameState
    lda #$05
    jsr LoadSceneScript
    jsr StageEnter
    jsr ResetScoreCounters
    lda #$05
    jmp SetGameMode

; @routine 0045:4B7 NextGameMode
; @run 69:54B7
; WHAT IT DOES: Goes on to the next game mode.
; HOW: zGameMode plus one, into SetGameMode.
; Entry: nothing. Exit: as SetGameMode, N and Z from the new mode.
NextGameMode:
    lda z:zGameMode
    inc a

; @routine 0045:4BA SetGameMode
; @run 69:54BA
; WHAT IT DOES: Sets the game mode, with its done flag and its sub-mode cleared.
; HOW: Writes zGameMode, clears zModeDone, then zGameSubMode and zSubStep.
; Entry: A mode. Exit: A 0. N, Z, C, V, X and Y kept.
SetGameMode:
    sta z:zGameMode
    stz z:zModeDone
    cla
    bra NextSubMode_at0

; @routine 0045:4C1 NextSubMode
; @run 69:54C1
; WHAT IT DOES: Goes on to the next sub-mode. Nothing in the kernel reaches it; overlays
; may.
; HOW: zGameSubMode plus one, with zSubStep cleared.
; Entry: nothing. Exit: A the sub-mode, N and Z from it. X, Y, C, V kept.
NextSubMode:
    lda z:zGameSubMode
    inc a
NextSubMode_at0:
    sta z:zGameSubMode
    stz z:zSubStep
    rts

; @routine 0045:4C9 ResetScoreCounters
; @run 69:54C9
; WHAT IT DOES: Starts a run's score and stock: score zero, the stock three.
; HOW: Clears the score to add, the score, and zField93.
; Entry: nothing. Exit: A 3, N and Z from it. X, Y, C, V kept.
ResetScoreCounters:
    stz z:zScoreAddLo
    stz z:zScoreAddHi
    stz z:zScore0
    stz z:zScore1
    stz z:zScore2
    lda #$03
    sta z:zStock
    stz z:zField93
    rts

; @routine 0045:587 FadeAndStopSound
; @run 69:5587
; WHAT IT DOES: Fades the CD out, unless zField8E says a stage's track goes on, and stops
; the sound.
; HOW: Bits 3 and 4 of zField8E; the BIOS's fade; SoundCommand.
; Entry: nothing. Exit: as SoundCommand.
FadeAndStopSound:
    lda z:zField8E
    and #$18
    bne StopSound
    lda #$0C
    jsr BiosCdFade

; @routine 0045:592 StopSound
; @run 69:5592
; WHAT IT DOES: Tells the sound driver to stop.
; HOW: Command $FF.
; Entry: nothing. Exit: as SoundCommand.
StopSound:
    lda #$FF
    jmp VecSoundCommand
