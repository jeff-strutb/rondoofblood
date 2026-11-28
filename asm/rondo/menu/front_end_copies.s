; @routine 0111:018 RunListChannel0111
; @run 6C:8018
; WHAT IT DOES: As RunListChannel00C5 (00C5:018), whose bytes these are.
; HOW: The same instructions.
; WAITS: no.
; Entry, exit: as RunListChannel00C5.
RunListChannel0111:
    dec a
    beq TitleSteps
    dec a
    dec a
    beq RunListChannel0111_at0
    lda a:ListChannelList,x
    phx
    jsr VramListFromIndex0111
    plx
    jmp EmitObjectSprites
RunListChannel0111_at1:
    dec a:ListChannelHold,x
    rts
    lda a:ListChannelHold,x
    bne RunListChannel0111_at1
    lda a:$9800
    sta z:zBiosAl
    lda a:$9801
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
    beq RunListChannel0111_at2
    sta a:ListChannelHold,x
    inc a:ListChannelPlace,x
    iny
    lda (zTemp00),y
    jmp VramListFromIndex0111
    stz a:ListChannelKind,x
    stz a:ListChannelList,x
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
RunListChannel0111_at3:
    rts
RunListChannel0111_at2:
    stz a:ListChannelHold,x
    stz a:ListChannelPlace,x
    bra RunListChannel0111
RunListChannel0111_at0:
    bbs0 zPaused,RunListChannel0111_at3
    lda a:ListChannelPlace,x
    bne RunListChannel0111_at4
    inc a:ListChannelPlace,x
RunListChannel0111_at7:
    lda a:ListChannelList,x
    asl a
    tay
    lda a:$829F,y
    sta a:ListWordLo,x
    lda a:$82A0,y
    sta a:ListWordHi,x
    bra TitleBlink
RunListChannel0111_at4:
    lda a:ListChannelHold,x
    bne RunListChannel0111_at1
    lda a:ListWordLo,x
    sta z:zTemp00
    lda a:ListWordHi,x
    sta z:zTemp01
    lda (zTemp00)
    bmi EmitObjectSprites
    beq RunListChannel0111_at5
    dec a
    beq RunListChannel0111_at6
    dec a
    beq RunListChannel0111_at7
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
RunListChannel0111_at6:
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
RunListChannel0111_at8:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne RunListChannel0111_at8
    txa
    plx
    bsr AddToListWord0111
    jmp TitleBlink
RunListChannel0111_at5:
    ldy #$01
    lda (zTemp00),y
    sta a:ListChannelHold,x
    lda #$02

; @routine 0111:133 AddToListWord0111
; @run 6C:8133
; WHAT IT DOES: As AddToListWord (00C5:133), whose bytes these are.
; HOW: The same instructions.
; WAITS: no.
; Entry, exit: as AddToListWord.
AddToListWord0111:
    clc
    adc a:$3CA0,x
    sta a:$3CA0,x
    cla
    adc a:$3CA8,x
    sta a:$3CA8,x
    rts

; @routine 0111:151 VramListFromIndex0111
; @run 6C:8151
; WHAT IT DOES: As VramListFromIndex (00C5:151), whose bytes these are.
; HOW: The same instructions.
; WAITS: no.
; Entry, exit: as VramListFromIndex.
VramListFromIndex0111:
    bmi *-15
    asl a
    tay
    lda a:$9802
    sta z:$F8
    lda a:$9803
    sta z:$F9
    lda ($F8),y
    sta z:$14
    iny
    lda ($F8),y
    sta z:$15
    cly

; @routine 0111:169 VramListRun0111
; @run 6C:8169
; WHAT IT DOES: As VramListRun (00C5:169), whose bytes these are.
; HOW: The same instructions.
; WAITS: no.
; Entry, exit: as VramListRun.
VramListRun0111:
    lda ($14),y
    cmp #$FF
    bne VramListRun0111_at0
    rts
VramListRun0111_at0:
    cmp #$00
    bne VramListRun0111_at1
    bbr0 zIdleFlag,VramListRun0111_at1
    lda #$31
VramListRun0111_at1:
    cmp #$28
    bcc VramListRun0111_at2
    cmp #$30
    bcs VramListRun0111_at2
    bbs0 zIdleFlag,VramListRun0111_at2
    sec
    sbc #$18
VramListRun0111_at2:
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    iny
    lda ($14),y
    sta z:zTemp11
    iny
    jsr VecClearVdcControlHigh
    jsr VecSelectVramWriteAddress
    lda ($14),y
    iny
    sta z:zMultiplierLo
    sta a:VDC_DATA_LO
    lda ($14),y
    iny
    sta z:zMultiplierHi
    sta a:VDC_DATA_HI
    lda ($14),y
    sta z:zTemp12
    iny
    lda ($14),y
    sta z:zProduct0
    iny
    jsr VecSelectVramData
    lda z:zTemp11
    bmi VramListRun0111_at3
    phy
    bsr VramCommandsFetch0111
    ply
    bra VramListRun0111
VramListRun0111_at3:
    lda ($14),y
    iny
    tax
    phy
    jsr CopyWordsToVdc0111
    ply
    bra VramListRun0111

; @routine 0111:1CD VramCommandsFetch0111
; @run 6C:81CD
; WHAT IT DOES: As VramCommandsFetch (00C5:1CD), whose bytes these are.
; HOW: The same instructions.
; WAITS: no.
; Entry, exit: as VramCommandsFetch.
VramCommandsFetch0111:
    cly
    lda (zTemp12),y
    bne VramCommandsFetch0111_at0
    jmp a:VramCommandsPairs0111
VramCommandsFetch0111_at0:
    bmi VramCommandsFetch0111_at1
    jmp a:VramCommandsRepeat0111
VramCommandsFetch0111_at1:
    cmp #$FF
    bne VramCommandsFetch0111_at2
    rts
VramCommandsFetch0111_at2:
    cmp #$FE
    bne VramCommandsFetch0111_at3
    jmp a:VramCommandsAddressAdd0111
VramCommandsFetch0111_at3:
    cmp #$FD
    bne VramCommandsFetch0111_at4
    jmp a:VramCommandsAddress0111
VramCommandsFetch0111_at4:
    and #$7F
    sta z:zTemp08
VramCommandsFetch0111_at5:
    iny
    lda (zTemp12),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp12),y
    sta a:VDC_DATA_HI
    dec z:zTemp08
    bne VramCommandsFetch0111_at5

; @routine 0111:201 VramCommandsNext0111
; @run 6C:8201
; WHAT IT DOES: As VramCommandsNext (00C5:201), whose bytes these are.
; HOW: The same instructions.
; WAITS: no.
; Entry, exit: as VramCommandsNext.
VramCommandsNext0111:
    iny
    tya
    clc
    adc z:zTemp12
    sta z:zTemp12
    bcc VramCommandsNext0111_at0
    inc z:zProduct0
VramCommandsNext0111_at0:
    bra VramCommandsFetch0111

; @routine 0111:20E VramCommandsRepeat0111
; @run 6C:820E
; WHAT IT DOES: As VramCommandsRepeat (00C5:20E), whose bytes these are.
; HOW: The same instructions.
; WAITS: no.
; Entry, exit: as VramCommandsRepeat.
VramCommandsRepeat0111:
    iny
    sta z:zTemp08
    lda (zTemp12),y
    sta z:zTemp09
    iny
    lda (zTemp12),y
    sta z:zTemp0A
VramCommandsRepeat0111_at0:
    lda z:zTemp09
    sta a:VDC_DATA_LO
    lda z:zTemp0A
    sta a:VDC_DATA_HI
    dec z:zTemp08
    bne VramCommandsRepeat0111_at0
    jmp a:VramCommandsNext0111

; @routine 0111:22B VramCommandsPairs0111
; @run 6C:822B
; WHAT IT DOES: As VramCommandsPairs (00C5:22B), whose bytes these are.
; HOW: The same instructions.
; WAITS: no.
; Entry, exit: as VramCommandsPairs.
VramCommandsPairs0111:
    iny
    lda (zTemp12),y
    sta z:zTemp08
    iny
    lda (zTemp12),y
    sta z:zTemp09
VramCommandsRepeat0111_at1:
    iny
    lda (zTemp12),y
    sta a:VDC_DATA_LO
    lda z:zTemp09
    sta a:VDC_DATA_HI
    dec z:zTemp08
    bne VramCommandsRepeat0111_at1
    jmp a:VramCommandsNext0111

; @routine 0111:247 VramCommandsAddressAdd0111
; @run 6C:8247
; WHAT IT DOES: As VramCommandsAddressAdd (00C5:247), whose bytes these are.
; HOW: The same instructions.
; WAITS: no.
; Entry, exit: as VramCommandsAddressAdd.
VramCommandsAddressAdd0111:
    iny
    jsr VecSelectVramWriteAddress
    lda (zTemp12),y
    clc
    adc z:zMultiplierLo
    sta a:VDC_DATA_LO
    iny
    lda (zTemp12),y
    adc z:zMultiplierHi
    bbs0 zTemp11,VramCommandsRepeat0111_at2
    and #$0F
VramCommandsRepeat0111_at2:
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    jmp a:VramCommandsNext0111

; @routine 0111:266 VramCommandsAddress0111
; @run 6C:8266
; WHAT IT DOES: As VramCommandsAddress (00C5:266), whose bytes these are.
; HOW: The same instructions.
; WAITS: no.
; Entry, exit: as VramCommandsAddress.
VramCommandsAddress0111:
    iny
    jsr VecSelectVramWriteAddress
    lda (zTemp12),y
    sta z:zMultiplierLo
    sta a:VDC_DATA_LO
    iny
    lda (zTemp12),y
    sta z:zMultiplierHi
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    jmp a:VramCommandsNext0111

; @routine 0111:27F CopyWordsToVdc0111
; @run 6C:827F
; WHAT IT DOES: As CopyWordsToVdc (00C5:27F), whose bytes these are.
; HOW: The same instructions.
; WAITS: no.
; Entry, exit: as CopyWordsToVdc.
CopyWordsToVdc0111:
    cly
CopyWordsToVdc0111_at0:
    lda (zTemp12),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp12),y
    sta a:VDC_DATA_HI
    iny
    cpy #$20
    bne CopyWordsToVdc0111_at0
    tya
    clc
    adc z:zTemp12
    sta z:zTemp12
    cla
    adc z:zProduct0
    sta z:zProduct0
    dex
    bne CopyWordsToVdc0111
    rts
