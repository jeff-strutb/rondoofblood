; @routine 050D:02D StageStart050D
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Stage 02), as StageStart with this overlay's section order table at $9C20.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart050DOrder = $9C20 ; the section record by the step through the stage
StageStart050D:
    jsr VecResetGameState
    jsr StartStageListsB
    jsr ExpandStageVramRunsB
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsB
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart050DOrder,y
    sta z:zField86
    jsr ResetStageVariables050D
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 07CD:02D StageStart07CD
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Stage 04), as StageStart with this overlay's section order table at $9CD6.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart07CDOrder = $9CD6 ; the section record by the step through the stage
StageStart07CD:
    jsr VecResetGameState
    jsr StartStageListsE
    jsr ExpandStageVramRunsE
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsE
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart07CDOrder,y
    sta z:zField86
    jsr ResetStageVariables07CD
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 092D:02D StageStart092D
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Stage 05), as StageStart with this overlay's section order table at $9CD1.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart092DOrder = $9CD1 ; the section record by the step through the stage
StageStart092D:
    jsr VecResetGameState
    jsr StartStageListsG
    jsr ExpandStageVramRunsG
    jsr InitStage
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsG
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart092DOrder,y
    sta z:zField86
    jsr ResetStageVariables092D
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 0A8D:02D StageStart0A8D
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Stage 06), as StageStart with this overlay's section order table at $9CAC.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart0A8DOrder = $9CAC ; the section record by the step through the stage
StageStart0A8D:
    jsr VecResetGameState
    jsr StartStageListsI
    jsr ExpandStageVramRunsI
    jsr InitStageB
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsI
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart0A8DOrder,y
    sta z:zField86
    jsr ResetStageVariables0A8D
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 0BED:02D StageStart0BED
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Stage 07), as StageStart with this overlay's section order table at $9CC4.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart0BEDOrder = $9CC4 ; the section record by the step through the stage
StageStart0BED:
    jsr VecResetGameState
    jsr StartStageListsK
    jsr ExpandStageVramRunsK
    jsr InitStageB
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsK
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart0BEDOrder,y
    sta z:zField86
    jsr ResetStageVariables0BED
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

; @routine 0D4D:02D StageStart0D4D
; @run 6B:802D
; @stop 806A
; WHAT IT DOES: A stage's start (Stage 08), as StageStart with this overlay's section order table at $9CE7.
; HOW: StageStartAt.
; WAITS: where its calls wait.
; Entry: nothing. Exit: as StageStart.
StageStart0D4DOrder = $9CE7 ; the section record by the step through the stage
StageStart0D4D:
    jsr VecResetGameState
    jsr StartStageListsM
    jsr ExpandStageVramRunsM
    jsr InitStageB
    jsr MapOverlayBanks5And6
    jsr MariaVecStart
    jsr SpawnStageObjectsRows01Long
    jsr SpawnStageObjectsRows34Long
    jsr SetUpStageObjectsM
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    ldy z:zField82
    lda a:StageStart0D4DOrder,y
    sta z:zField86
    jsr ResetStageVariables0D4D
    bbr0 zField8E,StageStartPlain
    lda z:zStage
    bmi StageStartMusicByStage
    cmp #$0C
    beq StageStartMusicKeep
    cmp #$10
    beq StageStartMusicBoss
    jmp VecCdPlayAgain

