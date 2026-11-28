; @routine 024E:25F SetUpStageObjects
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
SetUpStageObjectsValues = $AC95 ; table SetUpStageObjects reads
StageObjectRecords = $9BD2 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjects:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecords,y
    sta a:LevelStreamLo
    lda a:StageObjectRecords+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecords+2,y
    sta a:LevelAxis
    lda a:StageObjectRecords+3,y
    beq SetUpStageObjects_at0
    ldx #$1F
SetUpStageObjects_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjects_at1
SetUpStageObjects_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:SetUpStageObjectsValues
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 050E:25F SetUpStageObjectsB
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9D01 and its second bank offset is the byte at $ACCA.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
SetUpStageObjectsBValues = $ACCA ; table SetUpStageObjectsB reads
StageObjectRecordsB = $9D01 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsB:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsB,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsB+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsB+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsB+3,y
    beq SetUpStageObjectsB_at0
    ldx #$1F
SetUpStageObjectsB_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsB_at1
SetUpStageObjectsB_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:SetUpStageObjectsBValues
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 05BE:25F SetUpStageObjectsC
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9EBD and its second bank offset is the byte at $AC0E.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsC = $9EBD ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsC:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsC,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsC+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsC+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsC+3,y
    beq SetUpStageObjectsC_at0
    ldx #$1F
SetUpStageObjectsC_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsC_at1
SetUpStageObjectsC_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AC0E
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 071E:25F SetUpStageObjectsD
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9F90 and its second bank offset is the byte at $AC16.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsD = $9F90 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsD:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsD,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsD+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsD+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsD+3,y
    beq SetUpStageObjectsD_at0
    ldx #$1F
SetUpStageObjectsD_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsD_at1
SetUpStageObjectsD_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AC16
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 07CE:25F SetUpStageObjectsE
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9E19 and its second bank offset is the byte at $ABDC.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
SetUpStageObjectsEValues = $ABDC ; table SetUpStageObjectsE reads
StageObjectRecordsE = $9E19 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsE:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsE,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsE+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsE+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsE+3,y
    beq SetUpStageObjectsE_at0
    ldx #$1F
SetUpStageObjectsE_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsE_at1
SetUpStageObjectsE_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:SetUpStageObjectsEValues
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 087E:25F SetUpStageObjectsF
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9E90 and its second bank offset is the byte at $AB03.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsF = $9E90 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsF:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsF,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsF+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsF+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsF+3,y
    beq SetUpStageObjectsF_at0
    ldx #$1F
SetUpStageObjectsF_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsF_at1
SetUpStageObjectsF_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AB03
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 092E:25F SetUpStageObjectsG
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9E00 and its second bank offset is the byte at $AC8E.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
SetUpStageObjectsGValues = $AC8E ; table SetUpStageObjectsG reads
StageObjectRecordsG = $9E00 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsG:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsG,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsG+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsG+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsG+3,y
    beq SetUpStageObjectsG_at0
    ldx #$1F
SetUpStageObjectsG_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsG_at1
SetUpStageObjectsG_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:SetUpStageObjectsGValues
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 09DE:25F SetUpStageObjectsH
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9ED6 and its second bank offset is the byte at $AC5E.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsH = $9ED6 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsH:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsH,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsH+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsH+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsH+3,y
    beq SetUpStageObjectsH_at0
    ldx #$1F
SetUpStageObjectsH_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsH_at1
SetUpStageObjectsH_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AC5E
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 0A8E:25F SetUpStageObjectsI
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9E30 and its second bank offset is the byte at $ACE3.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsI = $9E30 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsI:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsI,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsI+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsI+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsI+3,y
    beq SetUpStageObjectsI_at0
    ldx #$1F
SetUpStageObjectsI_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsI_at1
SetUpStageObjectsI_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$ACE3
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 0B3E:25F SetUpStageObjectsJ
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9EA1 and its second bank offset is the byte at $AB27.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsJ = $9EA1 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsJ:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsJ,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsJ+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsJ+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsJ+3,y
    beq SetUpStageObjectsJ_at0
    ldx #$1F
SetUpStageObjectsJ_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsJ_at1
SetUpStageObjectsJ_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AB27
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 0BEE:25F SetUpStageObjectsK
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9E05 and its second bank offset is the byte at $ACA7.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsK = $9E05 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsK:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsK,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsK+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsK+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsK+3,y
    beq SetUpStageObjectsK_at0
    ldx #$1F
SetUpStageObjectsK_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsK_at1
SetUpStageObjectsK_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$ACA7
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 0C9E:25F SetUpStageObjectsL
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9EA4 and its second bank offset is the byte at $AC8B.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsL = $9EA4 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsL:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsL,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsL+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsL+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsL+3,y
    beq SetUpStageObjectsL_at0
    ldx #$1F
SetUpStageObjectsL_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsL_at1
SetUpStageObjectsL_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AC8B
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 0D4E:25F SetUpStageObjectsM
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9EA1 and its second bank offset is the byte at $AD91.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsM = $9EA1 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsM:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsM,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsM+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsM+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsM+3,y
    beq SetUpStageObjectsM_at0
    ldx #$1F
SetUpStageObjectsM_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsM_at1
SetUpStageObjectsM_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AD91
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 0DFE:25F SetUpStageObjectsN
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9ED3 and its second bank offset is the byte at $AB70.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsN = $9ED3 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsN:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsN,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsN+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsN+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsN+3,y
    beq SetUpStageObjectsN_at0
    ldx #$1F
SetUpStageObjectsN_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsN_at1
SetUpStageObjectsN_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AB70
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 0EAE:25F SetUpStageObjectsO
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9F36 and its second bank offset is the byte at $AD3F.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsO = $9F36 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsO:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsO,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsO+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsO+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsO+3,y
    beq SetUpStageObjectsO_at0
    ldx #$1F
SetUpStageObjectsO_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsO_at1
SetUpStageObjectsO_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AD3F
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 0F5E:25F SetUpStageObjectsP
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9F47 and its second bank offset is the byte at $ABA7.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsP = $9F47 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsP:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsP,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsP+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsP+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsP+3,y
    beq SetUpStageObjectsP_at0
    ldx #$1F
SetUpStageObjectsP_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsP_at1
SetUpStageObjectsP_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$ABA7
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 100E:25F SetUpStageObjectsQ
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9D8A and its second bank offset is the byte at $AD1B.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsQ = $9D8A ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsQ:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsQ,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsQ+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsQ+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsQ+3,y
    beq SetUpStageObjectsQ_at0
    ldx #$1F
SetUpStageObjectsQ_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsQ_at1
SetUpStageObjectsQ_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AD1B
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 10BE:25F SetUpStageObjectsR
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9F4A and its second bank offset is the byte at $ACC9.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsR = $9F4A ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsR:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsR,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsR+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsR+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsR+3,y
    beq SetUpStageObjectsR_at0
    ldx #$1F
SetUpStageObjectsR_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsR_at1
SetUpStageObjectsR_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$ACC9
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 116E:25F SetUpStageObjectsS
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9D54 and its second bank offset is the byte at $AD36.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsS = $9D54 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsS:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsS,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsS+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsS+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsS+3,y
    beq SetUpStageObjectsS_at0
    ldx #$1F
SetUpStageObjectsS_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsS_at1
SetUpStageObjectsS_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AD36
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 12CE:25F SetUpStageObjectsT
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9F7E and its second bank offset is the byte at $ACF7.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsT = $9F7E ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsT:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsT,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsT+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsT+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsT+3,y
    beq SetUpStageObjectsT_at0
    ldx #$1F
SetUpStageObjectsT_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsT_at1
SetUpStageObjectsT_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$ACF7
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 160E:25F SetUpStageObjectsU
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9E26 and its second bank offset is the byte at $AD31.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsU = $9E26 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsU:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsU,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsU+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsU+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsU+3,y
    beq SetUpStageObjectsU_at0
    ldx #$1F
SetUpStageObjectsU_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsU_at1
SetUpStageObjectsU_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AD31
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 16BE:25F SetUpStageObjectsV
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9F6D and its second bank offset is the byte at $AC65.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsV = $9F6D ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsV:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsV,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsV+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsV+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsV+3,y
    beq SetUpStageObjectsV_at0
    ldx #$1F
SetUpStageObjectsV_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsV_at1
SetUpStageObjectsV_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AC65
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects

; @routine 18CE:25F SetUpStageObjectsW
; @run 6B:8A5F
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and
; the axis from the section's record, clears the event flags when the record says to, maps the
; object banks, walks the whole stream once, and runs the first frame of objects.
; HOW: A four-byte record per zField83 gives the stream's position (two bytes), the axis and a
; clear-the-flags byte. Two bank pairs are mapped as StageScrollStep does; StreamFullWalk
; then walks the stream from the scroll, and StepScrollAndRunObjects ends it.
; The same code as SetUpStageObjects; this overlay's section records are at $9F31 and its second bank offset is the byte at $AC49.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StageObjectRecordsW = $9F31 ; four bytes by zField83: the stream's place, the axis, clear the flags
SetUpStageObjectsW:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageObjectRecordsW,y
    sta a:LevelStreamLo
    lda a:StageObjectRecordsW+1,y
    sta a:LevelStreamHi
    lda a:StageObjectRecordsW+2,y
    sta a:LevelAxis
    lda a:StageObjectRecordsW+3,y
    beq SetUpStageObjectsW_at0
    ldx #$1F
SetUpStageObjectsW_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjectsW_at1
SetUpStageObjectsW_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AC49
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp StepScrollAndRunObjects
