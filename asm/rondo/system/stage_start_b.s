; @routine 05BD:02D StageStart05BD
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Secret stage 02), as StageStart with this overlay's section order table at $9E77.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart05BDOrder = $9E77 ; the section record by the step through the stage
StageStart05BD:
    jsr VecResetGameState
    jsr StartStageListsC
    jsr ExpandStageVramRunsC
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsC
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart05BDOrder,y
    sta z:zField86
    jsr ResetStageVariables05BD
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 071D:02D StageStart071D
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Secret stage 03), as StageStart with this overlay's section order table at $9F3B.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart071DOrder = $9F3B ; the section record by the step through the stage
StageStart071D:
    jsr VecResetGameState
    jsr StartStageListsD
    jsr ExpandStageVramRunsD
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsD
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart071DOrder,y
    sta z:zField86
    jsr ResetStageVariables071D
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 087D:02D StageStart087D
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Secret stage 04), as StageStart with this overlay's section order table at $9E64.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart087DOrder = $9E64 ; the section record by the step through the stage
StageStart087D:
    jsr VecResetGameState
    jsr StartStageListsF
    jsr ExpandStageVramRunsF
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsF
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart087DOrder,y
    sta z:zField86
    jsr ResetStageVariables087D
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 09DD:02D StageStart09DD
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Secret stage 05), as StageStart with this overlay's section order table at $9EAB.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart09DDOrder = $9EAB ; the section record by the step through the stage
StageStart09DD:
    jsr VecResetGameState
    jsr StartStageListsH
    jsr ExpandStageVramRunsH
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsH
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart09DDOrder,y
    sta z:zField86
    jsr ResetStageVariables09DD
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 0B3D:02D StageStart0B3D
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Secret stage 06), as StageStart with this overlay's section order table at $9E76.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart0B3DOrder = $9E76 ; the section record by the step through the stage
StageStart0B3D:
    jsr VecResetGameState
    jsr StartStageListsJ
    jsr ExpandStageVramRunsJ
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsJ
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart0B3DOrder,y
    sta z:zField86
    jsr ResetStageVariables0B3D
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 0C9D:02D StageStart0C9D
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Secret stage 07), as StageStart with this overlay's section order table at $9E79.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart0C9DOrder = $9E79 ; the section record by the step through the stage
StageStart0C9D:
    jsr VecResetGameState
    jsr StartStageListsL
    jsr ExpandStageVramRunsL
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsL
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart0C9DOrder,y
    sta z:zField86
    jsr ResetStageVariables0C9D
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 0DFD:02D StageStart0DFD
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Secret stage 08), as StageStart with this overlay's section order table at $9EA7.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart0DFDOrder = $9EA7 ; the section record by the step through the stage
StageStart0DFD:
    jsr VecResetGameState
    jsr StartStageListsN
    jsr ExpandStageVramRunsN
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsN
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart0DFDOrder,y
    sta z:zField86
    jsr ResetStageVariables0DFD
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 0EAD:02D StageStart0EAD
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Stage 09), as StageStart with this overlay's section order table at $9DF6.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart0EADOrder = $9DF6 ; the section record by the step through the stage
StageStart0EAD:
    jsr VecResetGameState
    jsr StartStageListsO
    jsr ExpandStageVramRunsO
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01
    jsr SpawnStageObjectsRows34
    jsr SetUpStageObjectsO
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart0EADOrder,y
    sta z:zField86
    jsr ResetStageVariables0EAD
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 0F5D:02D StageStart0F5D
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Secret stage 09), as StageStart with this overlay's section order table at $9F1C.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart0F5DOrder = $9F1C ; the section record by the step through the stage
StageStart0F5D:
    jsr VecResetGameState
    jsr StartStageListsP
    jsr ExpandStageVramRunsP
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsP
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart0F5DOrder,y
    sta z:zField86
    jsr ResetStageVariables0F5D
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 100D:02D StageStart100D
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Stage 0A), as StageStart with this overlay's section order table at $9C75.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart100DOrder = $9C75 ; the section record by the step through the stage
StageStart100D:
    jsr VecResetGameState
    jsr StartStageListsQ
    jsr ExpandStageVramRunsQ
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01
    jsr SpawnStageObjectsRows34
    jsr SetUpStageObjectsQ
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart100DOrder,y
    sta z:zField86
    jsr ResetStageVariables100D
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 10BD:02D StageStart10BD
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Secret stage 0A), as StageStart with this overlay's section order table at $9F06.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart10BDOrder = $9F06 ; the section record by the step through the stage
StageStart10BD:
    jsr VecResetGameState
    jsr StartStageListsR
    jsr ExpandStageVramRunsR
    jsr InitStageB
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsR
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart10BDOrder,y
    sta z:zField86
    jsr ResetStageVariables10BD
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 116D:02D StageStart116D
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Stage 0B), as StageStart with this overlay's section order table at $9C77.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart116DOrder = $9C77 ; the section record by the step through the stage
StageStart116D:
    jsr VecResetGameState
    jsr StartStageListsS
    jsr ExpandStageVramRunsS
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01
    jsr SpawnStageObjectsRows34
    jsr SetUpStageObjectsS
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart116DOrder,y
    sta z:zField86
    jsr ResetStageVariables116D
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 12CD:02D StageStart12CD
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Stage 0C), as StageStart with this overlay's section order table at $9F35.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart12CDOrder = $9F35 ; the section record by the step through the stage
StageStart12CD:
    jsr VecResetGameState
    jsr StartStageListsT
    jsr ExpandStageVramRunsT
    jsr InitStageC
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsT
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart12CDOrder,y
    sta z:zField86
    jsr ResetStageVariables12CD
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 160D:02D StageStart160D
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Stage 0E), as StageStart with this overlay's section order table at $9CA5.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart160DOrder = $9CA5 ; the section record by the step through the stage
StageStart160D:
    jsr VecResetGameState
    jsr StartStageListsU
    jsr ExpandStageVramRunsU
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01
    jsr SpawnStageObjectsRows34
    jsr SetUpStageObjectsU
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart160DOrder,y
    sta z:zField86
    jsr ResetStageVariables160D
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 16BD:02D StageStart16BD
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Secret stage 0E), as StageStart with this overlay's section order table at $9F42.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart16BDOrder = $9F42 ; the section record by the step through the stage
StageStart16BD:
    jsr VecResetGameState
    jsr StartStageListsV
    jsr ExpandStageVramRunsT
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsV
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart16BDOrder,y
    sta z:zField86
    jsr ResetStageVariables16BD
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 18CD:02D StageStart18CD
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Stage 10), as StageStart with this overlay's section order table at $9EA3.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart18CDOrder = $9EA3 ; the section record by the step through the stage
StageStart18CD:
    jsr VecResetGameState
    jsr StartStageListsW
    jsr ExpandStageVramRunsV
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsW
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart18CDOrder,y
    sta z:zField86
    jsr ResetStageVariables18CD
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain
