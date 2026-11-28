; @routine 10C5:390 CopyTileBlockToVram10C5
; @run 6F:A390
; WHAT IT DOES: CopyTileBlockToVram's copy in the program at 10C5, its camera module $390 on.
; HOW: CopyTileBlockToVram's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CopyTileBlockToVram.
CopyTileBlockToVram10C5:
    pha
    lda #$0B
    adc #$00
    clc
    adc z:zRamBaseBank
    tam #$40
    pla
    stz z:zProduct0
    asl a
    rol z:zProduct0
    asl a
    rol z:zProduct0
    asl a
    rol z:zProduct0
    asl a
    rol z:zProduct0
    asl a
    rol z:zProduct0
    clc
    adc #$00
    sta z:zTemp12
    lda z:zProduct0
    adc #$C0
    sta z:zProduct0
    jsr VecClearVdcControlHigh
    stz z:zMultiplierHi
    cly
CopyTileBlockToVram10C5_at0:
    jsr VecSelectVramWriteAddress
    lda z:zTemp10
    sta a:VDC_DATA_LO
    lda z:zTemp11
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
CopyTileBlockToVram10C5_at1:
    lda (zTemp12),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp12),y
    sta a:VDC_DATA_HI
    iny
    inc z:zMultiplierHi
    lda z:zMultiplierHi
    and #$03
    bne CopyTileBlockToVram10C5_at1
    lda #$40
    clc
    adc z:zTemp10
    sta z:zTemp10
    lda #$00
    adc z:zTemp11
    and #$0F
    sta z:zTemp11
    lda z:zMultiplierHi
    cmp #$10
    bne CopyTileBlockToVram10C5_at0
    rts

; @routine 10C5:403 ScrollSpeedsFromBands10C5
; @run 6F:A403
; WHAT IT DOES: ScrollSpeedsFromBands's copy in the program at 10C5, its camera module $390 on.
; HOW: ScrollSpeedsFromBands's body.
; WAITS: no.
; Entry, exit: as ScrollSpeedsFromBands.
ScrollSpeedsFromBands10C5:
    ldx #$07
ScrollSpeedsFromBands10C5_at0:
    lda a:ScrollXLo,x
    sec
    sbc a:ScrollBgxLo,x
    sta a:ScrollSpeedX,x
    lda a:ScrollYLo,x
    sec
    sbc a:ScrollBgyLo,x
    sta a:ScrollSpeedY,x
    dex
    bpl ScrollSpeedsFromBands10C5_at0
    rts

; @routine 10C5:41D CameraStep10C5
; @run 6F:A41D
; WHAT IT DOES: CameraStep's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraStep's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraStep.
CameraStep10C5:
    jsr ClearScrollSpeedsB
    jsr StageCameraModeDispatch10C5
    jsr ScrollSpeedsFromBands10C5
    lda a:CameraMode
    cmp #$04
    beq CameraStep10C5_at0
    cmp #$05
    beq CameraStep10C5_at0
    cmp #$07
    beq CameraStep10C5_at0
    cmp #$09
    beq CameraStep10C5_at0
    cmp #$0D
    beq CameraStep10C5_at0
    cmp #$13
    beq CameraStep10C5_at0
    cmp #$15
    beq CameraStep10C5_at0
    cmp #$16
    beq CameraStep10C5_at0
    cmp #$1C
    beq CameraStep10C5_at0
    rts
CameraStep10C5_at0:
    stz z:zTemp00
    lda a:ScrollSpeedY
    eor #$FF
    inc a
    tay
    bpl CameraStep10C5_at1
    dec z:zTemp00
CameraStep10C5_at1:
    clc
    adc a:OBJ_Y_LO
    sta a:OBJ_Y_LO
    lda z:zTemp00
    adc a:OBJ_Y_HI
    sta a:OBJ_Y_HI
    rts

; @routine 10C5:46B StageCameraModeDispatch10C5
; @run 6F:A46B
; @stop A476
; WHAT IT DOES: StageCameraModeDispatch's copy in the program at 10C5, its camera module $390 on.
; HOW: StageCameraModeDispatch's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as StageCameraModeDispatch.
CameraModes10C5 = $A476
StageCameraModeDispatch10C5:
    lda a:CameraMode
    asl a
    bcc StageCameraModeDispatch10C5_at0
    rts
StageCameraModeDispatch10C5_at0:
    tax
    jmp (CameraModes10C5,x)

; @routine 10C5:4B2 ScrollYFromLevelWord10C5
; @run 6F:A4B2
; WHAT IT DOES: ScrollYFromLevelWord's copy in the program at 10C5, its camera module $390 on.
; HOW: ScrollYFromLevelWord's body.
; WAITS: no.
; Entry, exit: as ScrollYFromLevelWord.
ScrollYFromLevelWord10C5:
    lda a:RasterWaterWanted
    sta a:ScrollYLo
    lda a:$3EB9
    sta a:ScrollYHi
    rts

; @routine 10C5:4BF ScrollStepXHalfOf0To110C5
; @run 6F:A4BF
; WHAT IT DOES: ScrollStepXHalfOf0To1's copy in the program at 10C5, its camera module $390 on.
; HOW: ScrollStepXHalfOf0To1's body.
; WAITS: no.
; Entry, exit: as ScrollStepXHalfOf0To1.
ScrollStepXHalfOf0To110C5:
    ldy #$01

; @routine 10C5:4C1 ScrollStepXHalfOf010C5
; @run 6F:A4C1
; WHAT IT DOES: ScrollStepXHalfOf0's copy in the program at 10C5, its camera module $390 on.
; HOW: ScrollStepXHalfOf0's body.
; WAITS: no.
; Entry, exit: as ScrollStepXHalfOf0.
ScrollStepXHalfOf010C5:
    clx

; @routine 10C5:4C2 ScrollStepXHalf10C5
; @run 6F:A4C2
; WHAT IT DOES: ScrollStepXHalf's copy in the program at 10C5, its camera module $390 on.
; HOW: ScrollStepXHalf's body.
; WAITS: no.
; Entry, exit: as ScrollStepXHalf.
ScrollStepXHalf10C5:
    lda a:ScrollStepXHi,x
    asl a
    lda a:ScrollStepXHi,x
    ror a
    sta a:ScrollStepXHi,y
    lda a:ScrollStepXLo,x
    ror a
    sta a:ScrollStepXLo,y
    rts

; @routine 10C5:4D5 CameraMode1B10C5
; @run 6F:A4D5
; WHAT IT DOES: CameraMode1B's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode1B's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode1B.
CameraMode1B10C5:
    lda #$08
    cly
    jsr CameraMode00Set10C5
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    lda #$BF
    sta a:ScrollYLo+1
    lda #$C0
    sta a:ScrollStepXLo+1
    lda #$FF
    sta a:ScrollStepXHi+1
    ldx #$01
    jmp AdvanceScrollXB

; @routine 10C5:4F7 CameraMode1910C5
; @run 6F:A4F7
; WHAT IT DOES: CameraMode19's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode19's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode19.
CameraMode1910C5:
    lda #$03
    sta a:ScrollColumnBlocks
    lda #$40
    sta a:ScrollYLo
    jsr CameraMode0010C5
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    jsr ScrollStepXHalfOf0To110C5
    lda a:ScrollStepXLo+1
    sec
    sbc #$40
    sta a:ScrollStepXLo+4
    lda a:ScrollStepXHi+1
    sbc #$00
    sta a:ScrollStepXHi+4
    ldx #$01
    ldy #$02
    jsr ScrollStepXHalf10C5
    lda a:ScrollStepXLo+2
    sec
    sbc #$20
    sta a:ScrollStepXLo+5
    lda a:ScrollStepXHi+2
    sbc #$00
    sta a:ScrollStepXHi+5
    ldx #$02
    ldy #$03
    jsr ScrollStepXHalf10C5
    lda a:ScrollStepXLo+3
    sec
    sbc #$10
    sta a:ScrollStepXLo+6
    lda a:ScrollStepXHi+3
    sbc #$00
    sta a:ScrollStepXHi+6
    lda #$06
    sta a:ScrollColumnBlocks+1
    lda #$C0
    sta a:ScrollYLo+1
    ldx #$01
    jsr AdvanceScrollXB
    jsr ColumnDue10C5
    stz a:ScrollYLo+1
    lda #$06
    sta a:ScrollColumnBlocks+2
    lda #$A0
    sta a:ScrollYLo+2
    ldx #$02
    jsr AdvanceScrollXB
    jsr ColumnDue10C5
    stz a:ScrollYLo+2
    ldx #$03
    jsr AdvanceScrollXB
    inx
    jsr AdvanceScrollXB
    inx
    jsr AdvanceScrollXB
    inx
    jmp AdvanceScrollXB

; @routine 10C5:58B CameraMode1710C5
; @run 6F:A58B
; WHAT IT DOES: CameraMode17's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode17's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode17.
CameraMode1710C5:
    clx
    jsr AdvanceScrollXB
    jsr ColumnDue10C5
    clx
    jsr AdvanceScrollYB
    jsr RowDueAt10C5
    lda a:ScrollYLo
    sta a:ScrollYOffset
    rts

; @routine 10C5:5A0 CameraMode1610C5
; @run 6F:A5A0
; WHAT IT DOES: CameraMode16's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode16's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode16.
CameraMode1610C5:
    lda a:ScrollXHi
    sta a:ScrollXHi+2
    tst #$03,z:zWaveCounter
    bne CameraMode1510C5
    lda a:ScrollXLo+2
    inc a
    and #$3F
    sta a:ScrollXLo+2

; @routine 10C5:5B4 CameraMode1510C5
; @run 6F:A5B4
; WHAT IT DOES: CameraMode15's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode15's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode15.
CameraMode1510C5:
    jsr CameraWaterWave10C5
    stz a:ScrollYLo
    stz a:ScrollYHi
    jsr CameraMode0010C5
    jsr ScrollYFromLevelWord10C5
    lda a:ScrollYHiScreen
    inc a
    and #$FE
    sta a:ScrollYHiScreen
    rts

; @routine 10C5:5CD CameraWaterWave10C5
; @run 6F:A5CD
; WHAT IT DOES: CameraWaterWave's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraWaterWave's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraWaterWave.
CameraWaterWave10C5:
    lda #$00
    sta z:zTemp00
    lda #$0C
    sta z:zTemp01
    lda a:CameraWavePhase
    clc
    adc #$C0
    jsr VecScaleBySine
    stz z:zTemp00
    lda z:zPlaneRowsLeft
    bpl CameraWaterWave10C5_at0
    dec z:zTemp00
CameraWaterWave10C5_at0:
    lda z:zPlaneRowsLeft
    clc
    adc #$0C
    sta a:RasterWaterWanted
    lda z:zTemp00
    adc #$00
    sta a:LevelYHi
    lda #$01
    clc
    adc a:CameraWavePhase
    sta a:CameraWavePhase
    rts

; @routine 10C5:5FF CameraMode1410C5
; @run 6F:A5FF
; WHAT IT DOES: CameraMode14's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode14's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode14.
CameraMode1410C5:
    lda #$08
    cly
    jsr CameraMode00Set10C5
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    lda #$01
    sta a:ScrollYHi+4
    jmp CameraParallaxSteps10C5

; @routine 10C5:615 CameraMode1310C5
; @run 6F:A615
; WHAT IT DOES: CameraMode13's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode13's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode13.
CameraMode1310C5:
    ldy #$03
    jsr ScrollStepXHalfOf010C5
    ldx #$03
    ldy #$04
    jsr ScrollStepXHalf10C5
    ldx #$04
    ldy #$05
    jsr ScrollStepXHalf10C5
    lda a:ScrollStepXLo+4
    clc
    adc a:ScrollStepXLo+5
    sta a:ScrollStepXLo+2
    lda a:ScrollStepXHi+4
    adc a:ScrollStepXHi+5
    sta a:ScrollStepXHi+2
    lda a:ScrollStepXLo+3
    clc
    adc a:ScrollStepXLo+5
    sta a:ScrollStepXLo+1
    lda a:ScrollStepXHi+3
    adc a:ScrollStepXHi+5
    sta a:ScrollStepXHi+1
    lda a:ScrollStepXLo+1
    clc
    adc #$10
    sta a:ScrollStepXLo+1
    cla
    adc a:ScrollStepXHi+1
    sta a:ScrollStepXHi+1
    lda #$03
    ldy #$A0
    jsr CameraMode00Set10C5
    ldx #$02
    lda #$05
    sta a:ScrollColumnBlocks+2
    jsr AdvanceScrollXB
    jsr ColumnDue10C5
    jsr ScrollYFromLevelWord10C5
    lda #$01
    sta a:ScrollYHi
    lda a:ScrollYLo
    sta a:ScrollYLo+2
    ldx #$01
    jmp AdvanceScrollXB

; @routine 10C5:685 CameraMode1210C5
; @run 6F:A685
; WHAT IT DOES: CameraMode12's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode12's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode12.
CameraMode1210C5:
    lda a:ScrollStepXLo
    clc
    adc a:ScrollStepXLo+6
    sta a:ScrollStepXLo+3
    sta a:ScrollStepXLo+4
    lda a:ScrollStepXHi
    adc a:ScrollStepXHi+6
    sta a:ScrollStepXHi+3
    sta a:ScrollStepXHi+4
    lda a:ScrollStepXLo+4
    clc
    adc a:ScrollStepXLo+6
    sta a:ScrollStepXLo+4
    lda a:ScrollStepXHi+4
    adc a:ScrollStepXHi+6
    sta a:ScrollStepXHi+4
    stz a:ScrollStepXLo+6
    stz a:ScrollStepXHi+6
    jsr ScrollStepXHalfOf0To110C5
    lda #$3F
    sta a:ScrollYLo+1
    lda #$01
    sta a:ScrollYHi+1
    ldx #$01
    ldy #$05
    jsr ScrollStepXHalf10C5
    ldx #$05
    ldy #$02
    jsr ScrollStepXHalf10C5
    lda a:ScrollStepXLo+5
    clc
    adc a:ScrollStepXLo+2
    sta a:ScrollStepXLo+2
    lda a:ScrollStepXHi+5
    adc a:ScrollStepXHi+2
    sta a:ScrollStepXHi+2
    lda #$02
    ldy #$C0
    jsr CameraMode00Set10C5
    ldx #$03
    lda #$04
    sta a:ScrollColumnBlocks+3
    lda #$40
    sta a:ScrollYLo+3
    jsr AdvanceScrollXB
    jsr ColumnDue10C5
    lda #$47
    sta a:ScrollYLo+3
    ldx #$01
    jsr AdvanceScrollXB
    ldx #$04
    jsr AdvanceScrollXB
    ldx #$02
    lda #$02
    sta a:ScrollColumnBlocks+2
    stz a:ScrollYLo
    jsr AdvanceScrollXB
    jmp ColumnDue10C5

; @routine 10C5:71D CameraMode1110C5
; @run 6F:A71D
; WHAT IT DOES: CameraMode11's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode11's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode11.
CameraMode1110C5:
    clx
    jsr AdvanceScrollXB
    jmp CameraParallaxSteps10C5

; @routine 10C5:724 CameraMode1010C5
; @run 6F:A724
; WHAT IT DOES: CameraMode10's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode10's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode10.
CameraMode1010C5:
    lda #$08
    cly
    jsr CameraMode00Set10C5
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    jsr ScrollStepXHalfOf0To110C5
    lda #$80
    sta a:ScrollStepXLo+4
    ldx #$01
    ldy #$02
    jsr ScrollStepXHalf10C5
    lda #$40
    sta a:ScrollStepXLo+5
    ldx #$02
    ldy #$03
    jsr ScrollStepXHalf10C5
    lda #$20
    sta a:ScrollStepXLo+6
    jmp CameraParallaxAdvance10C5

; @routine 10C5:755 CameraMode0F10C5
; @run 6F:A755
; WHAT IT DOES: CameraMode0F's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode0F's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode0F.
CameraMode0F10C5:
    clx
    jsr AdvanceScrollXB
    jsr ScrollYFromLevelWord10C5
    stz a:ScrollYOffset
    lda #$40
    sta a:ScrollStepXLo+2
    lda #$01
    sta a:ScrollStepXHi+3
    lda #$04
    sta a:ScrollStepXHi+4
    lda #$08
    sta a:ScrollStepXHi+5
    lda #$0A
    sta a:ScrollStepXHi+6
    jmp CameraParallaxAdvance10C5

; @routine 10C5:77B CameraMode0D10C5
; @run 6F:A77B
; WHAT IT DOES: CameraMode0D's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode0D's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode0D.
CameraMode0D10C5:
    clx
    jsr AdvanceScrollXB
    jsr ScrollYFromLevelWord10C5
    stz a:ScrollYOffset
    rts

; @routine 10C5:786 CameraMode0310C5
; @run 6F:A786
; WHAT IT DOES: CameraMode03's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode03's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode03.
CameraMode0310C5:
    lda #$01
    ldy #$C0
    jsr CameraMode00Set10C5
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    ldy #$03
    jsr ScrollStepXHalfOf010C5
    ldx #$03
    ldy #$04
    jsr ScrollStepXHalf10C5
    lda a:ScrollStepXLo+3
    clc
    adc a:ScrollStepXLo+4
    sta a:ScrollStepXLo+2
    lda a:ScrollStepXHi+3
    adc a:ScrollStepXHi+4
    sta a:ScrollStepXHi+2
    lda #$06
    sta a:ScrollColumnBlocks+2
    ldx #$02
    jsr AdvanceScrollXB
    jsr ColumnDue10C5
    lda a:ScrollXHi
    lsr a
    sta a:ScrollXHi+1
    lda a:ScrollXLo
    ror a
    sta a:ScrollXLo+1
    stz a:ScrollYLo+1
    lda #$01
    sta a:ScrollYHi+1
    stz a:ScrollYOffset
    rts

; @routine 10C5:7DB CameraMode1C10C5
; @run 6F:A7DB
; WHAT IT DOES: CameraMode1C's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode1C's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode1C.
CameraMode1C10C5:
    stz a:ScrollYLo
    lda #$01
    sta a:ScrollYHi
    jsr CameraMode0010C5
    lda a:RasterWaterWanted
    ora a:LevelYHi
    beq CameraMode1C10C5_at0
    jsr ScrollYFromLevelWord10C5
CameraMode1C10C5_at0:
    lda #$01
    sta a:ScrollYOffset
    rts

; @routine 10C5:7F7 CameraMode0410C5
; @run 6F:A7F7
; WHAT IT DOES: CameraMode04's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode04's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode04.
CameraMode0410C5:
    stz a:ScrollYLo
    stz a:ScrollYHi
    jsr CameraMode0010C5
    jsr ScrollYFromLevelWord10C5
    stz a:ScrollYOffset
    rts

; @routine 10C6:007 CameraMode0610C5
; @run 6F:A807
; WHAT IT DOES: CameraMode06's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode06's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode06.
CameraMode0610C5:
    lda a:ScrollXLo+1
    sta a:ScrollBgxLo+1
    jsr CameraMode0010C5
    lda a:ScrollXHi
    lsr a
    sta a:ScrollXHi+1
    lda a:ScrollXLo
    ror a
    sta a:ScrollXLo+1
    stz a:ScrollYOffset
    rts

; @routine 10C6:022 CameraMode0710C5
; @run 6F:A822
; @stop A859
; WHAT IT DOES: CameraMode07's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode07's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode07.
CameraMode0710C5:
    lda a:ScrollYLo
    pha
    lda a:ScrollYHi
    pha
    lda a:ScrollYHiScreen
    pha
    jsr CameraMode0010C5
    pla
    sta a:ScrollYHiScreen
    pla
    sta a:ScrollYHi
    pla
    sta a:ScrollYLo
    lda a:ScrollXHiScreen
    cmp #$03
    beq ScrollYHeldByX10C5
    bcc ScrollYAtTop10C5
    cmp #$04
    beq ScrollYSlideBy410C5
    cmp #$05
    beq ScrollYSlideBy8410C5
    cmp #$06
    beq ScrollYAtBottom10C5
    lda a:ScrollYLo
    sta a:ScrollYOffset
    rts

; @routine 10C6:059 ScrollYHeldByX10C5
; @run 6F:A859
; WHAT IT DOES: ScrollYHeldByX's copy in the program at 10C5, its camera module $390 on.
; HOW: ScrollYHeldByX's body.
; WAITS: no.
; Entry, exit: as ScrollYHeldByX.
ScrollYHeldByX10C5:
    lda a:ScrollXLo
    cmp #$F8
    bcs ScrollYSlideDown10C5

; @routine 10C6:060 ScrollYAtTop10C5
; @run 6F:A860
; WHAT IT DOES: ScrollYAtTop's copy in the program at 10C5, its camera module $390 on.
; HOW: ScrollYAtTop's body.
; WAITS: no.
; Entry, exit: as ScrollYAtTop.
ScrollYAtTop10C5:
    stz a:ScrollYLo
    stz a:ScrollYOffset
    stz a:ScrollYHi
    stz a:ScrollYHiScreen
    rts

; @routine 10C6:06D ScrollYSlideDown10C5
; @run 6F:A86D
; WHAT IT DOES: ScrollYSlideDown's copy in the program at 10C5, its camera module $390 on.
; HOW: ScrollYSlideDown's body.
; WAITS: no.
; Entry, exit: as ScrollYSlideDown.
ScrollYSlideDown10C5:
    sec
    sbc #$F8
    lsr a
    sta a:ScrollYLo
    sta a:ScrollYOffset
    stz a:ScrollYHi
    stz a:ScrollYHiScreen
    rts

; @routine 10C6:07E ScrollYAtBottom10C5
; @run 6F:A87E
; WHAT IT DOES: ScrollYAtBottom's copy in the program at 10C5, its camera module $390 on.
; HOW: ScrollYAtBottom's body.
; WAITS: no.
; Entry, exit: as ScrollYAtBottom.
ScrollYAtBottom10C5:
    lda #$01
    sta a:ScrollYHi
    sta a:ScrollYHiScreen
    stz a:ScrollYLo
    stz a:ScrollYOffset
    rts

; @routine 10C6:08D ScrollYSlideBy8410C5
; @run 6F:A88D
; WHAT IT DOES: ScrollYSlideBy84's copy in the program at 10C5, its camera module $390 on.
; HOW: ScrollYSlideBy84's body.
; WAITS: no.
; Entry, exit: as ScrollYSlideBy84.
ScrollYSlideBy8410C5:
    lda a:ScrollXLo
    cmp #$F8
    bcs ScrollYAtBottom10C5
    lsr a
    clc
    adc #$84
    sta a:ScrollYLo
    sta a:ScrollYOffset
    stz a:ScrollYHi
    stz a:ScrollYHiScreen
    rts

; @routine 10C6:0A5 ScrollYSlideBy410C5
; @run 6F:A8A5
; WHAT IT DOES: ScrollYSlideBy4's copy in the program at 10C5, its camera module $390 on.
; HOW: ScrollYSlideBy4's body.
; WAITS: no.
; Entry, exit: as ScrollYSlideBy4.
ScrollYSlideBy410C5:
    lda a:ScrollXLo
    lsr a
    clc
    adc #$04
    sta a:ScrollYLo
    sta a:ScrollYOffset
    stz a:ScrollYHi
    stz a:ScrollYHiScreen
    rts

; @routine 10C6:0B9 CameraMode1A10C5
; @run 6F:A8B9
; WHAT IT DOES: CameraMode1A's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode1A's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode1A.
CameraMode1A10C5:
    bsr CameraMode00KeepY10C5
    lda a:ScrollXHiScreen
    cmp #$07
    bne CameraMode1A10C5_at0
    jmp ScrollYHeldByX10C5
CameraMode1A10C5_at0:
    bcc ScrollYAtTop10C5
    cmp #$08
    beq ScrollYSlideBy410C5
    cmp #$09
    beq ScrollYSlideBy8410C5
    cmp #$0A
    beq ScrollYAtBottom10C5
    lda a:ScrollYLo
    sta a:ScrollYOffset
    rts

; @routine 10C6:0DA CameraMode00KeepY10C5
; @run 6F:A8DA
; WHAT IT DOES: CameraMode00KeepY's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode00KeepY's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode00KeepY.
CameraMode00KeepY10C5:
    lda a:ScrollYLo
    pha
    lda a:ScrollYHi
    pha
    lda a:ScrollYHiScreen
    pha
    jsr CameraMode0010C5
    pla
    sta a:ScrollYHiScreen
    pla
    sta a:ScrollYHi
    pla
    sta a:ScrollYLo
    rts

; @routine 10C6:0F6 CameraMode0810C5
; @run 6F:A8F6
; WHAT IT DOES: CameraMode08's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode08's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode08.
CameraMode0810C5:
    lda #$08
    cly
    jsr CameraMode00Set10C5
    lda #$02
    sta a:ScrollYHiScreen+1
    stz a:ScrollYHi+1
    lda #$10
    sta a:ScrollColumnBlocks
    lda a:ScrollXLo
    sta a:ScrollStepXHi+1
    lda a:ScrollXHiScreen
    lsr a
    ror a:ScrollStepXHi+1
    lda a:ScrollStepXHi+1
    sec
    sbc a:ScrollXLo+1
    sta a:ScrollStepXHi+1
    stz a:ScrollStepXLo+1
    lda #$04
    sta a:ScrollColumnBlocks+1
    ldx #$01
    jsr AdvanceScrollXB
    jsr ColumnDue10C5
    stz a:ScrollYOffset
    rts

; @routine 10C6:134 CameraMode1D10C5
; @run 6F:A934
; WHAT IT DOES: CameraMode1D's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode1D's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode1D.
CameraMode1D10C5:
    lda #$08
    sta a:ScrollColumnBlocks
    cla
    sta a:ScrollYLo
    jsr CameraMode0010C5
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    lda a:ScrollXHiScreen
    inc a
    lsr a
    inc a
    sta a:ScrollXHi+1
    lda a:ScrollXLo
    ror a
    sta a:ScrollXLo+1
    rts

; @routine 10C6:159 CameraMode0E10C5
; @run 6F:A959
; WHAT IT DOES: CameraMode0E's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode0E's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode0E.
CameraMode0E10C5:
    lda #$08
    cly
    jsr CameraMode00Set10C5
    lda #$01
    sta a:ScrollYHiScreen+1
    sta a:ScrollYHi+1
    lda #$10
    sta a:ScrollColumnBlocks
    jsr ScrollStepXHalfOf0To110C5
    lda #$05
    sta a:ScrollColumnBlocks+1
    ldx #$01
    jsr AdvanceScrollXB
    jsr ColumnDue10C5
    stz a:ScrollYOffset
    rts

; @routine 10C6:180 CameraMode0910C5
; @run 6F:A980
; WHAT IT DOES: CameraMode09's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode09's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode09.
CameraMode0910C5:
    lda #$08
    cly
    jsr CameraMode00Set10C5
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    jsr ScrollStepXHalfOf0To110C5
    ldx #$01
    ldy #$02
    jsr ScrollStepXHalf10C5
    stz a:ScrollYHi
    lda #$01
    sta a:ScrollYHi+1
    sta a:ScrollYHi+2
    ldx #$01
    jsr AdvanceScrollXB
    inx
    jsr AdvanceScrollXB
    jsr ScrollYFromLevelWord10C5
    stz a:ScrollYOffset
    rts

; @routine 10C6:1B3 CameraMode0C10C5
; @run 6F:A9B3
; WHAT IT DOES: CameraMode0C's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode0C's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode0C.
CameraMode0C10C5:
    lda #$08
    sta a:ScrollColumnBlocks
    cla
    bra CameraBandsY10C5

; @routine 10C6:1BB CameraMode0B10C5
; @run 6F:A9BB
; WHAT IT DOES: CameraMode0B's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode0B's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode0B.
CameraMode0B10C5:
    lda #$04
    bra CameraBandsBlocks10C5

; @routine 10C6:1BF CameraMode0A10C5
; @run 6F:A9BF
; WHAT IT DOES: CameraMode0A's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode0A's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode0A.
CameraMode0A10C5:
    lda #$03

; @routine 10C6:1C1 CameraBandsBlocks10C5
; @run 6F:A9C1
; WHAT IT DOES: CameraBandsBlocks's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraBandsBlocks's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraBandsBlocks.
CameraBandsBlocks10C5:
    sta a:ScrollColumnBlocks
    lda #$60

; @routine 10C6:1C6 CameraBandsY10C5
; @run 6F:A9C6
; WHAT IT DOES: CameraBandsY's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraBandsY's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraBandsY.
CameraBandsY10C5:
    sta a:ScrollYLo
    jsr CameraMode0010C5
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks

; @routine 10C6:1D4 CameraParallaxSteps10C5
; @run 6F:A9D4
; WHAT IT DOES: CameraParallaxSteps's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraParallaxSteps's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraParallaxSteps.
CameraParallaxSteps10C5:
    jsr ScrollStepXHalfOf0To110C5
    lda a:ScrollStepXLo+1
    clc
    adc #$40
    sta a:ScrollStepXLo+4
    cla
    adc a:ScrollStepXHi+1
    sta a:ScrollStepXHi+4
    ldx #$01
    ldy #$02
    jsr ScrollStepXHalf10C5
    lda a:ScrollStepXLo+2
    clc
    adc #$20
    sta a:ScrollStepXLo+5
    cla
    adc a:ScrollStepXHi+2
    sta a:ScrollStepXHi+5
    ldx #$02
    ldy #$03
    jsr ScrollStepXHalf10C5
    lda a:ScrollStepXLo+3
    clc
    adc #$10
    sta a:ScrollStepXLo+6
    cla
    adc a:ScrollStepXHi+3
    sta a:ScrollStepXHi+6

; @routine 10C6:215 CameraParallaxAdvance10C5
; @run 6F:AA15
; WHAT IT DOES: CameraParallaxAdvance's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraParallaxAdvance's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraParallaxAdvance.
CameraParallaxAdvance10C5:
    ldx #$01
    jsr AdvanceScrollXB
    inx
    jsr AdvanceScrollXB
    inx
    jsr AdvanceScrollXB
    inx
    jsr AdvanceScrollXB
    inx
    jsr AdvanceScrollXB
    inx
    jmp AdvanceScrollXB

; @routine 10C6:22E CameraMode00Set10C5
; @run 6F:AA2E
; WHAT IT DOES: CameraMode00Set's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode00Set's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode00Set.
CameraMode00Set10C5:
    sta a:ScrollColumnBlocks
    sty a:ScrollYLo

; @routine 10C6:234 CameraMode0010C5
; @run 6F:AA34
; @stop AA3F
; WHAT IT DOES: CameraMode00's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode00's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode00.
CameraMode0010C5:
    clx
    bsr AdvanceScrollXB
    bsr ScreenMapOfEntryB
    sta a:CameraScreenByte
    jmp ColumnDue10C5

; @routine 10C6:23F CameraMode0110C5
; @run 6F:AA3F
; WHAT IT DOES: CameraMode01's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode01's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode01.
CameraMode0110C5:
    clx
    bsr AdvanceScrollYB
    bsr ScreenMapOfEntryB
    sta a:CameraScreenByte
    jmp RowDueAt10C5

; @routine 10C6:24A CameraMode0210C5
; @run 6F:AA4A
; WHAT IT DOES: CameraMode02's copy in the program at 10C5, its camera module $390 on.
; HOW: CameraMode02's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode02.
CameraMode0210C5:
    clx
    bsr AdvanceScrollXB
    bsr AdvanceScrollYB
    bsr ScreenMapOfEntryB
    sta a:CameraScreenByte
    lda a:ScrollStepXHi,x
    rol a
    rol a
    and #$01
    tay
    jsr ColumnDue10C5
    lda a:ScrollStepYHi,x
    rol a
    rol a
    and #$01
    tay
    jmp RowDueAt10C5

; @routine 10C6:314 ColumnDueLeft10C5
; @run 6F:AB14
; WHAT IT DOES: ColumnDueLeft's copy in the program at 10C5, its camera module $390 on.
; HOW: ColumnDueLeft's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as ColumnDueLeft.
ColumnDueLeft10C5:
    lda a:ScrollBgxLo,x
    dec a
    sta z:zTemp09
    lda a:ScrollXLo,x
    dec a
    eor z:zTemp09
    and #$20
    bne ColumnDueLeft10C5_at0
    rts
ColumnDueLeft10C5_at0:
    bra DrawBlockColumn10C5

; @routine 10C6:327 ColumnDue10C5
; @run 6F:AB27
; WHAT IT DOES: ColumnDue's copy in the program at 10C5, its camera module $390 on.
; HOW: ColumnDue's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as ColumnDue.
ColumnDue10C5:
    tya
    beq ColumnDueLeft10C5
    lda a:ScrollXLo,x
    eor a:ScrollBgxLo,x
    and #$20
    bne DrawBlockColumn10C5
    rts

; @routine 10C6:335 DrawBlockColumn10C5
; @run 6F:AB35
; WHAT IT DOES: DrawBlockColumn's copy in the program at 10C5, its camera module $390 on.
; HOW: DrawBlockColumn's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as DrawBlockColumn.
LayerPlaceOffsetsHi10C5 = $AB12
LayerPlaceOffsetsLo10C5 = $AB10
DrawBlockColumn10C5:
    lda a:ScrollColumnBlocks,x
    and #$7F
    sta z:zTemp08
    jsr ScrollXPlusOffsetB
    lda a:ScrollYLo,x
    sta z:zTemp06
    lda a:ScrollYHiScreen,x
    sta z:zTemp07
    lda a:ScrollColumnBlocks,x
    bpl DrawBlockColumn10C5_at0
    ldy #$01
    lda a:ScrollYLo,x
    clc
    adc a:LayerPlaceOffsetsLo10C5,y
    sta z:zTemp06
    lda a:ScrollYHiScreen,x
    adc a:LayerPlaceOffsetsHi10C5,y
    and #$0F
    sta z:zTemp07
DrawBlockColumn10C5_at0:
    jsr BlockIndexForPlace10C5
DrawBlockColumn10C5_at3:
    lda (zTemp04),y
    phy
    jsr CopyTileBlockToVram10C5
    ply
    dec z:zTemp08
    bne DrawBlockColumn10C5_at1
    rts
DrawBlockColumn10C5_at1:
    tya
    clc
    adc #$08
    tay
    cmp #$40
    bcs DrawBlockColumn10C5_at2
    jsr BitmapBitShifted10C5
    bra DrawBlockColumn10C5_at3
DrawBlockColumn10C5_at2:
    lda z:zTemp03
    sta z:zTemp00
    lda z:zTemp07
    inc a
    and #$0F
    sta z:zTemp07
    jsr ScreenMapAtB
    jsr SelectBlockTablesL
    lda z:zTemp02
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    tay
    jsr BitmapBitShifted10C5
    jmp DrawBlockColumn10C5_at3

; @routine 10C6:3CD BlockIndexForPlace10C5
; @run 6F:ABCD
; WHAT IT DOES: BlockIndexForPlace's copy in the program at 10C5, its camera module $390 on.
; HOW: BlockIndexForPlace's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as BlockIndexForPlace.
BlockIndexForPlace10C5:
    lda z:zTemp03
    ror a
    lda z:zTemp02
    ror a
    lsr a
    lsr a
    and #$3C
    sta z:zTemp10
    lda z:zTemp07
    lsr a
    lda z:zTemp06
    sta z:zTemp11
    ror z:zTemp11
    lsr z:zTemp11
    lsr z:zTemp11
    lsr z:zTemp11
    lsr z:zTemp11
    lda z:zTemp03
    sta z:zTemp00
    lda z:zTemp07
    jsr ScreenMapAtB
    jsr SelectBlockTablesL
    lda z:zTemp02
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zTemp06
    lsr a
    lsr a
    and #$38
    ora z:zTemp00
    tay

; @routine 10C6:408 BitmapBitShifted10C5
; @run 6F:AC08
; WHAT IT DOES: BitmapBitShifted's copy in the program at 10C5, its camera module $390 on.
; HOW: BitmapBitShifted's body.
; WAITS: no.
; Entry, exit: as BitmapBitShifted.
BitmapBitShifted10C5:
    phy
    tya
    lsr a
    lsr a
    lsr a
    and #$07
    tay
    lda (zTemp0C),y
    sta z:zTemp0E
    ply
    tya
    and #$07
    phx
    tax
    inx
BitmapBitShifted10C5_at0:
    lsr z:zTemp0E
    dex
    bne BitmapBitShifted10C5_at0
    plx
    rts

; @routine 10C6:444 RowDueUp10C5
; @run 6F:AC44
; WHAT IT DOES: RowDueUp's copy in the program at 10C5, its camera module $390 on.
; HOW: RowDueUp's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as RowDueUp.
RowDueUp10C5:
    lda a:ScrollBgyLo,x
    dec a
    sta z:zTemp09
    lda a:ScrollYLo,x
    dec a
    eor z:zTemp09
    and #$20
    bne RowDueUp10C5_at0
    rts
RowDueUp10C5_at0:
    bra DrawBlockRow10C5

; @routine 10C6:457 RowDueAt10C5
; @run 6F:AC57
; WHAT IT DOES: RowDueAt's copy in the program at 10C5, its camera module $390 on.
; HOW: RowDueAt's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as RowDueAt.
RowDueAt10C5:
    tya
    beq RowDueUp10C5
    lda a:ScrollYLo,x
    eor a:ScrollBgyLo,x
    and #$20
    bne DrawBlockRow10C5
    rts

; @routine 10C6:465 DrawBlockRow10C5
; @run 6F:AC65
; WHAT IT DOES: DrawBlockRow's copy in the program at 10C5, its camera module $390 on.
; HOW: DrawBlockRow's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as DrawBlockRow.
DrawBlockRow10C5:
    lda a:ScrollColumnBlocks
    and #$7F
    sta z:zTemp08
    jsr ScrollYPlusOffsetB
    lda a:ScrollXLo,x
    sta z:zTemp02
    lda a:ScrollXHiScreen,x
    sta z:zTemp03
    lda a:ScrollColumnBlocks
    bpl DrawBlockRow10C5_at0
    ldy #$01
    lda a:ScrollXLo,x
    clc
    adc a:LayerPlaceOffsetsLo10C5,y
    sta z:zTemp02
    lda a:ScrollXHiScreen,x
    adc a:LayerPlaceOffsetsHi10C5,y
    and #$0F
    sta z:zTemp03
DrawBlockRow10C5_at0:
    jsr BlockIndexForPlace10C5
DrawBlockRow10C5_at3:
    phy
    lda z:zTemp10
    pha
    lda z:zTemp11
    pha
    lda (zTemp04),y
    jsr CopyTileBlockToVram10C5
    pla
    sta z:zTemp11
    pla
    clc
    adc #$04
    and #$3F
    sta z:zTemp10
    ply
    dec z:zTemp08
    bne DrawBlockRow10C5_at1
    rts
DrawBlockRow10C5_at1:
    iny
    tya
    and #$07
    beq DrawBlockRow10C5_at2
    jsr BitmapBitShifted10C5
    bra DrawBlockRow10C5_at3
DrawBlockRow10C5_at2:
    lda z:zTemp03
    inc a
    and #$0F
    sta z:zTemp03
    sta z:zTemp00
    lda z:zTemp07
    jsr ScreenMapAtB
    jsr SelectBlockTablesL
    lda z:zTemp06
    lsr a
    lsr a
    and #$38
    tay
    jsr BitmapBitShifted10C5
    jmp DrawBlockRow10C5_at3

; @routine 10C6:4DC StageSetUpScroll10C5
; @run 6F:ACDC
; WHAT IT DOES: StageSetUpScroll's copy in the program at 10C5, its camera module $390 on.
; HOW: StageSetUpScroll's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as StageSetUpScroll.
StageSetUpScroll10C5:
    clx
    lda #$90
    sta a:ScrollColumnBlocks,x
    lda a:CameraMode
    cmp #$0D
    beq StageSetUpScroll10C5_at0
    cmp #$11
    bne StageSetUpScroll10C5_at1
StageSetUpScroll10C5_at0:
    lda #$05
    sta z:zSetUpColumnsLeft
    bra StageSetUpScroll10C5_at2
StageSetUpScroll10C5_at1:
    lda #$0C
    sta z:zSetUpColumnsLeft
    lda #$80
    bsr ScrollXAddWrap10C5
StageSetUpScroll10C5_at2:
    lda #$20
    sta a:ScrollStepXHi
StageSetUpScroll10C5_at3:
    cly
    jsr DrawBlockColumn10C5
    jsr AdvanceScrollXB
    dec z:zSetUpColumnsLeft
    bne StageSetUpScroll10C5_at3
    cly
    jsr DrawBlockColumn10C5
    rts

; @routine 10C6:510 ScrollXAddWrap10C5
; @run 6F:AD10
; WHAT IT DOES: ScrollXAddWrap's copy in the program at 10C5, its camera module $390 on.
; HOW: ScrollXAddWrap's body.
; WAITS: no.
; Entry, exit: as ScrollXAddWrap.
ScrollXAddWrap10C5:
    clc
    adc a:ScrollXLo,x
    sta a:ScrollXLo,x
    lda a:ScrollXHiScreen,x
    adc #$FE
    and #$0F
    sta a:ScrollXHiScreen,x
    sta a:ScrollXHi,x
    rts

; @routine 10C6:541 StageSetUpScrollByEffect10C5
; @run 6F:AD41
; WHAT IT DOES: StageSetUpScrollByEffect's copy in the program at 10C5, its camera module $390 on.
; HOW: StageSetUpScrollByEffect's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as StageSetUpScrollByEffect.
EffectColumnBlocks10C5 = $AD24
StageSetUpScrollByEffect10C5:
    ldy z:zRasterEffect
    lda a:EffectColumnBlocks10C5,y
    clx
    sta a:ScrollColumnBlocks,x
    lda #$C0
    bsr ScrollXAddWrap10C5
    cpy #$0F
    beq StageSetUpScrollByEffect10C5_at0
    cpy #$18
    beq StageSetUpScrollByEffect10C5_at1
    cpy #$0C
    bne StageSetUpScrollByEffect10C5_at2
StageSetUpScrollByEffect10C5_at1:
    lda #$10
    sta z:zSetUpColumnsLeft
    bra StageSetUpScrollByEffect10C5_at3
StageSetUpScrollByEffect10C5_at0:
    lda #$10
    sta z:zSetUpColumnsLeft
    bra StageSetUpScrollByEffect10C5_at4
StageSetUpScrollByEffect10C5_at2:
    cpy #$06
    beq StageSetUpScrollByEffect10C5_at3
    cpy #$07
    beq StageSetUpScrollByEffect10C5_at3
    cpy #$10
    beq StageSetUpScrollByEffect10C5_at3
StageSetUpScrollByEffect10C5_at4:
    inc a:ScrollYHiScreen,x
StageSetUpScrollByEffect10C5_at3:
    lda #$0F
    sta z:zSetUpColumnsLeft
    jmp StageSetUpScroll10C5_at2

; @routine 12D5:389 CopyTileBlockToVram12D5
; @run 6F:A389
; WHAT IT DOES: CopyTileBlockToVram's copy in the program at 12D5, its camera module $389 on.
; HOW: CopyTileBlockToVram's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CopyTileBlockToVram.
CopyTileBlockToVram12D5:
    pha
    lda #$0B
    adc #$00
    clc
    adc z:zRamBaseBank
    tam #$40
    pla
    stz z:zProduct0
    asl a
    rol z:zProduct0
    asl a
    rol z:zProduct0
    asl a
    rol z:zProduct0
    asl a
    rol z:zProduct0
    asl a
    rol z:zProduct0
    clc
    adc #$00
    sta z:zTemp12
    lda z:zProduct0
    adc #$C0
    sta z:zProduct0
    jsr VecClearVdcControlHigh
    stz z:zMultiplierHi
    cly
CopyTileBlockToVram12D5_at0:
    jsr VecSelectVramWriteAddress
    lda z:zTemp10
    sta a:VDC_DATA_LO
    lda z:zTemp11
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
CopyTileBlockToVram12D5_at1:
    lda (zTemp12),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp12),y
    sta a:VDC_DATA_HI
    iny
    inc z:zMultiplierHi
    lda z:zMultiplierHi
    and #$03
    bne CopyTileBlockToVram12D5_at1
    lda #$40
    clc
    adc z:zTemp10
    sta z:zTemp10
    lda #$00
    adc z:zTemp11
    and #$0F
    sta z:zTemp11
    lda z:zMultiplierHi
    cmp #$10
    bne CopyTileBlockToVram12D5_at0
    rts

; @routine 12D5:3FC ScrollSpeedsFromBands12D5
; @run 6F:A3FC
; WHAT IT DOES: ScrollSpeedsFromBands's copy in the program at 12D5, its camera module $389 on.
; HOW: ScrollSpeedsFromBands's body.
; WAITS: no.
; Entry, exit: as ScrollSpeedsFromBands.
ScrollSpeedsFromBands12D5:
    ldx #$07
ScrollSpeedsFromBands12D5_at0:
    lda a:ScrollXLo,x
    sec
    sbc a:ScrollBgxLo,x
    sta a:ScrollSpeedX,x
    lda a:ScrollYLo,x
    sec
    sbc a:ScrollBgyLo,x
    sta a:ScrollSpeedY,x
    dex
    bpl ScrollSpeedsFromBands12D5_at0
    rts

; @routine 12D5:416 CameraStep12D5
; @run 6F:A416
; WHAT IT DOES: CameraStep's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraStep's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraStep.
CameraStep12D5:
    jsr ClearScrollSpeedsC
    jsr StageCameraModeDispatch12D5
    jsr ScrollSpeedsFromBands12D5
    lda a:CameraMode
    cmp #$04
    beq CameraStep12D5_at0
    cmp #$05
    beq CameraStep12D5_at0
    cmp #$07
    beq CameraStep12D5_at0
    cmp #$09
    beq CameraStep12D5_at0
    cmp #$0D
    beq CameraStep12D5_at0
    cmp #$13
    beq CameraStep12D5_at0
    cmp #$15
    beq CameraStep12D5_at0
    cmp #$16
    beq CameraStep12D5_at0
    cmp #$1C
    beq CameraStep12D5_at0
    rts
CameraStep12D5_at0:
    stz z:zTemp00
    lda a:ScrollSpeedY
    eor #$FF
    inc a
    tay
    bpl CameraStep12D5_at1
    dec z:zTemp00
CameraStep12D5_at1:
    clc
    adc a:OBJ_Y_LO
    sta a:OBJ_Y_LO
    lda z:zTemp00
    adc a:OBJ_Y_HI
    sta a:OBJ_Y_HI
    rts

; @routine 12D5:464 StageCameraModeDispatch12D5
; @run 6F:A464
; @stop A46F
; WHAT IT DOES: StageCameraModeDispatch's copy in the program at 12D5, its camera module $389 on.
; HOW: StageCameraModeDispatch's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as StageCameraModeDispatch.
CameraModes12D5 = $A46F
StageCameraModeDispatch12D5:
    lda a:CameraMode
    asl a
    bcc StageCameraModeDispatch12D5_at0
    rts
StageCameraModeDispatch12D5_at0:
    tax
    jmp (CameraModes12D5,x)

; @routine 12D5:4AB ScrollYFromLevelWord12D5
; @run 6F:A4AB
; WHAT IT DOES: ScrollYFromLevelWord's copy in the program at 12D5, its camera module $389 on.
; HOW: ScrollYFromLevelWord's body.
; WAITS: no.
; Entry, exit: as ScrollYFromLevelWord.
ScrollYFromLevelWord12D5:
    lda a:RasterWaterWanted
    sta a:ScrollYLo
    lda a:$3EB9
    sta a:ScrollYHi
    rts

; @routine 12D5:4B8 ScrollStepXHalfOf0To112D5
; @run 6F:A4B8
; WHAT IT DOES: ScrollStepXHalfOf0To1's copy in the program at 12D5, its camera module $389 on.
; HOW: ScrollStepXHalfOf0To1's body.
; WAITS: no.
; Entry, exit: as ScrollStepXHalfOf0To1.
ScrollStepXHalfOf0To112D5:
    ldy #$01

; @routine 12D5:4BA ScrollStepXHalfOf012D5
; @run 6F:A4BA
; WHAT IT DOES: ScrollStepXHalfOf0's copy in the program at 12D5, its camera module $389 on.
; HOW: ScrollStepXHalfOf0's body.
; WAITS: no.
; Entry, exit: as ScrollStepXHalfOf0.
ScrollStepXHalfOf012D5:
    clx

; @routine 12D5:4BB ScrollStepXHalf12D5
; @run 6F:A4BB
; WHAT IT DOES: ScrollStepXHalf's copy in the program at 12D5, its camera module $389 on.
; HOW: ScrollStepXHalf's body.
; WAITS: no.
; Entry, exit: as ScrollStepXHalf.
ScrollStepXHalf12D5:
    lda a:ScrollStepXHi,x
    asl a
    lda a:ScrollStepXHi,x
    ror a
    sta a:ScrollStepXHi,y
    lda a:ScrollStepXLo,x
    ror a
    sta a:ScrollStepXLo,y
    rts

; @routine 12D5:4CE CameraMode1B12D5
; @run 6F:A4CE
; WHAT IT DOES: CameraMode1B's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode1B's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode1B.
CameraMode1B12D5:
    lda #$08
    cly
    jsr CameraMode00Set12D5
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    lda #$BF
    sta a:ScrollYLo+1
    lda #$C0
    sta a:ScrollStepXLo+1
    lda #$FF
    sta a:ScrollStepXHi+1
    ldx #$01
    jmp AdvanceScrollXC

; @routine 12D5:4F0 CameraMode1912D5
; @run 6F:A4F0
; WHAT IT DOES: CameraMode19's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode19's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode19.
CameraMode1912D5:
    lda #$03
    sta a:ScrollColumnBlocks
    lda #$40
    sta a:ScrollYLo
    jsr CameraMode0012D5
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    jsr ScrollStepXHalfOf0To112D5
    lda a:ScrollStepXLo+1
    sec
    sbc #$40
    sta a:ScrollStepXLo+4
    lda a:ScrollStepXHi+1
    sbc #$00
    sta a:ScrollStepXHi+4
    ldx #$01
    ldy #$02
    jsr ScrollStepXHalf12D5
    lda a:ScrollStepXLo+2
    sec
    sbc #$20
    sta a:ScrollStepXLo+5
    lda a:ScrollStepXHi+2
    sbc #$00
    sta a:ScrollStepXHi+5
    ldx #$02
    ldy #$03
    jsr ScrollStepXHalf12D5
    lda a:ScrollStepXLo+3
    sec
    sbc #$10
    sta a:ScrollStepXLo+6
    lda a:ScrollStepXHi+3
    sbc #$00
    sta a:ScrollStepXHi+6
    lda #$06
    sta a:ScrollColumnBlocks+1
    lda #$C0
    sta a:ScrollYLo+1
    ldx #$01
    jsr AdvanceScrollXC
    jsr ColumnDue12D5
    stz a:ScrollYLo+1
    lda #$06
    sta a:ScrollColumnBlocks+2
    lda #$A0
    sta a:ScrollYLo+2
    ldx #$02
    jsr AdvanceScrollXC
    jsr ColumnDue12D5
    stz a:ScrollYLo+2
    ldx #$03
    jsr AdvanceScrollXC
    inx
    jsr AdvanceScrollXC
    inx
    jsr AdvanceScrollXC
    inx
    jmp AdvanceScrollXC

; @routine 12D5:584 CameraMode1712D5
; @run 6F:A584
; WHAT IT DOES: CameraMode17's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode17's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode17.
CameraMode1712D5:
    clx
    jsr AdvanceScrollXC
    jsr ColumnDue12D5
    clx
    jsr AdvanceScrollY12D5
    jsr RowDueAt12D5
    lda a:ScrollYLo
    sta a:ScrollYOffset
    rts

; @routine 12D5:599 CameraMode1612D5
; @run 6F:A599
; WHAT IT DOES: CameraMode16's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode16's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode16.
CameraMode1612D5:
    lda a:ScrollXHi
    sta a:ScrollXHi+2
    tst #$03,z:zWaveCounter
    bne CameraMode1512D5
    lda a:ScrollXLo+2
    inc a
    and #$3F
    sta a:ScrollXLo+2

; @routine 12D5:5AD CameraMode1512D5
; @run 6F:A5AD
; WHAT IT DOES: CameraMode15's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode15's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode15.
CameraMode1512D5:
    jsr CameraWaterWave12D5
    stz a:ScrollYLo
    stz a:ScrollYHi
    jsr CameraMode0012D5
    jsr ScrollYFromLevelWord12D5
    lda a:ScrollYHiScreen
    inc a
    and #$FE
    sta a:ScrollYHiScreen
    rts

; @routine 12D5:5C6 CameraWaterWave12D5
; @run 6F:A5C6
; WHAT IT DOES: CameraWaterWave's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraWaterWave's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraWaterWave.
CameraWaterWave12D5:
    lda #$00
    sta z:zTemp00
    lda #$0C
    sta z:zTemp01
    lda a:CameraWavePhase
    clc
    adc #$C0
    jsr VecScaleBySine
    stz z:zTemp00
    lda z:zPlaneRowsLeft
    bpl CameraWaterWave12D5_at0
    dec z:zTemp00
CameraWaterWave12D5_at0:
    lda z:zPlaneRowsLeft
    clc
    adc #$0C
    sta a:RasterWaterWanted
    lda z:zTemp00
    adc #$00
    sta a:LevelYHi
    lda #$01
    clc
    adc a:CameraWavePhase
    sta a:CameraWavePhase
    rts

; @routine 12D5:5F8 CameraMode1412D5
; @run 6F:A5F8
; WHAT IT DOES: CameraMode14's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode14's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode14.
CameraMode1412D5:
    lda #$08
    cly
    jsr CameraMode00Set12D5
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    lda #$01
    sta a:ScrollYHi+4
    jmp CameraParallaxSteps12D5

; @routine 12D5:60E CameraMode1312D5
; @run 6F:A60E
; WHAT IT DOES: CameraMode13's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode13's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode13.
CameraMode1312D5:
    ldy #$03
    jsr ScrollStepXHalfOf012D5
    ldx #$03
    ldy #$04
    jsr ScrollStepXHalf12D5
    ldx #$04
    ldy #$05
    jsr ScrollStepXHalf12D5
    lda a:ScrollStepXLo+4
    clc
    adc a:ScrollStepXLo+5
    sta a:ScrollStepXLo+2
    lda a:ScrollStepXHi+4
    adc a:ScrollStepXHi+5
    sta a:ScrollStepXHi+2
    lda a:ScrollStepXLo+3
    clc
    adc a:ScrollStepXLo+5
    sta a:ScrollStepXLo+1
    lda a:ScrollStepXHi+3
    adc a:ScrollStepXHi+5
    sta a:ScrollStepXHi+1
    lda a:ScrollStepXLo+1
    clc
    adc #$10
    sta a:ScrollStepXLo+1
    cla
    adc a:ScrollStepXHi+1
    sta a:ScrollStepXHi+1
    lda #$03
    ldy #$A0
    jsr CameraMode00Set12D5
    ldx #$02
    lda #$05
    sta a:ScrollColumnBlocks+2
    jsr AdvanceScrollXC
    jsr ColumnDue12D5
    jsr ScrollYFromLevelWord12D5
    lda #$01
    sta a:ScrollYHi
    lda a:ScrollYLo
    sta a:ScrollYLo+2
    ldx #$01
    jmp AdvanceScrollXC

; @routine 12D5:67E CameraMode1212D5
; @run 6F:A67E
; WHAT IT DOES: CameraMode12's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode12's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode12.
CameraMode1212D5:
    lda a:ScrollStepXLo
    clc
    adc a:ScrollStepXLo+6
    sta a:ScrollStepXLo+3
    sta a:ScrollStepXLo+4
    lda a:ScrollStepXHi
    adc a:ScrollStepXHi+6
    sta a:ScrollStepXHi+3
    sta a:ScrollStepXHi+4
    lda a:ScrollStepXLo+4
    clc
    adc a:ScrollStepXLo+6
    sta a:ScrollStepXLo+4
    lda a:ScrollStepXHi+4
    adc a:ScrollStepXHi+6
    sta a:ScrollStepXHi+4
    stz a:ScrollStepXLo+6
    stz a:ScrollStepXHi+6
    jsr ScrollStepXHalfOf0To112D5
    lda #$3F
    sta a:ScrollYLo+1
    lda #$01
    sta a:ScrollYHi+1
    ldx #$01
    ldy #$05
    jsr ScrollStepXHalf12D5
    ldx #$05
    ldy #$02
    jsr ScrollStepXHalf12D5
    lda a:ScrollStepXLo+5
    clc
    adc a:ScrollStepXLo+2
    sta a:ScrollStepXLo+2
    lda a:ScrollStepXHi+5
    adc a:ScrollStepXHi+2
    sta a:ScrollStepXHi+2
    lda #$02
    ldy #$C0
    jsr CameraMode00Set12D5
    ldx #$03
    lda #$04
    sta a:ScrollColumnBlocks+3
    lda #$40
    sta a:ScrollYLo+3
    jsr AdvanceScrollXC
    jsr ColumnDue12D5
    lda #$47
    sta a:ScrollYLo+3
    ldx #$01
    jsr AdvanceScrollXC
    ldx #$04
    jsr AdvanceScrollXC
    ldx #$02
    lda #$02
    sta a:ScrollColumnBlocks+2
    stz a:ScrollYLo
    jsr AdvanceScrollXC
    jmp ColumnDue12D5

; @routine 12D5:716 CameraMode1112D5
; @run 6F:A716
; WHAT IT DOES: CameraMode11's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode11's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode11.
CameraMode1112D5:
    clx
    jsr AdvanceScrollXC
    jmp CameraParallaxSteps12D5

; @routine 12D5:71D CameraMode1012D5
; @run 6F:A71D
; WHAT IT DOES: CameraMode10's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode10's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode10.
CameraMode1012D5:
    lda #$08
    cly
    jsr CameraMode00Set12D5
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    jsr ScrollStepXHalfOf0To112D5
    lda #$80
    sta a:ScrollStepXLo+4
    ldx #$01
    ldy #$02
    jsr ScrollStepXHalf12D5
    lda #$40
    sta a:ScrollStepXLo+5
    ldx #$02
    ldy #$03
    jsr ScrollStepXHalf12D5
    lda #$20
    sta a:ScrollStepXLo+6
    jmp CameraParallaxAdvance12D5

; @routine 12D5:74E CameraMode0F12D5
; @run 6F:A74E
; WHAT IT DOES: CameraMode0F's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode0F's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode0F.
CameraMode0F12D5:
    clx
    jsr AdvanceScrollXC
    jsr ScrollYFromLevelWord12D5
    stz a:ScrollYOffset
    lda #$40
    sta a:ScrollStepXLo+2
    lda #$01
    sta a:ScrollStepXHi+3
    lda #$04
    sta a:ScrollStepXHi+4
    lda #$08
    sta a:ScrollStepXHi+5
    lda #$0A
    sta a:ScrollStepXHi+6
    jmp CameraParallaxAdvance12D5

; @routine 12D5:774 CameraMode0D12D5
; @run 6F:A774
; WHAT IT DOES: CameraMode0D's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode0D's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode0D.
CameraMode0D12D5:
    clx
    jsr AdvanceScrollXC
    jsr ScrollYFromLevelWord12D5
    stz a:ScrollYOffset
    rts

; @routine 12D5:77F CameraMode0312D5
; @run 6F:A77F
; WHAT IT DOES: CameraMode03's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode03's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode03.
CameraMode0312D5:
    lda #$01
    ldy #$C0
    jsr CameraMode00Set12D5
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    ldy #$03
    jsr ScrollStepXHalfOf012D5
    ldx #$03
    ldy #$04
    jsr ScrollStepXHalf12D5
    lda a:ScrollStepXLo+3
    clc
    adc a:ScrollStepXLo+4
    sta a:ScrollStepXLo+2
    lda a:ScrollStepXHi+3
    adc a:ScrollStepXHi+4
    sta a:ScrollStepXHi+2
    lda #$06
    sta a:ScrollColumnBlocks+2
    ldx #$02
    jsr AdvanceScrollXC
    jsr ColumnDue12D5
    lda a:ScrollXHi
    lsr a
    sta a:ScrollXHi+1
    lda a:ScrollXLo
    ror a
    sta a:ScrollXLo+1
    stz a:ScrollYLo+1
    lda #$01
    sta a:ScrollYHi+1
    stz a:ScrollYOffset
    rts

; @routine 12D5:7D4 CameraMode1C12D5
; @run 6F:A7D4
; WHAT IT DOES: CameraMode1C's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode1C's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode1C.
CameraMode1C12D5:
    stz a:ScrollYLo
    lda #$01
    sta a:ScrollYHi
    jsr CameraMode0012D5
    lda a:RasterWaterWanted
    ora a:LevelYHi
    beq CameraMode1C12D5_at0
    jsr ScrollYFromLevelWord12D5
CameraMode1C12D5_at0:
    lda #$01
    sta a:ScrollYOffset
    rts

; @routine 12D5:7F0 CameraMode0412D5
; @run 6F:A7F0
; WHAT IT DOES: CameraMode04's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode04's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode04.
CameraMode0412D5:
    stz a:ScrollYLo
    stz a:ScrollYHi
    jsr CameraMode0012D5
    jsr ScrollYFromLevelWord12D5
    stz a:ScrollYOffset
    rts

; @routine 12D6:000 CameraMode0612D5
; @run 6F:A800
; WHAT IT DOES: CameraMode06's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode06's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode06.
CameraMode0612D5:
    lda a:ScrollXLo+1
    sta a:ScrollBgxLo+1
    jsr CameraMode0012D5
    lda a:ScrollXHi
    lsr a
    sta a:ScrollXHi+1
    lda a:ScrollXLo
    ror a
    sta a:ScrollXLo+1
    stz a:ScrollYOffset
    rts

; @routine 12D6:01B CameraMode0712D5
; @run 6F:A81B
; @stop A852
; WHAT IT DOES: CameraMode07's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode07's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode07.
CameraMode0712D5:
    lda a:ScrollYLo
    pha
    lda a:ScrollYHi
    pha
    lda a:ScrollYHiScreen
    pha
    jsr CameraMode0012D5
    pla
    sta a:ScrollYHiScreen
    pla
    sta a:ScrollYHi
    pla
    sta a:ScrollYLo
    lda a:ScrollXHiScreen
    cmp #$03
    beq ScrollYHeldByX12D5
    bcc ScrollYAtTop12D5
    cmp #$04
    beq ScrollYSlideBy412D5
    cmp #$05
    beq ScrollYSlideBy8412D5
    cmp #$06
    beq ScrollYAtBottom12D5
    lda a:ScrollYLo
    sta a:ScrollYOffset
    rts

; @routine 12D6:052 ScrollYHeldByX12D5
; @run 6F:A852
; WHAT IT DOES: ScrollYHeldByX's copy in the program at 12D5, its camera module $389 on.
; HOW: ScrollYHeldByX's body.
; WAITS: no.
; Entry, exit: as ScrollYHeldByX.
ScrollYHeldByX12D5:
    lda a:ScrollXLo
    cmp #$F8
    bcs ScrollYSlideDown12D5

; @routine 12D6:059 ScrollYAtTop12D5
; @run 6F:A859
; WHAT IT DOES: ScrollYAtTop's copy in the program at 12D5, its camera module $389 on.
; HOW: ScrollYAtTop's body.
; WAITS: no.
; Entry, exit: as ScrollYAtTop.
ScrollYAtTop12D5:
    stz a:ScrollYLo
    stz a:ScrollYOffset
    stz a:ScrollYHi
    stz a:ScrollYHiScreen
    rts

; @routine 12D6:066 ScrollYSlideDown12D5
; @run 6F:A866
; WHAT IT DOES: ScrollYSlideDown's copy in the program at 12D5, its camera module $389 on.
; HOW: ScrollYSlideDown's body.
; WAITS: no.
; Entry, exit: as ScrollYSlideDown.
ScrollYSlideDown12D5:
    sec
    sbc #$F8
    lsr a
    sta a:ScrollYLo
    sta a:ScrollYOffset
    stz a:ScrollYHi
    stz a:ScrollYHiScreen
    rts

; @routine 12D6:077 ScrollYAtBottom12D5
; @run 6F:A877
; WHAT IT DOES: ScrollYAtBottom's copy in the program at 12D5, its camera module $389 on.
; HOW: ScrollYAtBottom's body.
; WAITS: no.
; Entry, exit: as ScrollYAtBottom.
ScrollYAtBottom12D5:
    lda #$01
    sta a:ScrollYHi
    sta a:ScrollYHiScreen
    stz a:ScrollYLo
    stz a:ScrollYOffset
    rts

; @routine 12D6:086 ScrollYSlideBy8412D5
; @run 6F:A886
; WHAT IT DOES: ScrollYSlideBy84's copy in the program at 12D5, its camera module $389 on.
; HOW: ScrollYSlideBy84's body.
; WAITS: no.
; Entry, exit: as ScrollYSlideBy84.
ScrollYSlideBy8412D5:
    lda a:ScrollXLo
    cmp #$F8
    bcs ScrollYAtBottom12D5
    lsr a
    clc
    adc #$84
    sta a:ScrollYLo
    sta a:ScrollYOffset
    stz a:ScrollYHi
    stz a:ScrollYHiScreen
    rts

; @routine 12D6:09E ScrollYSlideBy412D5
; @run 6F:A89E
; WHAT IT DOES: ScrollYSlideBy4's copy in the program at 12D5, its camera module $389 on.
; HOW: ScrollYSlideBy4's body.
; WAITS: no.
; Entry, exit: as ScrollYSlideBy4.
ScrollYSlideBy412D5:
    lda a:ScrollXLo
    lsr a
    clc
    adc #$04
    sta a:ScrollYLo
    sta a:ScrollYOffset
    stz a:ScrollYHi
    stz a:ScrollYHiScreen
    rts

; @routine 12D6:0B2 CameraMode1A12D5
; @run 6F:A8B2
; WHAT IT DOES: CameraMode1A's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode1A's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode1A.
CameraMode1A12D5:
    bsr CameraMode00KeepY12D5
    lda a:ScrollXHiScreen
    cmp #$07
    bne CameraMode1A12D5_at0
    jmp ScrollYHeldByX12D5
CameraMode1A12D5_at0:
    bcc ScrollYAtTop12D5
    cmp #$08
    beq ScrollYSlideBy412D5
    cmp #$09
    beq ScrollYSlideBy8412D5
    cmp #$0A
    beq ScrollYAtBottom12D5
    lda a:ScrollYLo
    sta a:ScrollYOffset
    rts

; @routine 12D6:0D3 CameraMode00KeepY12D5
; @run 6F:A8D3
; WHAT IT DOES: CameraMode00KeepY's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode00KeepY's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode00KeepY.
CameraMode00KeepY12D5:
    lda a:ScrollYLo
    pha
    lda a:ScrollYHi
    pha
    lda a:ScrollYHiScreen
    pha
    jsr CameraMode0012D5
    pla
    sta a:ScrollYHiScreen
    pla
    sta a:ScrollYHi
    pla
    sta a:ScrollYLo
    rts

; @routine 12D6:0EF CameraMode0812D5
; @run 6F:A8EF
; WHAT IT DOES: CameraMode08's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode08's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode08.
CameraMode0812D5:
    lda #$08
    cly
    jsr CameraMode00Set12D5
    lda #$02
    sta a:ScrollYHiScreen+1
    stz a:ScrollYHi+1
    lda #$10
    sta a:ScrollColumnBlocks
    lda a:ScrollXLo
    sta a:ScrollStepXHi+1
    lda a:ScrollXHiScreen
    lsr a
    ror a:ScrollStepXHi+1
    lda a:ScrollStepXHi+1
    sec
    sbc a:ScrollXLo+1
    sta a:ScrollStepXHi+1
    stz a:ScrollStepXLo+1
    lda #$04
    sta a:ScrollColumnBlocks+1
    ldx #$01
    jsr AdvanceScrollXC
    jsr ColumnDue12D5
    stz a:ScrollYOffset
    rts

; @routine 12D6:12D CameraMode1D12D5
; @run 6F:A92D
; WHAT IT DOES: CameraMode1D's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode1D's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode1D.
CameraMode1D12D5:
    lda #$08
    sta a:ScrollColumnBlocks
    cla
    sta a:ScrollYLo
    jsr CameraMode0012D5
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    lda a:ScrollXHiScreen
    inc a
    lsr a
    inc a
    sta a:ScrollXHi+1
    lda a:ScrollXLo
    ror a
    sta a:ScrollXLo+1
    rts

; @routine 12D6:152 CameraMode0E12D5
; @run 6F:A952
; WHAT IT DOES: CameraMode0E's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode0E's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode0E.
CameraMode0E12D5:
    lda #$08
    cly
    jsr CameraMode00Set12D5
    lda #$01
    sta a:ScrollYHiScreen+1
    sta a:ScrollYHi+1
    lda #$10
    sta a:ScrollColumnBlocks
    jsr ScrollStepXHalfOf0To112D5
    lda #$05
    sta a:ScrollColumnBlocks+1
    ldx #$01
    jsr AdvanceScrollXC
    jsr ColumnDue12D5
    stz a:ScrollYOffset
    rts

; @routine 12D6:179 CameraMode0912D5
; @run 6F:A979
; WHAT IT DOES: CameraMode09's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode09's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode09.
CameraMode0912D5:
    lda #$08
    cly
    jsr CameraMode00Set12D5
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    jsr ScrollStepXHalfOf0To112D5
    ldx #$01
    ldy #$02
    jsr ScrollStepXHalf12D5
    stz a:ScrollYHi
    lda #$01
    sta a:ScrollYHi+1
    sta a:ScrollYHi+2
    ldx #$01
    jsr AdvanceScrollXC
    inx
    jsr AdvanceScrollXC
    jsr ScrollYFromLevelWord12D5
    stz a:ScrollYOffset
    rts

; @routine 12D6:1AC CameraMode0C12D5
; @run 6F:A9AC
; WHAT IT DOES: CameraMode0C's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode0C's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode0C.
CameraMode0C12D5:
    lda #$08
    sta a:ScrollColumnBlocks
    cla
    bra CameraBandsY12D5

; @routine 12D6:1B4 CameraMode0B12D5
; @run 6F:A9B4
; WHAT IT DOES: CameraMode0B's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode0B's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode0B.
CameraMode0B12D5:
    lda #$04
    bra CameraBandsBlocks12D5

; @routine 12D6:1B8 CameraMode0A12D5
; @run 6F:A9B8
; WHAT IT DOES: CameraMode0A's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode0A's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode0A.
CameraMode0A12D5:
    lda #$03

; @routine 12D6:1BA CameraBandsBlocks12D5
; @run 6F:A9BA
; WHAT IT DOES: CameraBandsBlocks's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraBandsBlocks's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraBandsBlocks.
CameraBandsBlocks12D5:
    sta a:ScrollColumnBlocks
    lda #$60

; @routine 12D6:1BF CameraBandsY12D5
; @run 6F:A9BF
; WHAT IT DOES: CameraBandsY's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraBandsY's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraBandsY.
CameraBandsY12D5:
    sta a:ScrollYLo
    jsr CameraMode0012D5
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks

; @routine 12D6:1CD CameraParallaxSteps12D5
; @run 6F:A9CD
; WHAT IT DOES: CameraParallaxSteps's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraParallaxSteps's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraParallaxSteps.
CameraParallaxSteps12D5:
    jsr ScrollStepXHalfOf0To112D5
    lda a:ScrollStepXLo+1
    clc
    adc #$40
    sta a:ScrollStepXLo+4
    cla
    adc a:ScrollStepXHi+1
    sta a:ScrollStepXHi+4
    ldx #$01
    ldy #$02
    jsr ScrollStepXHalf12D5
    lda a:ScrollStepXLo+2
    clc
    adc #$20
    sta a:ScrollStepXLo+5
    cla
    adc a:ScrollStepXHi+2
    sta a:ScrollStepXHi+5
    ldx #$02
    ldy #$03
    jsr ScrollStepXHalf12D5
    lda a:ScrollStepXLo+3
    clc
    adc #$10
    sta a:ScrollStepXLo+6
    cla
    adc a:ScrollStepXHi+3
    sta a:ScrollStepXHi+6

; @routine 12D6:20E CameraParallaxAdvance12D5
; @run 6F:AA0E
; WHAT IT DOES: CameraParallaxAdvance's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraParallaxAdvance's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraParallaxAdvance.
CameraParallaxAdvance12D5:
    ldx #$01
    jsr AdvanceScrollXC
    inx
    jsr AdvanceScrollXC
    inx
    jsr AdvanceScrollXC
    inx
    jsr AdvanceScrollXC
    inx
    jsr AdvanceScrollXC
    inx
    jmp AdvanceScrollXC

; @routine 12D6:227 CameraMode00Set12D5
; @run 6F:AA27
; WHAT IT DOES: CameraMode00Set's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode00Set's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode00Set.
CameraMode00Set12D5:
    sta a:ScrollColumnBlocks
    sty a:ScrollYLo

; @routine 12D6:22D CameraMode0012D5
; @run 6F:AA2D
; @stop AA38
; WHAT IT DOES: CameraMode00's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode00's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode00.
CameraMode0012D5:
    clx
    bsr AdvanceScrollXC
    bsr ScreenMapAtEntry12D5
    sta a:CameraScreenByte
    jmp ColumnDue12D5

; @routine 12D6:238 CameraMode0112D5
; @run 6F:AA38
; WHAT IT DOES: CameraMode01's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode01's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode01.
CameraMode0112D5:
    clx
    bsr AdvanceScrollY12D5
    bsr ScreenMapAtEntry12D5
    sta a:CameraScreenByte
    jmp RowDueAt12D5

; @routine 12D6:243 CameraMode0212D5
; @run 6F:AA43
; WHAT IT DOES: CameraMode02's copy in the program at 12D5, its camera module $389 on.
; HOW: CameraMode02's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as CameraMode02.
CameraMode0212D5:
    clx
    bsr AdvanceScrollXC
    bsr AdvanceScrollY12D5
    bsr ScreenMapAtEntry12D5
    sta a:CameraScreenByte
    lda a:ScrollStepXHi,x
    rol a
    rol a
    and #$01
    tay
    jsr ColumnDue12D5
    lda a:ScrollStepYHi,x
    rol a
    rol a
    and #$01
    tay
    jmp RowDueAt12D5

; @routine 12D6:263 ScreenMapAtEntry12D5
; @run 6F:AA63
; WHAT IT DOES: ScreenMapAtEntry's copy in the program at 12D5, its camera module $389 on.
; HOW: ScreenMapAtEntry's body.
; WAITS: no.
; Entry, exit: as ScreenMapAtEntry.
ScreenMapAtEntry12D5:
    lda a:ScrollXHiScreen,x
    sta z:zTemp00
    lda a:ScrollYHiScreen,x

; @routine 12D6:26B ScreenMapAt12D5
; @run 6F:AA6B
; WHAT IT DOES: ScreenMapAt's copy in the program at 12D5, its camera module $389 on.
; HOW: ScreenMapAt's body.
; WAITS: no.
; Entry, exit: as ScreenMapAt.
ScreenMapAt12D5:
    asl a
    asl a
    asl a
    asl a
    ora z:zTemp00
    clc
    adc #$00
    sta z:zTemp00
    lda #$3D
    sta z:zTemp01
    lda (zTemp00)
    rts

; @routine 12D6:2B2 AdvanceScrollY12D5
; @run 6F:AAB2
; WHAT IT DOES: AdvanceScrollY's copy in the program at 12D5, its camera module $389 on.
; HOW: AdvanceScrollY's body with the sign table moved ($AAB0).
; WAITS: no.
; Entry, exit: as AdvanceScrollY.
AdvanceScrollY12D5:
    lda a:ScrollStepYHi,x
    rol a
    rol a
    and #$01
    tay
    lda a:ScrollStepYLo,x
    clc
    adc a:ScrollYFrac,x
    sta a:ScrollYFrac,x
    lda a:ScrollStepYHi,x
    adc a:ScrollYLo,x
    sta a:ScrollYLo,x
    lda #$00
    adc a:$AAB0,y
    pha
    clc
    adc a:ScrollYHi,x
    sta a:ScrollYHi,x
    pla
    clc
    adc a:ScrollYHiScreen,x
    and #$0F
    sta a:ScrollYHiScreen,x
    rts

; @routine 12D6:30D ColumnDueLeft12D5
; @run 6F:AB0D
; WHAT IT DOES: ColumnDueLeft's copy in the program at 12D5, its camera module $389 on.
; HOW: ColumnDueLeft's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as ColumnDueLeft.
ColumnDueLeft12D5:
    lda a:ScrollBgxLo,x
    dec a
    sta z:zTemp09
    lda a:ScrollXLo,x
    dec a
    eor z:zTemp09
    and #$20
    bne ColumnDueLeft12D5_at0
    rts
ColumnDueLeft12D5_at0:
    bra DrawBlockColumn12D5

; @routine 12D6:320 ColumnDue12D5
; @run 6F:AB20
; WHAT IT DOES: ColumnDue's copy in the program at 12D5, its camera module $389 on.
; HOW: ColumnDue's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as ColumnDue.
ColumnDue12D5:
    tya
    beq ColumnDueLeft12D5
    lda a:ScrollXLo,x
    eor a:ScrollBgxLo,x
    and #$20
    bne DrawBlockColumn12D5
    rts

; @routine 12D6:32E DrawBlockColumn12D5
; @run 6F:AB2E
; WHAT IT DOES: DrawBlockColumn's copy in the program at 12D5, its camera module $389 on.
; HOW: DrawBlockColumn's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as DrawBlockColumn.
LayerPlaceOffsetsHi12D5 = $AB0B
LayerPlaceOffsetsLo12D5 = $AB09
DrawBlockColumn12D5:
    lda a:ScrollColumnBlocks,x
    and #$7F
    sta z:zTemp08
    jsr ScrollXPlusOffsetC
    lda a:ScrollYLo,x
    sta z:zTemp06
    lda a:ScrollYHiScreen,x
    sta z:zTemp07
    lda a:ScrollColumnBlocks,x
    bpl DrawBlockColumn12D5_at0
    ldy #$01
    lda a:ScrollYLo,x
    clc
    adc a:LayerPlaceOffsetsLo12D5,y
    sta z:zTemp06
    lda a:ScrollYHiScreen,x
    adc a:LayerPlaceOffsetsHi12D5,y
    and #$0F
    sta z:zTemp07
DrawBlockColumn12D5_at0:
    jsr BlockIndexForPlace12D5
DrawBlockColumn12D5_at3:
    lda (zTemp04),y
    phy
    jsr CopyTileBlockToVram12D5
    ply
    dec z:zTemp08
    bne DrawBlockColumn12D5_at1
    rts
DrawBlockColumn12D5_at1:
    tya
    clc
    adc #$08
    tay
    cmp #$40
    bcs DrawBlockColumn12D5_at2
    jsr BitmapBitShifted12D5
    bra DrawBlockColumn12D5_at3
DrawBlockColumn12D5_at2:
    lda z:zTemp03
    sta z:zTemp00
    lda z:zTemp07
    inc a
    and #$0F
    sta z:zTemp07
    jsr ScreenMapAt12D5
    jsr SelectBlockTablesN
    lda z:zTemp02
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    tay
    jsr BitmapBitShifted12D5
    jmp DrawBlockColumn12D5_at3

; @routine 12D6:3C6 BlockIndexForPlace12D5
; @run 6F:ABC6
; WHAT IT DOES: BlockIndexForPlace's copy in the program at 12D5, its camera module $389 on.
; HOW: BlockIndexForPlace's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as BlockIndexForPlace.
BlockIndexForPlace12D5:
    lda z:zTemp03
    ror a
    lda z:zTemp02
    ror a
    lsr a
    lsr a
    and #$3C
    sta z:zTemp10
    lda z:zTemp07
    lsr a
    lda z:zTemp06
    sta z:zTemp11
    ror z:zTemp11
    lsr z:zTemp11
    lsr z:zTemp11
    lsr z:zTemp11
    lsr z:zTemp11
    lda z:zTemp03
    sta z:zTemp00
    lda z:zTemp07
    jsr ScreenMapAt12D5
    jsr SelectBlockTablesN
    lda z:zTemp02
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zTemp06
    lsr a
    lsr a
    and #$38
    ora z:zTemp00
    tay

; @routine 12D6:401 BitmapBitShifted12D5
; @run 6F:AC01
; WHAT IT DOES: BitmapBitShifted's copy in the program at 12D5, its camera module $389 on.
; HOW: BitmapBitShifted's body.
; WAITS: no.
; Entry, exit: as BitmapBitShifted.
BitmapBitShifted12D5:
    phy
    tya
    lsr a
    lsr a
    lsr a
    and #$07
    tay
    lda (zTemp0C),y
    sta z:zTemp0E
    ply
    tya
    and #$07
    phx
    tax
    inx
BitmapBitShifted12D5_at0:
    lsr z:zTemp0E
    dex
    bne BitmapBitShifted12D5_at0
    plx
    rts

; @routine 12D6:41B ScrollYPlusOffset12D5
; @run 6F:AC1B
; WHAT IT DOES: ScrollYPlusOffset's copy in the program at 12D5, its camera module $389 on.
; HOW: ScrollYPlusOffset's body with the offsets moved ($AB09).
; WAITS: no.
; Entry, exit: as ScrollYPlusOffset.
LayerPlaceOffsetsLoAlt12D5 = $AB07
ScrollYPlusOffset12D5:
    lda a:$3EBA
    beq ScrollYPlusOffset12D5_at0
    lda a:ScrollYLo,x
    clc
    adc a:LayerPlaceOffsetsLoAlt12D5,y
    bra ScrollYPlusOffset12D5_at1
ScrollYPlusOffset12D5_at0:
    lda a:ScrollYLo,x
    clc
    adc a:LayerPlaceOffsetsLo12D5,y
ScrollYPlusOffset12D5_at1:
    sta z:zTemp06
    lda a:ScrollYHiScreen,x
    adc a:LayerPlaceOffsetsHi12D5,y
    and #$0F
    sta z:zTemp07
    rts

; @routine 12D6:43D RowDueUp12D5
; @run 6F:AC3D
; WHAT IT DOES: RowDueUp's copy in the program at 12D5, its camera module $389 on.
; HOW: RowDueUp's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as RowDueUp.
RowDueUp12D5:
    lda a:ScrollBgyLo,x
    dec a
    sta z:zTemp09
    lda a:ScrollYLo,x
    dec a
    eor z:zTemp09
    and #$20
    bne RowDueUp12D5_at0
    rts
RowDueUp12D5_at0:
    bra DrawBlockRow12D5

; @routine 12D6:450 RowDueAt12D5
; @run 6F:AC50
; WHAT IT DOES: RowDueAt's copy in the program at 12D5, its camera module $389 on.
; HOW: RowDueAt's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as RowDueAt.
RowDueAt12D5:
    tya
    beq RowDueUp12D5
    lda a:ScrollYLo,x
    eor a:ScrollBgyLo,x
    and #$20
    bne DrawBlockRow12D5
    rts

; @routine 12D6:45E DrawBlockRow12D5
; @run 6F:AC5E
; WHAT IT DOES: DrawBlockRow's copy in the program at 12D5, its camera module $389 on.
; HOW: DrawBlockRow's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as DrawBlockRow.
DrawBlockRow12D5:
    lda a:ScrollColumnBlocks
    and #$7F
    sta z:zTemp08
    jsr ScrollYPlusOffset12D5
    lda a:ScrollXLo,x
    sta z:zTemp02
    lda a:ScrollXHiScreen,x
    sta z:zTemp03
    lda a:ScrollColumnBlocks
    bpl DrawBlockRow12D5_at0
    ldy #$01
    lda a:ScrollXLo,x
    clc
    adc a:LayerPlaceOffsetsLo12D5,y
    sta z:zTemp02
    lda a:ScrollXHiScreen,x
    adc a:LayerPlaceOffsetsHi12D5,y
    and #$0F
    sta z:zTemp03
DrawBlockRow12D5_at0:
    jsr BlockIndexForPlace12D5
DrawBlockRow12D5_at3:
    phy
    lda z:zTemp10
    pha
    lda z:zTemp11
    pha
    lda (zTemp04),y
    jsr CopyTileBlockToVram12D5
    pla
    sta z:zTemp11
    pla
    clc
    adc #$04
    and #$3F
    sta z:zTemp10
    ply
    dec z:zTemp08
    bne DrawBlockRow12D5_at1
    rts
DrawBlockRow12D5_at1:
    iny
    tya
    and #$07
    beq DrawBlockRow12D5_at2
    jsr BitmapBitShifted12D5
    bra DrawBlockRow12D5_at3
DrawBlockRow12D5_at2:
    lda z:zTemp03
    inc a
    and #$0F
    sta z:zTemp03
    sta z:zTemp00
    lda z:zTemp07
    jsr ScreenMapAt12D5
    jsr SelectBlockTablesN
    lda z:zTemp06
    lsr a
    lsr a
    and #$38
    tay
    jsr BitmapBitShifted12D5
    jmp DrawBlockRow12D5_at3

; @routine 12D6:4D5 StageSetUpScroll12D5
; @run 6F:ACD5
; WHAT IT DOES: StageSetUpScroll's copy in the program at 12D5, its camera module $389 on.
; HOW: StageSetUpScroll's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as StageSetUpScroll.
StageSetUpScroll12D5:
    clx
    lda #$90
    sta a:ScrollColumnBlocks,x
    lda a:CameraMode
    cmp #$0D
    beq StageSetUpScroll12D5_at0
    cmp #$11
    bne StageSetUpScroll12D5_at1
StageSetUpScroll12D5_at0:
    lda #$05
    sta z:zSetUpColumnsLeft
    bra StageSetUpScroll12D5_at2
StageSetUpScroll12D5_at1:
    lda #$0C
    sta z:zSetUpColumnsLeft
    lda #$80
    bsr ScrollXAddWrap12D5
StageSetUpScroll12D5_at2:
    lda #$20
    sta a:ScrollStepXHi
StageSetUpScroll12D5_at3:
    cly
    jsr DrawBlockColumn12D5
    jsr AdvanceScrollXC
    dec z:zSetUpColumnsLeft
    bne StageSetUpScroll12D5_at3
    cly
    jsr DrawBlockColumn12D5
    rts

; @routine 12D6:509 ScrollXAddWrap12D5
; @run 6F:AD09
; WHAT IT DOES: ScrollXAddWrap's copy in the program at 12D5, its camera module $389 on.
; HOW: ScrollXAddWrap's body.
; WAITS: no.
; Entry, exit: as ScrollXAddWrap.
ScrollXAddWrap12D5:
    clc
    adc a:ScrollXLo,x
    sta a:ScrollXLo,x
    lda a:ScrollXHiScreen,x
    adc #$FE
    and #$0F
    sta a:ScrollXHiScreen,x
    sta a:ScrollXHi,x
    rts

; @routine 12D6:53A StageSetUpScrollByEffect12D5
; @run 6F:AD3A
; WHAT IT DOES: StageSetUpScrollByEffect's copy in the program at 12D5, its camera module $389 on.
; HOW: StageSetUpScrollByEffect's body with the module's addresses and returns moved.
; WAITS: no.
; Entry, exit: as StageSetUpScrollByEffect.
EffectColumnBlocks12D5 = $AD1D
StageSetUpScrollByEffect12D5:
    ldy z:zRasterEffect
    lda a:EffectColumnBlocks12D5,y
    clx
    sta a:ScrollColumnBlocks,x
    lda #$C0
    bsr ScrollXAddWrap12D5
    cpy #$0F
    beq StageSetUpScrollByEffect12D5_at0
    cpy #$18
    beq StageSetUpScrollByEffect12D5_at1
    cpy #$0C
    bne StageSetUpScrollByEffect12D5_at2
StageSetUpScrollByEffect12D5_at1:
    lda #$10
    sta z:zSetUpColumnsLeft
    bra StageSetUpScrollByEffect12D5_at3
StageSetUpScrollByEffect12D5_at0:
    lda #$10
    sta z:zSetUpColumnsLeft
    bra StageSetUpScrollByEffect12D5_at4
StageSetUpScrollByEffect12D5_at2:
    cpy #$06
    beq StageSetUpScrollByEffect12D5_at3
    cpy #$07
    beq StageSetUpScrollByEffect12D5_at3
    cpy #$10
    beq StageSetUpScrollByEffect12D5_at3
StageSetUpScrollByEffect12D5_at4:
    inc a:ScrollYHiScreen,x
StageSetUpScrollByEffect12D5_at3:
    lda #$0F
    sta z:zSetUpColumnsLeft
    jmp StageSetUpScroll12D5_at2
