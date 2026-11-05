; @routine 0043:0F9 KernelStart
; @run 69:40F9
; WHAT IT DOES: Where the loader lands: zeroes RAM, starts the System Card's pad and
; display services and the sound driver, sets the display and first palette, installs
; the interrupts, and then idles forever, the game running from the frame interrupt.
; HOW: Block fills of zero page and game RAM, then BIOS and kernel calls in order.
; Entry: from the loader, zRamBaseBank set, MPR2-3 the kernel. Exit: never.
KernelStart:
    tai KernelZeros,zTemp00+$2000,$00DB
    tai KernelZeros,GameRamLow,$1D30
    cla
    ldx #$20
    ldy #$1E
    jsr BiosE06F
    cla
    jsr BiosE06C
    stz a:BiosScreenLo
    lda #$10
    sta a:BiosScreenHi
    jsr BiosE0A2
    lda #$00
    sta z:zGameMode
    lda #$0D
    clc
    adc z:zRamBaseBank
    tam #$10
    inc a
    tam #$20
    inc a
    tam #$40
    jsr SoundDrvInit
    jsr ResetDisplayControl
    lda #$0D
    jsr SelectVdcRegister
    lda #$E0
    sta a:VDC_DATA_LO
    lda #$04
    jsr LoadSceneScript
    lda #$06
    jsr LoadSceneScript
    lda #$24
    jsr AdpcmLoad
    lda #$FF
    sta z:zMusicWanted
    lda #$01
    ldx #$77
    ldy #$41
    jsr BiosE05D
    lda #$02
    ldx #$97
    ldy #$42
    jsr BiosE05D
    lda #$06
    sta z:BiosIrqDisable
    lda #$03
    clc
    adc z:zRamBaseBank
    tam #$10
    stz z:zIdleFlag
    jmp KernelIdle

; @routine 0043:174 KernelIdle
; @run 69:4174
; WHAT IT DOES: The main program's whole life after start: nothing, forever.
; HOW: A jump to itself; interrupts do the work.
; Entry: interrupts on. Exit: never.
KernelIdle:
    jmp KernelIdle

; @routine 0043:177 FrameInterrupt
; @run 69:4177
; WHAT IT DOES: The VDC's interrupt. A raster interrupt goes to the raster handler.
; A vblank runs a frame: all of video first, then with interrupts on the game's
; objects, sound, random numbers and mode logic. If the last frame is still
; running, only video and sound are kept up, and the frame is dropped.
; HOW: Saves everything including three MPRs and the selected VDC register, reads
; the VDC's status to zVdcStatus, and branches on its bits.
; Entry: interrupt. Exit: RTI; every register, flag, MPR and the VDC selection as
; they were. Clobbers what the frame's work does.
FrameInterrupt:
    pha
    phx
    phy
    tma #$10
    pha
    tma #$20
    pha
    tma #$40
    pha
    lda z:zVdcReg
    pha
    lda a:VDC_ADDR
    sta z:zVdcStatus
    bbr2 zVdcStatus,FrameInterrupt_at0
    jmp RasterInterrupt
FrameInterrupt_at0:
    bbs5 zVdcStatus,FrameInterrupt_at1
    jmp FrameInterrupt_at2
FrameInterrupt_at1:
    cli
    inc z:zFrameCount
    lda z:zFrameBusy
    beq FrameInterrupt_at3
    jmp LateFrame
FrameInterrupt_at3:
    inc z:zFrameBusy
    ldy z:zVideoMode
    lda a:VceControlTable,y
    eor #$04
    sta a:VCE_CONTROL
    tya
    asl a
    tay
    jsr WriteScreenTiming
    lda z:zVdcCrLo
    and #$FB
    sta z:zVdcCrLo
    lda z:zRasterEffect
    sta a:RasterEffectNow
    beq FrameInterrupt_at4
    lda z:zRasterArg
    sta a:RasterLine
    stz a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda a:RasterLine
    clc
    adc #$40
    sta a:VDC_DATA_LO
    cla
    adc #$00
    sta a:VDC_DATA_HI
    jsr RasterStart
    lda z:zVdcCrLo
    ora #$04
    sta z:zVdcCrLo
FrameInterrupt_at4:
    lda z:zVdcCrLo
    ldx z:zDisplayDelay
    beq FrameInterrupt_at5
    dec z:zDisplayDelay
    beq FrameInterrupt_at5
    and #$3F
FrameInterrupt_at5:
    ora #$08
    tay
    jsr SelectVdcControl
    sty a:VdcCrShadow
    sty a:VDC_DATA_LO
    lda a:ScrollXFrac
    sta a:ScrollBandFirst
    lda a:ScrollXLo
    sta a:ScrollBgxLo
    lda a:ScrollXHi
    sta a:ScrollBgxHi
    lda a:ScrollYFrac
    sta a:ScrollBandFourth
    lda a:ScrollYLo
    sta a:ScrollBgyLo
    lda a:ScrollYHi
    sta a:ScrollBgyHi
    clx
    jsr WriteBandScroll
    lda #$04
    clc
    adc z:zRamBaseBank
    tam #$10
    lda #$07
    clc
    adc z:zRamBaseBank
    tam #$20
    lda #$0C
    clc
    adc z:zRamBaseBank
    tam #$40
    jsr FrameOverlay8003
    jsr FrameOverlay8000
    jsr SoundFrameTick
    jsr BiosE063
    jsr NextRandom
    lda #$03
    clc
    adc z:zRamBaseBank
    tam #$10
    jsr FrameModeDispatch
    lda #$00
    clc
    adc z:zRamBaseBank
    tam #$08
    lda #$0A
    clc
    adc z:zRamBaseBank
    tam #$10
    lda #$07
    clc
    adc z:zRamBaseBank
    tam #$20
    jsr FrameOverlay8000
    lda #$02
    clc
    adc z:zRamBaseBank
    tam #$08
    lda #$13
    jsr SelectVdcRegister
    st2 #$10
    stz z:zFrameBusy
    stz z:zSoundBusy
    lda a:ScreenWideFlag
    beq FrameInterrupt_at2
    ldy #$02
    jsr WriteScreenTiming
FrameInterrupt_at2:
    pla
    jsr SelectVdcRegister
    pla
    tam #$40
    pla
    tam #$20
    pla
    tam #$10
    ply
    plx
    pla

; @routine 0043:297 IgnoredInterrupt
; @run 69:4297
; WHAT IT DOES: The handler for the interrupt the game has no use for: returns.
; HOW: RTI.
; Entry: interrupt. Exit: everything as it was.
IgnoredInterrupt:
    rti

; @routine 0043:298 LateFrame
; @run 69:4298
; WHAT IT DOES: A frame that began while the last was still running: only video and
; sound are kept up, and the game's logic loses the frame.
; HOW: Rewrites the raster line, the display control and the first band's scroll
; from their RAM copies, ticks the sound driver, and joins FrameInterrupt's exit.
; Entry: from FrameInterrupt, its saves on the stack. Exit: through its RTI.
LateFrame:
    lda z:zVdcCrLo
    and #$FB
    sta z:zVdcCrLo
    lda a:RasterEffectNow
    beq LateFrame_at0
    stz a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda a:RasterLine
    clc
    adc #$40
    sta a:VDC_DATA_LO
    cla
    adc #$00
    sta a:VDC_DATA_HI
    lda z:zVdcCrLo
    ora #$04
    sta z:zVdcCrLo
LateFrame_at0:
    jsr SelectVdcControl
    lda a:VdcCrShadow
    ora #$08
    sta a:VDC_DATA_LO
    clx
    jsr WriteBandScroll
    jsr SoundFrameTick
    bra FrameInterrupt_at2
