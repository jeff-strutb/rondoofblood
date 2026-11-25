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
SetUpStageObjects:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9BD2,y
    sta a:LevelStreamLo
    lda a:$9BD3,y
    sta a:LevelStreamHi
    lda a:$9BD4,y
    sta a:LevelAxis
    lda a:$9BD5,y
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
    lda a:$AC95
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
SetUpStageObjectsB:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9D01,y
    sta a:LevelStreamLo
    lda a:$9D02,y
    sta a:LevelStreamHi
    lda a:$9D03,y
    sta a:LevelAxis
    lda a:$9D04,y
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
    lda a:$ACCA
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
SetUpStageObjectsC:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9EBD,y
    sta a:LevelStreamLo
    lda a:$9EBE,y
    sta a:LevelStreamHi
    lda a:$9EBF,y
    sta a:LevelAxis
    lda a:$9EC0,y
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
SetUpStageObjectsD:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9F90,y
    sta a:LevelStreamLo
    lda a:$9F91,y
    sta a:LevelStreamHi
    lda a:$9F92,y
    sta a:LevelAxis
    lda a:$9F93,y
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
SetUpStageObjectsE:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9E19,y
    sta a:LevelStreamLo
    lda a:$9E1A,y
    sta a:LevelStreamHi
    lda a:$9E1B,y
    sta a:LevelAxis
    lda a:$9E1C,y
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
    lda a:$ABDC
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
SetUpStageObjectsF:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9E90,y
    sta a:LevelStreamLo
    lda a:$9E91,y
    sta a:LevelStreamHi
    lda a:$9E92,y
    sta a:LevelAxis
    lda a:$9E93,y
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
SetUpStageObjectsG:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9E00,y
    sta a:LevelStreamLo
    lda a:$9E01,y
    sta a:LevelStreamHi
    lda a:$9E02,y
    sta a:LevelAxis
    lda a:$9E03,y
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
    lda a:$AC8E
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
SetUpStageObjectsH:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9ED6,y
    sta a:LevelStreamLo
    lda a:$9ED7,y
    sta a:LevelStreamHi
    lda a:$9ED8,y
    sta a:LevelAxis
    lda a:$9ED9,y
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
SetUpStageObjectsI:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9E30,y
    sta a:LevelStreamLo
    lda a:$9E31,y
    sta a:LevelStreamHi
    lda a:$9E32,y
    sta a:LevelAxis
    lda a:$9E33,y
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
SetUpStageObjectsJ:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9EA1,y
    sta a:LevelStreamLo
    lda a:$9EA2,y
    sta a:LevelStreamHi
    lda a:$9EA3,y
    sta a:LevelAxis
    lda a:$9EA4,y
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
SetUpStageObjectsK:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9E05,y
    sta a:LevelStreamLo
    lda a:$9E06,y
    sta a:LevelStreamHi
    lda a:$9E07,y
    sta a:LevelAxis
    lda a:$9E08,y
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
SetUpStageObjectsL:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9EA4,y
    sta a:LevelStreamLo
    lda a:$9EA5,y
    sta a:LevelStreamHi
    lda a:$9EA6,y
    sta a:LevelAxis
    lda a:$9EA7,y
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
SetUpStageObjectsM:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9EA1,y
    sta a:LevelStreamLo
    lda a:$9EA2,y
    sta a:LevelStreamHi
    lda a:$9EA3,y
    sta a:LevelAxis
    lda a:$9EA4,y
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
SetUpStageObjectsN:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9ED3,y
    sta a:LevelStreamLo
    lda a:$9ED4,y
    sta a:LevelStreamHi
    lda a:$9ED5,y
    sta a:LevelAxis
    lda a:$9ED6,y
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
SetUpStageObjectsO:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9F36,y
    sta a:LevelStreamLo
    lda a:$9F37,y
    sta a:LevelStreamHi
    lda a:$9F38,y
    sta a:LevelAxis
    lda a:$9F39,y
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
SetUpStageObjectsP:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9F47,y
    sta a:LevelStreamLo
    lda a:$9F48,y
    sta a:LevelStreamHi
    lda a:$9F49,y
    sta a:LevelAxis
    lda a:$9F4A,y
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
SetUpStageObjectsQ:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9D8A,y
    sta a:LevelStreamLo
    lda a:$9D8B,y
    sta a:LevelStreamHi
    lda a:$9D8C,y
    sta a:LevelAxis
    lda a:$9D8D,y
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
SetUpStageObjectsR:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9F4A,y
    sta a:LevelStreamLo
    lda a:$9F4B,y
    sta a:LevelStreamHi
    lda a:$9F4C,y
    sta a:LevelAxis
    lda a:$9F4D,y
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
SetUpStageObjectsS:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9D54,y
    sta a:LevelStreamLo
    lda a:$9D55,y
    sta a:LevelStreamHi
    lda a:$9D56,y
    sta a:LevelAxis
    lda a:$9D57,y
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
SetUpStageObjectsT:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9F7E,y
    sta a:LevelStreamLo
    lda a:$9F7F,y
    sta a:LevelStreamHi
    lda a:$9F80,y
    sta a:LevelAxis
    lda a:$9F81,y
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
SetUpStageObjectsU:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9E26,y
    sta a:LevelStreamLo
    lda a:$9E27,y
    sta a:LevelStreamHi
    lda a:$9E28,y
    sta a:LevelAxis
    lda a:$9E29,y
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
SetUpStageObjectsV:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9F6D,y
    sta a:LevelStreamLo
    lda a:$9F6E,y
    sta a:LevelStreamHi
    lda a:$9F6F,y
    sta a:LevelAxis
    lda a:$9F70,y
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
SetUpStageObjectsW:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9F31,y
    sta a:LevelStreamLo
    lda a:$9F32,y
    sta a:LevelStreamHi
    lda a:$9F33,y
    sta a:LevelAxis
    lda a:$9F34,y
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
