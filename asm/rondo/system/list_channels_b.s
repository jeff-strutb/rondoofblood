; @routine 05C1:018 RunListChannel05C1
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $8E4B), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $8E4B, Secret stage 02's overlay (sector 05C1).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
RunListChannel05C1Copies = $8E4B ; kind 3's lists of VRAM copies, a word per list number
RunListChannel05C1:
    dec a
    beq RunListChannel05C1_at0
    dec a
    dec a
    beq RunListChannel05C1_at1
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp RunListChannel05C1_at2
RunListChannel05C1_at3:
    dec a:ListChannelHold,x
    rts
RunListChannel05C1_at0:
    lda a:ListChannelHold,x
    bne RunListChannel05C1_at3
    lda a:OverlayListChannelSeqs
    sta z:zBiosAl
    lda a:OverlayListChannelSeqs+1
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
    beq RunListChannel05C1_at2
    cmp #$FE
    beq RunListChannel05C1_at4
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
RunListChannel05C1_at2:
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel05C1_at5:
    rts
RunListChannel05C1_at4:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel05C1
RunListChannel05C1_at1:
    bbs0 zPaused,RunListChannel05C1_at5
    lda a:ListChannelPlace,x
    bne RunListChannel05C1_at6
    inc a:ListChannelPlace,x
RunListChannel05C1_at10:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:RunListChannel05C1Copies,y
    sta a:ListWordLo,x
    lda a:RunListChannel05C1Copies+1,y
    sta a:ListWordHi,x
    bra RunListChannel05C1_at7
RunListChannel05C1_at6:
    lda a:ListChannelHold,x
    bne RunListChannel05C1_at3
RunListChannel05C1_at7:
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi RunListChannel05C1_at2
    beq RunListChannel05C1_at8
    dec a
    beq RunListChannel05C1_at9
    dec a
    beq RunListChannel05C1_at10
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
    bra RunListChannel05C1_at11
RunListChannel05C1_at9:
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
RunListChannel05C1_at11:
    lda (zTemp00),y
    tay
RunListChannel05C1_at12:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel05C1_at12
    txa
    plx
    bsr AddToListWord
    jmp RunListChannel05C1_at7
RunListChannel05C1_at8:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 0721:018 RunListChannel0721
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $8E95), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $8E95, Secret stage 03's overlay (sector 0721).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
RunListChannel0721Copies = $8E95 ; kind 3's lists of VRAM copies, a word per list number
RunListChannel0721:
    dec a
    beq RunListChannel0721_at0
    dec a
    dec a
    beq RunListChannel0721_at1
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp RunListChannel0721_at2
RunListChannel0721_at3:
    dec a:ListChannelHold,x
    rts
RunListChannel0721_at0:
    lda a:ListChannelHold,x
    bne RunListChannel0721_at3
    lda a:OverlayListChannelSeqs
    sta z:zBiosAl
    lda a:OverlayListChannelSeqs+1
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
    beq RunListChannel0721_at2
    cmp #$FE
    beq RunListChannel0721_at4
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
RunListChannel0721_at2:
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel0721_at5:
    rts
RunListChannel0721_at4:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel0721
RunListChannel0721_at1:
    bbs0 zPaused,RunListChannel0721_at5
    lda a:ListChannelPlace,x
    bne RunListChannel0721_at6
    inc a:ListChannelPlace,x
RunListChannel0721_at10:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:RunListChannel0721Copies,y
    sta a:ListWordLo,x
    lda a:RunListChannel0721Copies+1,y
    sta a:ListWordHi,x
    bra RunListChannel0721_at7
RunListChannel0721_at6:
    lda a:ListChannelHold,x
    bne RunListChannel0721_at3
RunListChannel0721_at7:
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi RunListChannel0721_at2
    beq RunListChannel0721_at8
    dec a
    beq RunListChannel0721_at9
    dec a
    beq RunListChannel0721_at10
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
    bra RunListChannel0721_at11
RunListChannel0721_at9:
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
RunListChannel0721_at11:
    lda (zTemp00),y
    tay
RunListChannel0721_at12:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel0721_at12
    txa
    plx
    bsr AddToListWord
    jmp RunListChannel0721_at7
RunListChannel0721_at8:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 0881:018 RunListChannel0881
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $901E), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $901E, Secret stage 04's overlay (sector 0881).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
RunListChannel0881Copies = $901E ; kind 3's lists of VRAM copies, a word per list number
RunListChannel0881:
    dec a
    beq RunListChannel0881_at0
    dec a
    dec a
    beq RunListChannel0881_at1
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp RunListChannel0881_at2
RunListChannel0881_at3:
    dec a:ListChannelHold,x
    rts
RunListChannel0881_at0:
    lda a:ListChannelHold,x
    bne RunListChannel0881_at3
    lda a:OverlayListChannelSeqs
    sta z:zBiosAl
    lda a:OverlayListChannelSeqs+1
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
    beq RunListChannel0881_at2
    cmp #$FE
    beq RunListChannel0881_at4
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
RunListChannel0881_at2:
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel0881_at5:
    rts
RunListChannel0881_at4:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel0881
RunListChannel0881_at1:
    bbs0 zPaused,RunListChannel0881_at5
    lda a:ListChannelPlace,x
    bne RunListChannel0881_at6
    inc a:ListChannelPlace,x
RunListChannel0881_at10:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:RunListChannel0881Copies,y
    sta a:ListWordLo,x
    lda a:RunListChannel0881Copies+1,y
    sta a:ListWordHi,x
    bra RunListChannel0881_at7
RunListChannel0881_at6:
    lda a:ListChannelHold,x
    bne RunListChannel0881_at3
RunListChannel0881_at7:
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi RunListChannel0881_at2
    beq RunListChannel0881_at8
    dec a
    beq RunListChannel0881_at9
    dec a
    beq RunListChannel0881_at10
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
    bra RunListChannel0881_at11
RunListChannel0881_at9:
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
RunListChannel0881_at11:
    lda (zTemp00),y
    tay
RunListChannel0881_at12:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel0881_at12
    txa
    plx
    bsr AddToListWord
    jmp RunListChannel0881_at7
RunListChannel0881_at8:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 0E01:018 RunListChannel0E01
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $8D2E), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $8D2E, Secret stage 08's overlay (sector 0E01).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
RunListChannel0E01Copies = $8D2E ; kind 3's lists of VRAM copies, a word per list number
RunListChannel0E01:
    dec a
    beq RunListChannel0E01_at0
    dec a
    dec a
    beq RunListChannel0E01_at1
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp RunListChannel0E01_at2
RunListChannel0E01_at3:
    dec a:ListChannelHold,x
    rts
RunListChannel0E01_at0:
    lda a:ListChannelHold,x
    bne RunListChannel0E01_at3
    lda a:OverlayListChannelSeqs
    sta z:zBiosAl
    lda a:OverlayListChannelSeqs+1
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
    beq RunListChannel0E01_at2
    cmp #$FE
    beq RunListChannel0E01_at4
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
RunListChannel0E01_at2:
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel0E01_at5:
    rts
RunListChannel0E01_at4:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel0E01
RunListChannel0E01_at1:
    bbs0 zPaused,RunListChannel0E01_at5
    lda a:ListChannelPlace,x
    bne RunListChannel0E01_at6
    inc a:ListChannelPlace,x
RunListChannel0E01_at10:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:RunListChannel0E01Copies,y
    sta a:ListWordLo,x
    lda a:RunListChannel0E01Copies+1,y
    sta a:ListWordHi,x
    bra RunListChannel0E01_at7
RunListChannel0E01_at6:
    lda a:ListChannelHold,x
    bne RunListChannel0E01_at3
RunListChannel0E01_at7:
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi RunListChannel0E01_at2
    beq RunListChannel0E01_at8
    dec a
    beq RunListChannel0E01_at9
    dec a
    beq RunListChannel0E01_at10
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
    bra RunListChannel0E01_at11
RunListChannel0E01_at9:
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
RunListChannel0E01_at11:
    lda (zTemp00),y
    tay
RunListChannel0E01_at12:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel0E01_at12
    txa
    plx
    bsr AddToListWord
    jmp RunListChannel0E01_at7
RunListChannel0E01_at8:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 0EB1:018 RunListChannel0EB1
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $8FBB), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $8FBB, Stage 09's overlay (sector 0EB1).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
RunListChannel0EB1Copies = $8FBB ; kind 3's lists of VRAM copies, a word per list number
RunListChannel0EB1:
    dec a
    beq RunListChannel0EB1_at0
    dec a
    dec a
    beq RunListChannel0EB1_at1
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp RunListChannel0EB1_at2
RunListChannel0EB1_at3:
    dec a:ListChannelHold,x
    rts
RunListChannel0EB1_at0:
    lda a:ListChannelHold,x
    bne RunListChannel0EB1_at3
    lda a:OverlayListChannelSeqs
    sta z:zBiosAl
    lda a:OverlayListChannelSeqs+1
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
    beq RunListChannel0EB1_at2
    cmp #$FE
    beq RunListChannel0EB1_at4
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
RunListChannel0EB1_at2:
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel0EB1_at5:
    rts
RunListChannel0EB1_at4:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel0EB1
RunListChannel0EB1_at1:
    bbs0 zPaused,RunListChannel0EB1_at5
    lda a:ListChannelPlace,x
    bne RunListChannel0EB1_at6
    inc a:ListChannelPlace,x
RunListChannel0EB1_at10:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:RunListChannel0EB1Copies,y
    sta a:ListWordLo,x
    lda a:RunListChannel0EB1Copies+1,y
    sta a:ListWordHi,x
    bra RunListChannel0EB1_at7
RunListChannel0EB1_at6:
    lda a:ListChannelHold,x
    bne RunListChannel0EB1_at3
RunListChannel0EB1_at7:
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi RunListChannel0EB1_at2
    beq RunListChannel0EB1_at8
    dec a
    beq RunListChannel0EB1_at9
    dec a
    beq RunListChannel0EB1_at10
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
    bra RunListChannel0EB1_at11
RunListChannel0EB1_at9:
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
RunListChannel0EB1_at11:
    lda (zTemp00),y
    tay
RunListChannel0EB1_at12:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel0EB1_at12
    txa
    plx
    bsr AddToListWord
    jmp RunListChannel0EB1_at7
RunListChannel0EB1_at8:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 0F61:018 RunListChannel0F61
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $8D2E), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $8D2E, Secret stage 09's overlay (sector 0F61).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
RunListChannel0F61Copies = $8D2E ; kind 3's lists of VRAM copies, a word per list number
RunListChannel0F61:
    dec a
    beq RunListChannel0F61_at0
    dec a
    dec a
    beq RunListChannel0F61_at1
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp RunListChannel0F61_at2
RunListChannel0F61_at3:
    dec a:ListChannelHold,x
    rts
RunListChannel0F61_at0:
    lda a:ListChannelHold,x
    bne RunListChannel0F61_at3
    lda a:OverlayListChannelSeqs
    sta z:zBiosAl
    lda a:OverlayListChannelSeqs+1
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
    beq RunListChannel0F61_at2
    cmp #$FE
    beq RunListChannel0F61_at4
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
RunListChannel0F61_at2:
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel0F61_at5:
    rts
RunListChannel0F61_at4:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel0F61
RunListChannel0F61_at1:
    bbs0 zPaused,RunListChannel0F61_at5
    lda a:ListChannelPlace,x
    bne RunListChannel0F61_at6
    inc a:ListChannelPlace,x
RunListChannel0F61_at10:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:RunListChannel0F61Copies,y
    sta a:ListWordLo,x
    lda a:RunListChannel0F61Copies+1,y
    sta a:ListWordHi,x
    bra RunListChannel0F61_at7
RunListChannel0F61_at6:
    lda a:ListChannelHold,x
    bne RunListChannel0F61_at3
RunListChannel0F61_at7:
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi RunListChannel0F61_at2
    beq RunListChannel0F61_at8
    dec a
    beq RunListChannel0F61_at9
    dec a
    beq RunListChannel0F61_at10
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
    bra RunListChannel0F61_at11
RunListChannel0F61_at9:
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
RunListChannel0F61_at11:
    lda (zTemp00),y
    tay
RunListChannel0F61_at12:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel0F61_at12
    txa
    plx
    bsr AddToListWord
    jmp RunListChannel0F61_at7
RunListChannel0F61_at8:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 1011:018 RunListChannel1011
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $8F58), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $8F58, Stage 0A's overlay (sector 1011).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
RunListChannel1011Copies = $8F58 ; kind 3's lists of VRAM copies, a word per list number
RunListChannel1011:
    dec a
    beq RunListChannel1011_at0
    dec a
    dec a
    beq RunListChannel1011_at1
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp RunListChannel1011_at2
RunListChannel1011_at3:
    dec a:ListChannelHold,x
    rts
RunListChannel1011_at0:
    lda a:ListChannelHold,x
    bne RunListChannel1011_at3
    lda a:OverlayListChannelSeqs
    sta z:zBiosAl
    lda a:OverlayListChannelSeqs+1
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
    beq RunListChannel1011_at2
    cmp #$FE
    beq RunListChannel1011_at4
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
RunListChannel1011_at2:
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel1011_at5:
    rts
RunListChannel1011_at4:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel1011
RunListChannel1011_at1:
    bbs0 zPaused,RunListChannel1011_at5
    lda a:ListChannelPlace,x
    bne RunListChannel1011_at6
    inc a:ListChannelPlace,x
RunListChannel1011_at10:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:RunListChannel1011Copies,y
    sta a:ListWordLo,x
    lda a:RunListChannel1011Copies+1,y
    sta a:ListWordHi,x
    bra RunListChannel1011_at7
RunListChannel1011_at6:
    lda a:ListChannelHold,x
    bne RunListChannel1011_at3
RunListChannel1011_at7:
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi RunListChannel1011_at2
    beq RunListChannel1011_at8
    dec a
    beq RunListChannel1011_at9
    dec a
    beq RunListChannel1011_at10
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
    bra RunListChannel1011_at11
RunListChannel1011_at9:
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
RunListChannel1011_at11:
    lda (zTemp00),y
    tay
RunListChannel1011_at12:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel1011_at12
    txa
    plx
    bsr AddToListWord
    jmp RunListChannel1011_at7
RunListChannel1011_at8:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 10C1:018 RunListChannel10C1
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $8E56), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $8E56, Secret stage 0A's overlay (sector 10C1).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
RunListChannel10C1Copies = $8E56 ; kind 3's lists of VRAM copies, a word per list number
RunListChannel10C1:
    dec a
    beq RunListChannel10C1_at0
    dec a
    dec a
    beq RunListChannel10C1_at1
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp RunListChannel10C1_at2
RunListChannel10C1_at3:
    dec a:ListChannelHold,x
    rts
RunListChannel10C1_at0:
    lda a:ListChannelHold,x
    bne RunListChannel10C1_at3
    lda a:OverlayListChannelSeqs
    sta z:zBiosAl
    lda a:OverlayListChannelSeqs+1
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
    beq RunListChannel10C1_at2
    cmp #$FE
    beq RunListChannel10C1_at4
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
RunListChannel10C1_at2:
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel10C1_at5:
    rts
RunListChannel10C1_at4:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel10C1
RunListChannel10C1_at1:
    bbs0 zPaused,RunListChannel10C1_at5
    lda a:ListChannelPlace,x
    bne RunListChannel10C1_at6
    inc a:ListChannelPlace,x
RunListChannel10C1_at10:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:RunListChannel10C1Copies,y
    sta a:ListWordLo,x
    lda a:RunListChannel10C1Copies+1,y
    sta a:ListWordHi,x
    bra RunListChannel10C1_at7
RunListChannel10C1_at6:
    lda a:ListChannelHold,x
    bne RunListChannel10C1_at3
RunListChannel10C1_at7:
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi RunListChannel10C1_at2
    beq RunListChannel10C1_at8
    dec a
    beq RunListChannel10C1_at9
    dec a
    beq RunListChannel10C1_at10
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
    bra RunListChannel10C1_at11
RunListChannel10C1_at9:
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
RunListChannel10C1_at11:
    lda (zTemp00),y
    tay
RunListChannel10C1_at12:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel10C1_at12
    txa
    plx
    bsr AddToListWord
    jmp RunListChannel10C1_at7
RunListChannel10C1_at8:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 1171:018 RunListChannel1171
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $9049), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $9049, Stage 0B's overlay (sector 1171).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
RunListChannel1171Copies = $9049 ; kind 3's lists of VRAM copies, a word per list number
RunListChannel1171:
    dec a
    beq RunListChannel1171_at0
    dec a
    dec a
    beq RunListChannel1171_at1
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp RunListChannel1171_at2
RunListChannel1171_at3:
    dec a:ListChannelHold,x
    rts
RunListChannel1171_at0:
    lda a:ListChannelHold,x
    bne RunListChannel1171_at3
    lda a:OverlayListChannelSeqs
    sta z:zBiosAl
    lda a:OverlayListChannelSeqs+1
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
    beq RunListChannel1171_at2
    cmp #$FE
    beq RunListChannel1171_at4
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
RunListChannel1171_at2:
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel1171_at5:
    rts
RunListChannel1171_at4:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel1171
RunListChannel1171_at1:
    bbs0 zPaused,RunListChannel1171_at5
    lda a:ListChannelPlace,x
    bne RunListChannel1171_at6
    inc a:ListChannelPlace,x
RunListChannel1171_at10:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:RunListChannel1171Copies,y
    sta a:ListWordLo,x
    lda a:RunListChannel1171Copies+1,y
    sta a:ListWordHi,x
    bra RunListChannel1171_at7
RunListChannel1171_at6:
    lda a:ListChannelHold,x
    bne RunListChannel1171_at3
RunListChannel1171_at7:
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi RunListChannel1171_at2
    beq RunListChannel1171_at8
    dec a
    beq RunListChannel1171_at9
    dec a
    beq RunListChannel1171_at10
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
    bra RunListChannel1171_at11
RunListChannel1171_at9:
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
RunListChannel1171_at11:
    lda (zTemp00),y
    tay
RunListChannel1171_at12:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel1171_at12
    txa
    plx
    bsr AddToListWord
    jmp RunListChannel1171_at7
RunListChannel1171_at8:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 16C1:018 RunListChannel16C1
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $8FEB), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $8FEB, Secret stage 0E's overlay (sector 16C1).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
RunListChannel16C1Copies = $8FEB ; kind 3's lists of VRAM copies, a word per list number
RunListChannel16C1:
    dec a
    beq RunListChannel16C1_at0
    dec a
    dec a
    beq RunListChannel16C1_at1
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp RunListChannel16C1_at2
RunListChannel16C1_at3:
    dec a:ListChannelHold,x
    rts
RunListChannel16C1_at0:
    lda a:ListChannelHold,x
    bne RunListChannel16C1_at3
    lda a:OverlayListChannelSeqs
    sta z:zBiosAl
    lda a:OverlayListChannelSeqs+1
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
    beq RunListChannel16C1_at2
    cmp #$FE
    beq RunListChannel16C1_at4
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
RunListChannel16C1_at2:
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel16C1_at5:
    rts
RunListChannel16C1_at4:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel16C1
RunListChannel16C1_at1:
    bbs0 zPaused,RunListChannel16C1_at5
    lda a:ListChannelPlace,x
    bne RunListChannel16C1_at6
    inc a:ListChannelPlace,x
RunListChannel16C1_at10:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:RunListChannel16C1Copies,y
    sta a:ListWordLo,x
    lda a:RunListChannel16C1Copies+1,y
    sta a:ListWordHi,x
    bra RunListChannel16C1_at7
RunListChannel16C1_at6:
    lda a:ListChannelHold,x
    bne RunListChannel16C1_at3
RunListChannel16C1_at7:
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi RunListChannel16C1_at2
    beq RunListChannel16C1_at8
    dec a
    beq RunListChannel16C1_at9
    dec a
    beq RunListChannel16C1_at10
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
    bra RunListChannel16C1_at11
RunListChannel16C1_at9:
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
RunListChannel16C1_at11:
    lda (zTemp00),y
    tay
RunListChannel16C1_at12:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel16C1_at12
    txa
    plx
    bsr AddToListWord
    jmp RunListChannel16C1_at7
RunListChannel16C1_at8:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 18D1:018 RunListChannel18D1
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $92F6), any other runs one VRAM list
; and frees the channel.
; HOW: RunListChannelBody with the table at $92F6, Stage 10's overlay (sector 18D1).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
RunListChannel18D1Copies = $92F6 ; kind 3's lists of VRAM copies, a word per list number
RunListChannel18D1:
    dec a
    beq RunListChannel18D1_at0
    dec a
    dec a
    beq RunListChannel18D1_at1
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex
    plx
    jmp RunListChannel18D1_at2
RunListChannel18D1_at3:
    dec a:ListChannelHold,x
    rts
RunListChannel18D1_at0:
    lda a:ListChannelHold,x
    bne RunListChannel18D1_at3
    lda a:OverlayListChannelSeqs
    sta z:zBiosAl
    lda a:OverlayListChannelSeqs+1
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
    beq RunListChannel18D1_at2
    cmp #$FE
    beq RunListChannel18D1_at4
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex
RunListChannel18D1_at2:
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel18D1_at5:
    rts
RunListChannel18D1_at4:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel18D1
RunListChannel18D1_at1:
    bbs0 zPaused,RunListChannel18D1_at5
    lda a:ListChannelPlace,x
    bne RunListChannel18D1_at6
    inc a:ListChannelPlace,x
RunListChannel18D1_at10:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:RunListChannel18D1Copies,y
    sta a:ListWordLo,x
    lda a:RunListChannel18D1Copies+1,y
    sta a:ListWordHi,x
    bra RunListChannel18D1_at7
RunListChannel18D1_at6:
    lda a:ListChannelHold,x
    bne RunListChannel18D1_at3
RunListChannel18D1_at7:
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi RunListChannel18D1_at2
    beq RunListChannel18D1_at8
    dec a
    beq RunListChannel18D1_at9
    dec a
    beq RunListChannel18D1_at10
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
    bra RunListChannel18D1_at11
RunListChannel18D1_at9:
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
RunListChannel18D1_at11:
    lda (zTemp00),y
    tay
RunListChannel18D1_at12:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel18D1_at12
    txa
    plx
    bsr AddToListWord
    jmp RunListChannel18D1_at7
RunListChannel18D1_at8:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02
