; @routine 0043:2D4 ResetDisplayControl
; @run 69:42D4
; WHAT IT DOES: Sets up the display to come on: the game's control value becomes
; sprites and background on with the vblank interrupt enabled, scroll zero, no
; raster effect, and a four frame delay. Only the interrupt enable reaches the
; VDC now; the frame interrupt turns the display on when the delay runs out.
; HOW: Writes $C8 as the wanted value and its low nibble to the VDC.
; Entry: nothing. Exit: A 4, N 0, Z 0. X and Y kept.
ResetDisplayControl:
    jsr SelectVdcControl
    lda #$C8
    sta z:zVdcCrLo
    and #$0F
    sta a:VDC_DATA_LO
    cla
    sta z:zVdcCrHi
    sta a:VDC_DATA_HI
    stz a:ScrollXLo
    stz a:ScrollXHi
    stz a:ScrollYLo
    stz a:ScrollYHi
    stz z:zRasterEffect
    lda #$04
    sta z:zDisplayDelay
    rts

; @routine 0043:2F9 WriteBandScroll
; @run 69:42F9
; WHAT IT DOES: Writes one raster band's background scroll to the VDC.
; HOW: BXR then BYR, from the bands' parallel byte arrays.
; Entry: X band. Exit: A BYR's high byte, N and Z from it. X and Y kept.
WriteBandScroll:
    lda #$07
    jsr SelectVdcRegister
    lda a:ScrollBgxLo,x
    sta a:VDC_DATA_LO
    lda a:ScrollBgxHi,x
    sta a:VDC_DATA_HI
    lda #$08
    jsr SelectVdcRegister
    lda a:ScrollBgyLo,x
    sta a:VDC_DATA_LO
    lda a:ScrollBgyHi,x
    sta a:VDC_DATA_HI
    rts

; @routine 0043:31C WriteScreenTiming
; @run 69:431C
; WHAT IT DOES: Sets the VDC's horizontal timing for a screen width.
; HOW: HSR and HDR, each a word from a table by timing index times two.
; Entry: Y index. Exit: A HDR's high byte, N and Z from it. X and Y kept.
WriteScreenTiming:
    lda #$0A
    jsr SelectVdcRegister
    lda a:VdcHsrTable,y
    sta a:VDC_DATA_LO
    lda a:VdcHsrTable+1,y
    sta a:VDC_DATA_HI
    lda #$0B
    jsr SelectVdcRegister
    lda a:VdcHdrTable,y
    sta a:VDC_DATA_LO
    lda a:VdcHdrTable+1,y
    sta a:VDC_DATA_HI
    rts

; @routine 0043:3D8 WaitVblankDisplayOff
; @run 69:43D8
; WHAT IT DOES: Turns the display off at the next vertical blank, and with it
; any raster effect.
; HOW: Clears the status the interrupt keeps and waits until the interrupt has
; seen vblank; then clears the frame's raster effect and writes the control
; value the game wants, with its sprite and background enables off, without
; changing what the game wants.
; Entry: interrupts on. Exit: A the value written, N and Z from it. X and Y
; kept.
WaitVblankDisplayOff:
    stz z:zVdcStatus
WaitVblankDisplayOff_at0:
    bbr5 zVdcStatus,WaitVblankDisplayOff_at0
    stz a:RasterEffectNow
    jsr SelectVdcControl
    lda z:zVdcCrLo
    and #$3F
    sta a:VdcCrShadow
    sta a:VDC_DATA_LO
    rts

; @routine 0043:3EE WriteVdcControl
; @run 69:43EE
; WHAT IT DOES: Writes the VDC control value the game wants.
; HOW: Selects the register and writes both bytes.
; Entry: nothing. Exit: A the high byte, N and Z from it. X and Y kept.
WriteVdcControl:
    jsr SelectVdcControl
    lda z:zVdcCrLo
    sta a:VDC_DATA_LO
    lda z:zVdcCrHi
    sta a:VDC_DATA_HI
    rts

; @routine 0046:359 SelectVramData
; @run 69:5B59
; WHAT IT DOES: Selects the VDC's VRAM data port.
; HOW: Register 2 into SelectVdcRegister.
; Entry: nothing. Exit: A 2, N 0, Z 0. X and Y kept.
SelectVramData:
    lda #$02
    bra SelectVdcRegister

; @routine 0046:35D SelectVramReadAddress
; @run 69:5B5D
; WHAT IT DOES: Selects the VDC's VRAM read address.
; HOW: Register 1 into SelectVdcRegister.
; Entry: nothing. Exit: A 1, N 0, Z 0. X and Y kept.
SelectVramReadAddress:
    lda #$01
    bra SelectVdcRegister

; @routine 0046:361 SelectVramWriteAddress
; @run 69:5B61
; WHAT IT DOES: Selects the VDC's VRAM write address.
; HOW: Register 0 into SelectVdcRegister. CLA sets no flags.
; Entry: nothing. Exit: A 0. Flags, X and Y kept.
SelectVramWriteAddress:
    cla
    bra SelectVdcRegister

; @routine 0046:364 SelectVdcControl
; @run 69:5B64
; WHAT IT DOES: Selects the VDC's control register.
; HOW: Register 5 into SelectVdcRegister.
; Entry: nothing. Exit: A 5, N 0, Z 0. X and Y kept.
SelectVdcControl:
    lda #$05

; @routine 0046:366 SelectVdcRegister
; @run 69:5B66
; WHAT IT DOES: Selects a VDC register, remembering which so the frame
; interrupt can put it back.
; HOW: Stores it in zVdcReg, then writes it to the address port.
; Entry: A register. Exit: A, X, Y and flags kept.
SelectVdcRegister:
    sta z:zVdcReg
    sta a:VDC_ADDR
    rts

; @routine 0046:36C ClearVdcControlHigh
; @run 69:5B6C
; WHAT IT DOES: Zeroes the high byte of the VDC's control register.
; HOW: Selects the register and writes zero to its high data port.
; Entry: nothing. Exit: A 0, N 0, Z 1. X and Y kept.
ClearVdcControlHigh:
    cla
    pha
    bsr SelectVdcControl
    pla
    sta a:VDC_DATA_HI
    rts

; @routine 0046:375 ResetGameState
; @run 69:5B75
; WHAT IT DOES: Starts over from nothing: display off, the VCE's first colours
; black, the level pointer and all game RAM zero, then a blank screen.
; HOW: Waits for vblank with the display off, zeroes colours 0 and $100,
; block-fills $26D0 and $26F3 up with zero, then runs into ClearScreenState.
; Entry: interrupts on. Exit: as ClearScreenState.
ResetGameState:
    jsr WaitVblankDisplayOff
    stz a:VCE_ADDR_LO
    stz a:VCE_ADDR_HI
    stz a:VCE_DATA_LO
    stz a:VCE_DATA_HI
    stz a:VCE_ADDR_LO
    lda #$01
    sta a:VCE_ADDR_HI
    stz a:VCE_DATA_LO
    stz a:VCE_DATA_HI
    tai KernelZeros,LevelStreamLo,$0003
    tai KernelZeros,TileHit,$164D

; @routine 0046:3A0 ClearScreenState
; @run 69:5BA0
; WHAT IT DOES: Blanks the screen for a new scene: display off, no raster
; effect, the scroll and band tables zero, and the whole background map filled
; with the blank tile.
; HOW: Waits for vblank with the display off, block-fills $3E00 with zero, sets
; the VRAM write address to 0 and writes tile $0110 $1000 times.
; Entry: interrupts on. Exit: A 2, X 0, Y $FF, N 1, Z 0.
ClearScreenState:
    jsr WaitVblankDisplayOff
    lda #$04
    sta z:zDisplayDelay
    stz z:zRasterEffect
    stz z:zVideoMode
    tai KernelZeros,ScrollWanted,$00EE
    jsr ClearVdcControlHigh
    jsr SelectVramWriteAddress
    st1 #$00
    st2 #$00
    jsr SelectVramData
    clx
    ldy #$0F
ClearScreenState_at0:
    st1 #$10
    st2 #$01
    inx
    bne ClearScreenState_at0
    dey
    bpl ClearScreenState_at0
    rts
