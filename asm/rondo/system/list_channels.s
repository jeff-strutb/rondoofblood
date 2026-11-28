; @routine 00C5:018 RunListChannel00C5
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $829F), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $829F, the stage overlay at sector 00C5 (0111, the ending's, holds the
; same bytes).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
ChannelLists = $9800 ; channel lists
ChannelCopyLists00C5 = $829F ; the table of copy lists a list channel of kind 3 steps
RunListChannel00C5:
    dec a
    beq TitleSteps
    dec a
    dec a
    beq RunListChannel00C5_at0
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp EmitObjectSprites
RunListChannel00C5_at1:
    dec a:ListChannelHold,x
    rts
    lda a:ListChannelHold,x
    bne RunListChannel00C5_at1
    lda a:OverlayListChannelSeqs
    sta z:zBiosAl
    lda a:ChannelLists+1
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
    beq RunListChannel00C5_at2
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel00C5_at3:
    rts
RunListChannel00C5_at2:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel00C5
RunListChannel00C5_at0:
    bbs0 zPaused,RunListChannel00C5_at3
    lda a:ListChannelPlace,x
    bne RunListChannel00C5_at4
    inc a:ListChannelPlace,x
RunListChannel00C5_at7:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:ChannelCopyLists00C5,y
    sta a:ListWordLo,x
    lda a:ChannelCopyLists00C5+1,y
    sta a:ListWordHi,x
    bra TitleBlink
RunListChannel00C5_at4:
    lda a:ListChannelHold,x
    bne RunListChannel00C5_at1
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi EmitObjectSprites
    beq RunListChannel00C5_at5
    dec a
    beq RunListChannel00C5_at6
    dec a
    beq RunListChannel00C5_at7
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
RunListChannel00C5_at6:
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
RunListChannel00C5_at8:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel00C5_at8
    txa
    plx
    bsr AddToListWord
    jmp TitleBlink
RunListChannel00C5_at5:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 07D1:018 RunListChannel07D1
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $91F2), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $91F2, sector 07D1's overlay.
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
ListChannelCopySource07D1 = $9800 ; the word the VRAM copy lists take as source
ListChannelCopyLists07D1 = $91F2 ; the VRAM copy lists, a word by list number
RunListChannel07D1:
    dec a
    beq TitleSteps
    dec a
    dec a
    beq RunListChannel07D1_at0
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp EmitObjectSprites
RunListChannel07D1_at1:
    dec a:ListChannelHold,x
    rts
    lda a:ListChannelHold,x
    bne RunListChannel07D1_at1
    lda a:ListChannelCopySource07D1
    sta z:zBiosAl
    lda a:ListChannelCopySource07D1+1
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
    beq RunListChannel07D1_at2
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel07D1_at3:
    rts
RunListChannel07D1_at2:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel07D1
RunListChannel07D1_at0:
    bbs0 zPaused,RunListChannel07D1_at3
    lda a:ListChannelPlace,x
    bne RunListChannel07D1_at4
    inc a:ListChannelPlace,x
RunListChannel07D1_at7:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:ListChannelCopyLists07D1,y
    sta a:ListWordLo,x
    lda a:ListChannelCopyLists07D1+1,y
    sta a:ListWordHi,x
    bra TitleBlink
RunListChannel07D1_at4:
    lda a:ListChannelHold,x
    bne RunListChannel07D1_at1
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi EmitObjectSprites
    beq RunListChannel07D1_at5
    dec a
    beq RunListChannel07D1_at6
    dec a
    beq RunListChannel07D1_at7
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
RunListChannel07D1_at6:
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
RunListChannel07D1_at8:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel07D1_at8
    txa
    plx
    bsr AddToListWord
    jmp TitleBlink
RunListChannel07D1_at5:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 0931:018 RunListChannel0931
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $8DEC), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $8DEC, sector 0931's overlay.
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
ListChannelCopySource0931 = $9800 ; the word the VRAM copy lists take as source
ListChannelCopyLists0931 = $8DEC ; the VRAM copy lists, a word by list number
RunListChannel0931:
    dec a
    beq TitleSteps
    dec a
    dec a
    beq RunListChannel0931_at0
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp EmitObjectSprites
RunListChannel0931_at1:
    dec a:ListChannelHold,x
    rts
    lda a:ListChannelHold,x
    bne RunListChannel0931_at1
    lda a:ListChannelCopySource0931
    sta z:zBiosAl
    lda a:ListChannelCopySource0931+1
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
    beq RunListChannel0931_at2
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel0931_at3:
    rts
RunListChannel0931_at2:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel0931
RunListChannel0931_at0:
    bbs0 zPaused,RunListChannel0931_at3
    lda a:ListChannelPlace,x
    bne RunListChannel0931_at4
    inc a:ListChannelPlace,x
RunListChannel0931_at7:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:ListChannelCopyLists0931,y
    sta a:ListWordLo,x
    lda a:ListChannelCopyLists0931+1,y
    sta a:ListWordHi,x
    bra TitleBlink
RunListChannel0931_at4:
    lda a:ListChannelHold,x
    bne RunListChannel0931_at1
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi EmitObjectSprites
    beq RunListChannel0931_at5
    dec a
    beq RunListChannel0931_at6
    dec a
    beq RunListChannel0931_at7
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
RunListChannel0931_at6:
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
RunListChannel0931_at8:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel0931_at8
    txa
    plx
    bsr AddToListWord
    jmp TitleBlink
RunListChannel0931_at5:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 09E1:018 RunListChannel09E1
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $8FEA), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $8FEA, sector 09E1's overlay.
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
ListChannelCopySource09E1 = $9800 ; the word the VRAM copy lists take as source
ListChannelCopyLists09E1 = $8FEA ; the VRAM copy lists, a word by list number
RunListChannel09E1:
    dec a
    beq TitleSteps
    dec a
    dec a
    beq RunListChannel09E1_at0
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp EmitObjectSprites
RunListChannel09E1_at1:
    dec a:ListChannelHold,x
    rts
    lda a:ListChannelHold,x
    bne RunListChannel09E1_at1
    lda a:ListChannelCopySource09E1
    sta z:zBiosAl
    lda a:ListChannelCopySource09E1+1
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
    beq RunListChannel09E1_at2
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel09E1_at3:
    rts
RunListChannel09E1_at2:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel09E1
RunListChannel09E1_at0:
    bbs0 zPaused,RunListChannel09E1_at3
    lda a:ListChannelPlace,x
    bne RunListChannel09E1_at4
    inc a:ListChannelPlace,x
RunListChannel09E1_at7:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:ListChannelCopyLists09E1,y
    sta a:ListWordLo,x
    lda a:ListChannelCopyLists09E1+1,y
    sta a:ListWordHi,x
    bra TitleBlink
RunListChannel09E1_at4:
    lda a:ListChannelHold,x
    bne RunListChannel09E1_at1
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi EmitObjectSprites
    beq RunListChannel09E1_at5
    dec a
    beq RunListChannel09E1_at6
    dec a
    beq RunListChannel09E1_at7
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
RunListChannel09E1_at6:
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
RunListChannel09E1_at8:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel09E1_at8
    txa
    plx
    bsr AddToListWord
    jmp TitleBlink
RunListChannel09E1_at5:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 0B41:018 RunListChannel0B41
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $8DD8), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $8DD8, sector 0B41's overlay.
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
ListChannelCopySource0B41 = $9800 ; the word the VRAM copy lists take as source
ListChannelCopyLists0B41 = $8DD8 ; the VRAM copy lists, a word by list number
RunListChannel0B41:
    dec a
    beq TitleSteps
    dec a
    dec a
    beq RunListChannel0B41_at0
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp EmitObjectSprites
RunListChannel0B41_at1:
    dec a:ListChannelHold,x
    rts
    lda a:ListChannelHold,x
    bne RunListChannel0B41_at1
    lda a:ListChannelCopySource0B41
    sta z:zBiosAl
    lda a:ListChannelCopySource0B41+1
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
    beq RunListChannel0B41_at2
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel0B41_at3:
    rts
RunListChannel0B41_at2:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel0B41
RunListChannel0B41_at0:
    bbs0 zPaused,RunListChannel0B41_at3
    lda a:ListChannelPlace,x
    bne RunListChannel0B41_at4
    inc a:ListChannelPlace,x
RunListChannel0B41_at7:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:ListChannelCopyLists0B41,y
    sta a:ListWordLo,x
    lda a:ListChannelCopyLists0B41+1,y
    sta a:ListWordHi,x
    bra TitleBlink
RunListChannel0B41_at4:
    lda a:ListChannelHold,x
    bne RunListChannel0B41_at1
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi EmitObjectSprites
    beq RunListChannel0B41_at5
    dec a
    beq RunListChannel0B41_at6
    dec a
    beq RunListChannel0B41_at7
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
RunListChannel0B41_at6:
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
RunListChannel0B41_at8:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel0B41_at8
    txa
    plx
    bsr AddToListWord
    jmp TitleBlink
RunListChannel0B41_at5:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 12D1:018 RunListChannel12D1
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $8F7B), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $8F7B, sector 12D1 (13F1 holds the same bytes).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
ListChannelCopySource12D1 = $9800 ; the word the VRAM copy lists take as source
ListChannelCopyLists12D1 = $8F7B ; the VRAM copy lists, a word by list number
RunListChannel12D1:
    dec a
    beq TitleSteps
    dec a
    dec a
    beq RunListChannel12D1_at0
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp EmitObjectSprites
RunListChannel12D1_at1:
    dec a:ListChannelHold,x
    rts
    lda a:ListChannelHold,x
    bne RunListChannel12D1_at1
    lda a:ListChannelCopySource12D1
    sta z:zBiosAl
    lda a:ListChannelCopySource12D1+1
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
    beq RunListChannel12D1_at2
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel12D1_at3:
    rts
RunListChannel12D1_at2:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel12D1
RunListChannel12D1_at0:
    bbs0 zPaused,RunListChannel12D1_at3
    lda a:ListChannelPlace,x
    bne RunListChannel12D1_at4
    inc a:ListChannelPlace,x
RunListChannel12D1_at7:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:ListChannelCopyLists12D1,y
    sta a:ListWordLo,x
    lda a:ListChannelCopyLists12D1+1,y
    sta a:ListWordHi,x
    bra TitleBlink
RunListChannel12D1_at4:
    lda a:ListChannelHold,x
    bne RunListChannel12D1_at1
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi EmitObjectSprites
    beq RunListChannel12D1_at5
    dec a
    beq RunListChannel12D1_at6
    dec a
    beq RunListChannel12D1_at7
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
RunListChannel12D1_at6:
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
RunListChannel12D1_at8:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel12D1_at8
    txa
    plx
    bsr AddToListWord
    jmp TitleBlink
RunListChannel12D1_at5:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 1611:018 RunListChannel1611
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $8F06), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $8F06, sector 1611's overlay.
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
ListChannelCopySource1611 = $9800 ; the word the VRAM copy lists take as source
ListChannelCopyLists1611 = $8F06 ; the VRAM copy lists, a word by list number
RunListChannel1611:
    dec a
    beq TitleSteps
    dec a
    dec a
    beq RunListChannel1611_at0
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp EmitObjectSprites
RunListChannel1611_at1:
    dec a:ListChannelHold,x
    rts
    lda a:ListChannelHold,x
    bne RunListChannel1611_at1
    lda a:ListChannelCopySource1611
    sta z:zBiosAl
    lda a:ListChannelCopySource1611+1
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
    beq RunListChannel1611_at2
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel1611_at3:
    rts
RunListChannel1611_at2:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel1611
RunListChannel1611_at0:
    bbs0 zPaused,RunListChannel1611_at3
    lda a:ListChannelPlace,x
    bne RunListChannel1611_at4
    inc a:ListChannelPlace,x
RunListChannel1611_at7:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:ListChannelCopyLists1611,y
    sta a:ListWordLo,x
    lda a:ListChannelCopyLists1611+1,y
    sta a:ListWordHi,x
    bra TitleBlink
RunListChannel1611_at4:
    lda a:ListChannelHold,x
    bne RunListChannel1611_at1
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi EmitObjectSprites
    beq RunListChannel1611_at5
    dec a
    beq RunListChannel1611_at6
    dec a
    beq RunListChannel1611_at7
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
RunListChannel1611_at6:
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
RunListChannel1611_at8:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel1611_at8
    txa
    plx
    bsr AddToListWord
    jmp TitleBlink
RunListChannel1611_at5:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02
