; @routine 0251:018 RunListChannel0251
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $901F), any other runs one VRAM list and
; frees the channel.
; HOW: RunListChannelBody with the table at $901F, Stage 00's overlay (sector 0251).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
RunListChannel0251Lists = $901F ; kind 3's copy lists, a word by list number
RunListChannel0251:
    dec a
    beq RunListChannel0251_at0
    dec a
    dec a
    beq RunListChannel0251_at1
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex0111
    plx
    jmp RunListChannel0251_at2
RunListChannel0251_at3:
    dec a:ListChannelHold,x
    rts
RunListChannel0251_at0:
    lda a:ListChannelHold,x
    bne RunListChannel0251_at3
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
    beq RunListChannel0251_at2
    cmp #$FE
    beq RunListChannel0251_at4
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex0111
RunListChannel0251_at2:
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel0251_at5:
    rts
RunListChannel0251_at4:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel0251
RunListChannel0251_at1:
    bbs0 zPaused,RunListChannel0251_at5
    lda a:ListChannelPlace,x
    bne RunListChannel0251_at6
    inc a:ListChannelPlace,x
RunListChannel0251_at10:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:RunListChannel0251Lists,y
    sta a:ListWordLo,x
    lda a:RunListChannel0251Lists+1,y
    sta a:ListWordHi,x
    bra RunListChannel0251_at7
RunListChannel0251_at6:
    lda a:ListChannelHold,x
    bne RunListChannel0251_at3
RunListChannel0251_at7:
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi RunListChannel0251_at2
    beq RunListChannel0251_at8
    dec a
    beq RunListChannel0251_at9
    dec a
    beq RunListChannel0251_at10
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
    bra RunListChannel0251_at11
RunListChannel0251_at9:
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
RunListChannel0251_at11:
    lda (zTemp00),y
    tay
RunListChannel0251_at12:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel0251_at12
    txa
    plx
    bsr AddToListWord0111
    jmp RunListChannel0251_at7
RunListChannel0251_at8:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02


; @routine 0511:018 RunListChannel0511
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $9349), any other runs one VRAM list and
; frees the channel.
; HOW: RunListChannelBody with the table at $9349, Stage 02's overlay (sector 0511).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
RunListChannel0511Lists = $9349 ; kind 3's copy lists, a word by list number
RunListChannel0511:
    dec a
    beq RunListChannel0511_at0
    dec a
    dec a
    beq RunListChannel0511_at1
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex0111
    plx
    jmp RunListChannel0511_at2
RunListChannel0511_at3:
    dec a:ListChannelHold,x
    rts
RunListChannel0511_at0:
    lda a:ListChannelHold,x
    bne RunListChannel0511_at3
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
    beq RunListChannel0511_at2
    cmp #$FE
    beq RunListChannel0511_at4
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex0111
RunListChannel0511_at2:
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel0511_at5:
    rts
RunListChannel0511_at4:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel0511
RunListChannel0511_at1:
    bbs0 zPaused,RunListChannel0511_at5
    lda a:ListChannelPlace,x
    bne RunListChannel0511_at6
    inc a:ListChannelPlace,x
RunListChannel0511_at10:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:RunListChannel0511Lists,y
    sta a:ListWordLo,x
    lda a:RunListChannel0511Lists+1,y
    sta a:ListWordHi,x
    bra RunListChannel0511_at7
RunListChannel0511_at6:
    lda a:ListChannelHold,x
    bne RunListChannel0511_at3
RunListChannel0511_at7:
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi RunListChannel0511_at2
    beq RunListChannel0511_at8
    dec a
    beq RunListChannel0511_at9
    dec a
    beq RunListChannel0511_at10
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
    bra RunListChannel0511_at11
RunListChannel0511_at9:
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
RunListChannel0511_at11:
    lda (zTemp00),y
    tay
RunListChannel0511_at12:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel0511_at12
    txa
    plx
    bsr AddToListWord0111
    jmp RunListChannel0511_at7
RunListChannel0511_at8:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02


; @routine 0A91:018 RunListChannel0A91
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $8E25), any other runs one VRAM list and
; frees the channel.
; HOW: RunListChannelBody with the table at $8E25, Stage 06's overlay (sector 0A91).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
RunListChannel0A91Lists = $8E25 ; kind 3's copy lists, a word by list number
RunListChannel0A91:
    dec a
    beq RunListChannel0A91_at0
    dec a
    dec a
    beq RunListChannel0A91_at1
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex0111
    plx
    jmp RunListChannel0A91_at2
RunListChannel0A91_at3:
    dec a:ListChannelHold,x
    rts
RunListChannel0A91_at0:
    lda a:ListChannelHold,x
    bne RunListChannel0A91_at3
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
    beq RunListChannel0A91_at2
    cmp #$FE
    beq RunListChannel0A91_at4
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex0111
RunListChannel0A91_at2:
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel0A91_at5:
    rts
RunListChannel0A91_at4:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel0A91
RunListChannel0A91_at1:
    bbs0 zPaused,RunListChannel0A91_at5
    lda a:ListChannelPlace,x
    bne RunListChannel0A91_at6
    inc a:ListChannelPlace,x
RunListChannel0A91_at10:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:RunListChannel0A91Lists,y
    sta a:ListWordLo,x
    lda a:RunListChannel0A91Lists+1,y
    sta a:ListWordHi,x
    bra RunListChannel0A91_at7
RunListChannel0A91_at6:
    lda a:ListChannelHold,x
    bne RunListChannel0A91_at3
RunListChannel0A91_at7:
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi RunListChannel0A91_at2
    beq RunListChannel0A91_at8
    dec a
    beq RunListChannel0A91_at9
    dec a
    beq RunListChannel0A91_at10
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
    bra RunListChannel0A91_at11
RunListChannel0A91_at9:
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
RunListChannel0A91_at11:
    lda (zTemp00),y
    tay
RunListChannel0A91_at12:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel0A91_at12
    txa
    plx
    bsr AddToListWord0111
    jmp RunListChannel0A91_at7
RunListChannel0A91_at8:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02


; @routine 0BF1:018 RunListChannel0BF1
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $8CA7), any other runs one VRAM list and
; frees the channel.
; HOW: RunListChannelBody with the table at $8CA7, Stage 07's overlay (sector 0BF1).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
RunListChannel0BF1Lists = $8CA7 ; kind 3's copy lists, a word by list number
RunListChannel0BF1:
    dec a
    beq RunListChannel0BF1_at0
    dec a
    dec a
    beq RunListChannel0BF1_at1
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex0111
    plx
    jmp RunListChannel0BF1_at2
RunListChannel0BF1_at3:
    dec a:ListChannelHold,x
    rts
RunListChannel0BF1_at0:
    lda a:ListChannelHold,x
    bne RunListChannel0BF1_at3
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
    beq RunListChannel0BF1_at2
    cmp #$FE
    beq RunListChannel0BF1_at4
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex0111
RunListChannel0BF1_at2:
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel0BF1_at5:
    rts
RunListChannel0BF1_at4:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel0BF1
RunListChannel0BF1_at1:
    bbs0 zPaused,RunListChannel0BF1_at5
    lda a:ListChannelPlace,x
    bne RunListChannel0BF1_at6
    inc a:ListChannelPlace,x
RunListChannel0BF1_at10:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:RunListChannel0BF1Lists,y
    sta a:ListWordLo,x
    lda a:RunListChannel0BF1Lists+1,y
    sta a:ListWordHi,x
    bra RunListChannel0BF1_at7
RunListChannel0BF1_at6:
    lda a:ListChannelHold,x
    bne RunListChannel0BF1_at3
RunListChannel0BF1_at7:
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi RunListChannel0BF1_at2
    beq RunListChannel0BF1_at8
    dec a
    beq RunListChannel0BF1_at9
    dec a
    beq RunListChannel0BF1_at10
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
    bra RunListChannel0BF1_at11
RunListChannel0BF1_at9:
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
RunListChannel0BF1_at11:
    lda (zTemp00),y
    tay
RunListChannel0BF1_at12:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel0BF1_at12
    txa
    plx
    bsr AddToListWord0111
    jmp RunListChannel0BF1_at7
RunListChannel0BF1_at8:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02


; @routine 0D51:018 RunListChannel0D51
; @run 6C:8018
; @stop 8133
; WHAT IT DOES: Runs one list channel by its kind, as RunListChannel0441: 1 plays a list of VRAM lists with a hold
; between them, 3 steps a list of VRAM copies (its lists from the table at $8D0C), any other runs one VRAM list and
; frees the channel.
; HOW: RunListChannelBody with the table at $8D0C, Stage 08's overlay (sector 0D51).
; WAITS: no.
; Entry: A the kind (not zero), X the channel. Exit: as the path it took leaves them, X the channel.
RunListChannel0D51Lists = $8D0C ; kind 3's copy lists, a word by list number
RunListChannel0D51:
    dec a
    beq RunListChannel0D51_at0
    dec a
    dec a
    beq RunListChannel0D51_at1
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex0111
    plx
    jmp RunListChannel0D51_at2
RunListChannel0D51_at3:
    dec a:ListChannelHold,x
    rts
RunListChannel0D51_at0:
    lda a:ListChannelHold,x
    bne RunListChannel0D51_at3
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
    beq RunListChannel0D51_at2
    cmp #$FE
    beq RunListChannel0D51_at4
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex0111
RunListChannel0D51_at2:
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel0D51_at5:
    rts
RunListChannel0D51_at4:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel0D51
RunListChannel0D51_at1:
    bbs0 zPaused,RunListChannel0D51_at5
    lda a:ListChannelPlace,x
    bne RunListChannel0D51_at6
    inc a:ListChannelPlace,x
RunListChannel0D51_at10:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:RunListChannel0D51Lists,y
    sta a:ListWordLo,x
    lda a:RunListChannel0D51Lists+1,y
    sta a:ListWordHi,x
    bra RunListChannel0D51_at7
RunListChannel0D51_at6:
    lda a:ListChannelHold,x
    bne RunListChannel0D51_at3
RunListChannel0D51_at7:
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi RunListChannel0D51_at2
    beq RunListChannel0D51_at8
    dec a
    beq RunListChannel0D51_at9
    dec a
    beq RunListChannel0D51_at10
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
    bra RunListChannel0D51_at11
RunListChannel0D51_at9:
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
RunListChannel0D51_at11:
    lda (zTemp00),y
    tay
RunListChannel0D51_at12:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel0D51_at12
    txa
    plx
    bsr AddToListWord0111
    jmp RunListChannel0D51_at7
RunListChannel0D51_at8:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02


