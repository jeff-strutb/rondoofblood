; @routine 0048:4A6 StageEnter
; @run 6A:6CA6
; WHAT IT DOES: Starts the stage asked for in zStage. Clears the stage's counters and
; turns the request into the stage to run: a request with bit 6 is a restart; otherwise a
; main stage leads to its secret version, and a secret to the next stage on. Some stages
; go elsewhere when they end, and stage 2 goes back to the title's load. Then StageLoad.
; HOW: Keeps the request in zStageRequested; zField8A says how the stage starts.
; Entry: nothing. Exit: as StageLoad, or as AdpcmLoad back at the title's load.
StageEnter:
    stz z:zField80
    stz z:zField89
    stz z:zField85
    stz z:zField82
    stz z:zField8F
    stz z:zField90
    stz z:zField91
    stz z:zField92
    stz z:zField99
    lda #$05
    sta z:zField95
    lda z:zField8C
    and #$7F
    sta z:zField8C
    jsr VecResetGameState
    jsr VecSoundStopAll
    lda z:zStage
    sta z:zStageRequested
    and #$40
    beq StageEnter_at0
    lda z:zStage
    and #$BF
    sta z:zStage
    stz z:zField8A
    bra StageEnter_at1
StageEnter_at0:
    lda z:zStage
    bmi StageEnter_at2
    ora #$80
    and #$FE
    ldx z:zField8C
    cpx #$05
    bcc StageEnter_at3
    ora #$01
StageEnter_at3:
    sta z:zStage
    lda #$03
    sta z:zField8A
    bra StageEnter_at4
StageEnter_at2:
    and #$7F
    inc a
    inc a
    sta z:zStage
    lda #$01
    sta z:zField8A
StageEnter_at4:
    stz z:zField8C
StageEnter_at1:
    lda z:zStageRequested
    and #$40
    bne StageEnter_at5
    lda z:zStage
    cmp #$80
    beq StageEnter_at6
    cmp #$02
    beq StageEnter_at7
    cmp #$85
    beq StageEnter_at8
    cmp #$0B
    beq StageEnter_at9
    cmp #$8B
    beq StageEnter_at10
    cmp #$8C
    beq StageEnter_at11
StageEnter_at5:
    bra StageEnter_at12
StageEnter_at6:
    lda #$01
    sta z:zField8A
    lda #$02
    bra StageEnter_at13
StageEnter_at7:
    lda #$00
    sta z:zStage
    lda #$04
    jsr LoadSceneScript
    lda #$06
    jsr LoadSceneScript
    lda #$24
    jsr AdpcmLoad
    rts
StageEnter_at8:
    ldy z:zField83
    cpy #$0A
    bne StageEnter_at13
    lda #$01
    sta z:zField8A
    lda #$07
    bra StageEnter_at13
StageEnter_at9:
    lda a:StagesVisited+4
    ora a:$3F56
    and #$01
    bne StageEnter_at14
    lda #$4A
    sta z:zStageRequested
    lda #$0A
    bra StageEnter_at13
StageEnter_at14:
    lda #$0B
    bra StageEnter_at13
StageEnter_at10:
    lda #$01
    sta z:zField8A
    lda #$4C
    sta z:zStageRequested
    lda #$0C
    bra StageEnter_at13
StageEnter_at11:
    lda #$01
    sta z:zField8A
    lda #$4E
    sta z:zStageRequested
    lda #$0E
    bra StageEnter_at13
StageEnter_at13:
    sta z:zStage
StageEnter_at12:
    stz z:zField83

; @routine 0048:57C StageLoad
; @run 6A:6D7C
; WHAT IT DOES: Loads the stage in zStage: the character's scene if another is loaded,
; the stage's two scenes, its ADPCM and a seek to its CD track, so the stage can start
; with its music at once. Secret stages and stages $10 and $0C first copy VRAM to RAM.
; HOW: A table at $74B1 or, for bit 7, at $74C2 gives the scene by stage; tables at $7230
; and $7115, or $7241 and $7126, give the ADPCM and the track.
; Entry: nothing. Exit: as CdSearchTrack. Clobbers zTemp0A-0B.
StageLoad:
    lda z:zStage
    bpl StageLoad_at0
    lda #$C2
    sta z:zTemp0A
    lda #$74
    sta z:zTemp0B
    lda #$09
    sta z:zField8E
    bra StageLoad_at1
StageLoad_at0:
    lda #$B1
    sta z:zTemp0A
    lda #$74
    sta z:zTemp0B
    stz z:zField8E
    lda z:zStage
    bne StageLoad_at1
    inc z:zField8E
StageLoad_at1:
    lda z:zStage
    and #$7F
    clc
    adc z:zTemp0A
    sta z:zTemp0A
    cla
    adc z:zTemp0B
    sta z:zTemp0B
    lda z:zCharacter
    cmp z:zCharacterLoaded
    beq StageLoad_at2
    sta z:zCharacterLoaded
    jsr LoadSceneScript
StageLoad_at2:
    lda (zTemp0A)
    clc
    adc z:zCharacter
    jsr LoadSceneScript
    lda (zTemp0A)
    inc a
    inc a
    jsr LoadSceneScript
    lda z:zStage
    bpl StageLoad_at3
    jsr SaveVramToRam
    lda z:zStage
    and #$7F
    tax
    lda a:StageAdpcmSecret,x
    clc
    adc z:zCharacter
    jsr AdpcmLoad
    lda z:zStage
    and #$7F
    tax
    lda a:StageTrackSecret,x
    jmp CdSearchTrack
StageLoad_at3:
    cmp #$10
    beq StageLoad_at4
    cmp #$0C
    bne StageLoad_at5
StageLoad_at4:
    jsr SaveVramToRam
StageLoad_at5:
    lda z:zStage
    and #$7F
    tax
    lda a:StageAdpcm,x
    clc
    adc z:zCharacter
    jsr AdpcmLoad
    lda z:zStage
    and #$7F
    tax
    lda a:StageTrack,x
    jmp CdSearchTrack

; @routine 0048:60A SaveVramToRam
; @run 6A:6E0A
; WHAT IT DOES: Copies $400 bytes of VRAM from $7E00 to RAM at $9C00, in the bank at
; zRamBaseBank.
; HOW: Eight rows of $80 bytes, read through the VDC's data port, through a pointer in
; the BIOS's zBiosAl-Ah.
; Entry: nothing. Exit: A the pointer's low byte, N and Z from X reaching zero, C from its
; add, V too; X 0, Y $80.
SaveVramToRam:
    lda #$00
    clc
    adc z:zRamBaseBank
    tam #$10
    jsr VecSelectVramReadAddress
    st1 #$00
    st2 #$7E
    jsr VecSelectVramData
    stz z:zBiosAl
    lda #$9C
    sta z:zBiosAh
    ldx #$08
SaveVramToRam_at2:
    cly
SaveVramToRam_at0:
    lda a:VDC_DATA_LO
    sta (zBiosAl),y
    iny
    lda a:VDC_DATA_HI
    sta (zBiosAl),y
    iny
    cpy #$80
    bne SaveVramToRam_at0
    lda z:zBiosAl
    clc
    adc #$80
    sta z:zBiosAl
    bcc SaveVramToRam_at1
    inc z:zBiosAh
SaveVramToRam_at1:
    dex
    bne SaveVramToRam_at2
SaveVramToRam_at3:
    rts
