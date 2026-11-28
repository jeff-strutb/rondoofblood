; @routine 024F:19F ClearStageStateBytes
; @run 6B:919F
; WHAT IT DOES: Zeroes four of the stage's state bytes, the ones a stage restart or its
; start-of-stage handler clears together.
; HOW: Four stores of zero, into the stage overlay's state bytes (StageVar350E, StageVar3566,
; StageVar35BE and the blend step).
; WAITS: no.
; Entry: nothing. Exit: A, X, Y and flags kept.
ClearStageStateBytes:
    stz a:StageVar350E
    stz a:StageVar3566
    stz a:StageVar35BE
    stz a:StageBlendStep
    rts

; @routine 024F:33F SaveAndMapStageBanks
; @run 6B:933F
; WHAT IT DOES: Keeps the banks in MPR5 and MPR6 and then maps the stage's two banks there.
; HOW: Each MPR into its saved byte (the second bytes of the blend step's and StageVar371E's columns), then MapStageBanks, which this routine runs on into with no
; return of its own. RestoreStageBanks puts the saved ones back.
; WAITS: no.
; Entry: nothing. Exit: as MapStageBanks.
SaveAndMapStageBanks:
    tma #$20
    sta a:StageBlendStep+1
    tma #$40
    sta a:StageVar371E+1

; @routine 024F:349 MapStageBanks
; @run 6B:9349
; WHAT IT DOES: Maps the stage's two Super RAM banks into MPR5 and MPR6 ($A000 and $C000):
; the banks $10 and $11 above the card's first Super RAM bank.
; HOW: $10 plus zRamBaseBank into MPR5, that plus one into MPR6.
; WAITS: no.
; Entry: nothing. Exit: A the MPR6 bank, flags from the add and the increment (C and V from
; the add, N and Z from A). X and Y kept.
MapStageBanks:
    lda #$10
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    rts

; @routine 024F:354 RestoreStageBanks
; @run 6B:9354
; WHAT IT DOES: Puts back the banks SaveAndMapStageBanks kept in MPR5 and MPR6.
; HOW: MPR6 from the byte at the second byte of the StageVar371E column, then MPR5 from the blend step's second byte.
; WAITS: no.
; Entry: nothing. Exit: A the MPR5 bank, N and Z from it. X and Y kept.
RestoreStageBanks:
    lda a:StageVar371E+1
    tam #$40
    lda a:StageBlendStep+1
    tam #$20
    rts

; @routine 024F:3B0 SpawnStageObject
; @run 6B:93B0
; WHAT IT DOES: Puts a new object in the slot a row of the stage's spawn tables names: the
; slot is emptied, then given the row's frame, kind and X and Y low bytes.
; HOW: The five tables are read by the row, X; the slot comes from the first. The other stage
; group's copy, SpawnStageObjectLong, reads its own tables.
; WAITS: no.
; Entry: X the row, 0 to 7. Exit: A the Y low byte given, N and Z from it, Y the slot, X kept.
SpawnStageObject:
    lda a:StageSpawnSlot,x
    tay
    jsr VecClearObjectInY
    lda a:StageSpawnFrame,x
    sta a:OBJ_FRAME,y
    lda a:StageSpawnKind,x
    sta a:OBJ_2A10,y
    lda a:StageSpawnXLo,x
    sta a:OBJ_X_LO,y
    lda a:StageSpawnYLo,x
    sta a:OBJ_Y_LO,y
    rts

; @routine 05BF:651 SaveAndMapStageBanksLong
; @run 6B:9651
; WHAT IT DOES: SaveAndMapStageBanks in the thirteen stage overlays that keep their stage helpers $312 higher (05BF's
; group): keeps MPR5 and MPR6 and maps the stage's two banks there.
; HOW: The same instructions as SaveAndMapStageBanks; it runs on into MapStageBanksLong at $965B.
; WAITS: no.
; Entry: nothing. Exit: as MapStageBanks.
SaveAndMapStageBanksLong:
    tma #$20
    sta a:StageBlendStep+1
    tma #$40
    sta a:StageVar371E+1

; @routine 05BF:65B MapStageBanksLong
; @run 6B:965B
; WHAT IT DOES: MapStageBanks in 05BF's group of stage overlays: maps the stage's two Super RAM banks into MPR5 and MPR6.
; HOW: The same instructions as MapStageBanks.
; WAITS: no.
; Entry: nothing. Exit: as MapStageBanks.
MapStageBanksLong:
    lda #$10
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    rts

; @routine 05BF:666 RestoreStageBanksLong
; @run 6B:9666
; WHAT IT DOES: RestoreStageBanks in 05BF's group of stage overlays: puts back the banks SaveAndMapStageBanksLong kept.
; HOW: The same instructions as RestoreStageBanks.
; WAITS: no.
; Entry: nothing. Exit: as RestoreStageBanks.
RestoreStageBanksLong:
    lda a:StageVar371E+1
    tam #$40
    lda a:StageBlendStep+1
    tam #$20
    rts

; @routine 05BF:6C2 SpawnStageObjectLong
; @run 6B:96C2
; WHAT IT DOES: SpawnStageObject for the other stage group, which keeps its spawn tables at
; $96E2 to $9702.
; HOW: The same code with its own tables.
; WAITS: no.
; Entry: X the row, 0 to 7. Exit: as SpawnStageObject.
SpawnStageObjectLong:
    lda a:StageSpawnSlotB,x
    tay
    jsr VecClearObjectInY
    lda a:StageSpawnFrameB,x
    sta a:OBJ_FRAME,y
    lda a:StageSpawnKindB,x
    sta a:OBJ_2A10,y
    lda a:StageSpawnXLoB,x
    sta a:OBJ_X_LO,y
    lda a:StageSpawnYLoB,x
    sta a:OBJ_Y_LO,y
    rts
