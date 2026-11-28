; @routine 0441:018 RunListChannel0441
; @run 6C:8018
; @stop 8133
; @unreachable 6C:8018-6C:8132 no read the game makes loads disc sector 0441, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8018 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Runs one list channel by its kind: 1 plays a list of VRAM lists with a hold between them, 3 steps a
; list of VRAM copies (its lists from the table at $8E6A), any other runs one VRAM list and frees the channel.
; HOW: The kind counted down to pick the path. Kind 1 reads the overlay's table of channel lists at $9800, the hold
; from the list and the VRAM list after it (VramListFromIndex, a tail jump); $FF ends it, $FE starts it again and runs
; the routine from its top with A $FE. Kind 3 waits while the game is paused; each entry of its list is a hold (0),
; a restart (2), or a VRAM copy (words read from the read port and written back, from a plain address pair or, for 1,
; a pair packed in a byte); AddToListWord steps the list word on. Other kinds: VramListFromIndex, then the channel freed.
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
; liftcmp: 414 of 414 cases match its census draft (census_draft.py; callees as filed wrappers; curated
; census_list_channels_0441.txt, every path); every lifted line reached. The draft's line for $80FA is reported
; unreached but is in the one straight block from $80F9 to $8103 that curated case 4 runs.
ListChannelCopySource0441 = $9800 ; the word the VRAM copy lists take as source
ListChannelCopyLists0441 = $8E6A ; the VRAM copy lists, a word by list number
RunListChannel0441:
    dec a
    beq TitleSteps
    dec a
    dec a
    beq RunListChannel0441_at0
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp EmitObjectSprites
RunListChannel0441_at1:
    dec a:ListChannelHold,x
    rts
    lda a:ListChannelHold,x
    bne RunListChannel0441_at1
    lda a:ListChannelCopySource0441
    sta z:zBiosAl
    lda a:ListChannelCopySource0441+1
    sta z:zBiosAh
    lda a:ListChannelList,x
    asl a
    tay
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
    lda a:ListChannelPlace,x
    asl a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EmitObjectSprites
    cmp #$FE
    beq RunListChannel0441_at2
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel0441_at3:
    rts
RunListChannel0441_at2:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel0441
RunListChannel0441_at0:
    bbs0 zPaused,RunListChannel0441_at3
    lda a:ListChannelPlace,x
    bne RunListChannel0441_at4
    inc a:ListChannelPlace,x
RunListChannel0441_at7:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:ListChannelCopyLists0441,y
    sta a:ListWordLo,x
    lda a:ListChannelCopyLists0441+1,y
    sta a:ListWordHi,x
    bra TitleBlink
RunListChannel0441_at4:
    lda a:ListChannelHold,x
    bne RunListChannel0441_at1
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi EmitObjectSprites
    beq RunListChannel0441_at5
    dec a
    beq RunListChannel0441_at6
    dec a
    beq RunListChannel0441_at7
    jsr VecClearVdcControlHigh
    jsr VecSelectVramReadAddress
    ldy #$01
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    iny
    phx
    ldx #$06
    bra TitleCycle
RunListChannel0441_at6:
    jsr VecClearVdcControlHigh
    jsr VecSelectVramReadAddress
    ldy #$03
    lda (zTemp00),y
    pha
    and #$F0
    sta a:VDC_DATA_LO
    ldy #$01
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    pla
    and #$0F
    asl a
    asl a
    asl a
    asl a
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    phx
    ldx #$05
    ldy #$04
    lda (zTemp00),y
    tay
RunListChannel0441_at8:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel0441_at8
    txa
    plx
    bsr AddToListWord
    jmp TitleBlink
RunListChannel0441_at5:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 0651:018 RunListChannel0651
; @run 6C:8018
; @stop 8133
; @unreachable 6C:8018-6C:8132 no read the game makes loads disc sector 0651, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8018 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Runs one list channel by its kind: 1 plays a list of VRAM lists with a hold between them, 3 steps a
; list of VRAM copies (its lists from the table at $8DC1), any other runs one VRAM list and frees the channel.
; HOW: The kind counted down to pick the path. Kind 1 reads the overlay's table of channel lists at $9800, the hold
; from the list and the VRAM list after it (VramListFromIndex, a tail jump); $FF ends it, $FE starts it again and runs
; the routine from its top with A $FE. Kind 3 waits while the game is paused; each entry of its list is a hold (0),
; a restart (2), or a VRAM copy (words read from the read port and written back, from a plain address pair or, for 1,
; a pair packed in a byte); AddToListWord steps the list word on. Other kinds: VramListFromIndex, then the channel freed.
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
; liftcmp: 414 of 414 cases match its census draft (census_draft.py; callees as filed wrappers; curated
; census_list_channels_0651.txt, every path); every lifted line reached. The draft's line for $80FA is reported
; unreached but is in the one straight block from $80F9 to $8103 that curated case 4 runs.
ListChannelCopySource0651 = $9800 ; the word the VRAM copy lists take as source
ListChannelCopyLists0651 = $8DC1 ; the VRAM copy lists, a word by list number
RunListChannel0651:
    dec a
    beq TitleSteps
    dec a
    dec a
    beq RunListChannel0651_at0
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp EmitObjectSprites
RunListChannel0651_at1:
    dec a:ListChannelHold,x
    rts
    lda a:ListChannelHold,x
    bne RunListChannel0651_at1
    lda a:ListChannelCopySource0651
    sta z:zBiosAl
    lda a:ListChannelCopySource0651+1
    sta z:zBiosAh
    lda a:ListChannelList,x
    asl a
    tay
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
    lda a:ListChannelPlace,x
    asl a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EmitObjectSprites
    cmp #$FE
    beq RunListChannel0651_at2
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel0651_at3:
    rts
RunListChannel0651_at2:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel0651
RunListChannel0651_at0:
    bbs0 zPaused,RunListChannel0651_at3
    lda a:ListChannelPlace,x
    bne RunListChannel0651_at4
    inc a:ListChannelPlace,x
RunListChannel0651_at7:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:ListChannelCopyLists0651,y
    sta a:ListWordLo,x
    lda a:ListChannelCopyLists0651+1,y
    sta a:ListWordHi,x
    bra TitleBlink
RunListChannel0651_at4:
    lda a:ListChannelHold,x
    bne RunListChannel0651_at1
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi EmitObjectSprites
    beq RunListChannel0651_at5
    dec a
    beq RunListChannel0651_at6
    dec a
    beq RunListChannel0651_at7
    jsr VecClearVdcControlHigh
    jsr VecSelectVramReadAddress
    ldy #$01
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    iny
    phx
    ldx #$06
    bra TitleCycle
RunListChannel0651_at6:
    jsr VecClearVdcControlHigh
    jsr VecSelectVramReadAddress
    ldy #$03
    lda (zTemp00),y
    pha
    and #$F0
    sta a:VDC_DATA_LO
    ldy #$01
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    pla
    and #$0F
    asl a
    asl a
    asl a
    asl a
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    phx
    ldx #$05
    ldy #$04
    lda (zTemp00),y
    tay
RunListChannel0651_at8:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel0651_at8
    txa
    plx
    bsr AddToListWord
    jmp TitleBlink
RunListChannel0651_at5:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 1231:018 RunListChannel1231
; @run 6C:8018
; @stop 8133
; @unreachable 6C:8018-6C:8132 no read the game makes loads disc sector 1231, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8018 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Runs one list channel by its kind: 1 plays a list of VRAM lists with a hold between them, 3 steps a
; list of VRAM copies (its lists from the table at $8EA0), any other runs one VRAM list and frees the channel.
; HOW: The kind counted down to pick the path. Kind 1 reads the overlay's table of channel lists at $9800, the hold
; from the list and the VRAM list after it (VramListFromIndex, a tail jump); $FF ends it, $FE starts it again and runs
; the routine from its top with A $FE. Kind 3 waits while the game is paused; each entry of its list is a hold (0),
; a restart (2), or a VRAM copy (words read from the read port and written back, from a plain address pair or, for 1,
; a pair packed in a byte); AddToListWord steps the list word on. Other kinds: VramListFromIndex, then the channel freed.
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
; liftcmp: 414 of 414 cases match its census draft (census_draft.py; callees as filed wrappers; curated
; census_list_channels_1231.txt, every path); every lifted line reached. The draft's line for $80FA is reported
; unreached but is in the one straight block from $80F9 to $8103 that curated case 4 runs.
ListChannelCopySource1231 = $9800 ; the word the VRAM copy lists take as source
ListChannelCopyLists1231 = $8EA0 ; the VRAM copy lists, a word by list number
RunListChannel1231:
    dec a
    beq TitleSteps
    dec a
    dec a
    beq RunListChannel1231_at0
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp EmitObjectSprites
RunListChannel1231_at1:
    dec a:ListChannelHold,x
    rts
    lda a:ListChannelHold,x
    bne RunListChannel1231_at1
    lda a:ListChannelCopySource1231
    sta z:zBiosAl
    lda a:ListChannelCopySource1231+1
    sta z:zBiosAh
    lda a:ListChannelList,x
    asl a
    tay
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
    lda a:ListChannelPlace,x
    asl a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EmitObjectSprites
    cmp #$FE
    beq RunListChannel1231_at2
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel1231_at3:
    rts
RunListChannel1231_at2:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel1231
RunListChannel1231_at0:
    bbs0 zPaused,RunListChannel1231_at3
    lda a:ListChannelPlace,x
    bne RunListChannel1231_at4
    inc a:ListChannelPlace,x
RunListChannel1231_at7:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:ListChannelCopyLists1231,y
    sta a:ListWordLo,x
    lda a:ListChannelCopyLists1231+1,y
    sta a:ListWordHi,x
    bra TitleBlink
RunListChannel1231_at4:
    lda a:ListChannelHold,x
    bne RunListChannel1231_at1
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi EmitObjectSprites
    beq RunListChannel1231_at5
    dec a
    beq RunListChannel1231_at6
    dec a
    beq RunListChannel1231_at7
    jsr VecClearVdcControlHigh
    jsr VecSelectVramReadAddress
    ldy #$01
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    iny
    phx
    ldx #$06
    bra TitleCycle
RunListChannel1231_at6:
    jsr VecClearVdcControlHigh
    jsr VecSelectVramReadAddress
    ldy #$03
    lda (zTemp00),y
    pha
    and #$F0
    sta a:VDC_DATA_LO
    ldy #$01
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    pla
    and #$0F
    asl a
    asl a
    asl a
    asl a
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    phx
    ldx #$05
    ldy #$04
    lda (zTemp00),y
    tay
RunListChannel1231_at8:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel1231_at8
    txa
    plx
    bsr AddToListWord
    jmp TitleBlink
RunListChannel1231_at5:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 1A31:018 RunListChannel1A31
; @run 6C:8018
; @stop 8133
; @unreachable 6C:8018-6C:8132 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8018 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Runs one list channel by its kind: 1 plays a list of VRAM lists with a hold between them, 3 steps a
; list of VRAM copies (its lists from the table at $8E9A), any other runs one VRAM list and frees the channel.
; HOW: The kind counted down to pick the path. Kind 1 reads the overlay's table of channel lists at $9800, the hold
; from the list and the VRAM list after it (VramListFromIndex, a tail jump); $FF ends it, $FE starts it again and runs
; the routine from its top with A $FE. Kind 3 waits while the game is paused; each entry of its list is a hold (0),
; a restart (2), or a VRAM copy (words read from the read port and written back, from a plain address pair or, for 1,
; a pair packed in a byte); AddToListWord steps the list word on. Other kinds: VramListFromIndex, then the channel freed.
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
; liftcmp: 414 of 414 cases match its census draft (census_draft.py; callees as filed wrappers; curated
; census_list_channels_1A31.txt, every path); every lifted line reached. The draft's line for $80FA is reported
; unreached but is in the one straight block from $80F9 to $8103 that curated case 4 runs.
ListChannelCopySource1A31 = $9800 ; the word the VRAM copy lists take as source
RunListChannel1A31:
    dec a
    beq TitleSteps
    dec a
    dec a
    beq RunListChannel1A31_at0
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp EmitObjectSprites
RunListChannel1A31_at1:
    dec a:ListChannelHold,x
    rts
    lda a:ListChannelHold,x
    bne RunListChannel1A31_at1
    lda a:ListChannelCopySource1A31
    sta z:zBiosAl
    lda a:ListChannelCopySource1A31+1
    sta z:zBiosAh
    lda a:ListChannelList,x
    asl a
    tay
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
    lda a:ListChannelPlace,x
    asl a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EmitObjectSprites
    cmp #$FE
    beq RunListChannel1A31_at2
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel1A31_at3:
    rts
RunListChannel1A31_at2:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel1A31
RunListChannel1A31_at0:
    bbs0 zPaused,RunListChannel1A31_at3
    lda a:ListChannelPlace,x
    bne RunListChannel1A31_at4
    inc a:ListChannelPlace,x
RunListChannel1A31_at7:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:StageTileVramAddress+4,y
    sta a:ListWordLo,x
    lda a:StageTileVramAddress+5,y
    sta a:ListWordHi,x
    bra TitleBlink
RunListChannel1A31_at4:
    lda a:ListChannelHold,x
    bne RunListChannel1A31_at1
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi EmitObjectSprites
    beq RunListChannel1A31_at5
    dec a
    beq RunListChannel1A31_at6
    dec a
    beq RunListChannel1A31_at7
    jsr VecClearVdcControlHigh
    jsr VecSelectVramReadAddress
    ldy #$01
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    iny
    phx
    ldx #$06
    bra TitleCycle
RunListChannel1A31_at6:
    jsr VecClearVdcControlHigh
    jsr VecSelectVramReadAddress
    ldy #$03
    lda (zTemp00),y
    pha
    and #$F0
    sta a:VDC_DATA_LO
    ldy #$01
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    pla
    and #$0F
    asl a
    asl a
    asl a
    asl a
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    phx
    ldx #$05
    ldy #$04
    lda (zTemp00),y
    tay
RunListChannel1A31_at8:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel1A31_at8
    txa
    plx
    bsr AddToListWord
    jmp TitleBlink
RunListChannel1A31_at5:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 1B91:018 RunListChannel1B91
; @run 6C:8018
; @stop 8133
; @unreachable 6C:8018-6C:8132 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8018 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Runs one list channel by its kind: 1 plays a list of VRAM lists with a hold between them, 3 steps a
; list of VRAM copies (its lists from the table at $8CF2), any other runs one VRAM list and frees the channel.
; HOW: The kind counted down to pick the path. Kind 1 reads the overlay's table of channel lists at $9800, the hold
; from the list and the VRAM list after it (VramListFromIndex, a tail jump); $FF ends it, $FE starts it again and runs
; the routine from its top with A $FE. Kind 3 waits while the game is paused; each entry of its list is a hold (0),
; a restart (2), or a VRAM copy (words read from the read port and written back, from a plain address pair or, for 1,
; a pair packed in a byte); AddToListWord steps the list word on. Other kinds: VramListFromIndex, then the channel freed.
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
; liftcmp: 414 of 414 cases match its census draft (census_draft.py; callees as filed wrappers; curated
; census_list_channels_1B91.txt, every path); every lifted line reached. The draft's line for $80FA is reported
; unreached but is in the one straight block from $80F9 to $8103 that curated case 4 runs.
ListChannelCopySource1B91 = $9800 ; the word the VRAM copy lists take as source
ListChannelCopyLists1B91 = $8CF2 ; the VRAM copy lists, a word by list number
RunListChannel1B91:
    dec a
    beq TitleSteps
    dec a
    dec a
    beq RunListChannel1B91_at0
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp EmitObjectSprites
RunListChannel1B91_at1:
    dec a:ListChannelHold,x
    rts
    lda a:ListChannelHold,x
    bne RunListChannel1B91_at1
    lda a:ListChannelCopySource1B91
    sta z:zBiosAl
    lda a:ListChannelCopySource1B91+1
    sta z:zBiosAh
    lda a:ListChannelList,x
    asl a
    tay
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
    lda a:ListChannelPlace,x
    asl a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EmitObjectSprites
    cmp #$FE
    beq RunListChannel1B91_at2
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel1B91_at3:
    rts
RunListChannel1B91_at2:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel1B91
RunListChannel1B91_at0:
    bbs0 zPaused,RunListChannel1B91_at3
    lda a:ListChannelPlace,x
    bne RunListChannel1B91_at4
    inc a:ListChannelPlace,x
RunListChannel1B91_at7:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:ListChannelCopyLists1B91,y
    sta a:ListWordLo,x
    lda a:ListChannelCopyLists1B91+1,y
    sta a:ListWordHi,x
    bra TitleBlink
RunListChannel1B91_at4:
    lda a:ListChannelHold,x
    bne RunListChannel1B91_at1
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi EmitObjectSprites
    beq RunListChannel1B91_at5
    dec a
    beq RunListChannel1B91_at6
    dec a
    beq RunListChannel1B91_at7
    jsr VecClearVdcControlHigh
    jsr VecSelectVramReadAddress
    ldy #$01
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    iny
    phx
    ldx #$06
    bra TitleCycle
RunListChannel1B91_at6:
    jsr VecClearVdcControlHigh
    jsr VecSelectVramReadAddress
    ldy #$03
    lda (zTemp00),y
    pha
    and #$F0
    sta a:VDC_DATA_LO
    ldy #$01
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    pla
    and #$0F
    asl a
    asl a
    asl a
    asl a
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    phx
    ldx #$05
    ldy #$04
    lda (zTemp00),y
    tay
RunListChannel1B91_at8:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel1B91_at8
    txa
    plx
    bsr AddToListWord
    jmp TitleBlink
RunListChannel1B91_at5:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 1CF1:018 RunListChannel1CF1
; @run 6C:8018
; @stop 8133
; @unreachable 6C:8018-6C:8132 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8018 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Runs one list channel by its kind: 1 plays a list of VRAM lists with a hold between them, 3 steps a
; list of VRAM copies (its lists from the table at $9049), any other runs one VRAM list and frees the channel.
; HOW: The kind counted down to pick the path. Kind 1 reads the overlay's table of channel lists at $9800, the hold
; from the list and the VRAM list after it (VramListFromIndex, a tail jump); $FF ends it, $FE starts it again and runs
; the routine from its top with A $FE. Kind 3 waits while the game is paused; each entry of its list is a hold (0),
; a restart (2), or a VRAM copy (words read from the read port and written back, from a plain address pair or, for 1,
; a pair packed in a byte); AddToListWord steps the list word on. Other kinds: VramListFromIndex, then the channel freed.
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
ListChannelCopySource1CF1 = $9800 ; the word the VRAM copy lists take as source
ListChannelCopyLists1CF1 = $9049 ; the VRAM copy lists, a word by list number
RunListChannel1CF1:
    dec a
    beq TitleSteps
    dec a
    dec a
    beq RunListChannel1CF1_at0
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp EmitObjectSprites
RunListChannel1CF1_at1:
    dec a:ListChannelHold,x
    rts
    lda a:ListChannelHold,x
    bne RunListChannel1CF1_at1
    lda a:ListChannelCopySource1CF1
    sta z:zBiosAl
    lda a:ListChannelCopySource1CF1+1
    sta z:zBiosAh
    lda a:ListChannelList,x
    asl a
    tay
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
    lda a:ListChannelPlace,x
    asl a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EmitObjectSprites
    cmp #$FE
    beq RunListChannel1CF1_at2
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel1CF1_at3:
    rts
RunListChannel1CF1_at2:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel1CF1
RunListChannel1CF1_at0:
    bbs0 zPaused,RunListChannel1CF1_at3
    lda a:ListChannelPlace,x
    bne RunListChannel1CF1_at4
    inc a:ListChannelPlace,x
RunListChannel1CF1_at7:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:ListChannelCopyLists1CF1,y
    sta a:ListWordLo,x
    lda a:ListChannelCopyLists1CF1+1,y
    sta a:ListWordHi,x
    bra TitleBlink
RunListChannel1CF1_at4:
    lda a:ListChannelHold,x
    bne RunListChannel1CF1_at1
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi EmitObjectSprites
    beq RunListChannel1CF1_at5
    dec a
    beq RunListChannel1CF1_at6
    dec a
    beq RunListChannel1CF1_at7
    jsr VecClearVdcControlHigh
    jsr VecSelectVramReadAddress
    ldy #$01
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    iny
    phx
    ldx #$06
    bra TitleCycle
RunListChannel1CF1_at6:
    jsr VecClearVdcControlHigh
    jsr VecSelectVramReadAddress
    ldy #$03
    lda (zTemp00),y
    pha
    and #$F0
    sta a:VDC_DATA_LO
    ldy #$01
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    pla
    and #$0F
    asl a
    asl a
    asl a
    asl a
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    phx
    ldx #$05
    ldy #$04
    lda (zTemp00),y
    tay
RunListChannel1CF1_at8:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel1CF1_at8
    txa
    plx
    bsr AddToListWord
    jmp TitleBlink
RunListChannel1CF1_at5:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 25F7:018 RunListChannel25F7
; @run 6C:8018
; @stop 8133
; @unreachable 6C:8018-6C:8132 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8018 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Runs one list channel by its kind: 1 plays a list of VRAM lists with a hold between them, 3 steps a
; list of VRAM copies (its lists from the table at $829F), any other runs one VRAM list and frees the channel.
; HOW: The kind counted down to pick the path. Kind 1 reads the overlay's table of channel lists at $9800, the hold
; from the list and the VRAM list after it (VramListFromIndex, a tail jump); $FF ends it, $FE starts it again and runs
; the routine from its top with A $FE. Kind 3 waits while the game is paused; each entry of its list is a hold (0),
; a restart (2), or a VRAM copy (words read from the read port and written back, from a plain address pair or, for 1,
; a pair packed in a byte); AddToListWord steps the list word on. Other kinds: VramListFromIndex, then the channel freed.
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
ListChannelCopySource25F7 = $9800 ; the word the VRAM copy lists take as source
ListChannelCopyLists25F7 = $829F ; the VRAM copy lists, a word by list number
RunListChannel25F7:
    dec a
    beq TitleSteps
    dec a
    dec a
    beq RunListChannel25F7_at0
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp EmitObjectSprites
RunListChannel25F7_at1:
    dec a:ListChannelHold,x
    rts
    lda a:ListChannelHold,x
    bne RunListChannel25F7_at1
    lda a:ListChannelCopySource25F7
    sta z:zBiosAl
    lda a:ListChannelCopySource25F7+1
    sta z:zBiosAh
    lda a:ListChannelList,x
    asl a
    tay
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
    lda a:ListChannelPlace,x
    asl a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EmitObjectSprites
    cmp #$FE
    beq RunListChannel25F7_at2
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel25F7_at3:
    rts
RunListChannel25F7_at2:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel25F7
RunListChannel25F7_at0:
    bbs0 zPaused,RunListChannel25F7_at3
    lda a:ListChannelPlace,x
    bne RunListChannel25F7_at4
    inc a:ListChannelPlace,x
RunListChannel25F7_at7:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:ListChannelCopyLists25F7,y
    sta a:ListWordLo,x
    lda a:ListChannelCopyLists25F7+1,y
    sta a:ListWordHi,x
    bra TitleBlink
RunListChannel25F7_at4:
    lda a:ListChannelHold,x
    bne RunListChannel25F7_at1
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi EmitObjectSprites
    beq RunListChannel25F7_at5
    dec a
    beq RunListChannel25F7_at6
    dec a
    beq RunListChannel25F7_at7
    jsr VecClearVdcControlHigh
    jsr VecSelectVramReadAddress
    ldy #$01
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    iny
    phx
    ldx #$06
    bra TitleCycle
RunListChannel25F7_at6:
    jsr VecClearVdcControlHigh
    jsr VecSelectVramReadAddress
    ldy #$03
    lda (zTemp00),y
    pha
    and #$F0
    sta a:VDC_DATA_LO
    ldy #$01
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    pla
    and #$0F
    asl a
    asl a
    asl a
    asl a
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    phx
    ldx #$05
    ldy #$04
    lda (zTemp00),y
    tay
RunListChannel25F7_at8:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel25F7_at8
    txa
    plx
    bsr AddToListWord
    jmp TitleBlink
RunListChannel25F7_at5:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02
