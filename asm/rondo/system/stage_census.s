; @routine 026E:0A2 InitStage026E
; @run 6B:88A2
; @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 026E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
; object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
; loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
; HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
; WAITS: no.
; Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
InitStage026E:
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
    jsr VecClearScreenState
    bra InitStage026E_at0
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
InitStage026E_at0:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    stz a:OBJ_2FE8
    lda z:zField8E
    pha
    bsr LoadStageEntry026E
    pla
    sta z:zField8E
    jsr $80A0
    jsr $A8EC
    ldy z:zRasterEffect
    lda a:$8889,y
    beq InitStage026E_at1
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    lda z:zField8E
    pha
    bsr LoadStageEntry026E
    pla
    sta z:zField8E
    jsr $A94D
InitStage026E_at1:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088

; @routine 026E:108 LoadStageEntry026E
; @run 6B:8908
; @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 026E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
; limits, the player's start position and the raster effect.
; HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9C93, its character offsets at
; $89E8.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
; zTemp00-01.
LoadStageEntry026E:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:$9C93,y
    sta z:zTemp00
    lda a:$9C94,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntry026E_at0
    rmb1 z:zField8E
    iny
    iny
    bra LoadStageEntry026E_at1
LoadStageEntry026E_at0:
    lda (zTemp00),y
    bne LoadStageEntry026E_at2
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra LoadStageEntry026E_at3
LoadStageEntry026E_at2:
    iny
    sta a:OBJ_X_LO
LoadStageEntry026E_at3:
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq LoadStageEntry026E_at1
    lda a:OBJ_27A8
    cmp #$05
    bcc LoadStageEntry026E_at1
    sec
    sbc #$05
    cmp #$04
    bcs LoadStageEntry026E_at1
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$89E8,x
    sta a:OBJ_X_LO
LoadStageEntry026E_at1:
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntry026E_at4
    iny
    sta a:OBJ_2AC0
    bra LoadStageEntry026E_at5
LoadStageEntry026E_at4:
    iny
LoadStageEntry026E_at5:
    rts

; @routine 026E:1EC StageScrollStep026E
; @run 6B:89EC
; @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 026E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
; offset the overlay keeps (the byte at $ADCE), into MPR5 and MPR6, then steps the stream with the scroll.
; HOW: ScrollStepBody (stage_scroll.c).
; WAITS: no.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageScrollStep026E:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$ADCE
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 026E:215 SetUpStageObjects026E
; @run 6B:8A15
; @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 026E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
; section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
; and runs the first frame of objects.
; HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9D23; the tail jump goes to $8A05.
; WAITS: no.
; Entry: X the caller's. Exit: as RunObjects.
SetUpStageObjects026E:
    jsr VecClearEnemySlots
    jsr $80A0
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9D23,y
    sta a:LevelStreamLo
    lda a:$9D24,y
    sta a:LevelStreamHi
    lda a:$9D25,y
    sta a:LevelAxis
    lda a:$9D26,y
    beq SetUpStageObjects026E_at0
    ldx #$1F
SetUpStageObjects026E_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjects026E_at1
SetUpStageObjects026E_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$ADCE
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp $8A05

; @routine 026E:309 SpawnStageObjectsRows01026E
; @run 6B:8B09
; @unreachable 6B:8B09-6B:8B22 no read the game makes loads disc sector 026E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
; bit 5 of zField8E and starts list channel 4 with list $65.
; HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObject's.
; WAITS: no.
; Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
SpawnStageObjectsRows01026E:
    clx
    jsr $936E
    inx
    jsr $936E
    stz a:StageVar3616
    lda #$C0
    sta a:StageVar3616+1
    rmb5 z:zField8E
    lda #$65
    ldx #$04
    jmp VecStartListChannelKind2

; @routine 026E:323 StepStageFrame026E
; @run 6B:8B23
; @stop 8BCF
; @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 026E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
; number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
; the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
; changed.
; HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
; SetVramAddresses.
; WAITS: no.
; Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
StepStageFrame026E:
    lda a:$2ABF
    beq $8B22
    bbs1 zPaused,StepStageFrame026E_at0
    lda a:StageVar3616+1
    beq StepStageFrame026E_at1
    dec a:StageVar3616+1
    bne StepStageFrame026E_at1
    smb1 z:zPaused
StepStageFrame026E_at0:
    jsr $8EAF
    bra StepStageFrame026E_at2
StepStageFrame026E_at1:
    jsr $910F
    jsr $913F
    jsr $916A
StepStageFrame026E_at2:
    jsr $8BD8
    lda #$24
    sta a:$2D26
    ldx z:zField93
    lda a:$8BCF,x
    ldx z:zCharacter
    beq StepStageFrame026E_at3
    cmp #$0D
    beq StepStageFrame026E_at4
    cmp #$11
    bne StepStageFrame026E_at3
    ldx #$28
    stx a:$2D26
    lda #$71
    bra StepStageFrame026E_at3
StepStageFrame026E_at4:
    lda #$72
StepStageFrame026E_at3:
    sta a:$2ABE
    lda z:zPlayerHealth
    cmp z:zField99
    beq StepStageFrame026E_at5
    bcs StepStageFrame026E_at6
    jsr $8BF9
    ldx #$FF
    ldy #$F0
    jsr $9350
    dec z:zField99
    bra StepStageFrame026E_x0
StepStageFrame026E_at6:
    bbr5 zField8E,StepStageFrame026E_at7
    bbs6 zField8E,StepStageFrame026E_at7
    lda #$0E
    jsr VecSoundCommand
    smb6 z:zField8E
StepStageFrame026E_at7:
    inc z:zField99
    bsr $8BF9
    ldx #$00
    ldy #$0F
    jsr $935F
    bra StepStageFrame026E_at8
StepStageFrame026E_at5:
    smb5 z:zField8E
StepStageFrame026E_x0:
    bbr6 zField8E,StepStageFrame026E_at8
    rmb6 z:zField8E
    lda #$4A
    jsr VecSoundCommand
StepStageFrame026E_at8:
    jsr $9307
    stz a:StageBlendOn
    lda z:zField94
    cmp z:zField95
    beq StepStageFrame026E_at9
    sta z:zField95
    and #$0F
    ldx #$08
    jsr $8D3A
    lda z:zField94
    lsr a
    lsr a
    lsr a
    lsr a
    ldx #$09
    jsr $8D3A
StepStageFrame026E_at9:
    bbs2 zField8E,StepStageFrame026E_at10
    rts
StepStageFrame026E_at10:
    jmp $8C2A

; @routine 031E:0A2 InitStage031E
; @run 6B:88A2
; @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 031E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
; object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
; loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
; HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
; WAITS: no.
; Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
InitStage031E:
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
    jsr VecClearScreenState
    bra InitStage031E_at0
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
InitStage031E_at0:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    stz a:OBJ_2FE8
    lda z:zField8E
    pha
    bsr LoadStageEntry031E
    pla
    sta z:zField8E
    jsr $80A0
    jsr $A8EC
    ldy z:zRasterEffect
    lda a:$8889,y
    beq InitStage031E_at1
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    lda z:zField8E
    pha
    bsr LoadStageEntry031E
    pla
    sta z:zField8E
    jsr $A94D
InitStage031E_at1:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088

; @routine 031E:108 LoadStageEntry031E
; @run 6B:8908
; @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 031E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
; limits, the player's start position and the raster effect.
; HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9E3F, its character offsets at
; $89E8.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
; zTemp00-01.
LoadStageEntry031E:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:$9E3F,y
    sta z:zTemp00
    lda a:$9E40,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntry031E_at0
    rmb1 z:zField8E
    iny
    iny
    bra LoadStageEntry031E_at1
LoadStageEntry031E_at0:
    lda (zTemp00),y
    bne LoadStageEntry031E_at2
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra LoadStageEntry031E_at3
LoadStageEntry031E_at2:
    iny
    sta a:OBJ_X_LO
LoadStageEntry031E_at3:
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq LoadStageEntry031E_at1
    lda a:OBJ_27A8
    cmp #$05
    bcc LoadStageEntry031E_at1
    sec
    sbc #$05
    cmp #$04
    bcs LoadStageEntry031E_at1
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$89E8,x
    sta a:OBJ_X_LO
LoadStageEntry031E_at1:
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntry031E_at4
    iny
    sta a:OBJ_2AC0
    bra LoadStageEntry031E_at5
LoadStageEntry031E_at4:
    iny
LoadStageEntry031E_at5:
    rts

; @routine 031E:1EC StageScrollStep031E
; @run 6B:89EC
; @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 031E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
; offset the overlay keeps (the byte at $AD12), into MPR5 and MPR6, then steps the stream with the scroll.
; HOW: ScrollStepBody (stage_scroll.c).
; WAITS: no.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageScrollStep031E:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AD12
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 031E:215 SetUpStageObjects031E
; @run 6B:8A15
; @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 031E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
; section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
; and runs the first frame of objects.
; HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9E4F; the tail jump goes to $8A05.
; WAITS: no.
; Entry: X the caller's. Exit: as RunObjects.
SetUpStageObjects031E:
    jsr VecClearEnemySlots
    jsr $80A0
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9E4F,y
    sta a:LevelStreamLo
    lda a:$9E50,y
    sta a:LevelStreamHi
    lda a:$9E51,y
    sta a:LevelAxis
    lda a:$9E52,y
    beq SetUpStageObjects031E_at0
    ldx #$1F
SetUpStageObjects031E_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjects031E_at1
SetUpStageObjects031E_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AD12
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp $8A05

; @routine 031E:309 SpawnStageObjectsRows01031E
; @run 6B:8B09
; @unreachable 6B:8B09-6B:8B22 no read the game makes loads disc sector 031E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
; bit 5 of zField8E and starts list channel 4 with list $65.
; HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObjectLong's.
; WAITS: no.
; Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
SpawnStageObjectsRows01031E:
    clx
    jsr $964F
    inx
    jsr $964F
    stz a:StageVar3616
    lda #$C0
    sta a:StageVar3616+1
    rmb5 z:zField8E
    lda #$65
    ldx #$04
    jmp VecStartListChannelKind2

; @routine 031E:323 StepStageFrame031E
; @run 6B:8B23
; @stop 8BCF
; @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 031E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
; number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
; the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
; changed.
; HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
; SetVramAddressesLong.
; WAITS: no.
; Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
StepStageFrame031E:
    lda a:$2ABF
    beq $8B22
    bbs1 zPaused,StepStageFrame031E_at0
    lda a:StageVar3616+1
    beq StepStageFrame031E_at1
    dec a:StageVar3616+1
    bne StepStageFrame031E_at1
    smb1 z:zPaused
StepStageFrame031E_at0:
    jsr $8EA3
    bra StepStageFrame031E_at2
StepStageFrame031E_at1:
    jsr $9103
    jsr $9133
    jsr $915E
StepStageFrame031E_at2:
    jsr $8BD8
    lda #$24
    sta a:$2D26
    ldx z:zField93
    lda a:$8BCF,x
    ldx z:zCharacter
    beq StepStageFrame031E_at3
    cmp #$0D
    beq StepStageFrame031E_at4
    cmp #$11
    bne StepStageFrame031E_at3
    ldx #$28
    stx a:$2D26
    lda #$71
    bra StepStageFrame031E_at3
StepStageFrame031E_at4:
    lda #$72
StepStageFrame031E_at3:
    sta a:$2ABE
    lda z:zPlayerHealth
    cmp z:zField99
    beq StepStageFrame031E_at5
    bcs StepStageFrame031E_at6
    jsr $8BF9
    ldx #$FF
    ldy #$F0
    jsr $9631
    dec z:zField99
    bra StepStageFrame031E_x0
StepStageFrame031E_at6:
    bbr5 zField8E,StepStageFrame031E_at7
    bbs6 zField8E,StepStageFrame031E_at7
    lda #$0E
    jsr VecSoundCommand
    smb6 z:zField8E
StepStageFrame031E_at7:
    inc z:zField99
    bsr $8BF9
    ldx #$00
    ldy #$0F
    jsr $9640
    bra StepStageFrame031E_at8
StepStageFrame031E_at5:
    smb5 z:zField8E
StepStageFrame031E_x0:
    bbr6 zField8E,StepStageFrame031E_at8
    rmb6 z:zField8E
    lda #$4A
    jsr VecSoundCommand
StepStageFrame031E_at8:
    jsr $95E8
    stz a:StageBlendOn
    lda z:zField94
    cmp z:zField95
    beq StepStageFrame031E_at9
    sta z:zField95
    and #$0F
    ldx #$08
    jsr $8D3A
    lda z:zField94
    lsr a
    lsr a
    lsr a
    lsr a
    ldx #$09
    jsr $8D3A
StepStageFrame031E_at9:
    bbs2 zField8E,StepStageFrame031E_at10
    rts
StepStageFrame031E_at10:
    jmp $8C2A

; @routine 043E:0A2 InitStage043E
; @run 6B:88A2
; @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 043E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
; object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
; loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
; HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
; WAITS: no.
; Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
InitStage043E:
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
    jsr VecClearScreenState
    bra InitStage043E_at0
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
InitStage043E_at0:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    stz a:OBJ_2FE8
    lda z:zField8E
    pha
    bsr LoadStageEntry043E
    pla
    sta z:zField8E
    jsr $80A0
    jsr $A8EC
    ldy z:zRasterEffect
    lda a:$8889,y
    beq InitStage043E_at1
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    lda z:zField8E
    pha
    bsr LoadStageEntry043E
    pla
    sta z:zField8E
    jsr $A94D
InitStage043E_at1:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088

; @routine 043E:108 LoadStageEntry043E
; @run 6B:8908
; @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 043E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
; limits, the player's start position and the raster effect.
; HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9F02, its character offsets at
; $89E8.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
; zTemp00-01.
LoadStageEntry043E:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:$9F02,y
    sta z:zTemp00
    lda a:$9F03,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntry043E_at0
    rmb1 z:zField8E
    iny
    iny
    bra LoadStageEntry043E_at1
LoadStageEntry043E_at0:
    lda (zTemp00),y
    bne LoadStageEntry043E_at2
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra LoadStageEntry043E_at3
LoadStageEntry043E_at2:
    iny
    sta a:OBJ_X_LO
LoadStageEntry043E_at3:
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq LoadStageEntry043E_at1
    lda a:OBJ_27A8
    cmp #$05
    bcc LoadStageEntry043E_at1
    sec
    sbc #$05
    cmp #$04
    bcs LoadStageEntry043E_at1
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$89E8,x
    sta a:OBJ_X_LO
LoadStageEntry043E_at1:
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntry043E_at4
    iny
    sta a:OBJ_2AC0
    bra LoadStageEntry043E_at5
LoadStageEntry043E_at4:
    iny
LoadStageEntry043E_at5:
    rts

; @routine 043E:1EC StageScrollStep043E
; @run 6B:89EC
; @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 043E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
; offset the overlay keeps (the byte at $AD1A), into MPR5 and MPR6, then steps the stream with the scroll.
; HOW: ScrollStepBody (stage_scroll.c).
; WAITS: no.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageScrollStep043E:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AD1A
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 043E:215 SetUpStageObjects043E
; @run 6B:8A15
; @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 043E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
; section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
; and runs the first frame of objects.
; HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9F22; the tail jump goes to $8A05.
; WAITS: no.
; Entry: X the caller's. Exit: as RunObjects.
SetUpStageObjects043E:
    jsr VecClearEnemySlots
    jsr $80A0
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9F22,y
    sta a:LevelStreamLo
    lda a:$9F23,y
    sta a:LevelStreamHi
    lda a:$9F24,y
    sta a:LevelAxis
    lda a:$9F25,y
    beq SetUpStageObjects043E_at0
    ldx #$1F
SetUpStageObjects043E_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjects043E_at1
SetUpStageObjects043E_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AD1A
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp $8A05

; @routine 043E:309 SpawnStageObjectsRows01043E
; @run 6B:8B09
; @unreachable 6B:8B09-6B:8B22 no read the game makes loads disc sector 043E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
; bit 5 of zField8E and starts list channel 4 with list $65.
; HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObjectLong's.
; WAITS: no.
; Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
SpawnStageObjectsRows01043E:
    clx
    jsr $964F
    inx
    jsr $964F
    stz a:StageVar3616
    lda #$C0
    sta a:StageVar3616+1
    rmb5 z:zField8E
    lda #$65
    ldx #$04
    jmp VecStartListChannelKind2

; @routine 043E:323 StepStageFrame043E
; @run 6B:8B23
; @stop 8BCF
; @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 043E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
; number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
; the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
; changed.
; HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
; SetVramAddressesLong.
; WAITS: no.
; Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
StepStageFrame043E:
    lda a:$2ABF
    beq $8B22
    bbs1 zPaused,StepStageFrame043E_at0
    lda a:StageVar3616+1
    beq StepStageFrame043E_at1
    dec a:StageVar3616+1
    bne StepStageFrame043E_at1
    smb1 z:zPaused
StepStageFrame043E_at0:
    jsr $8EA3
    bra StepStageFrame043E_at2
StepStageFrame043E_at1:
    jsr $9103
    jsr $9133
    jsr $915E
StepStageFrame043E_at2:
    jsr $8BD8
    lda #$24
    sta a:$2D26
    ldx z:zField93
    lda a:$8BCF,x
    ldx z:zCharacter
    beq StepStageFrame043E_at3
    cmp #$0D
    beq StepStageFrame043E_at4
    cmp #$11
    bne StepStageFrame043E_at3
    ldx #$28
    stx a:$2D26
    lda #$71
    bra StepStageFrame043E_at3
StepStageFrame043E_at4:
    lda #$72
StepStageFrame043E_at3:
    sta a:$2ABE
    lda z:zPlayerHealth
    cmp z:zField99
    beq StepStageFrame043E_at5
    bcs StepStageFrame043E_at6
    jsr $8BF9
    ldx #$FF
    ldy #$F0
    jsr $9631
    dec z:zField99
    bra StepStageFrame043E_x0
StepStageFrame043E_at6:
    bbr5 zField8E,StepStageFrame043E_at7
    bbs6 zField8E,StepStageFrame043E_at7
    lda #$0E
    jsr VecSoundCommand
    smb6 z:zField8E
StepStageFrame043E_at7:
    inc z:zField99
    bsr $8BF9
    ldx #$00
    ldy #$0F
    jsr $9640
    bra StepStageFrame043E_at8
StepStageFrame043E_at5:
    smb5 z:zField8E
StepStageFrame043E_x0:
    bbr6 zField8E,StepStageFrame043E_at8
    rmb6 z:zField8E
    lda #$4A
    jsr VecSoundCommand
StepStageFrame043E_at8:
    jsr $95E8
    stz a:StageBlendOn
    lda z:zField94
    cmp z:zField95
    beq StepStageFrame043E_at9
    sta z:zField95
    and #$0F
    ldx #$08
    jsr $8D3A
    lda z:zField94
    lsr a
    lsr a
    lsr a
    lsr a
    ldx #$09
    jsr $8D3A
StepStageFrame043E_at9:
    bbs2 zField8E,StepStageFrame043E_at10
    rts
StepStageFrame043E_at10:
    jmp $8C2A

; @routine 064E:0A2 InitStage064E
; @run 6B:88A2
; @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 064E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
; object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
; loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
; HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
; WAITS: no.
; Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
InitStage064E:
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
    jsr VecClearScreenState
    bra InitStage064E_at0
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
InitStage064E_at0:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    stz a:OBJ_2FE8
    lda z:zField8E
    pha
    bsr LoadStageEntry064E
    pla
    sta z:zField8E
    jsr $80A0
    jsr $A8EC
    ldy z:zRasterEffect
    lda a:$8889,y
    beq InitStage064E_at1
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    lda z:zField8E
    pha
    bsr LoadStageEntry064E
    pla
    sta z:zField8E
    jsr $A94D
InitStage064E_at1:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088

; @routine 064E:108 LoadStageEntry064E
; @run 6B:8908
; @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 064E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
; limits, the player's start position and the raster effect.
; HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9D8E, its character offsets at
; $89E8.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
; zTemp00-01.
LoadStageEntry064E:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:$9D8E,y
    sta z:zTemp00
    lda a:$9D8F,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntry064E_at0
    rmb1 z:zField8E
    iny
    iny
    bra LoadStageEntry064E_at1
LoadStageEntry064E_at0:
    lda (zTemp00),y
    bne LoadStageEntry064E_at2
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra LoadStageEntry064E_at3
LoadStageEntry064E_at2:
    iny
    sta a:OBJ_X_LO
LoadStageEntry064E_at3:
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq LoadStageEntry064E_at1
    lda a:OBJ_27A8
    cmp #$05
    bcc LoadStageEntry064E_at1
    sec
    sbc #$05
    cmp #$04
    bcs LoadStageEntry064E_at1
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$89E8,x
    sta a:OBJ_X_LO
LoadStageEntry064E_at1:
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntry064E_at4
    iny
    sta a:OBJ_2AC0
    bra LoadStageEntry064E_at5
LoadStageEntry064E_at4:
    iny
LoadStageEntry064E_at5:
    rts

; @routine 064E:1EC StageScrollStep064E
; @run 6B:89EC
; @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 064E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
; offset the overlay keeps (the byte at $AD82), into MPR5 and MPR6, then steps the stream with the scroll.
; HOW: ScrollStepBody (stage_scroll.c).
; WAITS: no.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageScrollStep064E:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AD82
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 064E:215 SetUpStageObjects064E
; @run 6B:8A15
; @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 064E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
; section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
; and runs the first frame of objects.
; HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9E22; the tail jump goes to $8A05.
; WAITS: no.
; Entry: X the caller's. Exit: as RunObjects.
SetUpStageObjects064E:
    jsr VecClearEnemySlots
    jsr $80A0
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9E22,y
    sta a:LevelStreamLo
    lda a:$9E23,y
    sta a:LevelStreamHi
    lda a:$9E24,y
    sta a:LevelAxis
    lda a:$9E25,y
    beq SetUpStageObjects064E_at0
    ldx #$1F
SetUpStageObjects064E_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjects064E_at1
SetUpStageObjects064E_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AD82
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp $8A05

; @routine 064E:309 SpawnStageObjectsRows01064E
; @run 6B:8B09
; @unreachable 6B:8B09-6B:8B22 no read the game makes loads disc sector 064E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
; bit 5 of zField8E and starts list channel 4 with list $65.
; HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObject's.
; WAITS: no.
; Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
SpawnStageObjectsRows01064E:
    clx
    jsr $936E
    inx
    jsr $936E
    stz a:StageVar3616
    lda #$C0
    sta a:StageVar3616+1
    rmb5 z:zField8E
    lda #$65
    ldx #$04
    jmp VecStartListChannelKind2

; @routine 064E:323 StepStageFrame064E
; @run 6B:8B23
; @stop 8BCF
; @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 064E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
; number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
; the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
; changed.
; HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
; SetVramAddresses.
; WAITS: no.
; Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
StepStageFrame064E:
    lda a:$2ABF
    beq $8B22
    bbs1 zPaused,StepStageFrame064E_at0
    lda a:StageVar3616+1
    beq StepStageFrame064E_at1
    dec a:StageVar3616+1
    bne StepStageFrame064E_at1
    smb1 z:zPaused
StepStageFrame064E_at0:
    jsr $8EAF
    bra StepStageFrame064E_at2
StepStageFrame064E_at1:
    jsr $910F
    jsr $913F
    jsr $916A
StepStageFrame064E_at2:
    jsr $8BD8
    lda #$24
    sta a:$2D26
    ldx z:zField93
    lda a:$8BCF,x
    ldx z:zCharacter
    beq StepStageFrame064E_at3
    cmp #$0D
    beq StepStageFrame064E_at4
    cmp #$11
    bne StepStageFrame064E_at3
    ldx #$28
    stx a:$2D26
    lda #$71
    bra StepStageFrame064E_at3
StepStageFrame064E_at4:
    lda #$72
StepStageFrame064E_at3:
    sta a:$2ABE
    lda z:zPlayerHealth
    cmp z:zField99
    beq StepStageFrame064E_at5
    bcs StepStageFrame064E_at6
    jsr $8BF9
    ldx #$FF
    ldy #$F0
    jsr $9350
    dec z:zField99
    bra StepStageFrame064E_x0
StepStageFrame064E_at6:
    bbr5 zField8E,StepStageFrame064E_at7
    bbs6 zField8E,StepStageFrame064E_at7
    lda #$0E
    jsr VecSoundCommand
    smb6 z:zField8E
StepStageFrame064E_at7:
    inc z:zField99
    bsr $8BF9
    ldx #$00
    ldy #$0F
    jsr $935F
    bra StepStageFrame064E_at8
StepStageFrame064E_at5:
    smb5 z:zField8E
StepStageFrame064E_x0:
    bbr6 zField8E,StepStageFrame064E_at8
    rmb6 z:zField8E
    lda #$4A
    jsr VecSoundCommand
StepStageFrame064E_at8:
    jsr $9307
    stz a:StageBlendOn
    lda z:zField94
    cmp z:zField95
    beq StepStageFrame064E_at9
    sta z:zField95
    and #$0F
    ldx #$08
    jsr $8D3A
    lda z:zField94
    lsr a
    lsr a
    lsr a
    lsr a
    ldx #$09
    jsr $8D3A
StepStageFrame064E_at9:
    bbs2 zField8E,StepStageFrame064E_at10
    rts
StepStageFrame064E_at10:
    jmp $8C2A

; @routine 10DE:0A2 InitStage10DE
; @run 6B:88A2
; @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 10DE, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
; object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
; loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
; HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
; WAITS: no.
; Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
InitStage10DE:
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
    jsr VecClearScreenState
    bra InitStage10DE_at0
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
InitStage10DE_at0:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    stz a:OBJ_2FE8
    lda z:zField8E
    pha
    bsr LoadStageEntry10DE
    pla
    sta z:zField8E
    jsr $80A0
    jsr $A8EC
    ldy z:zRasterEffect
    lda a:$8889,y
    beq InitStage10DE_at1
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    lda z:zField8E
    pha
    bsr LoadStageEntry10DE
    pla
    sta z:zField8E
    jsr $A94D
InitStage10DE_at1:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088

; @routine 10DE:108 LoadStageEntry10DE
; @run 6B:8908
; @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 10DE, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
; limits, the player's start position and the raster effect.
; HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9EF0, its character offsets at
; $89E8.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
; zTemp00-01.
LoadStageEntry10DE:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:$9EF0,y
    sta z:zTemp00
    lda a:$9EF1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntry10DE_at0
    rmb1 z:zField8E
    iny
    iny
    bra LoadStageEntry10DE_at1
LoadStageEntry10DE_at0:
    lda (zTemp00),y
    bne LoadStageEntry10DE_at2
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra LoadStageEntry10DE_at3
LoadStageEntry10DE_at2:
    iny
    sta a:OBJ_X_LO
LoadStageEntry10DE_at3:
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq LoadStageEntry10DE_at1
    lda a:OBJ_27A8
    cmp #$05
    bcc LoadStageEntry10DE_at1
    sec
    sbc #$05
    cmp #$04
    bcs LoadStageEntry10DE_at1
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$89E8,x
    sta a:OBJ_X_LO
LoadStageEntry10DE_at1:
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntry10DE_at4
    iny
    sta a:OBJ_2AC0
    bra LoadStageEntry10DE_at5
LoadStageEntry10DE_at4:
    iny
LoadStageEntry10DE_at5:
    rts

; @routine 10DE:1EC StageScrollStep10DE
; @run 6B:89EC
; @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 10DE, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
; offset the overlay keeps (the byte at $AE5B), into MPR5 and MPR6, then steps the stream with the scroll.
; HOW: ScrollStepBody (stage_scroll.c).
; WAITS: no.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageScrollStep10DE:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AE5B
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 10DE:215 SetUpStageObjects10DE
; @run 6B:8A15
; @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 10DE, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
; section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
; and runs the first frame of objects.
; HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9F10; the tail jump goes to $8A05.
; WAITS: no.
; Entry: X the caller's. Exit: as RunObjects.
SetUpStageObjects10DE:
    jsr VecClearEnemySlots
    jsr $80A0
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9F10,y
    sta a:LevelStreamLo
    lda a:$9F11,y
    sta a:LevelStreamHi
    lda a:$9F12,y
    sta a:LevelAxis
    lda a:$9F13,y
    beq SetUpStageObjects10DE_at0
    ldx #$1F
SetUpStageObjects10DE_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjects10DE_at1
SetUpStageObjects10DE_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AE5B
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp $8A05

; @routine 10DE:309 SpawnStageObjectsRows0110DE
; @run 6B:8B09
; @unreachable 6B:8B09-6B:8B22 no read the game makes loads disc sector 10DE, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
; bit 5 of zField8E and starts list channel 4 with list $65.
; HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObjectLong's.
; WAITS: no.
; Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
SpawnStageObjectsRows0110DE:
    clx
    jsr $964F
    inx
    jsr $964F
    stz a:StageVar3616
    lda #$C0
    sta a:StageVar3616+1
    rmb5 z:zField8E
    lda #$65
    ldx #$04
    jmp VecStartListChannelKind2

; @routine 10DE:323 StepStageFrame10DE
; @run 6B:8B23
; @stop 8BCF
; @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 10DE, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
; number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
; the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
; changed.
; HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
; SetVramAddressesLong.
; WAITS: no.
; Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
StepStageFrame10DE:
    lda a:$2ABF
    beq $8B22
    bbs1 zPaused,StepStageFrame10DE_at0
    lda a:StageVar3616+1
    beq StepStageFrame10DE_at1
    dec a:StageVar3616+1
    bne StepStageFrame10DE_at1
    smb1 z:zPaused
StepStageFrame10DE_at0:
    jsr $8EA3
    bra StepStageFrame10DE_at2
StepStageFrame10DE_at1:
    jsr $9103
    jsr $9133
    jsr $915E
StepStageFrame10DE_at2:
    jsr $8BD8
    lda #$24
    sta a:$2D26
    ldx z:zField93
    lda a:$8BCF,x
    ldx z:zCharacter
    beq StepStageFrame10DE_at3
    cmp #$0D
    beq StepStageFrame10DE_at4
    cmp #$11
    bne StepStageFrame10DE_at3
    ldx #$28
    stx a:$2D26
    lda #$71
    bra StepStageFrame10DE_at3
StepStageFrame10DE_at4:
    lda #$72
StepStageFrame10DE_at3:
    sta a:$2ABE
    lda z:zPlayerHealth
    cmp z:zField99
    beq StepStageFrame10DE_at5
    bcs StepStageFrame10DE_at6
    jsr $8BF9
    ldx #$FF
    ldy #$F0
    jsr $9631
    dec z:zField99
    bra StepStageFrame10DE_x0
StepStageFrame10DE_at6:
    bbr5 zField8E,StepStageFrame10DE_at7
    bbs6 zField8E,StepStageFrame10DE_at7
    lda #$0E
    jsr VecSoundCommand
    smb6 z:zField8E
StepStageFrame10DE_at7:
    inc z:zField99
    bsr $8BF9
    ldx #$00
    ldy #$0F
    jsr $9640
    bra StepStageFrame10DE_at8
StepStageFrame10DE_at5:
    smb5 z:zField8E
StepStageFrame10DE_x0:
    bbr6 zField8E,StepStageFrame10DE_at8
    rmb6 z:zField8E
    lda #$4A
    jsr VecSoundCommand
StepStageFrame10DE_at8:
    jsr $95E8
    stz a:StageBlendOn
    lda z:zField94
    cmp z:zField95
    beq StepStageFrame10DE_at9
    sta z:zField95
    and #$0F
    ldx #$08
    jsr $8D3A
    lda z:zField94
    lsr a
    lsr a
    lsr a
    lsr a
    ldx #$09
    jsr $8D3A
StepStageFrame10DE_at9:
    bbs2 zField8E,StepStageFrame10DE_at10
    rts
StepStageFrame10DE_at10:
    jmp $8C2A

; @routine 122E:0A2 InitStage122E
; @run 6B:88A2
; @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 122E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
; object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
; loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
; HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
; WAITS: no.
; Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
InitStage122E:
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
    jsr VecClearScreenState
    bra InitStage122E_at0
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
InitStage122E_at0:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    stz a:OBJ_2FE8
    lda z:zField8E
    pha
    bsr LoadStageEntry122E
    pla
    sta z:zField8E
    jsr $80A0
    jsr $A8EC
    ldy z:zRasterEffect
    lda a:$8889,y
    beq InitStage122E_at1
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    lda z:zField8E
    pha
    bsr LoadStageEntry122E
    pla
    sta z:zField8E
    jsr $A94D
InitStage122E_at1:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088

; @routine 122E:108 LoadStageEntry122E
; @run 6B:8908
; @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 122E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
; limits, the player's start position and the raster effect.
; HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9D08, its character offsets at
; $89E8.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
; zTemp00-01.
LoadStageEntry122E:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:$9D08,y
    sta z:zTemp00
    lda a:$9D09,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntry122E_at0
    rmb1 z:zField8E
    iny
    iny
    bra LoadStageEntry122E_at1
LoadStageEntry122E_at0:
    lda (zTemp00),y
    bne LoadStageEntry122E_at2
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra LoadStageEntry122E_at3
LoadStageEntry122E_at2:
    iny
    sta a:OBJ_X_LO
LoadStageEntry122E_at3:
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq LoadStageEntry122E_at1
    lda a:OBJ_27A8
    cmp #$05
    bcc LoadStageEntry122E_at1
    sec
    sbc #$05
    cmp #$04
    bcs LoadStageEntry122E_at1
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$89E8,x
    sta a:OBJ_X_LO
LoadStageEntry122E_at1:
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntry122E_at4
    iny
    sta a:OBJ_2AC0
    bra LoadStageEntry122E_at5
LoadStageEntry122E_at4:
    iny
LoadStageEntry122E_at5:
    rts

; @routine 122E:1EC StageScrollStep122E
; @run 6B:89EC
; @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 122E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
; offset the overlay keeps (the byte at $AE33), into MPR5 and MPR6, then steps the stream with the scroll.
; HOW: ScrollStepBody (stage_scroll.c).
; WAITS: no.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageScrollStep122E:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AE33
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 122E:215 SetUpStageObjects122E
; @run 6B:8A15
; @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 122E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
; section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
; and runs the first frame of objects.
; HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9E48; the tail jump goes to $8A05.
; WAITS: no.
; Entry: X the caller's. Exit: as RunObjects.
SetUpStageObjects122E:
    jsr VecClearEnemySlots
    jsr $80A0
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9E48,y
    sta a:LevelStreamLo
    lda a:$9E49,y
    sta a:LevelStreamHi
    lda a:$9E4A,y
    sta a:LevelAxis
    lda a:$9E4B,y
    beq SetUpStageObjects122E_at0
    ldx #$1F
SetUpStageObjects122E_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjects122E_at1
SetUpStageObjects122E_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AE33
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp $8A05

; @routine 122E:309 SpawnStageObjectsRows01122E
; @run 6B:8B09
; @unreachable 6B:8B09-6B:8B22 no read the game makes loads disc sector 122E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
; bit 5 of zField8E and starts list channel 4 with list $65.
; HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObject's.
; WAITS: no.
; Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
SpawnStageObjectsRows01122E:
    clx
    jsr $936E
    inx
    jsr $936E
    stz a:StageVar3616
    lda #$C0
    sta a:StageVar3616+1
    rmb5 z:zField8E
    lda #$65
    ldx #$04
    jmp VecStartListChannelKind2

; @routine 122E:323 StepStageFrame122E
; @run 6B:8B23
; @stop 8BCF
; @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 122E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
; number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
; the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
; changed.
; HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
; SetVramAddresses.
; WAITS: no.
; Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
StepStageFrame122E:
    lda a:$2ABF
    beq $8B22
    bbs1 zPaused,StepStageFrame122E_at0
    lda a:StageVar3616+1
    beq StepStageFrame122E_at1
    dec a:StageVar3616+1
    bne StepStageFrame122E_at1
    smb1 z:zPaused
StepStageFrame122E_at0:
    jsr $8EAF
    bra StepStageFrame122E_at2
StepStageFrame122E_at1:
    jsr $910F
    jsr $913F
    jsr $916A
StepStageFrame122E_at2:
    jsr $8BD8
    lda #$24
    sta a:$2D26
    ldx z:zField93
    lda a:$8BCF,x
    ldx z:zCharacter
    beq StepStageFrame122E_at3
    cmp #$0D
    beq StepStageFrame122E_at4
    cmp #$11
    bne StepStageFrame122E_at3
    ldx #$28
    stx a:$2D26
    lda #$71
    bra StepStageFrame122E_at3
StepStageFrame122E_at4:
    lda #$72
StepStageFrame122E_at3:
    sta a:$2ABE
    lda z:zPlayerHealth
    cmp z:zField99
    beq StepStageFrame122E_at5
    bcs StepStageFrame122E_at6
    jsr $8BF9
    ldx #$FF
    ldy #$F0
    jsr $9350
    dec z:zField99
    bra StepStageFrame122E_x0
StepStageFrame122E_at6:
    bbr5 zField8E,StepStageFrame122E_at7
    bbs6 zField8E,StepStageFrame122E_at7
    lda #$0E
    jsr VecSoundCommand
    smb6 z:zField8E
StepStageFrame122E_at7:
    inc z:zField99
    bsr $8BF9
    ldx #$00
    ldy #$0F
    jsr $935F
    bra StepStageFrame122E_at8
StepStageFrame122E_at5:
    smb5 z:zField8E
StepStageFrame122E_x0:
    bbr6 zField8E,StepStageFrame122E_at8
    rmb6 z:zField8E
    lda #$4A
    jsr VecSoundCommand
StepStageFrame122E_at8:
    jsr $9307
    stz a:StageBlendOn
    lda z:zField94
    cmp z:zField95
    beq StepStageFrame122E_at9
    sta z:zField95
    and #$0F
    ldx #$08
    jsr $8D3A
    lda z:zField94
    lsr a
    lsr a
    lsr a
    lsr a
    ldx #$09
    jsr $8D3A
StepStageFrame122E_at9:
    bbs2 zField8E,StepStageFrame122E_at10
    rts
StepStageFrame122E_at10:
    jmp $8C2A

; @routine 1A2E:0A2 InitStage1A2E
; @run 6B:88A2
; @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 1A2E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
; object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
; loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
; HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
; WAITS: no.
; Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
InitStage1A2E:
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
    jsr VecClearScreenState
    bra InitStage1A2E_at0
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
InitStage1A2E_at0:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    stz a:OBJ_2FE8
    lda z:zField8E
    pha
    bsr LoadStageEntry1A2E
    pla
    sta z:zField8E
    jsr $80A0
    jsr $A8EC
    ldy z:zRasterEffect
    lda a:$8889,y
    beq InitStage1A2E_at1
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    lda z:zField8E
    pha
    bsr LoadStageEntry1A2E
    pla
    sta z:zField8E
    jsr $A94D
InitStage1A2E_at1:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088

; @routine 1A2E:108 LoadStageEntry1A2E
; @run 6B:8908
; @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 1A2E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
; limits, the player's start position and the raster effect.
; HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9D52, its character offsets at
; $89E8.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
; zTemp00-01.
LoadStageEntry1A2E:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:$9D52,y
    sta z:zTemp00
    lda a:$9D53,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntry1A2E_at0
    rmb1 z:zField8E
    iny
    iny
    bra LoadStageEntry1A2E_at1
LoadStageEntry1A2E_at0:
    lda (zTemp00),y
    bne LoadStageEntry1A2E_at2
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra LoadStageEntry1A2E_at3
LoadStageEntry1A2E_at2:
    iny
    sta a:OBJ_X_LO
LoadStageEntry1A2E_at3:
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq LoadStageEntry1A2E_at1
    lda a:OBJ_27A8
    cmp #$05
    bcc LoadStageEntry1A2E_at1
    sec
    sbc #$05
    cmp #$04
    bcs LoadStageEntry1A2E_at1
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$89E8,x
    sta a:OBJ_X_LO
LoadStageEntry1A2E_at1:
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntry1A2E_at4
    iny
    sta a:OBJ_2AC0
    bra LoadStageEntry1A2E_at5
LoadStageEntry1A2E_at4:
    iny
LoadStageEntry1A2E_at5:
    rts

; @routine 1A2E:1EC StageScrollStep1A2E
; @run 6B:89EC
; @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 1A2E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
; offset the overlay keeps (the byte at $ADC9), into MPR5 and MPR6, then steps the stream with the scroll.
; HOW: ScrollStepBody (stage_scroll.c).
; WAITS: no.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageScrollStep1A2E:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$ADC9
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 1A2E:215 SetUpStageObjects1A2E
; @run 6B:8A15
; @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 1A2E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
; section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
; and runs the first frame of objects.
; HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9E12; the tail jump goes to $8A05.
; WAITS: no.
; Entry: X the caller's. Exit: as RunObjects.
SetUpStageObjects1A2E:
    jsr VecClearEnemySlots
    jsr $80A0
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9E12,y
    sta a:LevelStreamLo
    lda a:$9E13,y
    sta a:LevelStreamHi
    lda a:$9E14,y
    sta a:LevelAxis
    lda a:$9E15,y
    beq SetUpStageObjects1A2E_at0
    ldx #$1F
SetUpStageObjects1A2E_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjects1A2E_at1
SetUpStageObjects1A2E_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$ADC9
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp $8A05

; @routine 1A2E:309 SpawnStageObjectsRows011A2E
; @run 6B:8B09
; @unreachable 6B:8B09-6B:8B22 no read the game makes loads disc sector 1A2E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
; bit 5 of zField8E and starts list channel 4 with list $65.
; HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObject's.
; WAITS: no.
; Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
SpawnStageObjectsRows011A2E:
    clx
    jsr $936E
    inx
    jsr $936E
    stz a:StageVar3616
    lda #$C0
    sta a:StageVar3616+1
    rmb5 z:zField8E
    lda #$65
    ldx #$04
    jmp VecStartListChannelKind2

; @routine 1A2E:323 StepStageFrame1A2E
; @run 6B:8B23
; @stop 8BCF
; @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 1A2E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
; number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
; the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
; changed.
; HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
; SetVramAddresses.
; WAITS: no.
; Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
StepStageFrame1A2E:
    lda a:$2ABF
    beq $8B22
    bbs1 zPaused,StepStageFrame1A2E_at0
    lda a:StageVar3616+1
    beq StepStageFrame1A2E_at1
    dec a:StageVar3616+1
    bne StepStageFrame1A2E_at1
    smb1 z:zPaused
StepStageFrame1A2E_at0:
    jsr $8EAF
    bra StepStageFrame1A2E_at2
StepStageFrame1A2E_at1:
    jsr $910F
    jsr $913F
    jsr $916A
StepStageFrame1A2E_at2:
    jsr $8BD8
    lda #$24
    sta a:$2D26
    ldx z:zField93
    lda a:$8BCF,x
    ldx z:zCharacter
    beq StepStageFrame1A2E_at3
    cmp #$0D
    beq StepStageFrame1A2E_at4
    cmp #$11
    bne StepStageFrame1A2E_at3
    ldx #$28
    stx a:$2D26
    lda #$71
    bra StepStageFrame1A2E_at3
StepStageFrame1A2E_at4:
    lda #$72
StepStageFrame1A2E_at3:
    sta a:$2ABE
    lda z:zPlayerHealth
    cmp z:zField99
    beq StepStageFrame1A2E_at5
    bcs StepStageFrame1A2E_at6
    jsr $8BF9
    ldx #$FF
    ldy #$F0
    jsr $9350
    dec z:zField99
    bra StepStageFrame1A2E_x0
StepStageFrame1A2E_at6:
    bbr5 zField8E,StepStageFrame1A2E_at7
    bbs6 zField8E,StepStageFrame1A2E_at7
    lda #$0E
    jsr VecSoundCommand
    smb6 z:zField8E
StepStageFrame1A2E_at7:
    inc z:zField99
    bsr $8BF9
    ldx #$00
    ldy #$0F
    jsr $935F
    bra StepStageFrame1A2E_at8
StepStageFrame1A2E_at5:
    smb5 z:zField8E
StepStageFrame1A2E_x0:
    bbr6 zField8E,StepStageFrame1A2E_at8
    rmb6 z:zField8E
    lda #$4A
    jsr VecSoundCommand
StepStageFrame1A2E_at8:
    jsr $9307
    stz a:StageBlendOn
    lda z:zField94
    cmp z:zField95
    beq StepStageFrame1A2E_at9
    sta z:zField95
    and #$0F
    ldx #$08
    jsr $8D3A
    lda z:zField94
    lsr a
    lsr a
    lsr a
    lsr a
    ldx #$09
    jsr $8D3A
StepStageFrame1A2E_at9:
    bbs2 zField8E,StepStageFrame1A2E_at10
    rts
StepStageFrame1A2E_at10:
    jmp $8C2A

; @routine 1B8E:0A2 InitStage1B8E
; @run 6B:88A2
; @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 1B8E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
; object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
; loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
; HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
; WAITS: no.
; Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
InitStage1B8E:
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
    jsr VecClearScreenState
    bra InitStage1B8E_at0
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
InitStage1B8E_at0:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    stz a:OBJ_2FE8
    lda z:zField8E
    pha
    bsr LoadStageEntry1B8E
    pla
    sta z:zField8E
    jsr $80A0
    jsr $A8EC
    ldy z:zRasterEffect
    lda a:$8889,y
    beq InitStage1B8E_at1
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    lda z:zField8E
    pha
    bsr LoadStageEntry1B8E
    pla
    sta z:zField8E
    jsr $A94D
InitStage1B8E_at1:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088

; @routine 1B8E:108 LoadStageEntry1B8E
; @run 6B:8908
; @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 1B8E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
; limits, the player's start position and the raster effect.
; HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9D63, its character offsets at
; $89E8.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
; zTemp00-01.
LoadStageEntry1B8E:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:$9D63,y
    sta z:zTemp00
    lda a:$9D64,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntry1B8E_at0
    rmb1 z:zField8E
    iny
    iny
    bra LoadStageEntry1B8E_at1
LoadStageEntry1B8E_at0:
    lda (zTemp00),y
    bne LoadStageEntry1B8E_at2
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra LoadStageEntry1B8E_at3
LoadStageEntry1B8E_at2:
    iny
    sta a:OBJ_X_LO
LoadStageEntry1B8E_at3:
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq LoadStageEntry1B8E_at1
    lda a:OBJ_27A8
    cmp #$05
    bcc LoadStageEntry1B8E_at1
    sec
    sbc #$05
    cmp #$04
    bcs LoadStageEntry1B8E_at1
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$89E8,x
    sta a:OBJ_X_LO
LoadStageEntry1B8E_at1:
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntry1B8E_at4
    iny
    sta a:OBJ_2AC0
    bra LoadStageEntry1B8E_at5
LoadStageEntry1B8E_at4:
    iny
LoadStageEntry1B8E_at5:
    rts

; @routine 1B8E:1EC StageScrollStep1B8E
; @run 6B:89EC
; @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 1B8E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
; offset the overlay keeps (the byte at $AE25), into MPR5 and MPR6, then steps the stream with the scroll.
; HOW: ScrollStepBody (stage_scroll.c).
; WAITS: no.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageScrollStep1B8E:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AE25
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 1B8E:215 SetUpStageObjects1B8E
; @run 6B:8A15
; @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 1B8E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
; section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
; and runs the first frame of objects.
; HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9EC3; the tail jump goes to $8A05.
; WAITS: no.
; Entry: X the caller's. Exit: as RunObjects.
SetUpStageObjects1B8E:
    jsr VecClearEnemySlots
    jsr $80A0
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9EC3,y
    sta a:LevelStreamLo
    lda a:$9EC4,y
    sta a:LevelStreamHi
    lda a:$9EC5,y
    sta a:LevelAxis
    lda a:$9EC6,y
    beq SetUpStageObjects1B8E_at0
    ldx #$1F
SetUpStageObjects1B8E_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjects1B8E_at1
SetUpStageObjects1B8E_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AE25
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp $8A05

; @routine 1B8E:309 SpawnStageObjectsRows011B8E
; @run 6B:8B09
; @unreachable 6B:8B09-6B:8B22 no read the game makes loads disc sector 1B8E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
; bit 5 of zField8E and starts list channel 4 with list $65.
; HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObject's.
; WAITS: no.
; Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
SpawnStageObjectsRows011B8E:
    clx
    jsr $936E
    inx
    jsr $936E
    stz a:StageVar3616
    lda #$C0
    sta a:StageVar3616+1
    rmb5 z:zField8E
    lda #$65
    ldx #$04
    jmp VecStartListChannelKind2

; @routine 1B8E:323 StepStageFrame1B8E
; @run 6B:8B23
; @stop 8BCF
; @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 1B8E, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
; number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
; the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
; changed.
; HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
; SetVramAddresses.
; WAITS: no.
; Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
StepStageFrame1B8E:
    lda a:$2ABF
    beq $8B22
    bbs1 zPaused,StepStageFrame1B8E_at0
    lda a:StageVar3616+1
    beq StepStageFrame1B8E_at1
    dec a:StageVar3616+1
    bne StepStageFrame1B8E_at1
    smb1 z:zPaused
StepStageFrame1B8E_at0:
    jsr $8EAF
    bra StepStageFrame1B8E_at2
StepStageFrame1B8E_at1:
    jsr $910F
    jsr $913F
    jsr $916A
StepStageFrame1B8E_at2:
    jsr $8BD8
    lda #$24
    sta a:$2D26
    ldx z:zField93
    lda a:$8BCF,x
    ldx z:zCharacter
    beq StepStageFrame1B8E_at3
    cmp #$0D
    beq StepStageFrame1B8E_at4
    cmp #$11
    bne StepStageFrame1B8E_at3
    ldx #$28
    stx a:$2D26
    lda #$71
    bra StepStageFrame1B8E_at3
StepStageFrame1B8E_at4:
    lda #$72
StepStageFrame1B8E_at3:
    sta a:$2ABE
    lda z:zPlayerHealth
    cmp z:zField99
    beq StepStageFrame1B8E_at5
    bcs StepStageFrame1B8E_at6
    jsr $8BF9
    ldx #$FF
    ldy #$F0
    jsr $9350
    dec z:zField99
    bra StepStageFrame1B8E_x0
StepStageFrame1B8E_at6:
    bbr5 zField8E,StepStageFrame1B8E_at7
    bbs6 zField8E,StepStageFrame1B8E_at7
    lda #$0E
    jsr VecSoundCommand
    smb6 z:zField8E
StepStageFrame1B8E_at7:
    inc z:zField99
    bsr $8BF9
    ldx #$00
    ldy #$0F
    jsr $935F
    bra StepStageFrame1B8E_at8
StepStageFrame1B8E_at5:
    smb5 z:zField8E
StepStageFrame1B8E_x0:
    bbr6 zField8E,StepStageFrame1B8E_at8
    rmb6 z:zField8E
    lda #$4A
    jsr VecSoundCommand
StepStageFrame1B8E_at8:
    jsr $9307
    stz a:StageBlendOn
    lda z:zField94
    cmp z:zField95
    beq StepStageFrame1B8E_at9
    sta z:zField95
    and #$0F
    ldx #$08
    jsr $8D3A
    lda z:zField94
    lsr a
    lsr a
    lsr a
    lsr a
    ldx #$09
    jsr $8D3A
StepStageFrame1B8E_at9:
    bbs2 zField8E,StepStageFrame1B8E_at10
    rts
StepStageFrame1B8E_at10:
    jmp $8C2A

; @routine 1CEE:0A2 InitStage1CEE
; @run 6B:88A2
; @unreachable 6B:88A2-6B:8907 no read the game makes loads disc sector 1CEE, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $88A2 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry record, maps the
; object bank and runs the overlay's set-up at $A8EC; when the section's effect byte has a second set-up, clears and
; loads again and runs the one at $A94D; then clears the scroll state once more and runs on into the entry loader.
; HOW: InitStageBody (stage_init.c) with this copy's return addresses and tables.
; WAITS: no.
; Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
InitStage1CEE:
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
    jsr VecClearScreenState
    bra InitStage1CEE_at0
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
InitStage1CEE_at0:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    stz a:OBJ_2FE8
    lda z:zField8E
    pha
    bsr LoadStageEntry1CEE
    pla
    sta z:zField8E
    jsr $80A0
    jsr $A8EC
    ldy z:zRasterEffect
    lda a:$8889,y
    beq InitStage1CEE_at1
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    lda z:zField8E
    pha
    bsr LoadStageEntry1CEE
    pla
    sta z:zField8E
    jsr $A94D
InitStage1CEE_at1:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088

; @routine 1CEE:108 LoadStageEntry1CEE
; @run 6B:8908
; @unreachable 6B:8908-6B:89EB no read the game makes loads disc sector 1CEE, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8908 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position, the camera mode and
; limits, the player's start position and the raster effect.
; HOW: LoadStageEntryBody (stage_entry.c); this copy's table of section records is at $9CF6, its character offsets at
; $89E8.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y. Clobbers
; zTemp00-01.
LoadStageEntry1CEE:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:$9CF6,y
    sta z:zTemp00
    lda a:$9CF7,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntry1CEE_at0
    rmb1 z:zField8E
    iny
    iny
    bra LoadStageEntry1CEE_at1
LoadStageEntry1CEE_at0:
    lda (zTemp00),y
    bne LoadStageEntry1CEE_at2
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra LoadStageEntry1CEE_at3
LoadStageEntry1CEE_at2:
    iny
    sta a:OBJ_X_LO
LoadStageEntry1CEE_at3:
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq LoadStageEntry1CEE_at1
    lda a:OBJ_27A8
    cmp #$05
    bcc LoadStageEntry1CEE_at1
    sec
    sbc #$05
    cmp #$04
    bcs LoadStageEntry1CEE_at1
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$89E8,x
    sta a:OBJ_X_LO
LoadStageEntry1CEE_at1:
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntry1CEE_at4
    iny
    sta a:OBJ_2AC0
    bra LoadStageEntry1CEE_at5
LoadStageEntry1CEE_at4:
    iny
LoadStageEntry1CEE_at5:
    rts

; @routine 1CEE:1EC StageScrollStep1CEE
; @run 6B:89EC
; @unreachable 6B:89EC-6B:8A04 no read the game makes loads disc sector 1CEE, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $89EC runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base bank and the second at the
; offset the overlay keeps (the byte at $AE38), into MPR5 and MPR6, then steps the stream with the scroll.
; HOW: ScrollStepBody (stage_scroll.c).
; WAITS: no.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageScrollStep1CEE:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AE38
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 1CEE:215 SetUpStageObjects1CEE
; @run 6B:8A15
; @unreachable 6B:8A15-6B:8A59 no read the game makes loads disc sector 1CEE, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8A15 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sets a stage's objects up: clears the enemy slots, places the object stream and the axis from the
; section's record, clears the event flags when the record says to, maps the object banks, walks the whole stream once,
; and runs the first frame of objects.
; HOW: SetUpObjectsBody (stage_objects.c); this copy's section records are at $9D76; the tail jump goes to $8A05.
; WAITS: no.
; Entry: X the caller's. Exit: as RunObjects.
SetUpStageObjects1CEE:
    jsr VecClearEnemySlots
    jsr $80A0
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9D76,y
    sta a:LevelStreamLo
    lda a:$9D77,y
    sta a:LevelStreamHi
    lda a:$9D78,y
    sta a:LevelAxis
    lda a:$9D79,y
    beq SetUpStageObjects1CEE_at0
    ldx #$1F
SetUpStageObjects1CEE_at1:
    stz a:EventFlags,x
    dex
    bpl SetUpStageObjects1CEE_at1
SetUpStageObjects1CEE_at0:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AE38
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamFullWalk
    jmp $8A05

; @routine 1CEE:309 SpawnStageObjectsRows011CEE
; @run 6B:8B09
; @unreachable 6B:8B09-6B:8B22 no read the game makes loads disc sector 1CEE, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B09 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state bytes (one to $C0), clears
; bit 5 of zField8E and starts list channel 4 with list $65.
; HOW: SpawnFirstPairBody (stage_spawn.c); the spawn code is SpawnStageObject's.
; WAITS: no.
; Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
SpawnStageObjectsRows011CEE:
    clx
    jsr $936E
    inx
    jsr $936E
    stz a:StageVar3616
    lda #$C0
    sta a:StageVar3616+1
    rmb5 z:zField8E
    lda #$65
    ldx #$04
    jmp VecStartListChannelKind2

; @routine 1CEE:323 StepStageFrame1CEE
; @run 6B:8B23
; @stop 8BCF
; @unreachable 6B:8B23-6B:8BCE no read the game makes loads disc sector 1CEE, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8B23 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening sequence or the three
; number blends, starts the wave's script, picks the frame the character shows, moves the health bar one step towards
; the health (with a sound each way), maps the stage banks and blends the two rows of the shown byte zField94 when it
; changed.
; HOW: StepStageFrameBody (stage_tick.c); this copy's health sounds are $0E and $4A; the bar's VRAM address routine is
; SetVramAddresses.
; WAITS: no.
; Entry: X and Y as the blends take them; V as the caller left it. Exit: as the last step left it.
StepStageFrame1CEE:
    lda a:$2ABF
    beq $8B22
    bbs1 zPaused,StepStageFrame1CEE_at0
    lda a:StageVar3616+1
    beq StepStageFrame1CEE_at1
    dec a:StageVar3616+1
    bne StepStageFrame1CEE_at1
    smb1 z:zPaused
StepStageFrame1CEE_at0:
    jsr $8EAF
    bra StepStageFrame1CEE_at2
StepStageFrame1CEE_at1:
    jsr $910F
    jsr $913F
    jsr $916A
StepStageFrame1CEE_at2:
    jsr $8BD8
    lda #$24
    sta a:$2D26
    ldx z:zField93
    lda a:$8BCF,x
    ldx z:zCharacter
    beq StepStageFrame1CEE_at3
    cmp #$0D
    beq StepStageFrame1CEE_at4
    cmp #$11
    bne StepStageFrame1CEE_at3
    ldx #$28
    stx a:$2D26
    lda #$71
    bra StepStageFrame1CEE_at3
StepStageFrame1CEE_at4:
    lda #$72
StepStageFrame1CEE_at3:
    sta a:$2ABE
    lda z:zPlayerHealth
    cmp z:zField99
    beq StepStageFrame1CEE_at5
    bcs StepStageFrame1CEE_at6
    jsr $8BF9
    ldx #$FF
    ldy #$F0
    jsr $9350
    dec z:zField99
    bra StepStageFrame1CEE_x0
StepStageFrame1CEE_at6:
    bbr5 zField8E,StepStageFrame1CEE_at7
    bbs6 zField8E,StepStageFrame1CEE_at7
    lda #$0E
    jsr VecSoundCommand
    smb6 z:zField8E
StepStageFrame1CEE_at7:
    inc z:zField99
    bsr $8BF9
    ldx #$00
    ldy #$0F
    jsr $935F
    bra StepStageFrame1CEE_at8
StepStageFrame1CEE_at5:
    smb5 z:zField8E
StepStageFrame1CEE_x0:
    bbr6 zField8E,StepStageFrame1CEE_at8
    rmb6 z:zField8E
    lda #$4A
    jsr VecSoundCommand
StepStageFrame1CEE_at8:
    jsr $9307
    stz a:StageBlendOn
    lda z:zField94
    cmp z:zField95
    beq StepStageFrame1CEE_at9
    sta z:zField95
    and #$0F
    ldx #$08
    jsr $8D3A
    lda z:zField94
    lsr a
    lsr a
    lsr a
    lsr a
    ldx #$09
    jsr $8D3A
StepStageFrame1CEE_at9:
    bbs2 zField8E,StepStageFrame1CEE_at10
    rts
StepStageFrame1CEE_at10:
    jmp $8C2A
